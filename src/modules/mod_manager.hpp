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

#pragma once

//#include "display.hpp"
#include "ts.h"
#include "ui.h"
#include "evtloop.h"
#include "char_const.h"
#include <mutex>

#define MAX_NUM_OF_PROFILES 10

static constexpr const char* T_ui_pages_module_prefix = "lampui.pages.module.";


/**
 * @brief an abstract class to implement dynamically loaded components or modules
 * practically it is just a class that is able to load/save it's state serialized,
 * has a periodic timer ticker and could attach/detach to event bus
 * 
 */
class GenericModule {

protected:
  // module's access mutex
  std::mutex mtx;

	// module label or "name", can't be changed once defined
	const char* const label;

	// if 'true' - then use shared module's config file for settings,
    // if 'false - or save in module's own config named as '${label}.json'
	const bool _def_config;

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

    /**
     * @brief generate configuration file's name
     * 
     * @return String 
     */
	String mkFileName();


public:

	/**
	 * @brief Construct a new Generic Module object
	 * 
	 * @param label - module label identifier
	 * @param interval - ticker execution interval in ms
	 */
	GenericModule(const char* label, bool default_cfg_file = true);
	virtual ~GenericModule(){};

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
	virtual void start() = 0;

	// stop module ticker
	virtual void stop() = 0;

	/**
	 * @brief Get module's configuration packed into a nested json object ['module_label']
	 * used to feed configuration values to WebUI/MQTT, calls virtual generate_cfg() under the hood
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
	 * @param value preset number to swich to
     * @param keepcurrent - if true, then do not load preset's setting, just change the index and keep current config
	 */
	virtual void switchPreset(int32_t value, bool keepcurrent = false){};

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

    /**
     * @brief set name for currently loaded preset
     * 
     * @param lbl 
     */
    virtual void setPresetLabel(const char* lbl){};

    /**
     * @brief Construct an EmbUI page with module's state/configuration
     * 
     * @param interf 
     * @param data 
     * @param action 
     */
    virtual void mkEmbUIpage(Interface *interf, JsonVariantConst data, const char* action);

    /**
     * @brief generate content frame for presets drop-down list
     * 
     * @param interf 
     */
    virtual void mkEmbUI_preset(Interface *interf){};

    /**
     * @brief fills provided array with a list of available config profiles
     * it generates data for drop-down selector list for EmbUI in a form of:
     * [ { "value":0, "label":"SomeProfile" } ]
     * 
     * @param arr Json array reference to fill-in
     * @return size_t number of elements in index
     */
    virtual size_t mkProfilesIndex(JsonArray arr) { return 0; };

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
    String _profilename;

    void _load_profile(int idx);

public:
    GenericModuleProfiles(const char* label) : GenericModule(label, false){}

    /**
     * @brief load module's config from persistent storage and calls start()
     * here it loads last used profile
     */
    void load() override final { switchPreset(-1); };

    void save() override final;

    /**
     * @brief switch to specific profile number
     * loads profile config from file, if specified argument is <0 or wrong, loads last used profile
     * @param value 
     */
    void switchPreset(int32_t idx, bool keepcurrent = false) override final;

    /**
     * @brief Get the Current Profile Num value
     * 
     * @return uint32_t 
     */
    uint32_t getCurrentProfileNum() const override final { return _profilenum; };

    /**
     * @brief returns number of available slots for stored profiles
     * 
     * @return uint32_t number of slots, if 0 is returned then profiles are not supported
     */
    uint32_t profilesAvailable() const override { return MAX_NUM_OF_PROFILES; }

    /**
     * @copydoc GenericModule::setPresetLabel(const char* lbl)
     */
    void setPresetLabel(const char* lbl) override { if (lbl) _profilename = lbl; };

    /**
     * @brief Construct an EmbUI page with module's state/configuration
     * this override will additionally load profile's selector list
     */
    virtual void mkEmbUIpage(Interface *interf, JsonVariantConst data, const char* action) override;

    /**
     * @copydoc GenericModule::mkEmbUI_preset(Interface *interf)
     */
    virtual void mkEmbUI_preset(Interface *interf) override;

    /**
     * @copydoc GenericModule::mkProfilesIndex(JsonArray arr)
     */
    size_t mkProfilesIndex(JsonArray arr) override;

};


/**
 * @brief a container object start spawns/destroys Modules
 * on start/demand
 * 
 */
class ModuleManager {

public:
    ModuleManager(){};
    ~ModuleManager();

    // copy semantics forbidden
    ModuleManager(const ModuleManager&) = delete;
    ModuleManager& operator=(const ModuleManager &) = delete;
    ModuleManager(ModuleManager &&) = delete;
    ModuleManager & operator=(ModuleManager &&) = delete;

    /**
     * @brief register handlers for EmbUI and event bus
     * required to enable EmbbUI control actions for UI pages, etc...
     * 
     */
    void setHandlers();

    // unregister handlers
    void unsetHandlers();

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
    void switchPreset(const char* label, int32_t idx);

    uint32_t profilesAvailable(const char* label) const;


private:

    // module instances container
    std::list<module_pt> _modules;

    esp_event_handler_instance_t _hdlr_cmd_evt = nullptr;

    // command events handler
    void _cmdEventHandler(esp_event_base_t base, int32_t id, void* data);

    /**
     * @brief spawn a new instance of a module with supplied config
     * used with configuration is suplied via webui for non existing modules
     * @param label 
     * @param cfg whether to use a supplied configuration or load from NVS
     * @param persistent if 'true' a spawned module will save supplied configuration to NVS, set this to false if spawning module with cfg FROM NVS to avoid exta writes
     */
    void _spawn(const char* label);

    /**
     * @brief a callback method for EmbUI to generate modules UI pages
     * will render a default module's setup/state page based on serialized configuration data 
     * 
     * @param interf 
     * @param data 
     * @param action 
     */
    void _make_embui_page(Interface *interf, JsonVariantConst data, const char* action);

    // EmbUI's handler to spawn/shutdown modules
    void _set_module_state(Interface *interf, JsonVariantConst data, const char* action);

    void _set_module_cfg(Interface *interf, JsonVariantConst data, const char* action);

    void _switch_module_preset(Interface *interf, JsonVariantConst data, const char* action);

    // rename current preset for the module
    void _set_module_preset_lbl(Interface *interf, JsonVariantConst data, const char* action);

    // save current config into another preset slot
    void _set_module_preset_clone(Interface *interf, JsonVariantConst data, const char* action);

};


// Unary predicate for Module's label search match
template <class T>
class MatchLabel {
    std::string_view _lookup;
public:
    explicit MatchLabel(const char* label) : _lookup(label) {}
    bool operator() (const T& item ){
        // T is module_pt
        return _lookup.compare(item->getLabel()) == 0;
    }
};



/*
static uint8_t inline alphaBlend( uint8_t a, uint8_t b, uint8_t alpha ) { return scale8(a, 255-alpha) + scale8(b, alpha); }
static CRGB alphaBlend( CRGB a, CRGB b, uint8_t alpha){
    return CRGB( alphaBlend( a.r, b.r, alpha ), alphaBlend( a.g, b.g, alpha ), alphaBlend( a.b, b.b, alpha ) );
};
*/


