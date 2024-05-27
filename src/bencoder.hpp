/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (Vortigont)

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
#include "ESP32Encoder.h"
#include "evtloop.h"
#include "ArduinoJson.h"
#include "freertos/timers.h"

#define BTN_EVENTS_CFG_JSIZE    4096
#define ENCODER_SWITCH_MODES    3       // number of modes encoder could switch to (brightness, effects, mp3volume)

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

    struct Encoder_counts {
        // track button state
        bool btn{false};
        // which mode currently encoder is switching now
        int8_t mode{0};
        // encoder counter
        int32_t last{0};
        // ignore next click event (after encoder mode change)
        bool ignore{false};
    };

    // button event instance
    esp_event_handler_instance_t _btn_einstance = nullptr;
    // encoder events
    esp_event_handler_instance_t _enc_events = nullptr;
    // lamp state events instance
    esp_event_handler_instance_t _lmp_einstance = nullptr;
    // lamp set events instance
    esp_event_handler_instance_t _lmp_set_events = nullptr;

    // events list
    std::list<ButtonAction> _event_map;

    // enable handling encoder events
    bool _encoderEnabled;
    // Button lock
    bool _btn_lock = false;
    // lamp power state
    bool _lamp_pwr = false;
    // flag tracks when Alarm event triggers, in this case any button event will generate Alarm cancelling event
    bool _alarm = false;

    // incr/decr multiplicator
    int _brightness_direction = 1;

    // encoder counts and values
    Encoder_counts _enc;

    static void event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

    // button & encoder events processor
    void _btnEventHandler(ESPButton::event_t e, const EventMsg* msg);
    // lamp events processor
    void _lmpEventHandler(esp_event_base_t base, int32_t id, void* data);


public:
    ButtonEventHandler(bool withEncoder = false);
    ~ButtonEventHandler(){ unsubscribe(); }

    void subscribe();

    void unsubscribe();


    void load(JsonVariantConst cfg);

    // get button lock state
    bool lock() const { return _btn_lock; }

    /**
     * @brief set button lock
     * i.e. disable button from accidental press, cats, dogs, etc...
     * 
     * @param lock 
     */
    void lock(bool lock);
};


class PCNT_Encoder : public ESP32Encoder {
    // lamp set events instance
    esp_event_handler_instance_t _lmp_set_events = nullptr;

    TimerHandle_t _tmr = nullptr;

    int64_t _cnt{0};

    // encoder poller
    void _poller();

public:
    //RotaryEncoderControl();

    void subscribe();

    void unsubscribe();


    void load(JsonVariantConst cfg);

};
