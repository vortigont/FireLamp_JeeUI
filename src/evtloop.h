/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023 Emil Muratov (vortigont)
    Copyright © 2020 Dmytro Korniienko (kDn)

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

#pragma once
#include "esp_event.h"

// helper macro to reduce typing
#define EVT_POST(event_base, event_id) esp_event_post_to(evt::get_hndlr(), event_base, event_id, NULL, 0, portMAX_DELAY)
#define EVT_POST_DATA(event_base, event_id, event_data, data_size) esp_event_post_to(evt::get_hndlr(), event_base, event_id, event_data, data_size, portMAX_DELAY)

// ESP32 event loop defines
ESP_EVENT_DECLARE_BASE(LAMP_CMD_EVENTS);        // declaration of Lamp Command events base
ESP_EVENT_DECLARE_BASE(LAMP_STATE_EVENTS);      // declaration of Lamp State publishing events base
ESP_EVENT_DECLARE_BASE(LAMP_CHANGE_EVENTS);     // declaration of Lamp State change notification events base

// Lamp's Event Loop
namespace evt {


// Lamp events
enum class lamp_t:int32_t {
  noop = 0,               // NoOp command
  // 0-15 are reserved for something extraordinary


  // lamp power and mode of operation
  pwron = 16,              // switch power
  pwroff,
  pwrtoggle,

  // brightness control
  brightness,
  brightnessScale,
  fadeStart,
  fadeEnd,

  // get state commands
  getpwr = 1024,


  noop_end                // NoOp
};


  // LighEvents loop handler
  static esp_event_loop_handle_t hndlr = nullptr;

  /**
   * @brief Starts Lamp's event loop task
   * this loop will manage events processing between Lamp's components and controls
   *
   * @return esp_event_loop_handle_t* a pointer to loop handle
   */
  void start();

  /**
   * @brief Stops Lamp's event loop task
   *
   * @return esp_event_loop_handle_t* a pointer to loop handle
   */
  void stop();

  esp_event_loop_handle_t get_hndlr();


  void debug();

  void debug_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);
} // namespace evt
