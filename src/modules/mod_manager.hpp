/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (vortigont)

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
    Informer that draws data to display overlay
*/

#pragma once

#include "display.hpp"
#include "ts.h"
#include "ui.h"
#include "evtloop.h"
#include "char_const.h"
#include "canvas/Arduino_Canvas_Mono.h"
#include <mutex>

#define DEFAULT_TEXT_COLOR  54000
#define MAX_NUM_OF_PROFILES 10

/**
 * @brief an abstract class to implement dynamically loaded components or modules
 * practically it is just a class that is able to load/save it's state serialized,
 * has a periodic timer ticker and could attach/detach to event bus
 * 
 */
class GenericModule : public Task {

protected:
  // module's access mutex
  std::mutex mtx;

	// module label or "name"
	const char* label;

	/**
	 * @brief derived method should generate object's configuration into provided JsonVariant
	 * 
	 * @param cfg 
	 * @return JsonVariantConst 
	 */
	virtual void generate_cfg(JsonVariant cfg) const = 0;

	/**
	 * @brief load configuration from a json object
	 * method should be implemented in derived class to process
	 * class specific json object
	 * @param cfg 
	 */
	virtual void load_cfg(JsonVariantConst cfg) = 0;

public:

    /**
     * @brief Construct a new Generic Module object
     * 
     * @param label - module label identifier
     * @param interval - ticker execution interval in ms
     */
    GenericModule(const char* label, unsigned interval);
    virtual ~GenericModule(){};

    // function to run on ticker call 
    virtual void moduleRunner() = 0;

    /**
     * @brief load module's config from persistent storage and calls start()
     * 
     */
    virtual void load();

    /**
     * @brief save current module's configuration to file
     * 
     */
    virtual void save();

    // start module ticker
    virtual void start(){ enable(); };

    // stop module ticker
    virtual void stop(){ disable(); };

    /**
     * @brief Get module's configuration packed into a nested json object ['module_label']
     * used to feed control's values to WebUI/MQTT
     */
    void getConfig(JsonObject obj) const;

    /**
     * @brief Set module's configuration packed into json object
     * this call will also SAVE supplied configuration to persistent storage
     */
    void setConfig(JsonVariantConst cfg);

    /**
     * @brief Get module's Label
     * 
     * @return const char* 
     */
    const char* getLabel() const { return label; }

    // Configuration profiles handling

    /**
     * @brief switch to specific profile number
     * 
     * @param value 
     * @return true on success
     * @return false if profile does not exist
     */
    virtual void switchProfile(int32_t value){};

    /**
     * @brief Get the Current Profile Num value
     * 
     * @return uint32_t 
     */
    virtual uint32_t getCurrentProfileNum() const { return 0; };

    /**
     * @brief returns number of available slots for stored profiles
     * 
     * @return uint32_t number of slots, if 0 is returned then profiles are not supported
     */
    virtual uint32_t profilesAvailable() const { return 0; }
};

using module_pt = std::unique_ptr<GenericModule>;

/*
class GenericGFXModule : public GenericModule {


protected:
    std::unique_ptr<LedFB_GFX> canvas;
    LedFB_GFX   *screen = nullptr;
    // буфер оверлея
    //std::shared_ptr<LedFB<CRGB> > overlay;

    // obtain canvas pointer
    bool getCanvas();

    // make/release display overlay
    //bool getOverlay();

    //void releaseOverlay();

public:
    GenericGFXModule(const char* wlabel, unsigned interval) : GenericModule(wlabel, interval){};
    virtual ~GenericGFXModule(){ };

};
*/

/**
 * @brief Module with a set of configuration profiles that could be 
 * stored and switched on-demand
 * 
 */
class GenericModuleProfiles : public GenericModule {

    int32_t _profilenum{0};

    String _mkFileName();

    void _load_profile(int idx);

public:
    GenericModuleProfiles(const char* wlabel, unsigned interval) : GenericModule(wlabel, interval){}

    /**
     * @brief load module's config from persistent storage and calls start()
     * here it loads last used profile
     */
    void load() override final { switchProfile(-1); };

    void save() override final;

    /**
     * @brief switch to specific profile number
     * loads profile config from file, if specified argument is <0 or wrong, loads last used profile
     * @param value 
     */
    void switchProfile(int32_t value) final;

    /**
     * @brief Get the Current Profile Num value
     * 
     * @return uint32_t 
     */
    uint32_t getCurrentProfileNum() const final { return _profilenum; };

    /**
     * @brief returns number of available slots for stored profiles
     * 
     * @return uint32_t number of slots, if 0 is returned then profiles are not supported
     */
    uint32_t profilesAvailable() const override { return MAX_NUM_OF_PROFILES; }

};

/**
 * @brief configuration for text bitmap block
 * 
 */
struct TextBitMapCfg {
    int16_t x, y;           // top left corner to place bitmap to
    uint16_t w,  h;         // bitmap WxH
    uint16_t color{DEFAULT_TEXT_COLOR};     // color in 5-6-5 mode
    uint8_t font_index;     // font to use
    int8_t baseline_shift_x;       // offset from left side of bitmap
    int8_t baseline_shift_y;       // ofset from bottom of bitmap
    // max text bounds - needed to track max block size to cover the clock text
    uint8_t alpha_bg;
    std::string datefmt{"%F"};
    overlay_cb_t cb{};
};


class ClockModule : public GenericModuleProfiles {

enum class ovrmixer_t {
    bgfade,
    alphablend,
    color_scale
};

struct Clock {
    int16_t x, y;                       // top left corner to place bitmap to
    uint16_t w, h;                      // bitmap WxH
    ovrmixer_t mixer;
    uint8_t baseline_shift_x, baseline_shift_y;
//    int16_t baseline
    uint16_t color_txt, color_bg;       // color in 5-6-5 mode
    uint8_t alpha_tx, alpha_bg;         // transparency
    uint8_t font_index;                 // font to use for hr:min
    uint8_t seconds_font_index;         // font to use for seconds
    bool show_seconds;                  // show seconds
    bool twelwehr;                      // 12/24 hour clock
    int16_t eff_num;                    // switch to effect number 'num'
    // max text bounds - needed to track max block size to cover the clock text
    overlay_cb_t cb{};
};

    // elements structs
    Clock clk{};
    TextBitMapCfg date{};
    bool date_show;
    // last timestamp
    std::time_t last_date;
    // flag that indicates screen needs a refresh
    bool redraw;
    // text mask buffer
    std::unique_ptr<Arduino_Canvas_Mono> _textmask_clk;
    std::unique_ptr<Arduino_Canvas_Mono> _textmask_date;

    esp_event_handler_instance_t _hdlr_lmp_change_evt = nullptr;
    esp_event_handler_instance_t _hdlr_lmp_state_evt = nullptr;

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) const override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    // print clock
    void _print_clock(std::tm *tm);

    // print date
    void _print_date(std::tm *tm);

    // cockoo/talking clock
    void _cockoo_events(std::tm *tm);

    static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

    // change events handler
    void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

    // set events handler
    //void _lmpSetEventHandler(esp_event_base_t base, int32_t id, void* data);

public:
    ClockModule();
    ~ClockModule();

    void moduleRunner() override;

    void start() override;
    void stop() override;
};

/**
 * @brief a container object start spawns/destroys Modules
 * on start/demand
 * 
 */
class ModuleManager {

    // module instances container
    std::list<module_pt> _modules;

    /**
     * @brief spawn a new instance of a module with supplied config
     * used with configuration is suplied via webui for non existing modules
     * @param label 
     * @param cfg whether to use a supplied configuration or load from NVS
     * @param persistent if 'true' a spawned module will save supplied configuration to NVS, set this to false if spawning module with cfg FROM NVS to avoid exta writes
     */
    void _spawn(const char* label);

public:
    //ModuleManager();
    //~ModuleManager(){};

    /**
     * @brief start module
     * if label is not given, then start all modules based on settings from NVRAM
     * 
     * @param label 
     */
    void start(const char* label = NULL);

    /**
     * @brief Stop specific module if it's instance is exist
     * 
     * @param label 
     */
    void stop(const char* label);

    //void register_handlers();
    //void unregister_handlers();

    /**
     * @brief load module's configuration into provided JsonObject
     * usually called from a WebUI/MQTT handler
     * @param obj 
     * @param label 
     */
    void getConfig(JsonObject obj, const char* label);

    /**
     * @brief Set the Configuration for specifit module object
     * 
     * @param label 
     * @param cfg 
     */
    void setConfig(const char* label, JsonVariantConst cfg);

    /**
     * @brief generate Interface values object representing boolen states
     * of currently active/inactive modules
     * 
     * @param interf 
     */
    void getModulesStatuses(Interface *interf) const;

    /**
     * @brief Get state of the specific Module active/inactive
     * 
     * @param label module's label
     * @return true
     * @return false 
     */
    bool getModuleStatus(const char* label) const;

    /**
     * @brief Get pointer to the instance of an active Module by it's label
     * returns nullptr if requested module is not currently runnning
     * @note a care should be taken when module pointer is used outside of manager object,
     * currently there is no exclusive locking performed and module instance could deleted any time via other call
     * 
     * 
     * @param[in] label 
     * @return GenericModule* 
     */
    GenericModule* getModulePtr(const char* label);

    /**
     * @brief switch module's configuration profile
     * 
     * @param label 
     * @param idx 
     */
    void switchProfile(const char* label, int32_t idx);
};


// Unary predicate for Module's label search match
template <class T>
class MatchLabel : public std::unary_function<T, bool>{
    std::string_view _lookup;
public:
    explicit MatchLabel(const char* label) : _lookup(label) {}
    bool operator() (const T& item ){
        // T is module_pt
        return _lookup.compare(item->getLabel()) == 0;
    }
};

class AlarmClock : public GenericModule {
    // типы будильника
    enum class alarm_t {
        onetime = 0,
        daily,
        workdays,
        weekends
    };

    // параметры будильника
    struct AlarmCfg {
        bool active;
        alarm_t type;
        // trigger time
        uint8_t hr{0}, min{0};
        // track name to play for alarm
        int track;
        // sunrise
        bool rise_on;       // enable light rise
        int sunrise_offset, sunrise_startBr, sunrise_endBr;
        uint32_t sunrise_duration, sunrise_eff;
        // sunset
        bool dusk_on;       // enable light dusk
        bool dusk_pwroff;   // shutdown lamp on dust fade end
        int dusk_startBr, dusk_endBr;
        uint32_t dusk_duration, dusk_eff;
    };

    // кукушка
    struct Cuckoo {
        uint8_t hr{0}, hhr{0}, quater{0};
        // inactive hours
        uint8_t on, off;
    };

    // a set of alarms
    std::array<AlarmCfg, 4> _alarms{};

    // Cuckoo configuration
    Cuckoo _cuckoo{};

    // flag that alarm is await for dusk fade-end
    bool _fade_await{false};

    // cockoo/talking clock
    void _cockoo_events(std::tm *tm);

    // рассвет
    void _sunrise_check();

    // pack class configuration into JsonObject
    void generate_cfg(JsonVariant cfg) const override;

    // load class configuration into JsonObject
    void load_cfg(JsonVariantConst cfg) override;

    esp_event_handler_instance_t _hdlr_lmp_change_evt = nullptr;

    // change events handler
    void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

public:
    AlarmClock();
    ~AlarmClock();

    void moduleRunner() override;

    void setAlarmItem(JsonVariant cfg);

    //void start() override;
    //void stop() override;
};



class TextScrollerWgdt : public GenericModuleProfiles {

struct WeatherCfg {
  String apikey;
  uint32_t city_id, refresh; // ms
  bool retry{false};
};

  TextBitMapCfg _bitmapcfg;
  WeatherCfg _weathercfg;

  std::unique_ptr<Arduino_Canvas_Mono> _textmask;

  int _cur_offset{0};
  int _scrollrate;
  uint32_t _last_redraw;
  uint16_t _txt_pixlen;
  bool _wupd{false};

  overlay_cb_t _renderer;

  std::string _txtstr{"обновление погоды..."};

  static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override;

  // load class configuration into JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void _getOpenWeather();

  // hook to check/update text sroller
  void _scroll_line(LedFB_GFX *gfx);

public:
  TextScrollerWgdt();
  ~TextScrollerWgdt();

  void moduleRunner() override;

  void start() override;
  void stop() override;
};

/*
static uint8_t inline alphaBlend( uint8_t a, uint8_t b, uint8_t alpha ) { return scale8(a, 255-alpha) + scale8(b, alpha); }
static CRGB alphaBlend( CRGB a, CRGB b, uint8_t alpha){
    return CRGB( alphaBlend( a.r, b.r, alpha ), alphaBlend( a.g, b.g, alpha ), alphaBlend( a.b, b.b, alpha ) );
};
*/


extern ModuleManager informer;
