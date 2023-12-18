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
#include <Arduino.h>
#include "esp32-hal.h"
#include "evtloop.h"

// LOGGING
#ifdef ARDUINO
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif

static const char* TAG = "evt";

ESP_EVENT_DEFINE_BASE(LAMP_SET_EVENTS);        // declaration of Lamp Command events base
ESP_EVENT_DEFINE_BASE(LAMP_GET_EVENTS);
ESP_EVENT_DEFINE_BASE(LAMP_STATE_EVENTS);      // declaration of Lamp State publishing events base
ESP_EVENT_DEFINE_BASE(LAMP_CHANGE_EVENTS);     // declaration of Lamp State change notification events base

namespace evt {

#define LOOP_EVT_Q_SIZE         8              // events loop queue size
#define LOOP_EVT_PRIORITY       1              // task priority is same as arduino's loop() to avoid extra context switches
#define LOOP_EVT_RUNNING_CORE   ARDUINO_RUNNING_CORE  //   tskNO_AFFINITY
#ifdef LAMP_DEBUG
 #define LOOP_EVT_STACK_SIZE     4096           // task stack size
#else
 #define LOOP_EVT_STACK_SIZE     2048           // task stack size
#endif

void start(){
  if (hndlr) return;

  ESP_LOGI(TAG, "Cretating Event loop");

  esp_event_loop_args_t evt_cfg;
  evt_cfg.queue_size = LOOP_EVT_Q_SIZE;
  evt_cfg.task_name = "evt_loop";
  evt_cfg.task_priority = LOOP_EVT_PRIORITY;            // uxTaskPriorityGet(NULL) // same as parent
  evt_cfg.task_stack_size = LOOP_EVT_STACK_SIZE;
  evt_cfg.task_core_id = LOOP_EVT_RUNNING_CORE;

  //ESP_ERROR_CHECK(esp_event_loop_create(&levt_cfg, &loop_levt_h));
  esp_err_t err = esp_event_loop_create(&evt_cfg, &hndlr);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "event loop creation failed!");
  }
}

void stop(){ esp_event_loop_delete(hndlr); hndlr = nullptr; };

esp_event_loop_handle_t get_hndlr(){ return hndlr; }

void debug(){
    ESP_ERROR_CHECK( esp_event_handler_instance_register_with(hndlr, ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, debug_hndlr, NULL, nullptr) );
}

void debug_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
  Serial.printf("evt tracker: %s id:%d\n", base, id);
}


} // namespace evt
