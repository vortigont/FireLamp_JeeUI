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

/*
    Informer that draws data to display overlay
*/

#pragma once

#include "display.hpp"
#include "ts.h"
#include "ui.h"
#include "evtloop.h"
#include "char_const.h"

#define DEFAULT_TEXT_COLOR  54000

/**
 * @brief an abstract class to implement small "apps" or widgets
 * practically it is just a class that is able to load/save it's state serialized,
 * has a periodic timer ticker and could attach/detach to event bus
 * 
 */
class GenericWidget : public Task {

protected:
    // widget label or "name"
    const char* label;

    /**
     * @brief derived method should generate object's configuration into provided JsonVariant
     * 
     * @param cfg 
     * @return JsonVariantConst 
     */
    virtual void generate_cfg(JsonVariant cfg) const = 0;

    /**
     * @brief load configuration from a json object
     * 
     * @param cfg 
     */
    virtual void load_cfg(JsonVariantConst cfg) = 0;

public:

    /**
     * @brief Construct a new Generic Widget object
     * 
     * @param wlabel - widget label identifier
     * @param interval - ticker execution interval in ms
     */
    GenericWidget(const char* wlabel, unsigned interval);
    virtual ~GenericWidget(){};

    // function to run on ticker call 
    virtual void widgetRunner() = 0;

    /**
     * @brief load widget's config from persistent storage and calls start()
     * 
     */
    void load(){ load(load_cfg_from_NVS(label)); };

    /**
     * @brief load widget's config from supplied config and calls start()
     * 
     */
    void load(JsonVariantConst cfg);

    /**
     * @brief save current widget's configuration to NVS
     * 
     */
    void save();

    /**
     * @brief save supplied widget's configuration to NVS
     * it will actually try to merge supplied object with the one stored in NVS
     * @param cfg 
     */
    void save(JsonVariantConst cfg);


    // start widget ticker
    virtual void start(){ enable(); };

    // stop widget ticker
    virtual void stop(){ disable(); };

    /**
     * @brief Get widget's configuration packed into json object
     * used to feed control's values to WebUI/MQTT
     * @return JsonVariant
     */
    JsonVariant getConfig() const;

    /**
     * @brief Set widget's configuration packed into json object
     * this call will also SAVE supplied configuration to persistent storage
     */
    void setConfig(JsonVariantConst cfg);

    /**
     * @brief Get widget's Label
     * 
     * @return const char* 
     */
    const char* getLabel() const { return label; }

    /**
     * @brief load widget config using widget name as a config selector key
     * 
     */
    static JsonVariant load_cfg_from_NVS(const char* lbl);

};

using widget_pt = std::unique_ptr<GenericWidget>;


class GenericGFXWidget : public GenericWidget {


protected:
    LedFB_GFX   *screen = nullptr;
    // буфер оверлея
    //std::shared_ptr<LedFB<CRGB> > overlay;

    // make/release display overlay
    bool getOverlay();

    void releaseOverlay();

public:
    GenericGFXWidget(const char* wlabel, unsigned interval) : GenericWidget(wlabel, interval){};
    virtual ~GenericGFXWidget(){ releaseOverlay(); };

};


class ClockWidget : public GenericGFXWidget {

struct Clock {
    int16_t x, y;       // cursor to print Clock
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index; // font to use
    uint8_t seconds_font_index; // font to use
    bool show_seconds;          // show seconds
    bool twelwehr;      // 12/24 hour clock
    // max text bounds - needed to track max block size to cover the clock text
    uint16_t maxW{0}, smaxW{0};   //, maxH{0};
    // save seconds starting position
    int16_t scursor_x, scursor_y;
};

struct Date {
    int16_t x, y;       // cursor to print Clock
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index; // font to use
    bool show;          // show date
    // max text bounds - needed to track max block size to cover the clock text
    uint16_t maxW{0};   //, maxH{0};
};

// кукушка
struct Cuckoo {
    uint8_t hr{0}, hhr{0}, quater{0};
};

    // elements structs
    Clock clk{};
    Date date{};
    Cuckoo _cuckoo{};
    // last timestamp
    std::time_t last_date;
    // flag that indicates screen needs a refresh
    bool redraw;

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
    ClockWidget();
    ~ClockWidget();

    void widgetRunner() override;

    void start() override;
    void stop() override;
};

class WidgetManager {

    // widgets container
    std::list<widget_pt> _widgets;

    /**
     * @brief spawn a new instance of a widget with supplied config
     * used with configuration is suplied via webui for non existing widgets
     * @param widget_label 
     * @param cfg whether to use a supplied configuration or load from NVS
     * @param persistent if 'true' a spawned widget will save supplied configuration to NVS, set this to false if spawning widget with cfg FROM NVS to avoid exta writes
     */
    void _spawn(const char* widget_label, JsonVariantConst cfg, bool persistent = false);

public:
    //WidgetManager();
    //~WidgetManager(){};

    void start(const char* label = NULL);
    void stop(const char* label);

    void register_handlers();
    void unregister_handlers();

    // can't be const due to EmbUI's value method, TODO: fix it
    JsonVariant getConfig(const char* widget_label);
    void setConfig(const char* widget_label, JsonVariantConst cfg);

    // generate values representing state of the active widgets
    void getWidgetsState(Interface *interf) const;
};


// Unary predicate for Widget's label search match
template <class T>
class MatchLabel : public std::unary_function<T, bool>{
    std::string_view _lookup;
public:
    explicit MatchLabel(const char* label) : _lookup(label) {}
    bool operator() (const T& item ){
        // T is widget_pt
        return _lookup.compare(item->getLabel()) == 0;
    }
};

class AlarmClock : public GenericWidget {
// кукушка
struct Cuckoo {
    // active minutes
    uint8_t hr{0}, hhr{0}, quater{0};
    // work hours
    uint8_t on, off;
};

    Cuckoo _cuckoo{};

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) const override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    // cockoo/talking clock
    void _cockoo_events(std::tm *tm);

public:
    AlarmClock();

    void widgetRunner() override;

    //void start() override;
    //void stop() override;
};

/**
 * @brief register EmbUI action handlers for managing widgets
 * 
 */
void register_widgets_handlers();


extern WidgetManager informer;
