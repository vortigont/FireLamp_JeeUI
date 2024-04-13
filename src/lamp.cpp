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
#include "display.hpp"
#include "effectmath.h"
#include "interface.h"
#include "actions.hpp"
#include "ledfb.hpp"
#include "evtloop.h"
#include "nvs_handle.hpp"


Lamp::Lamp() : effwrkr(&lampState){
  lampState.micAnalyseDivider = 1; // анализ каждый раз
  lampState.flags = 0; // сборосить все флаги состояния
  lampState.speedfactor = 1.0; // дефолтное значение
}

Lamp::~Lamp(){
  events_unsubsribe();
}

void Lamp::lamp_init(){
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

  // restore lamp flags from NVS
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READONLY, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->get_item(V_lampFlags, opts.pack);
  }

  if (opts.flag.restoreState && opts.flag.pwrState){
    opts.flag.pwrState = false;       // reset it first, so that power() method will know that we are in off state actually
    power(true);
  } else {
    opts.flag.pwrState = false;
  }

  // restore demo mode
  if (opts.flag.restoreState && opts.flag.demoMode){
    startDemoMode(embui.paramVariant(TCONST_DTimer) | DEFAULT_DEMO_TIMER);
  }

  // restore mike on/off state
  setMicOnOff(opts.flag.isMicOn);
}

void Lamp::handle(){
  static unsigned long mic_check = 0; // = 40000; // пропускаю первые 40 секунд
  if(effwrkr.status() && opts.flag.isMicOn && (opts.flag.pwrState) && mic_check + MIC_POLLRATE < millis()){
    if(effwrkr.isMicOn())
      micHandler();
    mic_check = millis();
  } else {
    // если микрофон не нужен, удаляем объект
    if (mw){ delete mw; mw = nullptr; }
  }

  // все что ниже, будет выполняться раз в 0.999 секундy
  static unsigned long wait_handlers;
  if (wait_handlers + 999U > millis())
      return;
  wait_handlers = millis();

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL>2
  EVERY_N_SECONDS(15){
    // fps counter
    LOG(printf_P, PSTR("Eff:%d, FastLED FPS: %u\n"), effwrkr.getCurrentEffectNumber(), FastLED.getFPS());
    LOG(printf_P, PSTR("MEM stat: %d, Time: %s\n"), ESP.getFreeHeap(), TimeProcessor::getInstance().getFormattedShortTime().c_str());
  }
#endif

}

void Lamp::power() {power(!opts.flag.pwrState);}

void Lamp::power(bool flag) // флаг включения/выключения меняем через один метод
{
  if (flag == opts.flag.pwrState) return;  // пропускаем холостые вызовы
  LOG(print, "Lamp powering "); LOG(println, flag ? "On": "Off");

  if (flag){
    // POWER ON

    // переключаемся на текущий эффект, переключение вызовет запуск движка калькулятора эффекта и фейдер (если необходимо)
    _switcheffect(effswitch_t::num, getFaderFlag(), effwrkr.getCurrentEffectNumber());
    // запускаем планировщик движка эффектов
    effectsTimer(SCHEDULER::T_ENABLE);

    // включаем демотаймер если был режим демо
    if(mode == LAMPMODE::MODE_DEMO)
      demoTimer(T_ENABLE);

    // generate pwr change state event 
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwron));
  } else  {
    // POWER OFF
    if(opts.flag.fadeEffects){
      // need to fade
      LEDFader::getInstance()->fadelight(0, FADE_TIME );
      // бегунок эффектов будет выключен в функции _fadeEventHandler() по окончании затухания
    } else {
      // no need to fade
      effectsTimer(SCHEDULER::T_DISABLE);
      EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwroff));
    }

    demoTimer(T_DISABLE);     // гасим Демо-таймер

    // событие о выключении будет сгенерированно в ответ на событие от фейдера когда его работа завершится
  }

  // update flag on last step to let other call understand in which state they were called
  opts.flag.pwrState = flag;
  save_flags();
}


/*
 * запускаем режим "ДЕМО"
 */
void Lamp::startDemoMode(uint8_t tmout)
{
  LOG(println,"Start Demo mode");
  power(true);  // "включаем" лампу

  if(mode == LAMPMODE::MODE_DEMO) return;   // уже и так в "демо" режиме, выходим
  
  storedEffect = ((static_cast<EFF_ENUM>(effwrkr.getCurrentEffectNumber()%256) == EFF_ENUM::EFF_WHITE_COLOR) ? storedEffect : effwrkr.getCurrentEffectNumber()); // сохраняем предыдущий эффект, если только это не белая лампа
  mode = LAMPMODE::MODE_DEMO;
  if (!opts.flag.demoMode){
    opts.flag.demoMode = true;
    save_flags();
  }
  demoTimer(T_ENABLE, tmout);
}

void Lamp::startNormalMode(bool forceOff)
{
  LOG(println,"Normal mode");
  if(forceOff)
    opts.flag.pwrState=false;
  mode = LAMPMODE::MODE_NORMAL;
  if (opts.flag.demoMode){
    opts.flag.demoMode = false;
    save_flags();
    demoTimer(T_DISABLE);
  }
}

//typedef enum {FIRSTSYMB=1,LASTSYMB=2} SYMBPOS;

void Lamp::micHandler()
{
  static uint8_t counter=0;
  if(effwrkr.getCurrentEffectNumber()==EFF_ENUM::EFF_NONE)
    return;
  if(!mw && lampState.micAnalyseDivider){ // обычный режим
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
  }
}

void Lamp::setMicOnOff(bool val) {
    opts.flag.isMicOn = val;
    lampState.isMicOn = val;
    if(effwrkr.getCurrentEffectNumber()==EFF_NONE || !effwrkr.status()) return;

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
    save_flags();
}

void Lamp::setBrightness(uint8_t tgtbrt, fade_t fade, bool bypass){
    LOG(printf, "Lamp::setBrightness(%u,%u,%u)\n", tgtbrt, static_cast<uint8_t>(fade), bypass);
    // if bypass flag is given, than this is a low level request with unscaled brightness that should not be saved or published anywhere
    if (bypass)
      return _brightness(tgtbrt, true);

    // when lamp in 'PowerOff' state, just change brightness w/o any notifications or saves
    if (!isLampOn())
      return _brightness(tgtbrt);

    if ( fade == fade_t::on || ( (fade == fade_t::preset) && opts.flag.fadeEffects) ) {
      // fader will publish event once it will finish brightness scaling
      LEDFader::getInstance()->fadelight(tgtbrt);
    } else {
      _brightness(tgtbrt);
      unsigned b = tgtbrt;
      EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(unsigned));
    }

    // set configured brightness variable
    globalBrightness = tgtbrt > _brightnessScale ? _brightnessScale : tgtbrt;
    embui.var(A_dev_brightness, globalBrightness);    // save brightness variable
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
void Lamp::_switcheffect(effswitch_t action, bool fade, uint16_t effnb) {
  lampState.setMicAnalyseDivider(1); // восстановить делитель, при любой активности (поскольку эффекты могут его перенастраивать под себя)

  // find real effect number we need to switch to
  switch (action) {
  case effswitch_t::next :
      _swState.pendingEffectNum = effwrkr.getNext();
      break;
  case effswitch_t::prev :
      _swState.pendingEffectNum = effwrkr.getPrev();
      break;
  case effswitch_t::num :
      _swState.pendingEffectNum = effnb;
      break;
  case effswitch_t::rnd :
      // do not include 0-empty effect
      _swState.pendingEffectNum = effwrkr.getByCnt(random(1, effwrkr.getEffectsListSize()));
      break;
  default:
      return;
  }

  LOGD(T_lamp, printf, "switcheffect() action=%d, fade=%d, effnb=%d\n", action, fade, _swState.pendingEffectNum);

  // проверяем нужно ли использовать затухание (только если лампа включена, и не идет разжигание)
  if (fade && opts.flag.pwrState && _swState.fadeState <1){
    // если уже идет угасание вниз, просто выходим, переключение произойдет на новый эффект после конца затухания
    if(_swState.fadeState == -1)
      return;
    // в противном случае запускаем затухание
    // если текущая абсолютная яркость больше чем 2*FADE_MINCHANGEBRT, то затухаем не полностью, а только до значения FADE_MINCHANGEBRT, в противном случае гаснем полностью
    LEDFader::getInstance()->fadelight( _get_brightness(true) < 3*MAX_BRIGHTNESS/FADE_LOWBRTFRACT/2 ? 0 : _brightnessScale/FADE_LOWBRTFRACT, FADE_TIME );
    _swState.fadeState = -1;
    return;
  }

  // затухание не требуется, переключаемся непосредственно на нужный эффект
  if(opts.flag.wipeOnEffChange || !effwrkr.getCurrentEffectNumber()){ // для EFF_NONE или для случая когда включена опция - чистим матрицу
    if (display.getCanvas())
      display.getCanvas()->clear();
  }

  effwrkr.switchEffect(_swState.pendingEffectNum);

  // need to reapply brightness as effect's curve might have changed and we might also need a fader
  // I use direct access to fader and _brightness, 'cause I do not want to re-publishing brightness value and re-save it to permanent storage
  if ( fade && opts.flag.fadeEffects) {
    LEDFader::getInstance()->fadelight(globalBrightness);
  } else {
    _brightness(globalBrightness);
  }

  // if lamp is not in Demo mode, then need to save new effect in config
  if(mode != LAMPMODE::MODE_DEMO){
    embui.var(V_effect_idx, _swState.pendingEffectNum);
  } else {
    myLamp.demoTimer(T_RESET);
  }

  // publish new effect's control to all available feeders
  publish_effect_controls(nullptr, nullptr, NULL);
  LOGD(T_lamp, println, "eof switcheffect()");
}

uint16_t Lamp::_getRealativeEffectNum(){
  return (_swState.fadeState != -1) ? effwrkr.getCurrentEffectNumber() : _swState.pendingEffectNum;
}

void Lamp::_fadeEventHandler(){
  // check if lamp is in "PowerOff" state, then we've just complete fade-out, need to send event
  if (!opts.flag.pwrState){
    _swState.fadeState = 0;
    effectsTimer(SCHEDULER::T_DISABLE);
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwroff));
    return;
  }

  // если статус был ожидать затухания, то переключаемся на разжигание и меняем эффект
  if (_swState.fadeState == -1){
    _swState.fadeState = 1;
    _switcheffect(effswitch_t::num, true, _swState.pendingEffectNum);
  } else if (_swState.fadeState == 1){
    // если ждали разгарания, то сбрасываем статус
    _swState.fadeState = 0;
  }
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
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err != ESP_OK) {
    // if NVS handle is unavailable then just quit
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    return;
  }

  handle->set_item(V_lampFlags, opts.pack);
}

void Lamp::_overlay_buffer(bool activate) {
  if (activate && !_overlay){
    LOGD(T_lamp, println, "Create Display overlay");
    _overlay = display.getOverlay();   // obtain overlay buffer
  }
  else{
    LOGD(T_lamp, println, "Release Display overlay");
    _overlay.reset();
  }
}

void Lamp::_events_subsribe(){
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, Lamp::event_hndlr, this, &_events_lamp_cmd));
}

void Lamp::event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
  LOGV(T_lamp, printf, "event_hndlr %s:%d\n", base, id);
  if (base == LAMP_SET_EVENTS || base == LAMP_GET_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_cmd(base, id, event_data);

  if (base == LAMP_CHANGE_EVENTS || base == LAMP_STATE_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_state(base, id, event_data);

}

void Lamp::events_unsubsribe(){
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, _events_lamp_cmd);
}

// handle command events and react accordingly with lamp actions
void Lamp::_event_picker_cmd(esp_event_base_t base, int32_t id, void* data){

    switch (static_cast<evt::lamp_t>(id)){
    // Power control
      case evt::lamp_t::pwron :
        power(true);
        return;
      case evt::lamp_t::pwroff :
        power(false);
        return;
      case evt::lamp_t::pwrtoggle :
        power();
        return;

    // Brightness control
      case evt::lamp_t::brightness : {
        if (base == LAMP_SET_EVENTS){
          setBrightness(*((int*) data));
        } else {
          // otherwise it's a GET event, publish current brightness
          int32_t b = getBrightness();
          EVT_POST_DATA( LAMP_STATE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(int32_t) );
        }
        return;
      }
      case evt::lamp_t::brightness_nofade :
        setBrightness(*((int*) data), fade_t::off);
        break;
      case evt::lamp_t::brightness_lcurve :
        setLumaCurve(*( (luma::curve*)data) );
        break;
      case evt::lamp_t::brightness_step :
        setBrightness(getBrightness() + *((int*) data), fade_t::off);
        break;
        
    // Effect switching
      case evt::lamp_t::effSwitchNext :
        switcheffect(effswitch_t::next);
        break;
      case evt::lamp_t::effSwitchPrev :
        switcheffect(effswitch_t::prev);
        break;
      case evt::lamp_t::effSwitchRnd :
        switcheffect(effswitch_t::rnd);
        break;
      case evt::lamp_t::effSwitchTo :
        switcheffect(effswitch_t::num, *((int*) data));
        break;
      case evt::lamp_t::effSwitchStep : {
        int32_t shift = _getRealativeEffectNum() + *((int*) data);
        if (shift < 0) shift += effwrkr.getEffectsListSize();
        shift %= effwrkr.getEffectsListSize();
        switcheffect(effswitch_t::num, shift);
        break;
      }

    // Get State Commands
      case evt::lamp_t::pwr :
        EVT_POST(LAMP_STATE_EVENTS, opts.flag.pwrState ? e2int(evt::lamp_t::pwron) : e2int(evt::lamp_t::pwroff));
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
      // check if effect switching is pending
      _fadeEventHandler();
      break;

    default:;
  }

}

// *********************************
/*  LEDFader class implementation */

void LEDFader::fadelight(int _targetbrightness, uint32_t _duration){
  if (!lmp) return;
  LOGD(T_Fade, printf, "tgt:%u, lamp:%u/%u, _br_scaled/_br_abs:%u/%u\n", _targetbrightness, lmp->getBrightness(), lmp->getBrightnessScale(), lmp->_get_brightness(), lmp->_get_brightness(true));

  _brt = lmp->_get_brightness(true);        // get current absolute Display brightness
  _tgtbrt = luma::curveMap(lmp->_curve, _targetbrightness, MAX_BRIGHTNESS, lmp->_brightnessScale);

  if (_brt == _tgtbrt){
    // no need to fade, already at this brightness
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd));
    return;
  }

  // calculate required steps
  int _steps = (abs(_tgtbrt - _brt) > FADE_MININCREMENT * _duration / FADE_STEPTIME) ? _duration / FADE_STEPTIME : abs(_tgtbrt - _brt)/FADE_MININCREMENT;
  if (_steps < 3) {   // no need to fade for such small difference
    LOGD(T_Fade, printf, "Fast: %d->%d\n", _brt, _tgtbrt);
    lmp->_brightness(_tgtbrt, true);
    abort();
    int b = _targetbrightness;
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd));
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), &b, sizeof(int));
    return;
  }

  _brtincrement = (_tgtbrt - _brt) / _steps;

  if (runner){
    // fading is already in progress, let's readjust it
    runner->setIterations(_steps);
    runner->restartDelayed();
  } else {
    runner = new Task(FADE_STEPTIME,
      _steps,
      [this](){ _brt += _brtincrement; lmp->_brightness(_brt, true);  // set absolute backend brightness here
                /* LOG(printf_P, PSTR("fd brt %d/%d, glbr:%d, gbr:%d, vid:%d, vid2:%d\n"), _brt, _brtincrement, lmp->getBrightness(), lmp->getBrightness(), brighten8_video(FastLED.getBrightness()), brighten8_video(brighten8_video(FastLED.getBrightness()))  ); */
              },
      &ts, true, nullptr,
      // onDisable
      [this, _targetbrightness](){
          lmp->_brightness(_tgtbrt, true);  // set exact target brightness value
          LOGD(T_Fade, printf, "fading to %d complete\n", _tgtbrt);
          int b = _targetbrightness;
          EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), &b, sizeof(b));

          // use new task for callback, 'cause effect switching will immidiatetly respawn new fader from callback, so I need to release a Task instance
          //if(_cb) { new Task(FADE_STEPTIME, TASK_ONCE, [this](){ if (_cb) { _cb(); _cb = nullptr; } }, &ts, true, nullptr, nullptr, true ); }
          runner = nullptr;
      },
      true  // self-destruct
    );
  }

  LOGD(T_Fade, printf, "Fading lamp/fled:%d/%d->%d/%u, steps/inc %d/%d\n", lmp->getBrightness(), lmp->_get_brightness(true), _targetbrightness, _tgtbrt, _steps, _brtincrement);
  // send fader event
  EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeStart));
}

void LEDFader::abort(){
  if (!runner) return;
  runner->abort();
  delete runner;
  runner = nullptr;
  LOGD(T_Fade, println,"aborted!");
}

// ============
// объект лампы
Lamp myLamp;
