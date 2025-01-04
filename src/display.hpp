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

/*

A header file for LED output devices, backends and buffers
*/

#pragma once
#include "ledfb_esp32.hpp"
#include "ledstripe.hpp"
#include "ArduinoJson.h"

#define FASTLED_CURRENT_LIMIT (2000U)                       // лимит по току для адресных лент в миллиамперах, 0 - выключить лимит
#define DEFAULT_TEXT_COLOR  54000

enum class engine_t:uint8_t  {
    ws2812 = 0,
    hub75
};

/**
 * @brief structure for 2D overlay texture callback object
 * 
 */
//using crgb_shared_buff = std::shared_ptr< PixelDataBuffer<CRGB> >;
//template <class COLOR_TYPE>
struct overlay_cb_t {
    uint32_t id;
    std::function <void (LedFB_GFX *buff)> callback;
    //std::function <void (std::shared_ptr< PixelDataBuffer<COLOR_TYPE> > buff)> callback;
};

/**
 * @brief configuration for text bitmap block
 * 
 */
struct TextBitMapCfg {
    int16_t x, y;           // top left corner to place bitmap to
    uint16_t w,  h;         // bitmap WxH
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index;     // font to use
    int8_t baseline_shift_x;       // offset from left side of bitmap
    int8_t baseline_shift_y;       // ofset from bottom of bitmap
    // max text bounds - needed to track max block size to cover the clock text
    uint8_t alpha_bg;
    std::string datefmt{"%F"};
    overlay_cb_t cb{};
};

// My LED engine controller
//template<EOrder RGB_ORDER = RGB>
class LEDDisplay {

    engine_t _etype;        // type of backend to use
    int _gpio{-1};          // fastled gpio
    uint32_t fastled_current_limit{FASTLED_CURRENT_LIMIT};      // Led strip current limit
    EOrder _color_ordr;     // FastLED color order for sw stripes
    uint8_t _brt{32};       // backend engine brightness, if supported

    // flag marking use of a double buffer
    bool _use_db = false;

    // LED stripe matrix with a desired topology and layout / or HUB75 panel buffer
    std::shared_ptr< LedFB<CRGB> > _canvas;

    // GFX object
    std::shared_ptr< LedFB_GFX > _gfx;

    // overlay buffer
    //std::weak_ptr< LedFB<uint16_t> > _ovr;

    // Addresable led strip topology transformation object
    LedTiles tiles;

    // An object ref I'll use to access LED rendering engine
    DisplayEngine<CRGB> *_dengine = nullptr;

    /**
     * @brief a stack of overlay callback structs
     * on buffer show, each struct in a stack is applied on top of canvas before rendering canvas to the engine
     * 
     */
    std::list< overlay_cb_t > _stack;




    bool _start_rmt(const JsonDocument& doc);
    bool _start_rmt_engine();
    bool _start_hub75(const JsonDocument& doc);

public:


    // load configuration and create objects for respective backend
    bool start();

    // *** Getters

    // display engine type
    engine_t get_engine_type() const { return _etype; }

    const LedTiles& getLayout() const { return tiles; } 

    // get FastLED gpio
    int getGPIO() const { return _gpio; }

    // get FastLED current limit
    uint32_t getCurrentLimit() const { return fastled_current_limit; }

    // get ws stripe colo order
    int getColorOrder() const;

    /*** SETTERS ***/

    // set FastLED gpio
    void setGPIO(int gpio){ if (gpio <= NUM_OUPUT_PINS) _gpio = gpio; }

    // set FastLED current limit
    void setCurrentLimit(uint32_t i);

    // set ws stripe colo order
    void setColorOrder(int order);

    // Update LED stripe topology and sizing
    void updateStripeLayout(uint16_t w, uint16_t h, uint16_t wcnt, uint16_t hcnt,
                            bool tsnake, bool tvert, bool tvmirr, bool thmirr,
                            bool snake, bool vert, bool vmirr, bool hmirr
    );

    void canvasProtect(bool v);

    std::shared_ptr< LedFB<CRGB> > getCanvas() { return _canvas; }

    // draw data to display
    void show();

    /**
     * @brief apply overlay to canvas
     * 
     */
    //void overlay_render();

    // Wipe all layers and buffers
    void clear(){ if (_dengine) _dengine->clear(); };

    // backend brightness control
    uint8_t brightness(uint8_t brt);

    // get current backend brightness, if supported by backend, otherwise returns stored brightness
    uint8_t brightness();

    // print stripe configuration in debug mode
    void print_stripe_cfg();

    /**
     * @brief attach overlay callback struct to stack
     * 
     * @param f 
     */
    void attachOverlay(overlay_cb_t f);

    /**
     * @brief dettach overlay callback struct to stack
     * 
     * @param id
     */
    void detachOverlay(uint32_t id);

};

extern LEDDisplay display;
