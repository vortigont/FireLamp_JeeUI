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

#include "modules/mod_manager.hpp"

class ClockModule : public GenericModuleProfiles, public Task {

enum class ovrmixer_t {
    bgfade,
    alphablend,
    color_scale
};

struct Clock {
    int16_t x, y;                       // top left corner to place bitmap to
    uint16_t w, h;                      // bitmap WxH
    ovrmixer_t mixer;
    uint8_t baseline_shift_x, baseline_shift_y;
//    int16_t baseline
    uint16_t color_txt, color_bg;       // color in 5-6-5 mode
    uint8_t alpha_tx, alpha_bg;         // transparency
    uint8_t font_index;                 // font to use for hr:min
    uint8_t seconds_font_index;         // font to use for seconds
    bool show_seconds;                  // show seconds
    bool twelwehr;                      // 12/24 hour clock
    int16_t eff_num;                    // switch to effect number 'num'
    // max text bounds - needed to track max block size to cover the clock text
    overlay_cb_t cb{};
};

    // elements structs
    Clock clk{};
    TextBitMapCfg date{};
    bool date_show;
    // last timestamp
    std::time_t last_date;
    // flag that indicates screen needs a refresh
    bool redraw;
    // text mask buffer
    std::unique_ptr<Arduino_Canvas_Mono> _textmask_clk;
    std::unique_ptr<Arduino_Canvas_Mono> _textmask_date;

    esp_event_handler_instance_t _hdlr_lmp_change_evt = nullptr;
    esp_event_handler_instance_t _hdlr_lmp_state_evt = nullptr;

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) const override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    // print clock
    void _print_clock(std::tm *tm);

    // print date
    void _print_date(std::tm *tm);

    // cockoo/talking clock
    void _cockoo_events(std::tm *tm);

    static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

    // change events handler
    void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

    // set events handler
    //void _lmpSetEventHandler(esp_event_base_t base, int32_t id, void* data);

public:
    ClockModule();
    ~ClockModule();

    void moduleRunner();

    void start() override;
    void stop() override;
};

class AlarmClock : public GenericModule, public Task {
    // типы будильника
    enum class alarm_t {
        onetime = 0,
        daily,
        workdays,
        weekends
    };

    // параметры будильника
    struct AlarmCfg {
        bool active;
        alarm_t type;
        // trigger time
        uint8_t hr{0}, min{0};
        // track name to play for alarm
        int track;
        // sunrise
        bool rise_on;       // enable light rise
        int sunrise_offset, sunrise_startBr, sunrise_endBr;
        uint32_t sunrise_duration, sunrise_eff;
        // sunset
        bool dusk_on;       // enable light dusk
        bool dusk_pwroff;   // shutdown lamp on dust fade end
        int dusk_startBr, dusk_endBr;
        uint32_t dusk_duration, dusk_eff;
    };

    // кукушка
    struct Cuckoo {
        uint8_t hr{0}, hhr{0}, quater{0};
        // inactive hours
        uint8_t on, off;
    };

    // a set of alarms
    std::array<AlarmCfg, 4> _alarms{};

    // Cuckoo configuration
    Cuckoo _cuckoo{};

    // flag that alarm is await for dusk fade-end
    bool _fade_await{false};

    // cockoo/talking clock
    void _cockoo_events(std::tm *tm);

    // рассвет
    void _sunrise_check();

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) const override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    esp_event_handler_instance_t _hdlr_lmp_change_evt = nullptr;

    // change events handler
    void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

public:
    AlarmClock();
    ~AlarmClock();

    void moduleRunner();

    void setAlarmItem(JsonVariant cfg);

    void start() override { enable(); };
    void stop() override { disable(); };

    /**
     * @brief Construct an EmbUI page with module's state/configuration
     * 
     * @param interf 
     * @param data 
     * @param action 
     */
    void mkEmbUIpage(Interface *interf, const JsonObject *data, const char* action) override;

};

