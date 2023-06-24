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
#include <vector>
//#include "log.h"

/**
 * @brief matrix configuration for LedFB
 * 
 */
class Mtrx_cfg {
friend class LedFB;
    uint16_t _w, _h;         // matrix width, height
    bool _snake;             // matrix type 1:snake( zigzag), 0:parallel
    bool _vertical;          // strip direction: 0 - horizontal, 1 - vertical
    bool _vmirror;           // vertical flip
    bool _hmirror;           // horizontal flip

public:
    Mtrx_cfg(uint16_t w, uint16_t h, bool snake = true, bool _vertical = false, bool vm=false, bool hm=false) : _w(w), _h(h), _snake(snake), _vertical(_vertical), _vmirror(vm), _hmirror(hm) {};
    Mtrx_cfg(Mtrx_cfg const & rhs) : _w(rhs._w), _h(rhs._h), _snake(rhs._snake), _vertical(rhs._vertical), _vmirror(rhs._vmirror), _hmirror(rhs._hmirror) {};

    // get configured matrix width
    uint16_t w() const {return _w;}
    // get configured matrix height
    uint16_t h() const {return _h;}

    bool snake()   const {return _snake;}
    bool vertical()const {return _vertical;}
    bool vmirror() const {return _vmirror;}
    bool hmirror() const {return _hmirror;}

    // return length of the longest side
    uint16_t maxDim() const { return _w>_h ? _w : _h; }
    // return length of the shortest side
    uint16_t minDim() const { return _w<_h ? _w : _h; }

    uint16_t maxHeightIndex() const { return _h-1; }
    uint16_t maxWidthIndex()  const { return _w-1; }

    // setters
    void snake(bool m) {_snake=m;}
    void vertical(bool m) {_vertical=m;}
    void vmirror(bool m){_vmirror=m;}
    void hmirror(bool m){_hmirror=m;}
};

class LedFB {

    /**
     * @brief a poor-man's mutex
     * points to the instance that owns global FastLED's buffer reference
     */
    CLEDController *cled = nullptr;
    std::vector<CRGB> fb;     // main frame buffer

    /**
     * @brief if this buffer is bound to CLED controller,
     * than reset it's pointer to this buffer's data array
     * required to call on iterator invalidation or move semantics
     */
    void _reset_cled(){ if (cled) {cled->setLeds(fb.data(), fb.size());} };

public:
    LedFB(uint16_t w, uint16_t h) : fb(w*h), cfg(w,h) {}

    // copy c-tor
    LedFB(LedFB const & rhs) : fb(rhs.fb), cfg(rhs.cfg) {};
    LedFB& operator=(LedFB const & rhs);

    // move semantics
    LedFB(LedFB&& rhs) noexcept;
    LedFB& operator=(LedFB&& rhs);

    // create from config struct
    LedFB(Mtrx_cfg &c) : fb(c.w()*c.h()), cfg(c) {};

    // d-tor
    ~LedFB();

    // buffer topology configuration
    Mtrx_cfg cfg;

    /**
     * @brief zero-copy swap CRGB data within two framebuffers
     * only CRGB data is swapped, config struct left intact.
     * CLED binding, if any, is updated to point to a newly swapped data.
     * If buffer sizes are different, then no swap occurs.
     * Only buffer sizes have to be equal, other configuration mismatch ignored,
     * have to deal with it elsewhere
     * @param rhs 
     * @return true if swap occured
     * @return false if buffer sizes are different
     */
    bool swap(LedFB& rhs);

    /**
     * @brief return size of FB in pixels
     * 
     */
    size_t size() const { return fb.size(); }

    // get direct access to FB array
    CRGB* data(){ return fb.data(); }

    /**
     * @brief bind this framebuffer to a CLEDController instance
     * 
     * @param pLed instanle of the CLEDController
     * @return true if bind success
     * @return false if this instance is already bound to CLEDController
     */
    bool bind(CLEDController *pLed);

    /**
     * @brief resize LED buffer to specified size
     * content will lost on resize
     * 
     * @param w width in px
     * @param h heigh in px
     */
    void resize(uint16_t w, uint16_t h);

    /***    access methods      ***/

    /**
     * @brief Transpose pixel coordinate x:y into framebuffer's array index
     * it calculates array index based on matrix orientation and configuration
     * no checking performed for supplied coordinates to be out of bound of pixel buffer!
     * for signed negative arguments the behaviour is undefined
     * @param x 
     * @param y 
     * @return size_t 
     */
    size_t transpose(unsigned x, unsigned y) const;

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    CRGB& at(size_t i);

    /**
     * @brief access pixel at coordinates x:y
     * if oob coordinates supplied returns blackhole element
     * @param x coordinate starting from top left corner
     * @param y coordinate starting from top left corner
     */
    CRGB& at(unsigned x, unsigned y);

    // access pixel at coordinates x:y
    CRGB& pixel(unsigned x, unsigned y){ return at(x,y); };

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    CRGB& operator[](size_t i){ return at(i); };

    /*
        iterators
        TODO: need proper declaration for this
    */
    inline std::vector<CRGB>::iterator begin(){ return fb.begin(); };
    inline std::vector<CRGB>::iterator end(){ return fb.end(); };


    /***    color operations      ***/

    /**
     * @brief apply FastLED fadeToBlackBy() func to buffer
     * 
     * @param v 
     */
    void fade(uint8_t v){ fadeToBlackBy(data(), size(), v); }

    /**
     * @brief apply FastLED nscale8() func to buffer
     * i.e.dim whole buffer to black
     * @param v 
     */
    void dim(uint8_t v){ nscale8(data(), size(), v); }

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
extern LedFB *mx;
