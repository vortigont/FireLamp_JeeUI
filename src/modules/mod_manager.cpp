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
    zookeeper that draws data to display overlay
*/

#include <ctime>
//#include "time.h"
#include <string_view>
#include "EmbUI.h"
#include "nvs_handle.hpp"
#include "mod_manager.hpp"
//#include "HTTPClient.h"

// modules
#include "fonts.h"
#include "clock/mod_clock.hpp"
#include "omnicron/omnicron.hpp"
#include "mod_textq.hpp"
#include "weather/weather.hpp"
#include "sensors/sensors.hpp"

#include "log.h"


// array with all available module names (labels) we can run
static constexpr std::array<const char*, 7> wdg_list = {
  T_alrmclock,
  T_clock,
  T_narodmon,
  T_omnicron,
  T_txtscroll,
  T_weather,
  T_sensors
};

static constexpr const char* T_ui_page_module_mask    = "ui_page_module_*";
static constexpr const char* T_ui_pages_modlist       = "lampui.pages.modules_list";
static constexpr const char* T_modpreset_lbl          = "modpreset_lbl";
static constexpr const char* T_modpreset_clone        = "modpreset_clone";

static constexpr const char* A_set_mod_state          = "set_modstate_*";                 // enable/disable Module by label
static constexpr const char* A_set_mod_cfg            = "set_modcfg_*";                   // set Modules's configuration
static constexpr const char* A_set_mod_preset         = "set_modpreset_*";                // switch module's preset
static constexpr const char* A_set_modpresetname      = "set_modpresetname";              // rename module's preset
static constexpr const char* A_set_modpresetclone     = "set_modpresetclone";             // clone current settings to another preset

// ****  GenericModule methods

GenericModule::GenericModule(const char* label, bool default_cfg_file) : label(label), _def_config(default_cfg_file) {
}

void GenericModule::getConfig(JsonObject obj) const {
  LOGD(T_Module, printf, "getConfig for module:%s\n", label);

  generate_cfg(obj);
}

void GenericModule::setConfig(JsonVariantConst cfg){
  LOGD(T_Module, printf, "%s: setConfig()\n", label);

  // apply supplied configuration to module 
  load_cfg(cfg);
  // save supplied config to NVS
  save();
}

void GenericModule::load(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());
  load_cfg(_def_config ? doc[label] : doc);
  start();
}

void GenericModule::save(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());
  if (doc.isNull())
    doc.to<JsonObject>();

  JsonObject o;
  if (_def_config){
    o = doc[label].isNull() ? doc[label].to<JsonObject>() : doc[label];
  } else 
    o = doc.as<JsonObject>();

  getConfig(_def_config ? doc[label].to<JsonObject>() : doc.to<JsonObject>());
  LOGD(T_Module, printf, "writing cfg to file: %s\n", mkFileName().c_str());
  embuifs::serialize2file(doc, mkFileName().c_str());
}

String GenericModule::mkFileName(){
  if (_def_config)
    return String(T_mod_mgr_cfg);

  // make file name
  String fname( "/" );
  fname += label;
  fname += ".json";
  return fname;
}

void GenericModule::mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action){
  String key(T_ui_pages_module_prefix);
  key += label;
  // load Module's structure from a EmbUI's UI data
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( key.c_str() );
  interf->json_frame_flush();
  // serialize and send module's configuration as a 'value' frame
  JsonDocument doc;
  getConfig(doc.to<JsonObject>());
  interf->json_frame_value(doc);

  // if this module is multi-profile, then send current profile's value
  if (profilesAvailable()){
    String s(A_set_mod_preset);
    s.remove(s.length()-1, 1);
    s += getLabel();
    interf->value(s, getCurrentProfileNum());
  }
  interf->json_frame_flush();
}


// ****  GenericModuleProfiles methods


void GenericModuleProfiles::switchPreset(int32_t idx, bool keepcurrent){
  // check I do not need to load preset's config
  if (keepcurrent){
    if (idx >= 0 && idx < MAX_NUM_OF_PROFILES)
      _profilenum = idx;
    return;
  }

  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  // restore last used profile if specified one is wrong or < 0
  if (idx < 0 || idx >= MAX_NUM_OF_PROFILES)
    _profilenum = doc[T_last_profile];
  else
    _profilenum = idx;


  LOGD(T_Module, printf, "%s switch profile:%d\n", label, _profilenum);
  JsonArray profiles = doc[T_profiles].as<JsonArray>();
  // load name
  JsonVariant v = profiles[_profilenum][P_label];
  if (v.is<const char*>())
    _profilename = v.as<const char*>();
  else {
    _profilename = T_profile;
    _profilename += idx;
  }
  // load module's config
  load_cfg(profiles[_profilenum][T_cfg]);
  start();
}

void GenericModuleProfiles::save(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  JsonVariant arr = doc[T_profiles].isNull() ? doc[T_profiles].to<JsonArray>() : doc[T_profiles];
  // if array does not have proper num of objects, prefill it with empty ones
  if (arr.size() < MAX_NUM_OF_PROFILES){
    size_t s = arr.size();
    JsonObject empty;
    while (s++ != MAX_NUM_OF_PROFILES){
      arr.add(empty);
    } 
  }

  // generate config to current profile cell
  JsonObject o = arr[_profilenum].to<JsonObject>();
  o[P_label] = _profilename;
  getConfig(o[T_cfg].to<JsonObject>());    // place config under {"cfg":{}} object

  doc[T_last_profile] = _profilenum;

  LOGD(T_Module, printf, "%s: writing cfg to file\n", label);
  embuifs::serialize2file(doc, mkFileName().c_str());
}

void GenericModuleProfiles::mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action){
  // load generic page
  GenericModule::mkEmbUIpage(interf, data, action);

  // in addition need to update profile's drop-down selector
  mkEmbUI_preset(interf);
}

void GenericModuleProfiles::mkEmbUI_preset(Interface *interf){
  interf->json_frame_interface();
  interf->json_section_content();

  // make selector's id
  String id(T_set_modpreset_);
  id += getLabel();

  JsonVariant d = interf->select(id, getCurrentProfileNum(), P_EMPTY, true);
  // fill in drop-down list with available profiles
  mkProfilesIndex(d[P_block]);
  interf->json_frame_flush();
}

size_t GenericModuleProfiles::mkProfilesIndex(JsonArray arr){
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  JsonArray profiles = doc[T_profiles];

  size_t idx{0};
  String p; 
  for(JsonVariant v : profiles) {
    JsonObject d = arr.add<JsonObject>();

    // check if profile config and label exists indeed
    if (v[P_label].is<JsonVariant>()){
      p = idx;
      p += " - ";
      p += v[P_label].as<const char*>();
    } else {
      // generate "profile1" string
      p = T_profile;
      p += idx;
    }

    d[P_label] = p;
    d[P_value] = idx;
    ++idx;
  }

  LOGD(T_Module, printf, "make index of %u profiles\n", idx);

  return idx;
}



// ****  GenericGFXModule methods
/*
bool GenericGFXModule::getOverlay(){
  if (screen) return true;
  auto overlay = display.getOverlay();  // obtain overlay buffer
  if (!overlay) return false;   // failed to allocate overlay, i.e. display configuration has not been done yet
  LOGD(T_Module, printf, "%s obtain overlay\n", label);
  screen = new LedFB_GFX(overlay);
  screen->setRotation(2);            // adafruit coordinates are different from LedFB, so need to rotate it
  return true;
}

void GenericGFXModule::releaseOverlay(){
  LOGD(T_Module, printf, "%s release overlay\n", label);
  delete(screen);
  screen = nullptr;
  //overlay.reset();
}

bool GenericGFXModule::getCanvas(){
  if (canvas) return true;
  auto c = display.getCanvas();   // obtain canvas buffer
  if (!c) return false;                 // failed to allocate overlay, i.e. display configuration has not been done yet
  LOGD(T_Module, printf, "%s obtain canvas\n", label);
  canvas = std::make_unique<LedFB_GFX>(c);
  //canvas->setRotation(2);            // adafruit coordinates are different from LedFB, so need to rotate it
  return true;
}
*/



// ****  Module Manager methods

ModuleManager::~ModuleManager(){
  unsetHandlers();
}



void ModuleManager::setHandlers(){
  // handler for modules list page
  embui.action.add(A_ui_page_modules,
    [this](Interface *interf, JsonObjectConst data, const char* action){
      interf->json_frame_interface();
      interf->json_section_uidata();
      interf->uidata_pick( T_ui_pages_modlist );
      interf->json_frame_flush();
      getModulesStatuses(interf);
      interf->json_frame_flush();
    }
  );

  // handler to start/stop module via EmbUI
  embui.action.add(A_set_mod_state, [this](Interface *interf, JsonObjectConst data, const char* action){ _set_module_state(interf, data, action); } );

  // handler for module's confiration page generators
  embui.action.add(T_ui_page_module_mask, [this](Interface *interf, JsonObjectConst data, const char* action){ _make_embui_page(interf, data, action); } );

  // handler to set module's configuration
  embui.action.add(A_set_mod_cfg, [this](Interface *interf, JsonObjectConst data, const char* action){ _set_module_cfg(interf, data, action); } );

  // switch module presets
  embui.action.add(A_set_mod_preset, [this](Interface *interf, JsonObjectConst data, const char* action){ _switch_module_preset(interf, data, action); } );

  // rename module's preset
  embui.action.add(A_set_modpresetname, [this](Interface *interf, JsonObjectConst data, const char* action){ _set_module_preset_lbl(interf, data, action); } );

  // clone module preset configuration
  embui.action.add(A_set_modpresetclone, [this](Interface *interf, JsonObjectConst data, const char* action){ _set_module_preset_clone(interf, data, action); } );

  esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID,
    [](void* self, esp_event_base_t base, int32_t id, void* data){ static_cast<ModuleManager*>(self)->_cmdEventHandler(base, id, data); },
    this, &_hdlr_cmd_evt
  );
}

void ModuleManager::unsetHandlers(){
  embui.action.remove(A_ui_page_modules);
  embui.action.remove(A_set_mod_state);
  embui.action.remove(T_ui_page_module_mask);
  embui.action.remove(A_set_mod_cfg);
  embui.action.remove(A_set_modpresetname);
  embui.action.remove(A_set_modpresetclone);


  esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, _hdlr_cmd_evt);
  _hdlr_cmd_evt = nullptr;
}


void ModuleManager::start(const char* label){
  LOGD(T_ModMGR, printf, "start: %s\n", label ? label : "ALL");
  if (label){
    // check if such module is already spawned
    auto i = std::find_if(_modules.cbegin(), _modules.cend(), MatchLabel<module_pt>(label));
    if ( i != _modules.cend() ){
      LOGD(T_ModMGR, println, "already running");
      return;
    }
  }

  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_module, NVS_READONLY, &err);

  if (err != ESP_OK) {
    // if NVS handle is unavailable then just quit
    LOGW(T_ModMGR, printf, "Err opening NVS handle:%s (%d)\n", esp_err_to_name(err), err);
    return;
  }

  // check if it's a boot-up and need start all modules based on previous state in NVS
  if (!label){
    if (err != ESP_OK) return;    // NVS namespace is not available, won't run any modules
    for (auto l : wdg_list){
      uint32_t state = 0; // value will default to 0, if not yet set in NVS
      handle->get_item(l, state);
      LOGD(T_ModMGR, printf, "Boot state for %s: %u\n", l, state);
      // if saved state is >0 then module is active, we can restore it
      if (state)
        _spawn(l);
    }
  } else {
    // start a specific module
    _spawn(label);
  }
}

void ModuleManager::stop(const char* label){
  if (!label) return;
  // check if such module is already spawned
  auto i = std::find_if(_modules.cbegin(), _modules.cend(), MatchLabel<module_pt>(label));
  if ( i != _modules.cend() ){
    LOGI(T_ModMGR, printf, "deactivate %s\n", label);
    _modules.erase(i);
    // remove state flag from NVS
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_module, NVS_READWRITE);
    handle->erase_item(label);
  }
}

void ModuleManager::getConfig(JsonObject obj, const char* label){
  LOGV(T_ModMGR, printf, "getConfig for: %s\n", label);

  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if ( i != _modules.end() ) {
    (*i)->getConfig(obj);
    String l(A_set_mod_preset, strlen(A_set_mod_preset) - 1 );   // chop last '*'
    l += label;
    obj[l] = (*i)->getCurrentProfileNum();
    
    return;
  }

  // module instance is not created, spawn a module and call to return it's config again
  _spawn(label);
  getConfig(obj, label);
}

void ModuleManager::setConfig(const char* label, JsonVariantConst cfg){
  LOGD(T_ModMGR, printf, "setConfig for: %s\n", label);

  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if ( i == _modules.end() ) {
    LOGV(T_ModMGR, println, "module does not exist, spawn a new one");
    // such module does not exist currently, spawn a new one and run same call again
    _spawn(label);
    setConfig(label, cfg);
    return;
  }

  // apply and save module's configuration
  (*i)->setConfig(cfg);
}

void ModuleManager::_spawn(const char* label){
  LOGD(T_ModMGR, printf, "spawn: %s\n", label);
  // spawn a new module based on label
  std::unique_ptr<GenericModule> w;

  if(std::string_view(label).compare(T_clock) == 0){
    w = std::make_unique<ClockModule>();
  } else if(std::string_view(label).compare(T_alrmclock) == 0){
    w = std::make_unique<AlarmClock>();
  } else if(std::string_view(label).compare(T_txtscroll) == 0){
    w = std::make_unique<ModTextScroller>();
  } else if(std::string_view(label).compare(T_omnicron) == 0){
    w = std::make_unique<OmniCron>();
  } else if(std::string_view(label).compare(T_weather) == 0){
    w = std::make_unique<ModWeatherSource>();
  } else if(std::string_view(label).compare(T_narodmon) == 0){
    w = std::make_unique<ModNarodMonSource>();
  } else if(std::string_view(label).compare(T_sensors) == 0){
    w = std::make_unique<SensorManager>();
  } else
    return;   // no such module exist

  // load module's config from file
  w->load();
  // move it into container
  _modules.emplace_back(std::move(w));

  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_module, NVS_READWRITE, &err);

  if (err != ESP_OK)
    return;

  uint32_t state = 1;
  handle->set_item(label, state);
}

void ModuleManager::getModulesStatuses(Interface *interf) const {
  if (!_modules.size()) return;

  interf->json_frame_value();
  // generate values 
  for ( auto i = _modules.cbegin(); i != _modules.cend(); ++i){
    String s(A_set_mod_state, std::string_view(A_set_mod_state).length() - 1 );   // chop '*' out of "set_modstate_*"
    interf->value( const_cast<char*>( (s + (*i)->getLabel()).c_str() ), true);
  }
  // not needed
  //interf->json_frame_flush();
}

GenericModule* ModuleManager::getModulePtr(const char* label){
  if (!_modules.size()) return nullptr;
  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if ( i == _modules.end() )
    return nullptr;

  return (*i).get();
}

bool ModuleManager::getModuleStatus(const char* label) const {
  auto i = std::find_if(_modules.cbegin(), _modules.cend(), MatchLabel<module_pt>(label));
  return (i != _modules.end());
}

void ModuleManager::switchPreset(const char* label, int32_t idx){
  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if (i != _modules.end())
    (*i)->switchPreset(idx);
}

uint32_t ModuleManager::profilesAvailable(const char* label) const {
  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if (i != _modules.end())
    return (*i)->profilesAvailable();

  return 0;
}

void ModuleManager::_make_embui_page(Interface *interf, JsonObjectConst data, const char* action){
  std::string_view lbl(action);
  lbl.remove_prefix(std::string_view(T_ui_page_module_mask).length()-1);    // chop off prefix string

  // check if such module instance exist, if not - then spawn it
  if (!getModuleStatus(lbl.data()))
    _spawn(lbl.data());

  auto p = getModulePtr(lbl.data());
  if (p){
    // call module's method to build it's UI page
    LOGD(T_ModMGR, printf, "mk mod page:%s\n", lbl.data());
    p->mkEmbUIpage(interf, data, action);
  }
}

// start/stop module EmbUI command
void ModuleManager::_set_module_state(Interface *interf, JsonObjectConst data, const char* action){
  bool state = data[action];
  // set_mod_state_*
  std::string_view lbl(action);
  lbl.remove_prefix(std::string_view(A_set_mod_state).length()-1);    // chop off prefix before '*'
  // start / stop module
  state ? start(lbl.data()) : stop(lbl.data());
}

// set module's configuration from WebUI
void ModuleManager::_set_module_cfg(Interface *interf, JsonObjectConst data, const char* action){
  std::string_view lbl(action);
  lbl.remove_prefix(std::string_view(A_set_mod_cfg).length()-1);    // chop off prefix before '*'
  setConfig(lbl.data(), data);
}

void ModuleManager::_switch_module_preset(Interface *interf, JsonObjectConst data, const char* action){

  std::string_view lbl(action);
  lbl.remove_prefix(std::string_view(A_set_mod_preset).length()-1); // chop off prefix before '*'

  switchPreset(lbl.data(), data[action]);

  // send to webUI refreshed module's config
  JsonDocument doc;
  getConfig(doc.to<JsonObject>(), lbl.data());
  interf->json_frame_value(doc);
  interf->json_frame_flush();
}

void ModuleManager::_set_module_preset_lbl(Interface *interf, JsonObjectConst data, const char* action){
  JsonVariantConst v = data[T_module];
  if (!v.is<const char*>()) return;

  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(v.as<const char*>()));
  if (i == _modules.end()) return;

  (*i)->setPresetLabel(data[T_modpreset_lbl]);
  (*i)->save();
  (*i)->mkEmbUI_preset(interf);
}

void ModuleManager::_set_module_preset_clone(Interface *interf, JsonObjectConst data, const char* action){
  JsonVariantConst v = data[T_module];
  if (!v.is<const char*>()) return;

  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(v.as<const char*>()));
  if (i == _modules.end()) return;

  (*i)->switchPreset(data[T_modpreset_clone], true);
  (*i)->save();
  (*i)->mkEmbUI_preset(interf);
}

void ModuleManager::_cmdEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Preset control
    case evt::lamp_t::modClk : {
      if (*reinterpret_cast<int32_t*>(data))
        start(T_clock);
      else
        stop(T_clock);
      break;
    }

    case evt::lamp_t::modClkPreset : {
      int idx = *reinterpret_cast<int*>(data);
      if (idx == -1)
        switchPreset(T_clock, random(profilesAvailable(T_clock)));
      else
        switchPreset(T_clock, idx);
      break;
    }

    case evt::lamp_t::modTxtScroller : {
      if (*reinterpret_cast<int32_t*>(data))
        start(T_txtscroll);
      else
        stop(T_txtscroll);
      break;
    }

    case evt::lamp_t::modTxtScrollerPreset : {
      int idx = *reinterpret_cast<int*>(data);
      if (idx == -1)
        switchPreset(T_txtscroll, random(profilesAvailable(T_txtscroll)));
      else
        switchPreset(T_txtscroll, idx);
      break;
    }
    default:;
  }
}

