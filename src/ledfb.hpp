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
#include "w2812-rmt.hpp"
#include <vector>
#include <memory>


/**
 * @brief Base class with CRGB data storage that acts as a pixel buffer storage
 * it provides basic operations with pixel data with no any backend engine to display data
 * 
 */
class PixelDataBuffer {

protected:
    std::vector<CRGB> fb;     // main frame buffer

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
    PixelDataBuffer& operator=(PixelDataBuffer const & rhs);

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
    std::vector<CRGB> &data(){ return fb; }


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
    CRGB& at(size_t i);

    // access pixel at coordinates x:y (obsolete)
    //CRGB& pixel(unsigned x, unsigned y){ return at(x,y); };

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
    void fill(CRGB color);

    /**
     * @brief clear buffer to black
     * 
     */
    void clear();

};

/**
 * @brief CledController Data Buffer - class with CRGB data storage (possibly) attached to CLEDController
 * and maintaining bound on move/copy/swap operations
 * 
 */
class CLedCDB : public PixelDataBuffer {
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


/**
 * @brief basic 2D buffer
 * provides generic abstraction for 2D topology
 * remaps x,y coordinates to linear pixel vector
 * basic class maps a simple row by row 2D buffer
 */
class LedFB {

protected:
    // buffer width, height
    uint16_t _w, _h;
    // pixel buffer storage
    std::shared_ptr<PixelDataBuffer> buffer;

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
    LedFB(uint16_t w, uint16_t h, std::shared_ptr<PixelDataBuffer> fb);

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
    virtual uint16_t w() const {return _w;}
    // get configured matrix height
    virtual uint16_t h() const {return _h;}

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
     * @brief Transpose pixel 2D coordinates (x,y) into framebuffer's array index
     * 0's index is at top-left corner, X axis goes to the 'right', Y axis goes 'down'
     * it calculates array index based on matrix orientation and configuration
     * no checking performed for supplied coordinates to be out of bound of pixel buffer!
     * for signed negative arguments the behaviour is undefined
     * @param x 
     * @param y 
     * @return size_t 
     */
    virtual size_t transpose(unsigned x, unsigned y) const { return y*_w+x; }


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
    CRGB& at(int16_t x, int16_t y);

    /**
     * @brief access pixel at index
     * 
     * @param idx 
     * @return CRGB& 
     */
    CRGB& at(size_t idx){ return buffer->at(idx); };

    /*
        iterators
        TODO: need proper declaration for this
    */
    inline std::vector<CRGB>::iterator begin(){ return buffer->begin(); };
    inline std::vector<CRGB>::iterator end(){ return buffer->end(); };


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
    void fill(CRGB color){ buffer->fill(color); };

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
class LedStripe : public LedFB {
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
    LedStripe(uint16_t w, uint16_t h, bool snake = true, bool _vertical = false, bool vm=false, bool hm=false) : LedFB(w, h), _snake(snake), _vertical(_vertical), _vmirror(vm), _hmirror(hm) {};
    LedStripe(LedStripe const & rhs) :  LedFB(rhs._w, rhs._h), _snake(rhs._snake), _vertical(rhs._vertical), _vmirror(rhs._vmirror), _hmirror(rhs._hmirror) {};
    LedStripe(LedFB const & rhs) :  LedFB(rhs.w(), rhs.h()), _snake(false), _vertical(false), _vmirror(false), _hmirror(false) {};
    LedStripe(uint16_t w, uint16_t h, std::shared_ptr<PixelDataBuffer> fb) : LedFB(w, h, fb), _snake(false), _vertical(false), _vmirror(false), _hmirror(false) {};

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
    virtual size_t transpose(unsigned x, unsigned y) const override;
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
    virtual std::shared_ptr<PixelDataBuffer> getCanvas() = 0;

    virtual std::shared_ptr<PixelDataBuffer> getOverlay() = 0;

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

    std::shared_ptr<PixelDataBuffer> getCanvas() override { return canvas; }

    /**
     * @brief Get a pointer to Overlay buffer
     * Note: consumer MUST release a pointer once overlay operations is no longer needed
     * 
     * @return std::shared_ptr<PixelDataBuffer> 
     */
    std::shared_ptr<PixelDataBuffer> getOverlay() override;

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


//  *** TEMPLATE IMPLEMENTATION FOLLOWS *** //

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
std::shared_ptr<PixelDataBuffer> ESP32RMTOverlayEngine<RGB_ORDER>::getOverlay(){
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