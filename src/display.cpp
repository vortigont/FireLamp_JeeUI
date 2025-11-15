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

An object file for LED output devices, backends and buffers
*/

#include "display.hpp"
#include "devices.h"
#include "embuifs.hpp"
#include "char_const.h"
#include "ESP32-HUB75-MatrixPanel-I2S-DMA.h"
#include "log.h"

#define FASTLED_VOLTAGE         5
#define FASTLED_MIN_CURRENT     1000
#define DEFAULT_I2S_CLOCK_RATE  8000000

bool LEDDisplay::start(){
    if (_dengine) return true;   // Overlay engine already running

    JsonDocument doc;
    // if config can't be loaded, then just quit, 'cause we need at least an engine type to run
    if (embuifs::deserializeFile(doc, TCONST_fcfg_display)) return false;

    // a shortcut for hub75 testing
    if (doc[T_display_type] == static_cast<int>(engine_t::hub75))
        return _start_hub75(doc);

    return _start_rmt(doc);
}

bool LEDDisplay::_start_rmt(const JsonDocument& doc){
    if (_dengine) return true;  // RMT already running

    LOGI(T_Display, println, "starting RMT engine");

    // shortcut
    JsonVariantConst o = doc[T_ws2812];

    if (o == nullptr) return false;    // no object with stripe config

    // load gpio value, if defined
    int gpio = o[T_mx_gpio] | -1;
    setGPIO(gpio);

    setColorOrder(o[T_col_order]);

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

    return _start_rmt_engine();
}

bool LEDDisplay::_start_rmt_engine(){
  // RMT engine setup
  if (_gpio == -1){
      LOGW(T_Display, println, "Won't run on GPIO -1");
      return false;      // won't run on disabled pin
  }

  LOGD(T_Display, printf, "run on GPIO %d\n", _gpio);

  // create new led strip object using our configured pin
  _dengine = new ESP32RMTDisplayEngine(_gpio, _color_ordr, tiles.canvas_w() * tiles.canvas_h());

  // attach buffer to an object that will perform matrix layout trasformation on buffer access
  if (!_canvas){
      _canvas = std::make_shared< LedFB<CRGB> >(tiles.canvas_w(), tiles.canvas_h(), _dengine->getBuffer());
      _canvas->setRemapFunction( [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->tiles.transpose(w, h, x, y); } );
    _gfx = std::make_shared< LedFB_GFX >(_canvas);
  }

  brightness(_brt);   // reset FastLED brightness level

  print_stripe_cfg();

  return true;
}

bool LEDDisplay::_start_hub75(const JsonDocument& doc){
  // do not mess with existing engine
  if (_dengine) return false;
    _etype = engine_t::hub75;


    // shortcut to hub75 config object
    JsonVariantConst o = doc[T_hub75];

    // check if config is empty
    if (o.isNull() || !o.size())
        return false;

    tiles.setTileDimensions(o[T_width], o[T_height], 1, 1);

    // HUB75 config struct
    HUB75_I2S_CFG::i2s_pins _pins{ o[T_R1], o[T_G1], o[T_B1], o[T_R2], o[T_G2], o[T_B2],
                                    o[T_A], o[T_B], o[T_C], o[T_D], o[T_E],
                                    o[T_LAT], o[T_OE], o[T_CLK]
    };

    HUB75_I2S_CFG mxconfig(
                        o[T_width], o[T_height],
                        1,      // chain length
                        _pins,   // pin mapping
                        static_cast<HUB75_I2S_CFG::shift_driver>( o[T_shift_drv] ),     // driver chip
                        false,              // double buff (we do not need it)
                        static_cast<HUB75_I2S_CFG::clk_speed>( o[T_clk_rate] | DEFAULT_I2S_CLOCK_RATE ),
                        o[T_lat_blank],
                        o[T_clk_phase],
                        o[T_min_refresh],
                        o[T_color_depth]
    );

    // check if panel size not a zero
    if (!mxconfig.mx_width || !mxconfig.mx_height) return false;

    _dengine = new ESP32HUB75_DisplayEngine(mxconfig);

    // attach buffer to an object that will perform matrix layout trasformation on buffer access
    _canvas = std::make_shared< LedFB<CRGB> >(o[T_width], o[T_height], _dengine->getBuffer());
    // this is a simple flat matrix so I use default 2D transformation
    _gfx = std::make_shared< LedFB_GFX >(_canvas);

    brightness(_brt);   // reset brightness level

    return true;
}

/*
std::shared_ptr< LedFB<uint16_t> > LEDDisplay::getOverlay(){
  auto instance = _ovr.lock();

  // if engine or canvas does not exist (yet) just return empty obj here
  if (!_dengine || !_canvas)
    return instance;

  if (!instance){
    // no overlay exist at the moment, let's create one
    //instance = std::make_shared< LedFB<CRGB> >(tiles.canvas_w(), tiles.canvas_h(), _dengine->getOverlay());
    instance = std::make_shared< LedFB<uint16_t> >( tiles.canvas_w(), tiles.canvas_h(), std::make_shared<PixelDataBuffer<uint16_t>>(tiles.canvas_w() * tiles.canvas_h()) );

    if (_etype == engine_t::ws2812){
        // set topology mapper
        instance->setRemapFunction( [this](unsigned w, unsigned h, unsigned x, unsigned y) -> size_t { return this->tiles.transpose(w, h, x, y); } );
    }

    // add instance watcher
    _ovr = instance;
  }
  return instance;
}
*/

void LEDDisplay::updateStripeLayout(uint16_t w, uint16_t h, uint16_t wcnt, uint16_t hcnt,
                            bool snake, bool vert, bool vmirr, bool hmirr,
                            bool tsnake, bool tvert, bool tvmirr, bool thmirr){

    if (_etype == engine_t::hub75) return;   // no resize for HUB75 driver

    tiles.setLayout(snake, vert, vmirr, hmirr);
    tiles.tileLayout.setLayout(tsnake, tvert, tvmirr, thmirr);

    // start rmt, if has not been started yet
    if (!_start_rmt_engine()){
        LOG(println, "can't start RMT engine");
    }

    // check if I really need to resize LED buffer
    if (w != tiles.tile_w() || h != tiles.tile_h() || wcnt != tiles.tile_wcnt() || hcnt != tiles.tile_hcnt()){
        if (!_dengine) return;  // no engine running

        _dengine->clear();
        _canvas->resize(w*wcnt, h*hcnt);
        //auto instance = _ovr.lock();
        //if (instance) instance->resize(w*wcnt, h*hcnt);
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
    LOGD(T_Display, printf, "Stripe layout W=%dx%d, H=%dx%d\n", tiles.tile_w(), tiles.tile_wcnt(), tiles.tile_h(), tiles.tile_hcnt() );
    LOGD(T_Display, printf, "Matrix: snake:%o, vert:%d, vflip:%d, hflip:%d\n", tiles.snake(), tiles.vertical(), tiles.vmirror(), tiles.hmirror() );
    LOGD(T_Display, printf, "Tiles: snake:%o, vert:%d, vflip:%d, hflip:%d\n", tiles.tileLayout.snake(), tiles.tileLayout.vertical(), tiles.tileLayout.vmirror(), tiles.tileLayout.hmirror() );
}

void LEDDisplay::setCurrentLimit(uint32_t i){
    //fastled_current_limit = i < FASTLED_MIN_CURRENT ? FASTLED_MIN_CURRENT : i;
    fastled_current_limit = i;
    FastLED.setMaxPowerInVoltsAndMilliamps(FASTLED_VOLTAGE, i);
}

void LEDDisplay::setColorOrder(int order){

    EOrder c_order;

    switch(order){
        case 1 :
            c_order = RBG;
            break;
        case 2 :
            c_order = GRB;
            break;
        case 3 :
            c_order = GBR;
            break;
        case 4 :
            c_order = BRG;
            break;
        case 5 :
            c_order = BGR;
            break;
        default :
            c_order = RGB;
    };

    _color_ordr = c_order;
}

int LEDDisplay::getColorOrder() const {
    switch(static_cast<uint32_t>(_color_ordr)){
        case 1 :
            return RBG;
        case 2 :
            return GRB;
        case 3 :
            return GBR;
        case 4 :
            return BRG;
        case 5 :
            return BGR;
        default :
            return RGB;
    };
}

void LEDDisplay::show(){
  if (!_dengine) return;
  if (_use_db){
    std::lock_guard<std::mutex> lock(_dbuff_mtx);
    // save buffer content
    _dengine->copyFront2Back();
  }

  // for all overlay structs in stack call a callback function that will render it over canvas
  {
    std::lock_guard<std::mutex> lock(_stack_mtx);
    for (auto &s : _stack)
      s.callback( _gfx.get() );
  }

  _dengine->show();

  if (_use_db){
    std::lock_guard<std::mutex> lock(_dbuff_mtx);
    // restore buffer content
    _dengine->flipBuffer();
  }
};

void LEDDisplay::canvasProtect(bool v){
  if (_dengine){
    std::lock_guard<std::mutex> lock(_dbuff_mtx);
    _dengine->doubleBuffer(v);
    _use_db = v;
  }
}

void LEDDisplay::attachOverlay( overlay_cb_t f){
  //LOGV(T_Display, printf, "new ovr:%u\n", &f);
  auto cb = std::find_if(_stack.begin(), _stack.end(), [&f](const overlay_cb_t& fn){ return f.id == fn.id; } );
  if (cb == _stack.end()){
    std::lock_guard<std::mutex> lock(_stack_mtx);
    LOGD(T_Display, printf, "add overlay: %u\n", f.id);
    _stack.push_back(f);
  } else {
    LOGV(T_Display, println, "overlay cb already exist");
  }
}

void LEDDisplay::detachOverlay( uint32_t id){
  //LOGV(T_Display, printf, "try remove ovr:%u\n", *(long *)(char *)&f);
  std::lock_guard<std::mutex> lock(_stack_mtx);
  auto cb = std::find_if(_stack.cbegin(), _stack.cend(), [id](const overlay_cb_t& fn){ return id == fn.id; } );
  if ( cb != _stack.cend() ){
    LOGD(T_Display, printf, "remove overlay: %u\n", id);
    _stack.erase(cb);
  } else {
    LOGV(T_Display, println, "overlay cb not found!");
  }
}


// template to compare std::function pointee
// https://stackoverflow.com/questions/20833453/comparing-stdfunctions-for-equality


/*
void LEDDisplay::overlay_render(){
  if (_ovr.expired()) return;

  // need to apply overlay on canvas
  auto ovr = _ovr.lock();
  if (_canvas->size() != ovr->size()) return;  // a safe-check for buffer sizes

  // since overlay has same remapping function, I could simply apply it pixel to pixel
  for (size_t i = 0; i != _canvas->size();  ++i ){
    _canvas->at(i) = LedFB_GFX::colorCRGB( ovr->at(i) );
  }

  // draw non key-color pixels from canvas, otherwise from overlay
  for (size_t y = 0; y != _canvas->h();  ++y )
    for (size_t x = 0; x != _canvas->w();  ++x ){
      //CRGB c = ovr->at(i) == _transparent_color ? _canvas->at(i) : ovr->at(i);
      if (ovr->at(x, y)) // if pixel in overlay is not 'black', draw it on canvas
        _canvas->  at(x, y) = LedFB_GFX::colorCRGB( ovr->at(x, y) );
        //_canvas->hub75.drawPixelRGB888( i % canvas->hub75.getCfg().mx_width, i / canvas->hub75.getCfg().mx_width, c.r, c.g, c.b);
    }
}
*/


// my display object
LEDDisplay display;
