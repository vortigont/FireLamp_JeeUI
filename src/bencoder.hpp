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

#pragma once
#include "espasyncbutton.hpp"
#include "evtloop.h"
#include "ArduinoJson.h"

#define BTN_EVENTS_CFG_JSIZE 4096

struct ButtonAction {
    ESPButton::event_t e;
    evt::lamp_t evt_lamp;
    int32_t clicks;
    int32_t arg;
    bool enabled;
    // lamp power state
    bool lamppwr;
    ButtonAction(ESPButton::event_t e, evt::lamp_t evt_lamp, int32_t clicks = 0, int32_t arg = 0, bool enabled = true, bool lamppwr = true) : e(e), evt_lamp(evt_lamp), clicks(clicks), arg(arg), enabled(enabled), lamppwr(lamppwr) {}
};

class ButtonEventHandler {

    esp_event_loop_handle_t _loop;
    // button event instance
    esp_event_handler_instance_t _btn_einstance = nullptr;
    // lamp state events instance
    esp_event_handler_instance_t _lmp_einstance = nullptr;

    std::list<ButtonAction> _event_map;

    // lamp power state
    bool _lamp_pwr = false;
    // incr/decr multiplicator
    int _brightness_direction = 1;

    static void event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

    void _btnEventHandler(ESPButton::event_t e, const EventMsg* msg);
    void _lmpEventHandler(esp_event_base_t base, int32_t id, void* data);

public:
    ~ButtonEventHandler(){ unsubscribe(); }

    void subscribe( esp_event_loop_handle_t loop );

    void unsubscribe();


    void load(JsonVariantConst cfg);

};


