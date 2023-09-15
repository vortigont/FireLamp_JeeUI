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

#include "ledfb.hpp"
//#include "log.h"

// Timings from FastLED chipsets.h
// WS2812@800kHz - 250ns, 625ns, 375ns
// время "отправки" кадра в матрицу, мс. где 1.5 эмпирический коэффициент
// #define FastLED_SHOW_TIME = WIDTH * HEIGHT * 24 * (0.250 + 0.625) / 1000 * 1.5

// stub pixel that is mapped to either nonexistent buffer access or blackholed CLedController mapping
static CRGB blackhole;

// copy via assignment
PixelDataBuffer& PixelDataBuffer::operator=(PixelDataBuffer const& rhs){
    fb = rhs.fb;
    return *this;
}

// move assignment
PixelDataBuffer& PixelDataBuffer::operator=(PixelDataBuffer&& rhs){
    fb = std::move(rhs.fb);
    return *this;
}

CRGB& PixelDataBuffer::at(size_t i){ return i < fb.size() ? fb.at(i) : blackhole; };

void PixelDataBuffer::fill(CRGB color){ fb.assign(fb.size(), color); };

void PixelDataBuffer::clear(){ fill(CRGB::Black); };

bool PixelDataBuffer::resize(size_t s){
    fb.reserve(s);
    clear();
    return fb.size() == s;
};

// *** CLedCDB implementation ***

// move construct
CLedCDB::CLedCDB(CLedCDB&& rhs) noexcept : PixelDataBuffer(std::forward<CLedCDB>(rhs)) {
    cled = rhs.cled;
    if (rhs.cled){  // steal cled pointer, if set
        rhs.cled = nullptr;
    }
    _reset_cled();      // if we moved data from rhs, than need to reset cled controller
    //LOG(printf, "Move Constructing: %u From: %u\n", reinterpret_cast<size_t>(fb.data()), reinterpret_cast<size_t>(rhs.fb.data()));
};

// move assignment
CLedCDB& CLedCDB::operator=(CLedCDB&& rhs){
    fb = std::move(rhs.fb);

    if (cled && rhs.cled && (cled != rhs.cled)){
        /* oops... we are moving from a buff binded to some other cled controller
        * since there is no method to detach active controller from CFastLED
        * than let's use a dirty WA - bind a blackhole to our cled and steal rhs's ptr
        */
        cled->setLeds(&blackhole, 1);
    }

    if (rhs.cled){ cled = rhs.cled; rhs.cled = nullptr; }   // steal a pointer from rhs
    _reset_cled();      // if we moved data from rhs, than need to reset cled controller
    //LOG(printf, "Move assign: %u from: %u\n", reinterpret_cast<size_t>(fb.data()), reinterpret_cast<size_t>(rhs.fb.data()));
    // : fb(std::move(rhs.fb)), cfg(rhs.cfg){ LOG(printf, "Move Constructing: %u From: %u\n", reinterpret_cast<size_t>(&fb), reinterpret_cast<size_t>(&rhs.fb)); };
    return *this;
}

CLedCDB::~CLedCDB(){
    if (cled){
        /* oops... somehow we ended up in destructor with binded cled controller
        * since there is no method to detach active controller from CFastLED
        * than let's use a dirty WA - bind a blackhole to cled untill some other buffer
        * regains control
        */
       cled->setLeds(&blackhole, 1);
    }
}

void CLedCDB::swap(PixelDataBuffer& rhs){
    rhs.swap(*this);
    _reset_cled();
}

void CLedCDB::swap(CLedCDB& rhs){
    std::swap(fb, rhs.fb);
    _reset_cled();
    rhs._reset_cled();
}

bool CLedCDB::resize(size_t s){
    bool result = PixelDataBuffer::resize(s);
    _reset_cled();
    return result;
};

bool CLedCDB::bind(CLEDController *pLed){
    if (!pLed) return false;  // some empty pointer

    /* since there is no method to unbind CRGB buffer from CLEDController,
    than if there is a pointer already exist we refuse to rebind to a new cled.
    It's up to user to deal with it
    */
    if (cled && (cled !=pLed)) return false;

    cled = pLed;
    _reset_cled();
    return true;
}

void CLedCDB::rebind(CLedCDB &rhs){
    //LOG(println, "PixelDataBuffer rebind");
    std::swap(cled, rhs.cled);  // swap pointers, if any
    _reset_cled();
    rhs._reset_cled();
}


// *** Topology mapping classes implementation ***

LedFB::LedFB(uint16_t w, uint16_t h) : _w(w), _h(h){
    buffer = std::make_shared<PixelDataBuffer>(PixelDataBuffer(w*h));
}

LedFB::LedFB(uint16_t w, uint16_t h, std::shared_ptr<PixelDataBuffer> fb): _w(w), _h(h), buffer(fb){
    // a safety check if supplied buffer and dimentions are matching
    if (buffer->size() != w*h)   buffer->resize(w*h);
};

LedFB::LedFB(LedFB const & rhs) : _w(rhs._w), _h(rhs._h) {
    buffer = rhs.buffer;
    // deep copy
    //buffer = std::make_shared<PixelDataBuffer>(*rhs.buffer.get());
}

CRGB& LedFB::at(int16_t x, int16_t y){
    return ( static_cast<uint16_t>(x) >= w() || static_cast<uint16_t>(y) >= h() ) ? blackhole : buffer->at(transpose(static_cast<uint16_t>(x), static_cast<uint16_t>(y)));
};

bool LedFB::resize(uint16_t w, uint16_t h){
    // safety check
    if (buffer->resize(w*h) && buffer->size() == w*h){
        _w=w; _h=h;
        return true;
    }
    return false;
}

// matrix stripe layout transformation
size_t LedStripe::transpose(unsigned x, unsigned y) const {
    unsigned idx = y*w()+x;
    if ( vertical() ){
        // verticaly ordered stripes
        bool ivm{hmirror()}, ihm{vmirror()};                // reverse mirror
        bool virtual_mirror = (snake() && x%2) ? !ihm : ihm;    // for snake-shaped strip, invert vertical odd columns
        size_t xx = virtual_mirror ? w() - idx/h()-1 : idx/h();
        size_t yy = ivm ? h()-idx%h()-1 : idx%h();
        return yy * w() + xx;
    } else {
        // horizontaly ordered stripes
        bool virtual_mirror = (snake() && y%2) ? !hmirror() : hmirror(); // for snake-shaped displays, invert horizontal odd rows
        size_t yy = vmirror() ? h() - idx/w()-1 : idx/w();
        size_t xx = virtual_mirror ? w()-idx%w()-1 : idx%w();
        return yy * w() + xx;
    }
}

