/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include "lamp.h"
#include "effectmath.h"
#include "interface.h"
//#include "fontHEX.h"
#include "actions.hpp"
#include "ledfb.hpp"
#include "evtloop.h"


Lamp::Lamp() : effwrkr(&lampState){
  lampState.isInitCompleted = false; // завершилась ли инициализация лампы
  lampState.isStringPrinting = false; // печатается ли прямо сейчас строка?
  lampState.isEffectsDisabledUntilText = false;
  lampState.isOffAfterText = false;
  lampState.dawnFlag = false; // флаг устанавливается будильником "рассвет"
//#ifdef MIC_EFFECTS
  lampState.isCalibrationRequest = false; // находимся ли в режиме калибровки микрофона
  lampState.micAnalyseDivider = 1; // анализ каждый раз
//#endif
  lampState.flags = 0; // сборосить все флаги состояния
  lampState.speedfactor = 1.0; // дефолтное значение
}

Lamp::~Lamp(){
  events_unsubsribe();
}

void Lamp::lamp_init()
{
  // subscribe to CMD events
  _events_subsribe();

  _brightnessScale = embui.paramVariant(V_dev_brtscale)  | DEF_BRT_SCALE;
  globalBrightness = embui.paramVariant(A_dev_brightness) | DEF_BRT_SCALE/2;

  _brightness(0, true);          // начинаем с полностью потушеной матрицы 0-й яркости

  // initialize fader instance
  LEDFader::getInstance()->setLamp(this);

  // GPIO's
  DynamicJsonDocument doc(512);
  if (embuifs::deserializeFile(doc, TCONST_fcfg_gpio)){
    // restore fet gpio
    fet_gpio = doc[TCONST_mosfet_gpio] | static_cast<int>(GPIO_NUM_NC);
    fet_ll = doc[TCONST_mosfet_ll];

    aux_gpio = doc[TCONST_aux_gpio] | static_cast<int>(GPIO_NUM_NC);
    aux_ll = doc[TCONST_aux_ll];
    // gpio that controls FET (for disabling matrix)
    if (fet_gpio > static_cast<int>(GPIO_NUM_NC)){
      pinMode(fet_gpio, OUTPUT);
      digitalWrite(fet_gpio, !fet_ll);
    }
    // gpio that controls AUX/Alarm pin
    if (aux_gpio > static_cast<int>(GPIO_NUM_NC)){
      pinMode(aux_gpio, OUTPUT);
      digitalWrite(aux_gpio, !aux_ll);
    }
  }

  // switch to last running effect
  run_action(ra::eff_switch, embui.paramVariant(V_effect_idx));

  // restore lamp flags from cfg
  flags.lampflags = embui.paramVariant(V_lampFlags);
  if (flags.restoreState && flags.ONflag){
    flags.ONflag = false;       // reset it first, so that power() method will know that we are in off state actually
    power(true);
  } else {
    flags.ONflag = false;
  }
}

void Lamp::handle(){
#ifdef MIC_EFFECTS
  static unsigned long mic_check = 0; // = 40000; // пропускаю первые 40 секунд
  if(effwrkr.status() && flags.isMicOn && (flags.ONflag || isMicCalibration()) && !isAlarm() && mic_check + MIC_POLLRATE < millis()){
    if(effwrkr.isMicOn() || isMicCalibration())
      micHandler();
    mic_check = millis();
  } else {
    // если микрофон не нужен, удаляем объект
    if (mw){ delete mw; mw = nullptr; }
  }
#endif

  // все что ниже, будет выполняться раз в 0.999 секундy
  static unsigned long wait_handlers;
  if (wait_handlers + 999U > millis())
      return;
  wait_handlers = millis();

  EVERY_N_SECONDS(15){
    lampState.freeHeap = ESP.getFreeHeap();
#ifdef ESP8266
    lampState.HeapFragmentation = ESP.getHeapFragmentation();
#else
    lampState.HeapFragmentation = 0;
#endif
    lampState.rssi = WiFi.RSSI();

#ifdef LAMP_DEBUG
    // fps counter
    LOG(printf_P, PSTR("Eff:%d, FastLED FPS: %u\n"), effwrkr.getCurrent(), FastLED.getFPS());
#ifdef ESP8266

    LOG(printf_P, PSTR("MEM stat: %d, HF: %d, Time: %s\n"), lampState.freeHeap, lampState.HeapFragmentation, TimeProcessor::getInstance().getFormattedShortTime().c_str());

#else
    LOG(printf_P, PSTR("MEM stat: %d, Time: %s\n"), lampState.freeHeap, TimeProcessor::getInstance().getFormattedShortTime().c_str());
#endif
#endif
  }

}

void Lamp::power() {power(!flags.ONflag);}

void Lamp::power(bool flag) // флаг включения/выключения меняем через один метод
{
  if (flag == flags.ONflag) return;  // пропускаем холостые вызовы
  LOG(print, "Lamp powering "); LOG(println, flag ? "On": "Off");

  if (flag){
    // POWER ON

    // переключаемся на текущий эффект, переключение вызовет запуск движка калькулятора эффекта и фейдер (если необходимо)
    _switcheffect(effswitch_t::num, getFaderFlag(), effwrkr.getCurrent());
    // запускаем планировщик движка эффектов
    effectsTimer(SCHEDULER::T_ENABLE);

    // включаем демотаймер если был режим демо
    if(mode == LAMPMODE::MODE_DEMO)
      demoTimer(T_ENABLE);

    // generate pwr change state event 
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwron));
  } else  {
    // POWER OFF
    if(flags.isFaderON){
      // need to fade
      LEDFader::getInstance()->fadelight(0, FADE_TIME, [this](){ effectsTimer(SCHEDULER::T_DISABLE); } );     // гасим эффект-процессор
    } else {
      // no need to fade
      effectsTimer(SCHEDULER::T_DISABLE);
      EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwroff));
    }

    demoTimer(T_DISABLE);     // гасим Демо-таймер

    // событие о выключении будет сгенерированно в ответ на событие от фейдера когда его работа завершится
  }

  // update flag on last step to let other call understand in which state they were called
  flags.ONflag = flag;
  save_flags();
}

#ifdef MP3PLAYER
/*
temporary disable
void Lamp::playEffect(bool isPlayName, EFFSWITCH action){
  if(mp3!=nullptr && mp3->isOn() && effwrkr.getCurrent()>0 && (flags.playEffect || ((isLampOn() || millis()>5000) && flags.playMP3 && action!=EFFSWITCH::SW_NEXT_DEMO && action!=EFFSWITCH::SW_RND))){
    LOG(printf_P, PSTR("playEffect soundfile:%s, effect:%d, delayed:%d\n"), effwrkr.getSoundfile().c_str(), effwrkr.getCurrent(), (flags.playName && !flags.playMP3));
    if(!flags.playMP3 || (flags.playEffect && action!=EFFSWITCH::SW_NEXT_DEMO && action!=EFFSWITCH::SW_RND)) // для mp3-плеера есть отдельное управление
      mp3->playEffect(effwrkr.getCurrent(), effwrkr.getSoundfile(), (isPlayName && mp3!=nullptr && mp3->isOn() && !flags.playMP3)); // влияние на отложенное воспроизведение, но не для MP3-плеера
  } else {
    mp3->setCurEffect(effwrkr.getCurrent());
  }
}
*/
#endif  // MP3PLAYER

void Lamp::startRGB(CRGB &val){
  rgbColor = val;
  storedMode = ((mode == LAMPMODE::MODE_RGBLAMP) ? storedMode: mode);
  mode = LAMPMODE::MODE_RGBLAMP;
  demoTimer(T_DISABLE);     // гасим Демо-таймер
  effectsTimer(T_ENABLE);
}

void Lamp::stopRGB(){
  if (mode != LAMPMODE::MODE_RGBLAMP) return;

  mode = (storedMode != LAMPMODE::MODE_RGBLAMP ? storedMode : LAMPMODE::MODE_NORMAL); // возвращаем предыдущий режим
  if(mode==LAMPMODE::MODE_DEMO)
    demoTimer(T_ENABLE);     // вернуть демо-таймер
  if (flags.ONflag)
    effectsTimer(T_ENABLE);
}


/*
 * запускаем режим "ДЕМО"
 */
void Lamp::startDemoMode(uint8_t tmout)
{
  LOG(println,"Start Demo mode");
  power(true);  // "включаем" лампу

  if(mode == LAMPMODE::MODE_DEMO) return;   // уже и так в "демо" режиме, выходим
  
  storedEffect = ((static_cast<EFF_ENUM>(effwrkr.getCurrent()%256) == EFF_ENUM::EFF_WHITE_COLOR) ? storedEffect : effwrkr.getCurrent()); // сохраняем предыдущий эффект, если только это не белая лампа
  mode = LAMPMODE::MODE_DEMO;
  demoTimer(T_ENABLE, tmout);
}

void Lamp::storeEffect()
{
  storedEffect = ((static_cast<EFF_ENUM>(effwrkr.getCurrent()%256) == EFF_ENUM::EFF_WHITE_COLOR) ? storedEffect : effwrkr.getCurrent()); // сохраняем предыдущий эффект, если только это не белая лампа
  storedBright = getBrightness();
  lampState.isMicOn = false;
  LOG(printf_P, PSTR("storeEffect() %d,%d\n"),storedEffect,storedBright);
}

void Lamp::restoreStored()
{
  LOG(printf_P, PSTR("restoreStored() %d,%d\n"),storedEffect,storedBright);
  if(storedBright)
    setBrightness(storedBright);
  lampState.isMicOn = flags.isMicOn;
  if (static_cast<EFF_ENUM>(storedEffect) != EFF_NONE) {    // ничего не должно происходить, включаемся на текущем :), текущий всегда определен...
    Task *_t = new Task(3 * TASK_SECOND, TASK_ONCE, [this](){ run_action( ra::eff_switch, storedEffect); }, &ts, false, nullptr, nullptr, true);
    _t->enableDelayed();
  } else if(static_cast<EFF_ENUM>(effwrkr.getCurrent()%256) == EFF_NONE) { // если по каким-то причинам текущий пустой, то выбираем рандомный
    Task *_t = new Task(3 * TASK_SECOND, TASK_ONCE, [this](){ run_action(ra::eff_rnd); }, &ts, false, nullptr, nullptr, true);
    _t->enableDelayed();
  }
}

void Lamp::startNormalMode(bool forceOff)
{
  LOG(println,"Normal mode");
  if(forceOff)
    flags.ONflag=false;
  mode = LAMPMODE::MODE_NORMAL;
  demoTimer(T_DISABLE);
  restoreStored();
}

typedef enum {FIRSTSYMB=1,LASTSYMB=2} SYMBPOS;

#ifdef MIC_EFFECTS
void Lamp::micHandler()
{
  static uint8_t counter=0;
  if(effwrkr.getCurrent()==EFF_ENUM::EFF_NONE)
    return;
  if(!mw && !lampState.isCalibrationRequest && lampState.micAnalyseDivider){ // обычный режим
    //mw = new(std::nothrow) MicWorker(lampState.mic_scale,lampState.mic_noise,!counter);
    mw = new(std::nothrow) MicWorker(lampState.mic_scale,lampState.mic_noise,true);   // создаем полноценный объект и держим в памяти

    if(!mw) {
      return; // не удалось выделить память, на выход
    }
    
    lampState.samp_freq = mw->process(lampState.noise_reduce); // возвращаемое значение - частота семплирования
    lampState.last_min_peak = mw->getMinPeak();
    lampState.last_max_peak = mw->getMaxPeak();
    lampState.cur_val = mw->getCurVal();

    if(!counter) // раз на N измерений берем частоту, т.к. это требует обсчетов
      lampState.last_freq = mw->analyse(); // возвращаемое значение - частота главной гармоники
    if(lampState.micAnalyseDivider)
      counter = (counter+1)%(0x01<<(lampState.micAnalyseDivider-1)); // как часто выполнять анализ
    else
      counter = 1; // при micAnalyseDivider == 0 - отключено

    // EVERY_N_SECONDS(1){
    //   LOG(println, counter);
    // }

    //LOG(println, last_freq);
    //mw->debug();

    //delete mw;    // не удаляем, пока пользуемся
    //mw = nullptr;
  } else if(lampState.isCalibrationRequest) {
    if(!mw){ // калибровка начало
      mw = new(std::nothrow) MicWorker();
      if(!mw) return;   // was not able to alloc mem
    }
    mw->calibrate();
    if(!mw->isCaliblation()){ // калибровка конец
      lampState.mic_noise = mw->getNoise();
      lampState.mic_scale = mw->getScale();
      lampState.isCalibrationRequest = false; // завершили
      delete mw;
      mw = nullptr;

      DynamicJsonDocument doc(512);
      JsonObject obj = doc.to<JsonObject>();

      //remote_action(RA::RA_MIC, NULL);
      //CALL_INTF_OBJ(show_settings_mic);
      if (embui.ws.count()){
        Interface interf(&embui.feeders, SMALL_JSON_SIZE);
        interf.json_frame_value();
        interf.value(obj);
        interf.json_frame_flush();
      }
    }
  }
}

void Lamp::setMicOnOff(bool val) {
    flags.isMicOn = val;
    lampState.isMicOn = val;
    if(effwrkr.getCurrent()==EFF_NONE || !effwrkr.status()) return;

    unsigned foundc7 = 0;
    LList<std::shared_ptr<UIControl>>&controls = effwrkr.getControls();
    if(val){
        for(unsigned i=3; i<controls.size(); i++) {
            if(controls[i]->getId()==7 && controls[i]->getName().startsWith(TINTF_020)==1){
                effwrkr.setDynCtrl(controls[i].get());
                return;
            } else if(controls[i]->getId()==7) {
                foundc7 = i;
            }
        }
    }

    UIControl ctrl(7,(CONTROL_TYPE)18,String(TINTF_020), val ? "1" : "0", "0", "1", "1");
    effwrkr.setDynCtrl(&ctrl);
    if(foundc7){ // был найден 7 контрол, но не микрофон
        effwrkr.setDynCtrl(controls[foundc7].get());
    }
}
#endif  // MIC_EFFECTS

void Lamp::setBrightness(uint8_t tgtbrt, fade_t fade, bool bypass){
    LOG(printf, "Lamp::setBrightness(%u,%u,%u)\n", tgtbrt, static_cast<uint8_t>(fade), bypass);
    // if bypass flag is given, than this is a low level request with unscaled brightness that should not be saved or published anywhere
    if (bypass)
      return _brightness(tgtbrt, true);

    // when lamp in 'PowerOff' state, just change brightness w/o any notifications or saves
    if (!isLampOn())
      return _brightness(tgtbrt);

    if ( fade == fade_t::on || ( (fade == fade_t::preset) && flags.isFaderON) ) {
      // fader will publish event once it will finish brightness scaling
      LEDFader::getInstance()->fadelight(tgtbrt);
    } else {
      _brightness(tgtbrt);
      unsigned b = tgtbrt;
      EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(unsigned));
    }

    globalBrightness = tgtbrt;              // set configured brightness variable
    embui.var(A_dev_brightness, tgtbrt);    // save brightness variable
}

/*
 * Set display brightness
 * note: this method is called by fader also
 * @param bool natural
 */
void Lamp::_brightness(uint8_t brt, bool absolute){
    if (!absolute) brt = luma::curveMap(_curve, brt, MAX_BRIGHTNESS, _brightnessScale);
    if ( brt == display.brightness()) return;  // nothing to change here

    display.brightness(brt);
}

uint8_t Lamp::_get_brightness(bool absolute){
  return absolute ? display.brightness() : luma::curveUnMap(_curve, display.brightness(), MAX_BRIGHTNESS, _brightnessScale);
}

void Lamp::setLumaCurve(luma::curve c){
  if (c == _curve) return;
  _curve = c;
  setBrightness(getBrightness(), fade_t::off);    // switch to the adjusted brightness level
};

void Lamp::switcheffect(effswitch_t action, uint16_t effnb){
  if (isLampOn())
    _switcheffect(action, getFaderFlag(), effnb);
  else
    _switcheffect(action, false, effnb);
}

/*
 * переключатель эффектов для других методов,
 * может использовать фейдер, выбирать случайный эффект для демо
 * @param effswitch_t action - вид переключения (пред, след, случ.)
 * @param fade - переключаться через фейдер или сразу
 */
void Lamp::_switcheffect(effswitch_t action, bool fade, uint16_t effnb, bool skip) {
#ifdef MIC_EFFECTS
    lampState.setMicAnalyseDivider(1); // восстановить делитель, при любой активности (поскольку эффекты могут его перенастраивать под себя)
#endif

#ifdef ENCODER
 exitSettings();
#endif

  if (!skip) {
    uint16_t next_eff_num = effnb;
    switch (action) {
    case effswitch_t::next :
        next_eff_num = effwrkr.getNext();
        break;
    case effswitch_t::prev :
        next_eff_num = effwrkr.getPrev();
        break;
    case effswitch_t::num :
        next_eff_num = effnb;
        break;
    case effswitch_t::rnd :
        next_eff_num = effwrkr.getByCnt(random(0, effwrkr.getEffectsListSize()));
        break;
    default:
        return;
    }

    LOG(printf, "Lamp::switcheffect() action=%d, fade=%d, effnb=%d\n", action, fade, next_eff_num);
    // тухнем "вниз" только на включенной лампе
    if (fade && flags.ONflag) {
      // запускаем фейдер и уходим на второй круг переключения
      // если текущая абсолютная яркость больше чем 2*FADE_MINCHANGEBRT, то затухаем не полностью, а только до значения FADE_MINCHANGEBRTб в противном случае гаснем полностью
      LEDFader::getInstance()->fadelight( _get_brightness(true) < 3*MAX_BRIGHTNESS/FADE_LOWBRTFRACT/2 ? 0 : _brightnessScale/FADE_LOWBRTFRACT,
                                          FADE_TIME,
                                          [this, action, fade, next_eff_num](){ _switcheffect(action, fade, next_eff_num, true); }
                                        );
      return;
    } else {
      // do direct switch to effect if fading is not required
      effwrkr.switchEffect(next_eff_num);
    }

  } else {
    LOG(printf, "Lamp::switcheffect() postfade act=%d, fade=%d, effnb=%d\n", action, fade, effnb);
  }

  if(flags.isEffClearing || !effwrkr.getCurrent()){ // для EFF_NONE или для случая когда включена опция - чистим матрицу
    if (display.getCanvas())
      display.getCanvas()->clear();
  }

  // move to 'selected' only if lamp is On and fader is in effect (i.e. it's a second call after fade),
  // otherwise it's been switched already
  if (fade && flags.ONflag)
    effwrkr.switchEffect(effnb, true);

  bool isShowName = (mode==LAMPMODE::MODE_DEMO && flags.showName);
#ifdef MP3PLAYER
  bool isPlayName = (isShowName && flags.playName && !flags.playMP3 && effwrkr.getCurrent()>0);
#endif

  // show effects's name on screen and play name over speaker (if set)
  if(isShowName){
#ifdef MP3PLAYER
    if(isPlayName && mp3!=nullptr && mp3->isOn()) // воспроизведение 
      mp3->playName(effwrkr.getCurrent());
#endif
  }

#ifdef MP3PLAYER
  playEffect(isPlayName, action); // воспроизведение звука, с проверкой текущего состояния
#endif

  setBrightness(globalBrightness);      // need to reapply brightness as effect's curve might have changed

  // if lamp is not in Demo mode, then need to save new effect in config
  if(mode != LAMPMODE::MODE_DEMO){
    embui.var(V_effect_idx, effnb);
    //myLamp.effwrkr.autoSaveConfig();
  } else {
    myLamp.demoTimer(T_RESET);
  }

  // publish new effect's control to all awailable feeders
  publish_effect_controls(nullptr, nullptr, NULL);
  LOG(println, "eof switcheffect()");
}

/*
 * включает/выключает режим "демо"
 * @param SCHEDULER enable/disable/reset - вкл/выкл/сброс
 */
void Lamp::demoTimer(SCHEDULER action, uint8_t tmout){
  switch (action)
  {
  case SCHEDULER::T_DISABLE :
    delete demoTask;
    demoTask = nullptr;
    break;
  case SCHEDULER::T_ENABLE :
    if (!tmout && demoTask){
      delete demoTask;
      demoTask = nullptr;
      return;
    }
    if(demoTask){
      demoTask->setInterval(tmout * TASK_SECOND);
      return;
    }
    demoTask = new Task(tmout * TASK_SECOND, TASK_FOREVER, [](){run_action(ra::demo_next);}, &ts, false);    
    demoTask->enableDelayed();
    break;
  case SCHEDULER::T_RESET :
    if (mode==LAMPMODE::MODE_DEMO && demoTask)
      demoTask->restartDelayed();
    break;
  default:
    return;
  }
}

/*
 * включает/выключает таймер обработки эффектов
 * @param SCHEDULER enable/disable/reset - вкл/выкл/сброс
 */
void Lamp::effectsTimer(SCHEDULER action) {
  LOG(printf, "effectsTimer: %u\n", (unsigned)action);
  switch (action){
  case SCHEDULER::T_ENABLE :
  case SCHEDULER::T_RESET :
    effwrkr.start();
    break;
  default:
    effwrkr.stop();
  }
}

//-----------------------------

void Lamp::fillDrawBuf(CRGB color) {
  if(_overlay) _overlay->fill(color);
}

void Lamp::writeDrawBuf(CRGB color, uint16_t x, uint16_t y){
  if (_overlay) { _overlay->at(x,y) = color; }
}

void Lamp::save_flags(){
  //if (flags.restoreState)
  embui.var(V_lampFlags, flags.lampflags);
}

void Lamp::_overlay_buffer(bool activate) {
  if (activate && !_overlay){
    LOG(println, "Create Display overlay");
    _overlay = display.getOverlay();   // obtain overlay buffer
  }
  else{
    LOG(println, "Release Display overlay");
    _overlay.reset();
  }
}

void Lamp::_events_subsribe(){
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, Lamp::event_hndlr, this, &_events_lamp_cmd));
}

void Lamp::event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
  LOG(printf, "Lamp::event_hndlr %s:%d\n", base, id);
  if (base == LAMP_SET_EVENTS || base == LAMP_GET_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_cmd(base, id, event_data);

  if (base == LAMP_CHANGE_EVENTS || base == LAMP_STATE_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_state(base, id, event_data);

}

void Lamp::events_unsubsribe(){
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, _events_lamp_cmd);
}

void Lamp::_event_picker_cmd(esp_event_base_t base, int32_t id, void* data){


    switch (static_cast<evt::lamp_t>(id)){
    // Power control
      case evt::lamp_t::pwron :
        power(true);
        break;
      case evt::lamp_t::pwroff :
        power(false);
        break;
      case evt::lamp_t::pwrtoggle :
        power();
        break;

    // Brightness control
      case evt::lamp_t::brightness :
        setBrightness(*((int*) data));
        break;
      case evt::lamp_t::brightness_nofade :
        setBrightness(*((int*) data), fade_t::off);
        break;
      case evt::lamp_t::brightness_lcurve :
        setLumaCurve(*( (luma::curve*)data) );
        break;
        

    // Get State Commands
      case evt::lamp_t::pwr :
        EVT_POST(LAMP_STATE_EVENTS, flags.ONflag ? e2int(evt::lamp_t::pwron) : e2int(evt::lamp_t::pwroff));
        break;

      default:;
    }

}

void Lamp::_event_picker_state(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    case evt::lamp_t::pwron :
      // enable MOSFET
      if (fet_gpio > static_cast<int>(GPIO_NUM_NC)) digitalWrite(fet_gpio,  fet_ll);
      break;
    case evt::lamp_t::pwroff :
      // disable MOSFET
      if (fet_gpio > static_cast<int>(GPIO_NUM_NC)) digitalWrite(fet_gpio,  !fet_ll);
      break;
    case evt::lamp_t::fadeEnd :
      // check if lamp is in "PowerOff" state, then we've just complete fade-out, need to send event
      if (!flags.ONflag)
        EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwroff));
      break;

    default:;
  }

}

// *********************************
/*  LEDFader class implementation */

void LEDFader::fadelight(const uint8_t _targetbrightness, const uint32_t _duration, std::function<void()> callback){
  if (!lmp) return;
  LOG(printf, "Fader: tgt:%u, lamp:%u/%u, _br_scaled/_br_abs:%u/%u\n", _targetbrightness, lmp->getBrightness(), lmp->getBrightnessScale(), lmp->_get_brightness(), lmp->_get_brightness(true));

  int b = _targetbrightness;
  EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(int));

  _brt = lmp->_get_brightness(true);        // get current absolute Display brightness
  _tgtbrt = luma::curveMap(lmp->_curve, _targetbrightness, MAX_BRIGHTNESS, lmp->_brightnessScale);

  if (_brt == _tgtbrt){
    // no need to fade, already at this brightness
    if (callback) callback();
    return;
  }

  _cb = callback;
  // calculate required steps
  int _steps = (abs(_tgtbrt - _brt) > FADE_MININCREMENT * _duration / FADE_STEPTIME) ? _duration / FADE_STEPTIME : abs(_tgtbrt - _brt)/FADE_MININCREMENT;
  if (_steps < 3) {   // no need to fade for such small difference
    LOG(printf, "Fast fade %d->%d\n", _brt, _tgtbrt);
    lmp->_brightness(_tgtbrt, true);
    abort();
    int b = _targetbrightness;
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(int));
    if (callback) callback();
    return;
  }

  _brtincrement = (_tgtbrt - _brt) / _steps;

  if (runner){
    // fading is already in progress, let's readjust it
    runner->setIterations(_steps);
    runner->restartDelayed();
  } else {
    runner = new Task((unsigned long)FADE_STEPTIME,
      _steps,
      [this](){ _brt += _brtincrement; lmp->_brightness(_brt, true);  // set absolute backend brightness here
                /* LOG(printf_P, PSTR("fd brt %d/%d, glbr:%d, gbr:%d, vid:%d, vid2:%d\n"), _brt, _brtincrement, lmp->getBrightness(), lmp->getBrightness(), brighten8_video(FastLED.getBrightness()), brighten8_video(brighten8_video(FastLED.getBrightness()))  ); */
              },
      &ts, true, nullptr,
      // onDisable
      [this, _targetbrightness](){
          lmp->_brightness(_tgtbrt, true);  // set exact target brightness value
          LOG(printf, "Fading to %d done\n", _tgtbrt);
          EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd));
          // use new task for callback, 'cause effect switching will immidiatetly respawn new fader from callback, so I need to release a Task instance
          if(_cb) { new Task(FADE_STEPTIME, TASK_ONCE, [this](){ if (_cb) { _cb(); _cb = nullptr; } }, &ts, true, nullptr, nullptr, true ); }
          runner = nullptr;
      },
      true  // self-destruct
    );
  }

  LOG(printf, "Fading lamp/fled:%d/%d->%d/%u, steps/inc %d/%d\n", lmp->getBrightness(), lmp->_get_brightness(true), _targetbrightness, _tgtbrt, _steps, _brtincrement);
  // send fader event
  EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeStart));
}

void LEDFader::abort(){
  if (!runner) return;
  runner->abort();
  delete runner;
  runner = nullptr;
  LOG(println,"Fader aborted");
}

// ============
// объект лампы
Lamp myLamp;
