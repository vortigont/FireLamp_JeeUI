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

#define FASTLED_VOLTAGE     5



bool LEDDisplay::start(){
    if (_dengine) return true;   // Overlay engine already running

    DynamicJsonDocument doc(512);
    // if config can't be loaded, then just quit, 'cause we need at least an engine type to run
    if (!embuifs::deserializeFile(doc, TCONST_fcfg_display)) return false;

    // a shortcut for hub75 testing
    if (doc[TCONST_dtype] == static_cast<int>(engine_t::hub75)){
        tiles.setTileDimensions(64, 32, 1, 1);
        return _start_hub75();
    }

    if (!doc.containsKey(TCONST_ws2812)) return false;    // no object with stripe config

    // shortcut
    JsonVariant o = doc[TCONST_ws2812];

    // load gpio value
    _gpio = o[TCONST_mx_gpio].as<int>();

    // set current
    setCurrentLimit(o[TCONST_CLmt]);

    // load canvas topology
    tiles.setTileDimensions(
        o[TCONST_width],
        o[TCONST_height],
        o[TCONST_wcnt],
        o[TCONST_hcnt]
    );

    // matrix layout
    tiles.setLayout( o[TCONST_snake], o[TCONST_vertical], o[TCONST_vflip], o[TCONST_hflip] );
    // tiles layout
    tiles.tileLayout.setLayout(o[TCONST_tsnake], o[TCONST_tvertical], o[TCONST_tvflip], o[TCONST_thflip]);

    return _start_rmt();
}

//template<EOrder RGB_ORDER>
bool LEDDisplay::_start_rmt(){
    if (_dengine) return true;  // RMT already running

    // RMT engine setup
    if (_gpio == -1) return false;      // won't run on disabled pin

    // create new led strip object using our configured pin
    _dengine = new ESP32RMTDisplayEngine<COLOR_ORDER>(_gpio, tiles.canvas_w() * tiles.canvas_h());

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    if (!_canvas){
        _canvas = new LedFB<CRGB>(tiles.canvas_w(), tiles.canvas_h(), _dengine->getCanvas());
        _canvas->setRemapFunction( [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->tiles.transpose(w, h, x, y); } );
    }

    brightness(_brt);   // reset FastLED brightness level

    print_stripe_cfg();

    return true;
}

bool LEDDisplay::_start_hub75(){
    // static configuration
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
        _canvas = new LedFB<CRGB>(64, 32, _dengine->getCanvas());
        // this is a simple flat matrix so I use default 2D transformation

    }

    brightness(_brt);   // reset brightness level

    return true;
}

std::shared_ptr< LedFB<CRGB> > LEDDisplay::getOverlay(){
    auto instance = _ovr.lock();

    if (!instance){
        // no overlay exist at the moment, let's create one
        instance = std::make_shared< LedFB<CRGB> >(LedFB<CRGB>(tiles.canvas_w(), tiles.canvas_h(), _dengine->getOverlay()));

        if (_etype == engine_t::ws2812){
            // set topology mapper
            instance->setRemapFunction( [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->tiles.transpose(w, h, x, y); } );
        }

        // add instance watcher
        _ovr = instance;
    }
    return instance;
}

void LEDDisplay::updateStripeLayout(uint16_t w, uint16_t h, uint16_t wcnt, uint16_t hcnt,
                            bool snake, bool vert, bool vmirr, bool hmirr,
                            bool tsnake, bool tvert, bool tvmirr, bool thmirr){

    if (_etype == engine_t::hub75) return;   // no resize for HUB75 driver

    tiles.setLayout(snake, vert, vmirr, hmirr);
    tiles.tileLayout.setLayout(tsnake, tvert, tvmirr, thmirr);

    // start rmt, if has not been started yet
    if (!_start_rmt()){
        LOG(println, "can't start RMT engine");
    }

    // check if I really need to resize LED buffer
    if (w != tiles.tile_w() || h != tiles.tile_h() || wcnt != tiles.tile_wcnt() || hcnt != tiles.tile_hcnt()){
        if (!_dengine) return;  // no engine running

        _dengine->clear();
        _canvas->resize(w*wcnt, h*hcnt);
        auto instance = _ovr.lock();
        if (instance) instance->resize(w*wcnt, h*hcnt);
        tiles.setTileDimensions(w, h, wcnt, hcnt);
    }
    print_stripe_cfg();
}

uint8_t LEDDisplay::brightness(uint8_t brt){
    _brt = brt;
    return _dengine ? _dengine->brightness(brt) : brt;
};


uint8_t LEDDisplay::brightness(){
    return _etype == engine_t::hub75 ? _brt : FastLED.getBrightness();
}

void LEDDisplay::print_stripe_cfg(){
    LOG(printf, "Stripe layout W=%dx%d, H=%dx%d\n", tiles.tile_w(), tiles.tile_wcnt(), tiles.tile_h(), tiles.tile_hcnt() );
    LOG(printf, "Matrix: snake:%o, vert:%d, vflip:%d, hflip:%d\n", tiles.snake(), tiles.vertical(), tiles.vmirror(), tiles.hmirror() );
    LOG(printf, "Tiles: snake:%o, vert:%d, vflip:%d, hflip:%d\n", tiles.tileLayout.snake(), tiles.tileLayout.vertical(), tiles.tileLayout.vmirror(), tiles.tileLayout.hmirror() );
}

void LEDDisplay::setCurrentLimit(uint32_t i){
    fastled_current_limit = i < FASTLED_MIN_CURRENT ? FASTLED_MIN_CURRENT : i;
    FastLED.setMaxPowerInVoltsAndMilliamps(FASTLED_VOLTAGE, fastled_current_limit);
}


// my display object
LEDDisplay display;