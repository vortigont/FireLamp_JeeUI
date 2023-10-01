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

An object file for LED output devices, backends and buffers
*/
#include "display.hpp"
#include "embuifs.hpp"
#include "char_const.h"
#include "log.h"
#include "hub75.h"
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"

// compatibility LED buffer object reference
LedFB<CRGB> *mx = nullptr;

//template<EOrder RGB_ORDER>
bool LEDDisplay::start(engine_t e){
    if (_dengine) return true;   // Overlay engine already running

    // a shortcut for hub75 testing
    if (e == engine_t::hub75){
        return _start_hub75();
    }

    DynamicJsonDocument doc(512);
    embuifs::deserializeFile(doc, TCONST_fcfg_gpio);

    // load gpio value
    _gpio = doc[TCONST_mx_gpio].as<int>();

    embuifs::deserializeFile(doc, TCONST_fcfg_ledstrip);
    JsonObject o = doc.as<JsonObject>();

    // in case if deserialization has failed, I create a default 16x16 buffer 
    _w = o[TCONST_width]  | 16;
    _h = o[TCONST_height] | 16;

    //_set_layout(o[TCONST_snake].as<bool>(), o[TCONST_vertical].as<bool>(), o[TCONST_vflip].as<bool>(), o[TCONST_hflip].as<bool>());
    stripe.snake(o[TCONST_snake]);
    stripe.vertical(o[TCONST_vertical]);
    stripe.vmirror(o[TCONST_vflip]);
    stripe.hmirror(o[TCONST_hflip]);

    return _start_rmt();
}

//template<EOrder RGB_ORDER>
bool LEDDisplay::_start_rmt(){
    if (_dengine) return true;  // RMT already running

    // RMT engine setup
    if (_gpio == -1) return false;      // won't run on disabled pin

    // create new led strip object using our configured pin
    _dengine = new ESP32RMTDisplayEngine<COLOR_ORDER>(_gpio, _w*_h);

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    if (!_canvas){
        _canvas = new LedFB<CRGB>(_w, _h, _dengine->getCanvas());
        auto callback = [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->stripe.transpose(w, h, x, y); };
        _canvas->setRemapFunction(callback);
    }

    brightness(_brt);   // reset FastLED brightness level
    LOG(printf, "RMT LED cfg: w,h:(%d,%d) snake:%d, vert:%d, vflip:%d, hflip:%d\n", _w, _h, stripe.snake(), stripe.vertical(), stripe.vmirror(), stripe.hmirror());

    // compatibility stub
    mx = _canvas;
    return true;
}

bool LEDDisplay::_start_hub75(){
    _w = 64; _h = 32;
    HUB75_I2S_CFG::i2s_pins _pins={R1, G1, BL1, R2, G2, BL2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(
                        64,     // width
                        32,     // height
                        1,      // chain length
                        _pins   // pin mapping
                        //HUB75_I2S_CFG::FM6126A      // driver chip
    );

    _dengine = new ESP32HUB75_DisplayEngine(mxconfig);

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    if (!_canvas){
        _canvas = new LedFB<CRGB>(_w, _h, _dengine->getCanvas());
        // this is a simple flat matrix so I use default 2D transformation

    }

    brightness(_brt);   // reset brightness level

    // compatibility stub
    mx = _canvas;
    return true;
}

std::shared_ptr< LedFB<CRGB> > LEDDisplay::getOverlay(){
    auto instance = _ovr.lock();

    if (!instance){
        // no overlay exist at the moment, let's create one
        instance = std::make_shared< LedFB<CRGB> >(LedFB<CRGB>(_w, _h, _dengine->getOverlay()));

        if (_etype == engine_t::ws2812){
            // set topology
            auto callback = [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->stripe.transpose(w, h, x, y); };
            instance->setRemapFunction(callback);
        }

        // add instance watcher
        _ovr = instance;
    }
    return instance;
}

void LEDDisplay::updateTopo(int w, int h, bool snake, bool vert, bool vmirr, bool hmirr){
    if (_etype == engine_t::hub75) return;   // no resize for HUB75 driver

    if (w != _w || _h != h){
        _w = w; _h = h;
        _dengine->clear();
        _canvas->resize(w, h);
        auto instance = _ovr.lock();
        if (instance) instance->resize(w, h);
    }

    stripe.snake(snake);
    stripe.vertical(vert);
    stripe.vmirror(vmirr);
    stripe.hmirror(hmirr);
}

uint8_t LEDDisplay::brightness(uint8_t brt){
    _brt = brt;
    return _dengine ? _dengine->brightness(brt) : brt;
};


uint8_t LEDDisplay::brightness(){
    return _etype == engine_t::hub75 ? _brt : FastLED.getBrightness();
}


// my display object
LEDDisplay display;