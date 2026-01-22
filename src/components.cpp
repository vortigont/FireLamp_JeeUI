/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (vortigont)

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

#include "components.hpp"
#include "embuifs.hpp"
#include "Wire.h"

GPIO_Controller gpio_ctl;
// Module Manager instance
ModuleManager zookeeper;





void GPIO_Controller::start(){
  setup_gpios();
}

void GPIO_Controller::setup_gpios(){
  // GPIO's
  JsonDocument doc;
  if (!embuifs::deserializeFile(doc, TCONST_fcfg_gpio)){
    // panel/matrix control fet gpio
    _fet = doc[T_fet][T_gpio] | static_cast<int>(GPIO_NUM_NC);
    _fet_ll = doc[T_fet][T_logicL];
    // gpio that controls FET (for disabling matrix)
    if (_fet > static_cast<int>(GPIO_NUM_NC)){
      pinMode(_fet, OUTPUT);
      digitalWrite(_fet, !_fet_ll);
      // trace panel events 
      _events_subsribe();
    }
  }

  JsonObject i2c0 = doc[T_i2c0];
  if (!i2c0.isNull()){
    // read i2c gpio's
    int _i2c_scl = i2c0[T_scl] | -1;
    int _i2c_sda = i2c0[T_sda] | -1;

    if (_i2c_sda == -1 || _i2c_scl == -1){
      LOGE(T_sensors, printf, "i2c bus pins are not configured, sda:%d, scl:%d\n", _i2c_scl, _i2c_scl);
      return;
    }

    // init i2c bus
    Wire.setPins(_i2c_sda, _i2c_scl);
    if (!Wire.begin()){
      LOGE(T_sensors, println, "i2c init err");
    }
  }

  // todo: init second bus
}

void GPIO_Controller::_events_subsribe(){
  esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID,
  [](void* self, esp_event_base_t base, int32_t id, void* data){ static_cast<GPIO_Controller*>(self)->_event_picker_change(id, data); },
  this, &_change_events_hndlr);
}

void GPIO_Controller::_events_unsubsribe(){
  if (_change_events_hndlr)
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _change_events_hndlr);
}

void GPIO_Controller::_event_picker_change(int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    case evt::lamp_t::pwron :
      // enable panel power via MOSFET
      if (_fet > static_cast<int>(GPIO_NUM_NC)) digitalWrite(_fet,  _fet_ll);
      break;
    case evt::lamp_t::pwroff :
      // disable panel power via MOSFET
      if (_fet > static_cast<int>(GPIO_NUM_NC)) digitalWrite(_fet,  !_fet_ll);
      break;

    default:;
  }

}
