/*
Copyright © 2023 Emil Muratov (Vortigont)
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

#pragma once

// original lib is https://github.com/AKJ7/TM1637
// current mod is from https://github.com/vortigont/TM1637
#include "TM1637.h"
#include "evtloop.h"
#include "ts.h"
#include <WiFi.h>

#define TM_BRIGHTNESS_MAX 7U //яркость дисплея, 0..7
#define TM_BRIGHTNESS_ON  5
#define TM_BRIGHTNESS_OFF 1

class TMDisplay : private TM1637 {
public:
  TMDisplay(uint8_t clkPin, uint8_t dataPin) : TM1637 (clkPin, dataPin) {};
  ~TMDisplay();

  // use 12hr mode for clock display
  bool clk_12h = false;

  // display leading zero for clock if current hours is <10
  bool clk_lzero = false;

  // initialize display and attach to event bus
  void init();

  /**
   * @brief set display brightness
   * 
   * @param b - 0-7
   * @param lampon - flag if lamp is on or off
   */
  void brightness(uint8_t b, bool lampon = true);

  /**
   * @brief static event handler
   * wraps class members access for event loop
   * 
   * @param handler_args 
   * @param base 
   * @param id 
   * @param event_data 
   */
  static void event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);

private:

  // brightness when lamp is on
  uint8_t brtOn = TM_BRIGHTNESS_ON;
  // brightness when lamp is off
  uint8_t brtOff = TM_BRIGHTNESS_OFF;

  // scheduler worker
  Task _wrkr;

  // WiFi events callback handler
  void _onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
  wifi_event_id_t eid;

  // mesasge display timeout
  unsigned timer{0};
  // how may times to repeat message scroll
  unsigned repeat{0};

  bool showColon{false};

  // display clock
  void _showClock();

  // scroll something
  void _scrool();

  // starts or appends new scroll text
  void _addscroll(const char* t, int rpt = 0);

  // *** Event bus members    ***

  // instance that holds tm's command events handlers
  esp_event_handler_instance_t _evt_ch_hndlr;
  esp_event_handler_instance_t _evt_set_hndlr;

  /**
   * @brief event picker method, processes incoming command events from a event_hndlr wrapper
   * 
   * @param base 
   * @param id 
   * @param event_data 
   */
  void _event_picker(esp_event_base_t base, int32_t id, void* data);

  /**
   * @brief loop call that is triggered by scheduler
   * 
   */
  void _loop();
};

extern TMDisplay *tm1637;
