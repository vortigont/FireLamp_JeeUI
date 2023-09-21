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
#include <vector>
#include <memory>
#include <variant>
#include "w2812-rmt.hpp"
#include <Adafruit_GFX.h>


// Out-of-bound CRGB placeholder - stub pixel that is mapped to either nonexistent buffer access or blackholed CLedController mapping
static CRGB blackhole;

/**
 * @brief Base class with CRGB data storage that acts as a pixel buffer storage
 * it provides basic operations with pixel data with no any backend engine to display data
 * 
 */
template <class COLOR_TYPE = CRGB>
class PixelDataBuffer {

protected:
    std::vector<COLOR_TYPE> fb;     // container that holds pixel data

public:
    PixelDataBuffer(size_t size) : fb(size) {}

    /**
     * @brief Copy-Construct a new Led FB object
     *  it also does NOT copy persistence flag
     * @param rhs 
     */
    PixelDataBuffer(PixelDataBuffer const & rhs) : fb(rhs.fb) {};

    /**
     * @brief Copy-assign a new Led FB object
     * @param rhs 
     */
    PixelDataBuffer& operator=(PixelDataBuffer<COLOR_TYPE> const & rhs);

    // move semantics
    /**
     * @brief Move Construct a new PixelDataBuffer object
     * constructor will steal a cled pointer from a rhs object
     * @param rhs 
     */
    PixelDataBuffer(PixelDataBuffer&& rhs) noexcept : fb(std::move(rhs.fb)){};

    /**
     * @brief Move assignment operator
     * @param rhs 
     * @return PixelDataBuffer& 
     */
    PixelDataBuffer& operator=(PixelDataBuffer&& rhs);

    // d-tor
    virtual ~PixelDataBuffer() = default;

    /**
     * @brief return size of FB in pixels
     * 
     */
    virtual size_t size() const { return fb.size(); }

    /**
     * @brief zero-copy swap CRGB data within two framebuffers
     * config struct is also swapped between object instances
     * @param rhs - object to swap with
     */
    virtual void swap(PixelDataBuffer& rhs){ std::swap(fb, rhs.fb); };

    // get direct access to FB array
    std::vector<COLOR_TYPE> &data(){ return fb; }


    /**
     * @brief resize LED buffer to specified size
     * content will be lost on resize
     * 
     * @param s new number of pixels
     */
    virtual bool resize(size_t s);

    /***    access methods      ***/

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    COLOR_TYPE& at(size_t i);

    // access pixel at coordinates x:y (obsolete)
    //CRGB& pixel(unsigned x, unsigned y){ return at(x,y); };

    /**
     * @brief access CRGB pixel at specified position
     * in case of oob index supplied returns a reference to blackhole
     * @param i offset index
     * @return CRGB& 
     */
    COLOR_TYPE& operator[](size_t i){ return at(i); };

    /*
        iterators
        TODO: need proper declaration for this
    */
    typename std::vector<COLOR_TYPE>::iterator begin(){ return fb.begin(); };
    typename std::vector<COLOR_TYPE>::iterator end(){ return fb.end(); };


    /***    color operations      ***/

    /**
     * @brief apply FastLED fadeToBlackBy() func to buffer
     * 
     * @param v 
     */
    void fade(uint8_t v){ fadeToBlackBy(data().data(), size(), v); }

    /**
     * @brief apply FastLED nscale8() func to buffer
     * i.e.dim whole buffer to black
     * @param v 
     */
    void dim(uint8_t v){ nscale8(data().data(), size(), v); }

    /**
     * @brief fill the buffer with solid color
     * 
     */
    void fill(COLOR_TYPE color);

    /**
     * @brief clear buffer to black
     * 
     */
    void clear();

    // stub pixel that is mapped to either nonexistent buffer access or blackholed CLedController mapping
    static COLOR_TYPE stub_pixel;
};

// static definition
template <class COLOR_TYPE> COLOR_TYPE PixelDataBuffer<COLOR_TYPE>::stub_pixel;

/**
 * @brief CledController Data Buffer - class with CRGB data storage (possibly) attached to CLEDController
 * and maintaining bound on move/copy/swap operations
 * 
 */
class CLedCDB : public PixelDataBuffer<CRGB> {
    /**
     * @brief a poor-man's mutex
     * points to the instance that owns global FastLED's buffer reference
     */
    CLEDController *cled = nullptr;

    /**
     * @brief if this buffer is bound to CLED controller,
     * than reset it's pointer to this buffer's data array
     * required to call on iterator invalidation or move semantics
     */
    void _reset_cled(){ if (cled) {cled->setLeds(fb.data(), fb.size());} };

public:
    // c-tor
    CLedCDB(size_t size) : PixelDataBuffer(size) {}

    /**
     * @brief Copy-assign a new Led FB object
     *  operator only copies data, it does NOT copy/move cled assignment (if any)
     * @param rhs 
     */
    //CLedCDB& operator=(CLedCDB const & rhs);

    // move semantics
    /**
     * @brief Move Construct a new CLedCDB object
     * constructor will steal a cled pointer from a rhs object
     * @param rhs - object to move from
     */
    CLedCDB(CLedCDB&& rhs) noexcept;

    /**
     * @brief Move assignment operator
     * will steal a cled pointer from a rhs object
     * @param rhs 
     * @return CLedCDB& 
     */
    CLedCDB& operator=(CLedCDB&& rhs);

    // d-tor
    ~CLedCDB();

    /**
     * @brief zero-copy swap CRGB data within two framebuffers
     * config struct is also swapped between object instances
     * local CLED binding is updated
     * @param rhs 
     */
    void swap(PixelDataBuffer& rhs) override;

    /**
     * @brief zero-copy swap CRGB data within two framebuffers
     * config struct is also swapped between object instances
     * CLED binding, if any, is updated to point to a newly swapped data both in local and rhs objects
     * @param rhs 
     */
    void swap(CLedCDB& rhs);

    /**
     * @brief bind this framebuffer to a CLEDController instance
     * 
     * @param pLed instanle of the CLEDController
     * @return true if bind success
     * @return false if this instance is already bound to CLEDController or parameter pointer is null
     */
    bool bind(CLEDController *pLed);

    /**
     * @brief swap bindings to CLED controller(s) with another PixelDataBuffer instance
     * if only one of the istances has a bound, then another instance steals it
     * 
     * @param rhs other instance that must have active CLED binding
     */
    void rebind(CLedCDB &rhs);

    /**
     * @brief returns 'true' if buffer is currently bound to CLEDController instance
     */
    bool isBound() const { return cled; }

    /**
     * @brief resize LED buffer to specified size
     * content will be lost on resize
     * 
     * @param s new number of pixels
     */
    bool resize(size_t s) override;
};



// coordinate transformation callback prototype
using transpose_t = std::function<size_t(unsigned w, unsigned h, unsigned x, unsigned y)>;

// a default (x,y) 2D mapper to 1-d vector index
static size_t map_2d(unsigned w, unsigned h, unsigned x, unsigned y) { return y*w+x; };


/**
 * @brief basic 2D buffer
 * provides generic abstraction for 2D topology
 * remaps x,y coordinates to linear pixel vector
 * basic class maps a simple row by row 2D buffer
 */
template <class COLOR_TYPE = CRGB>
class LedFB {

protected:
    // buffer width, height
    uint16_t _w, _h;
    // pixel buffer storage
    std::shared_ptr<PixelDataBuffer<COLOR_TYPE>> buffer;
    // coordinate to buffer index mapper callback
    transpose_t _xymap;

public:
    // c-tor
    /**
     * @brief Construct a new LedFB object
     * will spawn a new data buffer with requested dimensions
     * @param w - width
     * @param h - heigh
     */
    LedFB(uint16_t w, uint16_t h);

    /**
     * @brief Construct a new LedFB object
     * will spawn a new data buffer with requested dimensions
     * @param w - width
     * @param h - heigh
     * @param fb - preallocated buffer storage
     */
    LedFB(uint16_t w, uint16_t h, std::shared_ptr<PixelDataBuffer<COLOR_TYPE>> fb);

    /**
     * @brief Copy Construct a new LedFB object
     * A new instance will inherit a SHARED underlying data buffer member
     * @param rhs source object
     */
    LedFB(LedFB const & rhs);

    virtual ~LedFB() = default;

    /**
     * @brief Copy-assign not implemented (yet)
     * @param rhs 
     */
    LedFB& operator=(LedFB const & rhs) = delete;

    // DIMENSIONS

    // get configured matrix width
    uint16_t w() const {return _w;}
    // get configured matrix height
    uint16_t h() const {return _h;}

    // get size in pixels
    uint16_t size() const {return w()*h();}

    // return length of the longest side
    virtual uint16_t maxDim() const { return _w>_h ? _w : _h; }
    // return length of the shortest side
    virtual uint16_t minDim() const { return _w<_h ? _w : _h; }

    virtual uint16_t maxHeightIndex() const { return h()-1; }
    virtual uint16_t maxWidthIndex()  const { return w()-1; }

    // Topology transformation

    /**
     * @brief Set topology Remap Function
     * it will remap (x,y) coordinate into underlaying buffer vector index
     * could be used for various layouts of led canvases, tiles, stripes, etc...
     * affect access methods like at(), drawPixel(), etc...
     * 
     * @param mapper 
     * @return * assign 
     */
    void setRemapFunction(transpose_t mapper){ _xymap = mapper; };

    /**
     * @brief Transpose pixel 2D coordinates (x,y) into framebuffer's array index
     * 0's index is at top-left corner, X axis goes to the 'right', Y axis goes 'down'
     * it calculates array index based on matrix orientation and configuration
     * no checking performed for supplied coordinates to be out of bound of pixel buffer!
     * for signed negative arguments the behaviour is undefined
     * @param x 
     * @param y 
     * @return size_t 
     */
    //size_t transpose(unsigned x, unsigned y) const { return _xymap(_w, _h, x, y); }


    // DATA BUFFER OPERATIONS

    /**
     * @brief resize member data buffer to the specified size
     * Note: data buffer might NOT support resize operation, in this case resize does() nothing
     * @param w - new width
     * @param h - new height
     */
    virtual bool resize(uint16_t w, uint16_t h);

    //virtual void switchBuffer(std::shared_ptr<PixelDataBuffer> buff);

    // Pixel data access

    /**
     * @brief access pixel at coordinates x:y
     * if oob coordinates supplied returns blackhole element
     * @param x coordinate starting from top left corner
     * @param y coordinate starting from top left corner
     */
    COLOR_TYPE& at(int16_t x, int16_t y);

    /**
     * @brief access pixel at index
     * 
     * @param idx 
     * @return CRGB& 
     */
    COLOR_TYPE& at(size_t idx){ return buffer->at(idx); };

    // mimic Adafruit's low-level methods
    //virtual void drawPixel(int16_t x, int16_t y, uint16_t color){ at(x, y) = color16toCRGB(color); }
    //virtual void drawPixel(int16_t x, int16_t y, CRGB color){ at(x, y) = color; }

    /*
        iterators
        TODO: need proper declaration for this
    */
    typename std::vector<COLOR_TYPE>::iterator begin(){ return buffer->begin(); };
    typename std::vector<COLOR_TYPE>::iterator end(){ return buffer->end(); };


    // FastLED buffer-wide color functions (here just a wrappers, but could be overriden in derived classes)

    /**
     * @brief apply FastLED fadeToBlackBy() func to buffer
     * 
     * @param v 
     */
    virtual void fade(uint8_t v){ buffer->fade(v); }

    /**
     * @brief apply FastLED nscale8() func to buffer
     * i.e.dim whole buffer to black
     * @param v 
     */
    void dim(uint8_t v){ buffer->dim(v); }

    /**
     * @brief fill the buffer with solid color
     * 
     */
    void fill(COLOR_TYPE color){ buffer->fill(color); };

    /**
     * @brief clear buffer to black
     * 
     */
    void clear(){ buffer->clear(); };

};

/**
 * @brief implementation a rectangular canvas made from a single piece of a LED Stripe
 * with possible orientation and transformations:
 * - V/H mirroring
 * - snake/zigzag rows
 * - transpose rows vs columns, i.e. stripe goes horisontaly (default) or vertically
 * 
 */
class LedStripe {
protected:
    bool _snake;             // matrix type 1:snake( zigzag), 0:parallel
    bool _vertical;          // strip direction: 0 - horizontal, 1 - vertical
    bool _vmirror;           // vertical flip
    bool _hmirror;           // horizontal flip

public:
    /**
     * @brief Construct a new Led Stripe object
     * 
     * @param w - width
     * @param h - height
     * @param snake - snake or zigzag
     * @param _vertical - transpose rows vs cols
     * @param vm - vertical mirroring
     * @param hm - horizontal mirroring
     */
    LedStripe(bool snake = true, bool _vertical = false, bool vm=false, bool hm=false) : _snake(snake), _vertical(_vertical), _vmirror(vm), _hmirror(hm) {};
    virtual ~LedStripe() = default;

    // getters
    bool snake()   const {return _snake;}
    bool vertical()const {return _vertical;}
    bool vmirror() const {return _vmirror;}
    bool hmirror() const {return _hmirror;}

    // setters
    void snake(bool m) {_snake=m;}
    void vertical(bool m) {_vertical=m;}
    void vmirror(bool m){_vmirror=m;}
    void hmirror(bool m){_hmirror=m;}

    /**
     * @brief Transpose pixel 2D coordinates (x,y) into framebuffer's array index
     * 0's index is at top-left corner, X axis goes to the 'right', Y axis goes 'down'
     * it calculates array index based on matrix orientation and configuration
     * no checking performed for supplied coordinates to be out of bound of pixel buffer!
     * for signed negative arguments the behaviour is undefined
     * @param x 
     * @param y 
     * @return size_t 
     */
    virtual size_t transpose(unsigned w, unsigned h, unsigned x, unsigned y) const;
};


/**
 * @brief abstract overlay engine
 * it works as a renderer for canvas, creating/mixing overlay/back buffer with canvas
 */
class OverlayEngine {

public:
    // virtual d-tor
    virtual ~OverlayEngine(){};

    /**
     * @brief Get a reference to canvas buffer
     * 
     * @return PixelDataBuffer* 
     */
    virtual std::shared_ptr<PixelDataBuffer<CRGB>> getCanvas() = 0;

    virtual std::shared_ptr<PixelDataBuffer<CRGB>> getOverlay() = 0;

    /**
     * @brief protect canvs buffer from altering by overlay mixing
     * i.e. canvas buffer is used as a persistent storage
     * in that case Engine will try to use back buffer for overlay mixing (if implemented) 
     * @param v - true of false
     */
    void canvasProtect(bool v){ _canvas_protect = v; };

    /**
     * @brief show buffer content on display
     * it will draw a canvas content (or render an overlay over it if necessary)
     * 
     */
    virtual void show(){};

    /**
     * @brief wipe buffers and draw a blank screen
     * 
     */
    virtual void clear();

protected:
    CRGB _transparent_color = CRGB::Black;

    /**
     * @brief flag that marks canvas buffer as persistent that should NOT be changed
     * by overlay mixing operations. In that case overlay is applied on top of a canvas copy in back buffer (if possible)
     * 
     */
    bool _canvas_protect = false;
};


/**
 * @brief Overlay engine based on ESP32 RMT backend for ws2818 LED strips  
 * 
 * @tparam RGB_ORDER 
 */
template<EOrder RGB_ORDER = RGB>
class ESP32RMTOverlayEngine : public OverlayEngine {

    std::shared_ptr<CLedCDB>  canvas;      // canvas buffer where background data is stored
    std::unique_ptr<CLedCDB>  backbuff;    // back buffer, where we will mix data with overlay before sending to LEDs
    std::weak_ptr<CLedCDB>    overlay;     // overlay buffer weak pointer

    // FastLED controller
    CLEDController *cled = nullptr;
    // led strip driver
    ESP32RMT_WS2812B<RGB_ORDER> *wsstrip;

public:
    /**
     * @brief Construct a new Overlay Engine object
     * 
     * @param gpio - gpio to bind ESP32 RMT engine
     */
    ESP32RMTOverlayEngine(int gpio);

    /**
     * @brief Construct a new Overlay Engine object
     * 
     * @param gpio - gpio to bind
     * @param buffsize - CLED buffer object
     */
    ESP32RMTOverlayEngine(int gpio, std::shared_ptr<CLedCDB> buffer);

    /**
     * @brief Construct a new Overlay Engine object
     * 
     * @param gpio - gpio to bind
     * @param buffsize - desired LED buffer size
     */
    ESP32RMTOverlayEngine(int gpio, size_t buffsize) : ESP32RMTOverlayEngine<RGB_ORDER>(gpio, std::make_shared<CLedCDB>(buffsize)) {};

    /**
     * @brief take a buffer pointer and attach it to ESP32 RMT engine
     * 
     * @param fb - a pointer to CLedC buffer object
     * @return true - on success
     * @return false - if canvas has been attached attached already
     */
    bool attachCanvas(std::shared_ptr<CLedCDB> &fb);

    std::shared_ptr<PixelDataBuffer<CRGB>> getCanvas() override { return canvas; }

    /**
     * @brief Get a pointer to Overlay buffer
     * Note: consumer MUST release a pointer once overlay operations is no longer needed
     * 
     * @return std::shared_ptr<PixelDataBuffer<CRGB>> 
     */
    std::shared_ptr<PixelDataBuffer<CRGB>> getOverlay() override;

    /**
     * @brief show buffer content on display
     * it will draw a canvas content (or render an overlay over it if necessary)
     * 
     */
    void show() override;

    /**
     * @brief wipe buffers and draw a blank screen
     * 
     */
    void clear();

private:
    /**
     * @brief apply overlay to canvas
     * pixels with _transparent_color will be skipped
     */
    void _ovr_overlap();

    /**
     * @brief switch active output to a back buffe
     * used in case if canvas is marked as persistent and should not be changed with overlay data
     * 
     */
    void _switch_to_bb();
};


// overload pattern and deduction guide. Lambdas provide call operator
template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

/**
 * @brief GFX class for LedFB
 * it provides Adafruit API for uderlaying buffer with either CRGB and uint16 color container
 * 
 */
class LedFB_GFX : public Adafruit_GFX {

    void _drawPixelCRGB( LedFB<CRGB> *b, int16_t x, int16_t y, CRGB c){ b->at(x,y) = c; };
    void _drawPixelCRGB( LedFB<uint16_t> *b, int16_t x, int16_t y, CRGB c){ b->at(x,y) = colorCRGBto16(c); };

    void _drawPixelC16( LedFB<CRGB> *b, int16_t x, int16_t y, uint16_t c){ b->at(x,y) = color16toCRGB(c); };
    void _drawPixelC16( LedFB<uint16_t> *b, int16_t x, int16_t y, uint16_t c){ b->at(x,y) = c; };

    void _fillScreenCRGB(LedFB<CRGB> *b, CRGB c){ b->fill(c); };
    void _fillScreenCRGB(LedFB<uint16_t> *b, CRGB c){ b->fill(colorCRGBto16(c)); };

    void _fillScreenC16(LedFB<CRGB> *b, uint16_t c){ b->fill(color16toCRGB(c)); };
    void _fillScreenC16(LedFB<uint16_t> *b, uint16_t c){ b->fill(c); };

/*
    template<typename V, typename X, typename Y, typename C>
    void _visit_drawPixelCRGB(const V& variant, X x, Y y, C color){
        std::visit( Overload{ [&x, &y, &color](const auto& variant_item) { _drawPixelCRGB(variant_item, x, y, color); }, }, variant);
    };
*/

protected:
    // LedFB container variant
    std::variant< std::shared_ptr< LedFB<CRGB> >, std::shared_ptr< LedFB<uint16_t> >  > _fb;

public:
    /**
     * @brief Construct a new LedFB_GFX object from a LedFB<CRGB> 24 bit color
     * 
     * @param buff - a shared pointer to the LedFB object
     */
    LedFB_GFX(std::shared_ptr< LedFB<CRGB> > buff) : Adafruit_GFX(buff->w(), buff->h()), _fb(buff) {}

    /**
     * @brief Construct a new LedFB_GFX object from a LedFB<uint16_t> 16 bit color
     * 
     * @param buff - a shared pointer to the LedFB object
     */
    LedFB_GFX(std::shared_ptr< LedFB<uint16_t> > buff) : Adafruit_GFX(buff->w(), buff->h()), _fb(buff) {}

    virtual ~LedFB_GFX() = default;

    // Adafruit overrides
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void fillScreen(uint16_t color) override;

    // Adafruit-like methods for CRGB
    void drawPixel(int16_t x, int16_t y, CRGB color);
    void fillScreen(CRGB color);

    // Color conversion
    static CRGB color16toCRGB(uint16_t c){ return CRGB(c>>11 & 0xf8, c>>5 & 0xfc, c<<3); }
    static uint16_t colorCRGBto16(CRGB c){ return c.r >> 3 << 11 | c.g >> 2 << 5 | c.b >> 3; }


    // Additional methods
};








//  *** TEMPLATES IMPLEMENTATION FOLLOWS *** //

// copy via assignment
template <class COLOR_TYPE>
PixelDataBuffer<COLOR_TYPE>& PixelDataBuffer<COLOR_TYPE>::operator=(PixelDataBuffer<COLOR_TYPE> const& rhs){
    fb = rhs.fb;
    return *this;
}

// move assignment
template <class COLOR_TYPE>
PixelDataBuffer<COLOR_TYPE>& PixelDataBuffer<COLOR_TYPE>::operator=(PixelDataBuffer<COLOR_TYPE>&& rhs){
    fb = std::move(rhs.fb);
    return *this;
}

template <class COLOR_TYPE>
COLOR_TYPE& PixelDataBuffer<COLOR_TYPE>::at(size_t i){ return i < fb.size() ? fb.at(i) : stub_pixel; };      // blackhole is only of type CRGB, need some other specialisations

template <class COLOR_TYPE>
void PixelDataBuffer<COLOR_TYPE>::fill(COLOR_TYPE color){ fb.assign(fb.size(), color); };

template <class COLOR_TYPE>
void PixelDataBuffer<COLOR_TYPE>::clear(){ fill(COLOR_TYPE()); };

template <class COLOR_TYPE>
bool PixelDataBuffer<COLOR_TYPE>::resize(size_t s){
    fb.reserve(s);
    clear();
    return fb.size() == s;
};

template<EOrder RGB_ORDER>
ESP32RMTOverlayEngine<RGB_ORDER>::ESP32RMTOverlayEngine(int gpio){
    wsstrip = new(std::nothrow) ESP32RMT_WS2812B<RGB_ORDER>(gpio);
}

template<EOrder RGB_ORDER>
ESP32RMTOverlayEngine<RGB_ORDER>::ESP32RMTOverlayEngine(int gpio, std::shared_ptr<CLedCDB> buffer) : canvas(buffer) {
    wsstrip = new(std::nothrow) ESP32RMT_WS2812B<RGB_ORDER>(gpio);
    if (wsstrip && canvas){
        // attach buffer to RMT engine
        cled = &FastLED.addLeds(wsstrip, canvas->data(), canvas->size());
        // hook framebuffer to contoller
        canvas->bind(cled);
        show();
    }
};

template<EOrder RGB_ORDER>
bool ESP32RMTOverlayEngine<RGB_ORDER>::attachCanvas(std::shared_ptr<CLedCDB> &fb){
    if (cled) return false; // this function is not idempotent, so refuse to mess with existing controller

    // share data buffer instance
    canvas = fb;

    if (wsstrip && canvas){
        // attach buffer to RMT engine
        cled = &FastLED.addLeds(wsstrip, canvas->data().data(), canvas->size());
        // hook framebuffer to contoller
        canvas->bind(cled);
        show();
        return true;
    }

    return false;   // somethign went either wrong or already been setup 
}

template<EOrder RGB_ORDER>
void ESP32RMTOverlayEngine<RGB_ORDER>::show(){
    if (!overlay.expired() && _canvas_protect && !backbuff)    // check if I need to switch to back buff due to canvas persistency and overlay data present 
        _switch_to_bb();

    // check if back-buffer is present but no longer needed (if no overlay present or canvas is not persistent anymore)
    if (backbuff && (overlay.expired() || !_canvas_protect)){
        //LOG(println, "BB release");
        canvas->rebind(*backbuff.get());
        backbuff.release();
    }

    if (!overlay.expired()) _ovr_overlap(); // apply overlay to either canvas or back buffer, if bb is present
    FastLED.show();
};

template<EOrder RGB_ORDER>
void ESP32RMTOverlayEngine<RGB_ORDER>::clear(){
    if (!canvas) return;
    canvas->clear();
    if (backbuff){
        // release BackBuffer, it will be recreated if required
        canvas->rebind(*backbuff.get());
        backbuff.release();
    }
    auto ovr = overlay.lock();
    if (ovr) ovr->clear();          // clear overlay
    FastLED.show();
}

template<EOrder RGB_ORDER>
std::shared_ptr<PixelDataBuffer<CRGB>> ESP32RMTOverlayEngine<RGB_ORDER>::getOverlay(){
    auto p = overlay.lock();
    if (!p){
        // no overlay exist at the moment
        p = std::make_shared<CLedCDB>(canvas->size());
        overlay = p;
    }
    return p;
}

template<EOrder RGB_ORDER>
void ESP32RMTOverlayEngine<RGB_ORDER>::_ovr_overlap(){
    auto ovr = overlay.lock();
    if (canvas->size() != ovr->size()) return;  // a safe-check for buffer sizes

    auto ovr_iterator = ovr->begin();

    if (backbuff.get()){
        auto bb_iterator = backbuff->begin();
        // fill BackBuffer with either canvas or overlay based on keycolor
        for (auto i = canvas->begin(); i != canvas->end(); ++i ){
            *bb_iterator = (*ovr_iterator == _transparent_color) ? *i : *ovr_iterator;
            ++ovr_iterator;
            ++bb_iterator;
        }
    } else {
        // apply all non key-color pixels to canvas
        for (auto i = canvas->begin(); i != canvas->end(); ++i ){
            if (*ovr_iterator != _transparent_color)
                *i = *ovr_iterator;
            ++ovr_iterator;
        }
    }   
}

template<EOrder RGB_ORDER>
void ESP32RMTOverlayEngine<RGB_ORDER>::_switch_to_bb(){
    //LOG(println, "Switch to BB");
    backbuff = std::make_unique<CLedCDB>(canvas->size());
    backbuff->rebind(*canvas);    // switch backend binding
}



template <class COLOR_TYPE>
LedFB<COLOR_TYPE>::LedFB(uint16_t w, uint16_t h) : _w(w), _h(h), _xymap(map_2d) {
    buffer = std::make_shared<PixelDataBuffer<COLOR_TYPE>>(PixelDataBuffer<COLOR_TYPE>(w*h));
}

template <class COLOR_TYPE>
LedFB<COLOR_TYPE>::LedFB(uint16_t w, uint16_t h, std::shared_ptr<PixelDataBuffer<COLOR_TYPE>> fb): _w(w), _h(h), buffer(fb), _xymap(map_2d) {
    // a safety check if supplied buffer and dimentions are matching
    if (buffer->size() != w*h)   buffer->resize(w*h);
};

template <class COLOR_TYPE>
LedFB<COLOR_TYPE>::LedFB(LedFB<COLOR_TYPE> const & rhs) : _w(rhs._w), _h(rhs._h), _xymap(map_2d) {
    buffer = rhs.buffer;
    // deep copy
    //buffer = std::make_shared<PixelDataBuffer>(*rhs.buffer.get());
}

template <class COLOR_TYPE>
COLOR_TYPE& LedFB<COLOR_TYPE>::at(int16_t x, int16_t y){
    return ( buffer->at(_xymap(_w, _h, static_cast<uint16_t>(x), static_cast<uint16_t>(y))) );
};

template <class COLOR_TYPE>
bool LedFB<COLOR_TYPE>::resize(uint16_t w, uint16_t h){
    // safety check
    if (buffer->resize(w*h) && buffer->size() == w*h){
        _w=w; _h=h;
        return true;
    }
    return false;
}

