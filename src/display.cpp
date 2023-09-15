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

// compatibility LED buffer object reference
LedStripe *mx = nullptr;

//template<EOrder RGB_ORDER>
bool LEDDisplay::start(){
    if (_oengine) return true;   // RMT already running

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
    _set_layout(o[TCONST_snake], o[TCONST_vertical], o[TCONST_vflip], o[TCONST_hflip]);

    return _start_rmt();
}

//template<EOrder RGB_ORDER>
bool LEDDisplay::_start_rmt(){
    if (_oengine) return true;  // RMT already running

    // RMT engine setup
    if (_gpio == -1) return false;      // won't run on disabled pin

    // create new led strip object using our configured pin
    _oengine = new ESP32RMTOverlayEngine<COLOR_ORDER>(_gpio);

    // create CLED data buffer
    auto data_buffer = std::make_shared<CLedCDB>(CLedCDB(_w*_h));

    // attach buffer to dispplay
    if (_oengine)
        _oengine->attachCanvas(data_buffer);
    else
        return false;

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    if (!_canvas)
        _canvas = new LedStripe(_w, _h, data_buffer);

    // apply our layout and topology parameters
    _apply_layout();

    //LOG(printf, "LED cfg: w,h:(%d,%d) snake:%d, vert:%d, vflip:%d, hflip:%d\n", _w, _h, _sn, _vrt, _vm, _hm);

    // compatibility stub
    mx = _canvas;
    return true;
}

std::shared_ptr<LedStripe> LEDDisplay::getOverlay(){
    auto instance = _ovr.lock();

    if (!instance){
        // no overlay exist at the moment, let's create one
        instance = std::make_shared<LedStripe>(LedStripe(_w, _h, _oengine->getOverlay()));

        // set topology
        instance->snake(_sn);
        instance->vertical(_vrt);
        instance->vmirror(_vm);
        instance->hmirror(_hm);

        // add instance watcher
        _ovr = instance;
    }
    return instance;
}

void LEDDisplay::updateTopo(int w, int h, bool snake, bool vert, bool vmirr, bool hmirr){
    if (w != _w || _h != h){
        _w = w; _h = h;
        _oengine->clear();
        _canvas->resize(w, h);
        auto instance = _ovr.lock();
        if (instance) instance->resize(w, h);
    }

    _set_layout(snake, vert, vmirr, hmirr);
    _apply_layout();
}

void LEDDisplay::_set_layout(bool snake, bool vert, bool vmirr, bool hmirr){
    _sn = snake;
    _vrt = vert;
    _vm = vmirr;
    _hm = hmirr;
}

void LEDDisplay::_apply_layout(){
    // apply our layout parameters
    if (_canvas){
        _canvas->snake(_sn);
        _canvas->vertical(_vrt);
        _canvas->vmirror(_vm);
        _canvas->hmirror(_hm);
    }

    auto instance = _ovr.lock();

    if (instance){
        instance->snake(_sn);
        instance->vertical(_vrt);
        instance->vmirror(_vm);
        instance->hmirror(_hm);
    }
}

// my display object
LEDDisplay display;