/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023 Emil Muratov (vortigont)

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

// this file contains implementation for Button/Encoder control devices bound to event bus
#include "nvs_handle.hpp"
#include "bencoder.hpp"
#include "embuifs.hpp"
#include "traits.hpp"
#include "char_const.h"
#include "constants.h"
#include "log.h"

ButtonEventHandler::ButtonEventHandler(){
  // restore button lock state
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READONLY, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->get_item(A_dev_btnlock, _btn_lock);
  }
}

void ButtonEventHandler::subscribe(){
  // Register the handler for task iteration event; need to pass instance handle for later unregistration.
  if (!_lmp_einstance){
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, ButtonEventHandler::event_hndlr, this, &_lmp_einstance));
  }

  if (!_btn_einstance){
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), EBTN_EVENTS, ESP_EVENT_ANY_ID, ButtonEventHandler::event_hndlr, this, &_btn_einstance));
  }

  if (!_lmp_set_events)
    esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, ButtonEventHandler::event_hndlr, this, &_lmp_set_events);
}

void ButtonEventHandler::unsubscribe(){
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _lmp_einstance));
  _lmp_einstance = nullptr;
  ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), EBTN_EVENTS, ESP_EVENT_ANY_ID, _btn_einstance));
  _btn_einstance = nullptr;
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, _lmp_set_events);
  _lmp_set_events = nullptr;
};

void ButtonEventHandler::event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  //LOG(printf, "ButtonEventHandler::event_hndlr %s:%d\n", base, id);
  if (base == EBTN_EVENTS)
    return static_cast<ButtonEventHandler*>(handler)->_btnEventHandler(ESPButton::int2event_t(id), reinterpret_cast<EventMsg*>(event_data));

  if ( base == LAMP_CHANGE_EVENTS || base == LAMP_SET_EVENTS )
    return static_cast<ButtonEventHandler*>(handler)->_lmpEventHandler(base, id, event_data);
}

void ButtonEventHandler::_btnEventHandler(ESPButton::event_t e, const EventMsg* msg){
  // ignore all events when button lock is engaged, except when alarm is playing, 'cause alarm is too annoying and could be stopped via button only
  if (_btn_lock && !_alarm)
    return;

  LOGD(T_btn_event, printf, "ID:%u gpio:%d, ctr:%u\n", e, msg->gpio, msg->cntr);

  // if Alarm flag is set, any button event will generate Alarm cancelling event
  if (_alarm){
    _alarm = false;
    EVT_POST(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::alarmStop));
    return;
  }

  // static event longRelease will toggle brightness control direction
  // I do not like it, maybe will rework it later somehow
  if (e == ESPButton::event_t::longRelease){
    _brightness_direction *= -1;
    return;
  }


  for (auto &it : _event_map ){
    //LOG(printf, "Lookup event: it_en:%u, it.e:%u, e:%u ilp:%u lp:%u\n", it.enabled, it.e, e, it.lamppwr, _lamp_pwr );
    if ( it.enabled && (it.e == e) && (it.lamppwr == _lamp_pwr) ){
      // check for multiclicks
      if (e == ESPButton::event_t::multiClick && msg->cntr != it.clicks)
        continue;

      // event matches
      LOGD(T_btn_event, printf, "Execute LampEvent:%u\n", e2int( it.evt_lamp ) );

      switch (it.evt_lamp){
        case evt::lamp_t::effSwitchTo:
          EVT_POST_DATA(LAMP_SET_EVENTS, e2int(it.evt_lamp), &it.arg, sizeof(it.arg));
          break;
        case evt::lamp_t::brightness_step: {
          int step = it.arg * _brightness_direction;
          EVT_POST_DATA(LAMP_SET_EVENTS, e2int(it.evt_lamp), &step, sizeof(int));
          break;
        }
        default:
          EVT_POST(LAMP_SET_EVENTS, e2int(it.evt_lamp));
      }
      return;
    }
  }
}

void ButtonEventHandler::_lmpEventHandler(esp_event_base_t base, int32_t id, void* data){
    switch (static_cast<evt::lamp_t>(id)){
    // Power control
      case evt::lamp_t::pwron :
        _lamp_pwr = true;
        break;
      case evt::lamp_t::pwroff :
        _lamp_pwr = false;
        break;
      case evt::lamp_t::alarmTrigger :
        _alarm = true;
        break;
      case evt::lamp_t::btnLock :
        lock(true);
        break;
      case evt::lamp_t::btnUnLock :
        lock(false);
        break;
    }
}

void ButtonEventHandler::load(JsonVariantConst cfg){
  JsonArrayConst array = cfg.as<JsonArrayConst>();
  if (!array){
    return;   // bad document
  }

  _event_map.clear();

  for(JsonVariantConst v : array) {
    //LOG(printf, "Add cfg Event:%u\n", v[T_btn_event].as<int>() );
    _event_map.emplace_back(ButtonAction(static_cast<ESPButton::event_t>(v[T_btn_event].as<int>()), static_cast<evt::lamp_t>(v[T_lamp_event].as<int>()), v[T_clicks], v[T_arg], v[T_enabled], v[T_pwr] ));
  }

}

void ButtonEventHandler::lock(bool lock){
  if (lock == _btn_lock)
    return;   // no change

  _btn_lock = lock;
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->set_item(A_dev_btnlock, _btn_lock);
  }
  LOGI(T_btn_event, printf, "Button lock:%u\n", lock );
}

