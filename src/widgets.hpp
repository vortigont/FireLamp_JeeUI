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
#include "char_const.h"

#define DEFAULT_TEXT_COLOR  54000

/**
 * @brief an abstract class to implement small "apps" or widgets
 * practically it is just a class that is able to load/save it's state serialized,
 * has a periodic timer ticker and could attach/detach to event bus
 * 
 */
class GenericWidget : public Task {

    /**
     * @brief load widget config using widget name as a config selector key
     * 
     */
    void _deserialize_cfg();

protected:
    // widget label or "name"
    const char* label;

    /**
     * @brief derived method should generate object's configuration into provided JsonVariant
     * 
     * @param cfg 
     * @return JsonVariantConst 
     */
    virtual void generate_cfg(JsonVariant cfg) = 0;

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
     * @brief load widget's config from persistent storage and runs ticker
     * 
     */
    void begin();

    /**
     * @brief Get widget's configuration packed into json object
     * used to feed control's values to WebUI/MQTT
     * @return JsonVariant
     */
    JsonVariant getConfig();

    /**
     * @brief Set widget's configuration packed into json object
     * 
     */
    void setConfig(JsonVariantConst cfg);

};


class GenericGFXWidget : public GenericWidget {


protected:
    LedFB_GFX   *screen = nullptr;
    // буфер оверлея
    std::shared_ptr<LedFB<CRGB> > overlay;

    // make/release display overlay
    bool getOverlay();

    void releaseOverlay();

public:
    GenericGFXWidget(const char* wlabel, unsigned interval) : GenericWidget(wlabel, interval){};
    virtual ~GenericGFXWidget(){};

};


class ClockWidget : public GenericGFXWidget {

struct Clock {
    int16_t x, y;       // cursor to print Clock
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index; // font to use
    uint8_t seconds_font_index; // font to use
    bool show_seconds;          // show seconds
    bool twelwehr;      // 12/24 hour clock
    bool fresh;         // flag that indicates if date has been displayed yet or needs a refresh
    // max text bounds - needed to track max block size to cover the clock text
    int16_t minX{0x7fff}, minY{0x7fff};
    uint16_t maxW{0}, maxH{0};
    // same for seconds
    int16_t sminX{0x7fff}, sminY{0x7fff};
    uint16_t smaxW{0}, smaxH{0};
};

struct Date {
    int16_t x, y;       // cursor to print Clock
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index; // font to use
    bool show;          // show date
    bool fresh;         // flag that indicates if date has been displayed yet or needs a refresh
    // max text bounds - needed to track max block size to cover the clock text
    int16_t minX{32762}, minY{32762};
    uint16_t maxW{0}, maxH{0};
};

    // elements structs
    Clock clk{};
    Date date{};
    // last timestamp
    std::time_t last_date;

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    // print clock
    void _print_clock(std::tm *tm);

    // print date
    void _print_date(std::tm *tm);

public:
    ClockWidget() : GenericGFXWidget(T_w_clock, TASK_SECOND){}
    void widgetRunner() override;
};

class WidgetManager {

    bool registered{false};

    std::unique_ptr<GenericGFXWidget> clock;

public:
    //WidgetManager();

    void start();
    void stop();

    void register_handlers();
    void unregister_handlers();

    JsonVariant getConfig(const char* widget_label);
    void setConfig(const char* widget_label, JsonVariantConst cfg);
};


// EmbUI Widgets page
void ui_page_widgets(Interface *interf, const JsonObject *data, const char* action);

extern WidgetManager informer;
