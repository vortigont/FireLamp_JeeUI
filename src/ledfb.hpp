/*
Copyright © 2023 Emil Muratov (https://github.com/vortigont/FireLamp_JeeUI)
Copyright © 2020 Dmytro Korniienko (https://github.com/DmytroKorniienko/FireLamp_EmbUI)
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
#include "config.h"
#include <FastLED.h>

constexpr uint16_t num_leds = WIDTH * HEIGHT;       // for backward compat

// TODO: design templated container object with common array's methods to access trasposed data based on matrix configuration type
struct LedFB {
    struct mtrx_t {
        const uint16_t w, h;    // width, height
        //bool mxtype;          // matrix type 0:zigzag, 1:parallel   (not implemented yet)
        //uint8_t direction;    // strip direction        (not implemented yet)
        //uint8_t angle;        // strip connection angle (not implemented yet)
        bool vmirror{0};        // зеркалирование по вертикали
        bool hmirror{0};        // зеркалирование по горизонтали
        mtrx_t(uint16_t w, uint16_t h) : w(w), h(h) {};
    };

    mtrx_t cfg;
    std::array<CRGB, num_leds> *fb;     // main frame buffer

    LedFB(uint16_t w, uint16_t h) : cfg(w,h) { fb = new(std::nothrow) std::array<CRGB, num_leds>; }

    /**
     * @brief return size of FB in pixels
     * 
     */
    static constexpr size_t size() { return num_leds; }

    // get direct access to FB array
    CRGB* data(){ return fb->data(); }
    inline CRGB *getUnsafeLedsArray(){ return fb->data(); };    // obsolete, left for compatibility

    /**
     * @brief Transpose poxel coordinate x:y int framebuffer's array index
     * it calculates array index based on matrix orientation and configuration
     * no checking performed for supplied coordinates to be out of bound of pixel buffer!
     * @param x 
     * @param y 
     * @return size_t 
     */
    size_t transpose(uint16_t x, uint16_t y);

    /**
     * @brief access pixel at coordinates x:y
     * if oob coordinates supplied returns element at (0,0)
     * @param x coordinate starting from top left corner
     * @param y coordinate starting from top left corner
     */
    inline CRGB& pixel(uint16_t x, uint16_t y){ return at(transpose(x,y)); };

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    CRGB& at(size_t i);

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    CRGB& operator[](size_t i){ return at(i); };


    /*      color operations        */

    /**
     * @brief apply FastLED fadeToBlackBy() func to buffer
     * 
     * @param v 
     */
    void fade(uint8_t v){ fadeToBlackBy(data(), size(), v); }

    /**
     * @brief fill the buffer with solid color
     * 
     */
    void fill(const CRGB &color);

    /**
     * @brief clear buffer to black
     * 
     */
    void clear();
};

/* a backward compatible wrappers for accessing LedMatrix obj instance,
should be removed once other code refactoring is complete
*/
#define getUnsafeLedsArray      mx.getUnsafeLedsArray
#define getPixelNumber(X,Y)     mx.transpose(X,Y)
extern LedFB mx;
