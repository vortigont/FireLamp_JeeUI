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
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "log.h"

#define FASTLED_VOLTAGE     5
#define FASTLED_MIN_CURRENT 1000


bool LEDDisplay::start(){
    if (_dengine) return true;   // Overlay engine already running

    DynamicJsonDocument doc(1024);
    // if config can't be loaded, then just quit, 'cause we need at least an engine type to run
    if (!embuifs::deserializeFile(doc, TCONST_fcfg_display)) return false;

    // a shortcut for hub75 testing
    if (doc[T_display_type] == static_cast<int>(engine_t::hub75))
        return _start_hub75(doc);

    if (!doc.containsKey(T_ws2812)) return false;    // no object with stripe config

    // shortcut
    JsonVariant o = doc[T_ws2812];

    // load gpio value, if defined
    setGPIO(o[T_mx_gpio].as<int>());

    // set current, if defined
    setCurrentLimit(o[T_CLmt]);

    // load canvas topology
    tiles.setTileDimensions(
        o[T_width],
        o[T_height],
        o[T_wcnt],
        o[T_hcnt]
    );

    // matrix layout
    tiles.setLayout( o[T_snake], o[T_vertical], o[T_vflip], o[T_hflip] );
    // tiles layout
    tiles.tileLayout.setLayout(o[T_tsnake], o[T_tvertical], o[T_tvflip], o[T_thflip]);

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
        _canvas = std::make_shared< LedFB<CRGB> >(tiles.canvas_w(), tiles.canvas_h(), _dengine->getCanvas());
        _canvas->setRemapFunction( [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->tiles.transpose(w, h, x, y); } );
    }

    brightness(_brt);   // reset FastLED brightness level

    print_stripe_cfg();

    return true;
}

bool LEDDisplay::_start_hub75(const DynamicJsonDocument& doc){
    _etype = engine_t::hub75;


    // shortcut to hub75 config object
    JsonVariantConst o = doc[T_hub75];

    // check if config is empty
    if (o.isNull() || !o.size())
        return false;

    tiles.setTileDimensions(o[T_width], o[T_height], 1, 1);

    // HUB75 config struct
    HUB75_I2S_CFG::i2s_pins _pins={ o[T_R1], o[T_G1], o[T_B1], o[T_R2], o[T_G2], o[T_B2],
                                    o[T_A], o[T_B], o[T_C], o[T_D], o[T_E],
                                    o[T_LAT], o[T_OE], o[T_CLK]
    };

    HUB75_I2S_CFG mxconfig(
                        o[T_width], o[T_height],
                        1,      // chain length
                        _pins,   // pin mapping
                        static_cast<HUB75_I2S_CFG::shift_driver>( o[T_shift_drv] ),     // driver chip
                        false,              // double buff (we do not need it)
                        static_cast<HUB75_I2S_CFG::clk_speed>( o[T_clk_rate] ),
                        o[T_lat_blank],
                        o[T_clk_phase],
                        o[T_min_refresh],
                        o[T_color_depth]
    );

    _dengine = new ESP32HUB75_DisplayEngine(mxconfig);

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    if (!_canvas){
        _canvas = std::make_shared< LedFB<CRGB> >(o[T_width], o[T_height], _dengine->getCanvas());
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
    //fastled_current_limit = i < FASTLED_MIN_CURRENT ? FASTLED_MIN_CURRENT : i;
    fastled_current_limit = i;
    FastLED.setMaxPowerInVoltsAndMilliamps(FASTLED_VOLTAGE, i);
}


// my display object
LEDDisplay display;