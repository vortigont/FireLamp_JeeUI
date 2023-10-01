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

/*

A header file for LED output devices, backends and buffers
*/

#pragma once
#include "config.h"
#include "ledfb.hpp"

/* a backward compatible wrappers for accessing LedMatrix obj instance,
should be removed once other code refactoring is complete
*/
extern LedFB<CRGB> *mx;

enum class engine_t:uint8_t  {
    ws2812 = 0,
    hub75
};

// My LED engine controller
//template<EOrder RGB_ORDER = RGB>
class LEDDisplay {

    engine_t _etype;        // type of backend to use
    int _gpio{-1};          // fastled gpio
    int _w{16}, _h{16};     // display dimensions
    uint8_t _brt{32};       // backend engine brightness, if supported

    // An object ref I'll use to access LED device
    DisplayEngine *_dengine = nullptr;

    // LED stripe matrix with a desired topology and layout  
    LedFB<CRGB> *_canvas = nullptr;

    // overlay buffer
    std::weak_ptr< LedFB<CRGB> > _ovr;

    // Addresable led strip topology transformation object
    LedStripe stripe;

    bool _start_rmt();
    bool _start_hub75();

public:
    bool start(engine_t e);

    void updateTopo(int w, int h, bool snake, bool vert, bool vmirr, bool hmirr);

    void canvasProtect(bool v){ if (_dengine) _dengine->canvasProtect(v); };

    LedFB<CRGB>* getCanvas() { return _canvas; }

    /**
     * @brief Get a pointer to Overlay buffer
     * Note: consumer MUST release a pointer once overlay operations is no longer needed to save RAM and CPU cycles on overlay mixing
     * 
     * @return std::shared_ptr<LedStripe> 
     */
    std::shared_ptr< LedFB<CRGB> > getOverlay();

    // draw data to display
    void show(){ if (_dengine) _dengine->show(); };

    // Wipe all layers and buffers
    void clear(){ if (_dengine) _dengine->clear(); };

    // backend brightness control
    uint8_t brightness(uint8_t brt);

    // get current backend brightness, if supported by backend, otherwise returns stored brightness
    uint8_t brightness();
};

extern LEDDisplay display;
