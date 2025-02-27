/*
Copyright © 2023-2024 Emil Muratov (Vortigont)
Copyright © 2020 Dmytro Korniienko (kDn)

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
#include "log.h"

#define DEFAULT_EFFECT_NUM    0                 // empty

#define FADE_MINSTEPTIME      100U              // minimum time between fade steps, ms
#define FADE_MININCREMENT     2U                // Minimal absolute increment for fading steps
#define FADE_LOWBRTFRACT      5U                // доля от максимальной шкалы яркости, до которой работает затухание при смене эффектов. Если текущая яркость ниже двойной доли, то затухание пропускается


Lamp::Lamp() {
  // initialize fader instance
  LEDFader::getInstance()->setLamp(this);

  // demo on/off
  embui.action.add(T_demoOn, [this](Interface *interf, JsonObjectConst data, const char* action){ _embui_demoOn(interf, data, action); } );
  embui.action.add(T_demoRndCtrls, [this](Interface *interf, JsonObjectConst data, const char* action){ _embui_demoRndCtrls(interf, data, action); } );
  embui.action.add(T_demoRndOrder, [this](Interface *interf, JsonObjectConst data, const char* action){ _embui_demoRndOrder(interf, data, action); } );
}

Lamp::~Lamp(){
  events_unsubsribe();
  embui.action.remove(T_demoOn);
  embui.action.remove(T_demoRndCtrls);
  embui.action.remove(T_demoRndOrder);
}

void Lamp::lamp_init(){
  // subscribe to CMD events
  _events_subsribe();

  // open NVS storage
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READONLY, &err);

  if (err == ESP_OK) {
    // restore lamp flags from NVS
    handle->get_item(V_lampFlags, opts.pack);
    handle->get_item(T_bright, globalBrightness);
    handle->get_item(V_dev_brtscale, _brightnessScale);
    // restore demo time
    handle->get_item(T_DemoTime, demoTime);
  } else {
    LOGE(T_lamp, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
  }

  _brightness(0, true);          // начинаем с полностью потушенного дисплея 0-й яркости

  // GPIO's
  JsonDocument doc;
  if (!embuifs::deserializeFile(doc, TCONST_fcfg_gpio)){

    // restore fet gpio
    _pins.fet = doc[TCONST_mosfet_gpio] | static_cast<int>(GPIO_NUM_NC);
    _pins.fet_ll = doc[TCONST_mosfet_ll];
    // gpio that controls FET (for disabling matrix)
    if (_pins.fet > static_cast<int>(GPIO_NUM_NC)){
      pinMode(_pins.fet, OUTPUT);
      digitalWrite(_pins.fet, !_pins.fet_ll);
    }

    _pins.aux = doc[TCONST_aux_gpio] | static_cast<int>(GPIO_NUM_NC);
    _pins.aux_ll = doc[TCONST_aux_ll];
    // gpio that controls AUX/Alarm pin
    if (_pins.aux > static_cast<int>(GPIO_NUM_NC)){
      pinMode(_pins.aux, OUTPUT);
      digitalWrite(_pins.aux, !_pins.aux_ll);
    }
  }

  // load effect's index
  effwrkr.loadIndex();

  // switch to last running effect
  if (err == ESP_OK) {
    uint32_t eff_idx{DEFAULT_EFFECT_NUM};
    handle->get_item(V_effect_idx, eff_idx);
    LOGD(T_lamp, printf, "restore last eff:%u\n", eff_idx);
    // switch to last running effect
    if (eff_idx)
      _switcheffect(effswitch_t::num, false, static_cast<effect_t>(eff_idx));
  }

  // set fade flag to fade-in
  if (opts.flag.fadeEffects)
    _swState.fadeState = 1;   // fade-in

  // if other options need to be restored, then just quit
  if (!opts.flag.restoreState){
    vopts.flag.initialized = true;
    return;
  }

  // restore demo mode
  vopts.flag.demoMode = opts.flag.demoMode;
  demoTask = new Task(demoTime * TASK_SECOND, TASK_FOREVER, [this](){demoNext();}, &ts, false);

  // if panel was On, switch it back to On
  if (opts.flag.pwrState){
    power(true);
  }

  vopts.flag.initialized = true;
}

void Lamp::power(bool pwr){
  if (pwr == vopts.flag.pwrState) return;  // пропускаем холостые вызовы
  LOGI(T_lamp, printf, "Powering %s\n", pwr ? "On": "Off");

  if (pwr){
    // POWER ON
    // запускаем планировщик движка эффектов
    effectsTimer(true);

    if(vopts.flag.demoMode && demoTask)
      // вторично переключаемся на текущий же эффект, переключение вызовет фейдер (если необходимо)
      demoTask->restart();
    else
      _switcheffect(effswitch_t::num, getFaderFlag(), effwrkr.getCurrentEffectNumber());

    // generate pwr change state event
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwron));
  } else  {
    // POWER OFF
    // гасим Демо-таймер
    if(demoTask)
      demoTask->disable();

    if(opts.flag.fadeEffects){
      // need to fade
      LEDFader::getInstance()->fadelight(0, FADE_TIME );
      // бегунок эффектов будет выключен в функции _fadeEventHandler() по окончании затухания
    } else {
      // no need to fade
      effectsTimer(false);
      EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::pwroff));
    }

    // событие о выключении будет сгенерированно в ответ на событие от фейдера когда его работа завершится
  }

  // update flag on last step to let other call understand in which state they were called
  vopts.flag.pwrState = pwr;
  // save power state if required
  if (opts.flag.restoreState && (opts.flag.pwrState != pwr)){
    opts.flag.pwrState = pwr;
    save_flags();
  }
}

void Lamp::setBrightness(uint8_t tgtbrt, fade_t fade, bool bypass){
    LOGD(T_lamp, printf, "setBrightness(%u,%u,%u)\n", tgtbrt, static_cast<uint8_t>(fade), bypass);
    // if bypass flag is given, than this is a low level request with unscaled brightness that should not be saved or published anywhere
    if (bypass)
      return _brightness(tgtbrt, true);

    // when lamp in 'PowerOff' state, just change brightness w/o any notifications or saves
    if (!getPwr())
      return _brightness(tgtbrt);

    if ( fade == fade_t::on || ( (fade == fade_t::preset) && opts.flag.fadeEffects) ) {
      // fader will publish event once it will finish brightness scaling
      LEDFader::getInstance()->fadelight(tgtbrt);
    } else {
      LOGV(T_lamp, println, "skip fade");
      _brightness(tgtbrt);
      unsigned b = tgtbrt;
      EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(unsigned));
    }

    // set configured brightness variable
    globalBrightness = tgtbrt > _brightnessScale ? _brightnessScale : tgtbrt;
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err != ESP_OK) return;
  // save brightness variable
  handle->set_item(T_bright, globalBrightness);
}

/*
 * Set display brightness
 * note: this method is called by fader also
 */
void Lamp::_brightness(uint8_t brt, bool absolute){
    if (!absolute) brt = luma::curveMap(_curve, brt, MAX_BRIGHTNESS, _brightnessScale);
    if ( brt == display.brightness()) return;  // nothing to change here

    display.brightness(brt);
}

uint8_t Lamp::_get_brightness(bool absolute){
  return absolute ? display.brightness() : luma::curveUnMap(_curve, display.brightness(), MAX_BRIGHTNESS, _brightnessScale);
}

void Lamp::setBrightnessScale(uint8_t scale){
  _brightnessScale = scale ? scale : DEF_BRT_SCALE;
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err != ESP_OK) return;
  // save brightness variable
  handle->set_item(V_dev_brtscale, _brightnessScale);
}

void Lamp::setLumaCurve(luma::curve c){
  if (c == _curve) return;
  _curve = c;
  setBrightness(getBrightness(), fade_t::off);    // switch to the adjusted brightness level
};

void Lamp::gradualFade(evt::gradual_fade_t arg){
  LOGI(T_lamp, println, "Initiate gradual fade");
  if (arg.fromB == -1) arg.fromB = getBrightness();
  if (arg.toB == -1) arg.toB = getBrightness();
  if (arg.fromB == arg.toB) return;
  // direct change start brightness value
  _brightness(arg.fromB);
  LEDFader::getInstance()->fadelight(arg.toB, arg.duration);
}

void Lamp::switcheffect(effswitch_t action, effect_t effnb){
  _switcheffect(action, getPwr() ? getFaderFlag() : false, effnb);
  // if in demo mode, and this switch came NOT from demo timer, delay restart demo timer
  // a bit hakish but will work. Otherwise I have to segregate demo switches from all other
  if(vopts.flag.demoMode && demoTask && ts.timeUntilNextIteration(*demoTask) < demoTask->getInterval())
    demoTask->delay();

  // if lamp is not in Demo mode, then need to save new effect to NVS
  if(!vopts.flag.demoMode){
    esp_err_t err;
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);
    if (err == ESP_OK){
      uint32_t idx = e2int(effwrkr.getCurrentEffectNumber());
      handle->set_item(V_effect_idx, idx);
      LOGD(T_lamp, printf, "save new effnum:%u to NVS\n", idx);
    }
  }

}

/*
 * переключатель эффектов для других методов,
 * может использовать фейдер, выбирать случайный эффект для демо
 * @param effswitch_t action - вид переключения (пред, след, случ.)
 * @param fade - переключаться через фейдер или сразу
 */
void Lamp::_switcheffect(effswitch_t action, bool fade, effect_t effnb) {

  // find real effect number we need to switch to
  switch (action) {
  case effswitch_t::next :
    // если в демо-режиме, ищем следующий эффект для демо, иначе просто следующий эффект
    _swState.pendingEffectNum = vopts.flag.demoMode ? effwrkr.getNextEffIndexForDemo(opts.flag.demoRndOrderSwitching) : effwrkr.getNext();
    break;
  case effswitch_t::prev :
    _swState.pendingEffectNum = effwrkr.getPrev();
    break;
  case effswitch_t::num :
    _swState.pendingEffectNum = effnb;
    break;
  case effswitch_t::rnd :
    // next random effect in demo mode
    _swState.pendingEffectNum = effwrkr.getNextEffIndexForDemo(opts.flag.demoRndOrderSwitching);
    break;
  default:
      return;
  }

  LOGD(T_lamp, printf, "switcheffect() action=%u, fade=%d, effnb=%u\n", static_cast<uint32_t>(action), fade, _swState.pendingEffectNum);

  // проверяем нужно ли использовать затухание (только если лампа включена, и не идет разжигание)
  if (fade && vopts.flag.pwrState && _swState.fadeState <1){
    // если уже идет угасание вниз, просто выходим, переключение произойдет на новый эффект после конца затухания
    if(_swState.fadeState == -1)
      return;

    // если текущая скалированная яркость больше чем 2*FADE_MINCHANGEBRT, то используем затухание до значения FADE_LOWBRTFRACT, в противном случае пропускаем затухание
    if (_get_brightness() > 2*FADE_LOWBRTFRACT){
      LEDFader::getInstance()->fadelight(FADE_LOWBRTFRACT);
      _swState.fadeState = -1;
      return;
    }
  }

  // затухание не требуется, переключаемся непосредственно на нужный эффект
  if(opts.flag.wipeOnEffChange || effwrkr.getCurrentEffectNumber() == effect_t::empty){ // для пустышки или для случая когда включена опция - чистим матрицу
    if (display.getCanvas())
      display.getCanvas()->clear();
  }

  // if current worker's effect is same as the target one, then I do not need to do actual switch
  if (effwrkr.getCurrentEffectNumber() != _swState.pendingEffectNum){
    effwrkr.switchEffect(_swState.pendingEffectNum, vopts.flag.demoMode && opts.flag.demoRndEffControls);

    // publish new effect's control to all available feeders
    effwrkr.embui_publish();
  }

  // need to reapply brightness as effect's curve might have changed and we might also need a fader
  // I use direct access to fader and _brightness, 'cause I do not want to re-publishing brightness value and re-save it to permanent storage
  if ( fade && opts.flag.fadeEffects) {
    LEDFader::getInstance()->fadelight(globalBrightness);
  } else {
    _brightness(globalBrightness);
  }

  LOGD(T_lamp, println, "eof switcheffect()");
}

effect_t Lamp::_getRealativeEffectNum(){
  return (_swState.fadeState != -1) ? effwrkr.getCurrentEffectNumber() : _swState.pendingEffectNum;
}

void Lamp::_fadeEventHandler(){
  // check if lamp is in "PowerOff" state, then we've just complete fade-out, need to send event
  if (!vopts.flag.pwrState){
    _swState.fadeState = 0;
    effectsTimer(false);
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
 * запускаем режим "ДЕМО"
 */
void Lamp::setDemoMode(bool active){
  if (active == vopts.flag.demoMode) return;   // уже и так в нужном "демо" режиме, выходим
  LOGI(T_lamp, printf, "Demo %s, time: %u\n", active ? T_On : T_Off, demoTime);

  vopts.flag.demoMode = active;
  if (active){
    // enable demo
    if (!demoTask)
      demoTask = new Task(demoTime * TASK_SECOND, TASK_FOREVER, [this](){ demoNext(); }, &ts, false);
    demoTask->enable();
  } else {
    if (demoTask){
      delete demoTask;
      demoTask = nullptr;
    }
  }
  // save demo state if required
  if (opts.flag.restoreState && vopts.flag.initialized){
    opts.flag.demoMode = active;
    save_flags();
  }
}

/*
 * включает/выключает режим "демо"
 * @param SCHEDULER enable/disable/reset - вкл/выкл/сброс
 */
void Lamp::setDemoTime(uint32_t seconds){
  // if time is zero - disable demo
  if (!seconds)
    setDemoMode(false);

  demoTime = seconds;
  if(demoTask){
    demoTask->setInterval(demoTime * TASK_SECOND);
  }
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err != ESP_OK) return;
  handle->set_item(T_DemoTime, demoTime);
}

/**
 * @brief switch to next effect in demo mode
 * 
 */
void Lamp::demoNext(){
  LOGD(T_lamp, println, "demoNext");

  if (opts.flag.demoRndOrderSwitching)
    switcheffect(effswitch_t::rnd);
  else
    switcheffect(effswitch_t::next);
}

/*
 * включает/выключает таймер обработки эффектов
 * @param SCHEDULER enable/disable/reset - вкл/выкл/сброс
 */
void Lamp::effectsTimer(bool action) {
  LOGI(T_lamp, printf, "effectsTimer: %u\n", (unsigned)action);
  if (action)
    effwrkr.start();
  else
    effwrkr.stop();
}

void Lamp::save_flags(){
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err != ESP_OK) {
    // if NVS handle is unavailable then just quit
    LOGD(T_ModMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    return;
  }

  handle->set_item(V_lampFlags, opts.pack);
}

void Lamp::_events_subsribe(){
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, Lamp::event_hndlr, this, &_events_lamp_cmd));
}

void Lamp::event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
  LOGV(T_lamp, printf, "event_hndlr %s:%d\n", base, id);
  if (base == LAMP_SET_EVENTS)
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_cmd(base, id, event_data);

  if (base == LAMP_GET_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_get(base, id, event_data);

  if (base == LAMP_CHANGE_EVENTS || base == LAMP_STATE_EVENTS )
    return reinterpret_cast<Lamp*>(handler_args)->_event_picker_state(base, id, event_data);

}

void Lamp::events_unsubsribe(){
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, _events_lamp_cmd);
  _events_lamp_cmd = nullptr;
}

// handle command events and react accordingly with lamp actions
void Lamp::_event_picker_cmd(esp_event_base_t base, int32_t id, void* data){

    switch (static_cast<evt::lamp_t>(id)){
    // Power control
      case evt::lamp_t::pwr : {
        uint32_t v = *reinterpret_cast<uint32_t*>(data);
        if (v == 2)
          power();
        else
          power(v);
        }
        return;
      case evt::lamp_t::pwron :
        power(true);
        return;
      case evt::lamp_t::pwroff :
        power(false);
        return;
      case evt::lamp_t::pwrtoggle :
      case evt::lamp_t::pwronengine :
        power();
        return;

    // Brightness control
      case evt::lamp_t::brightness :
        setBrightness(*((int*) data));
        return;
      case evt::lamp_t::brightness_nofade :
        setBrightness(*((int*) data), fade_t::off);
        break;
      case evt::lamp_t::brightness_lcurve :
        setLumaCurve(*( (luma::curve*)data) );
        break;
      case evt::lamp_t::brightness_step :{
        setBrightness(constrain(getBrightness() + *((int*) data), 0, 255), fade_t::off);
        break;
      }
      case evt::lamp_t::gradualFade :{
        gradualFade(*reinterpret_cast<evt::gradual_fade_t*>(data));
        break;
      }
        
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
      case evt::lamp_t::effSwitchTo :{
        uint32_t n = *((uint32_t*) data);
        switcheffect(effswitch_t::num, static_cast<effect_t>(n));
        break;
      }
      case evt::lamp_t::effSwitchStep : {
        int32_t n = *((int32_t*) data);
        switcheffectStep(n);
        break;
      }

    // Demo control
      case evt::lamp_t::demo : {
        uint32_t v = *reinterpret_cast<uint32_t*>(data);
        if (v == 2)
          setDemoMode(!getDemoMode());
        else
          setDemoMode(v);
        return;
      }
      case evt::lamp_t::demoTimer :
        setDemoTime(*reinterpret_cast<uint32_t*>(data));
        break;

      default:;
    }

}

void Lamp::_event_picker_get(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
  // Get State Commands
    case evt::lamp_t::pwr :
      EVT_POST(LAMP_STATE_EVENTS, vopts.flag.pwrState ? e2int(evt::lamp_t::pwron) : e2int(evt::lamp_t::pwroff));
      break;

    case evt::lamp_t::brightness :{
      int32_t b = getBrightness();
      EVT_POST_DATA( LAMP_STATE_EVENTS, e2int(evt::lamp_t::brightness), &b, sizeof(int32_t) );
      break;
    }

    default:;
  }
}

void Lamp::_event_picker_state(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    case evt::lamp_t::pwron :
      // enable MOSFET
      if (_pins.fet > static_cast<int>(GPIO_NUM_NC)) digitalWrite(_pins.fet,  _pins.fet_ll);
      break;
    case evt::lamp_t::pwroff :
      // disable MOSFET
      if (_pins.fet > static_cast<int>(GPIO_NUM_NC)) digitalWrite(_pins.fet,  !_pins.fet_ll);
      break;
    case evt::lamp_t::fadeEnd :
      // check if effect switching is pending
      _fadeEventHandler();
      break;

    default:;
  }

}

void Lamp::_embui_demoOn(Interface *interf, JsonObjectConst data, const char* action){
  if (data){
    setDemoMode(data[T_demoOn]);
    return;
  }
  // todo: sent demoOn value
}

void Lamp::_embui_demoRndOrder(Interface *interf, JsonObjectConst data, const char* action){
  if (data){
    setDemoRndSwitch(data[T_demoRndOrder]);
    return;
  }
  // todo: send demo val
}

void Lamp::_embui_demoRndCtrls(Interface *interf, JsonObjectConst data, const char* action){
  if (data){
    setDemoRndEffControls(data[T_demoRndCtrls]);
    return;
  }
  // todo: send demo val
}


// *********************************
/*  LEDFader class implementation */

void LEDFader::fadelight(int targetbrightness, uint32_t duration){
  if (!lmp) return;
  LOGD(T_Fade, printf, "tgt:%u, lamp:%u/%u, _br scaled/abs:%u/%u, time:%u\n", targetbrightness, lmp->getBrightness(), lmp->getBrightnessScale(), lmp->_get_brightness(), lmp->_get_brightness(true), duration);

  _brt = lmp->_get_brightness(true);        // get current absolute Display brightness
  _tgtbrt = luma::curveMap(lmp->_curve, targetbrightness, MAX_BRIGHTNESS, lmp->_brightnessScale);

  if (_brt == _tgtbrt){
    // no need to fade, already at this brightness
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd));
    return;
  }

  // calculate required steps
  int32_t _steps = ( abs(_tgtbrt - _brt) > (FADE_MININCREMENT * duration / FADE_MINSTEPTIME) ) ? duration / FADE_MINSTEPTIME : abs(_tgtbrt - _brt)/FADE_MININCREMENT;
  if (_steps < 3) {   // no need to fade for such small difference
    LOGD(T_Fade, printf, "fast: %hhu->%hhu, steps<:%u\n", _brt, _tgtbrt, _steps);
    lmp->_brightness(_tgtbrt, true);
    abort();
    int b = targetbrightness;
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd));
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), &b, sizeof(int));
    return;
  }

  uint32_t interval = duration / _steps;
  _brtincrement = (_tgtbrt - _brt) / _steps;

  if (runner){
    // fading is already in progress, let's readjust it
    runner->setIterations(_steps);
    runner->restartDelayed();
  } else {
    runner = new Task(interval,
      _steps,
      [this](){ _brt += _brtincrement; lmp->_brightness(_brt, true);  // set absolute backend brightness here
                /* LOG(printf_P, PSTR("fd brt %d/%d, glbr:%d, gbr:%d, vid:%d, vid2:%d\n"), _brt, _brtincrement, lmp->getBrightness(), lmp->getBrightness(), brighten8_video(FastLED.getBrightness()), brighten8_video(brighten8_video(FastLED.getBrightness()))  ); */
              },
      &ts, true, nullptr,
      // onDisable
      [this, targetbrightness](){
          lmp->_brightness(_tgtbrt, true);  // set exact target brightness value
          LOGD(T_Fade, printf, "to %hhu complete\n", _tgtbrt);
          int b = targetbrightness;
          EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), &b, sizeof(b));
          // use new task for callback, 'cause effect switching will immidiatetly respawn new fader from callback, so I need to release a Task instance
          //if(_cb) { new Task(FADE_MINSTEPTIME, TASK_ONCE, [this](){ if (_cb) { _cb(); _cb = nullptr; } }, &ts, true, nullptr, nullptr, true ); }
          runner = nullptr;
      },
      true  // self-destruct
    );
  }

  LOGD(T_Fade, printf, "lamp/display:%hhu/%hhu->%d/%hhu, steps:%hu, inc:%hd, interval:%u\n", lmp->getBrightness(), lmp->_get_brightness(true), targetbrightness, _tgtbrt, _steps, _brtincrement, interval);
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
