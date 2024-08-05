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
#include "time.h"
#include <string_view>
#include "mod_manager.hpp"
#include "EmbUI.h"
#include "nvs_handle.hpp"
#include "HTTPClient.h"
#include "log.h"

// modules
#include "fonts.h"
#include "clock/mod_clock.hpp"
#include "omnicron/omnicron.hpp"


#define CLOCK_DEFAULT_YOFFSET   14          // default offset for clock module
#define DEF_BITMAP_WIDTH        64
#define DEF_BITMAP_HEIGHT       8
#define DEF_OVERLAY_ALPHA       32
#define DEF_BITMAP_YOFFSET      20
#define DEF_WEATHER_RETRY       5000

// array with all available module names (labels) we can run
static constexpr std::array<const char*, 4> wdg_list = {
  T_clock,
  T_alrmclock,
  T_txtscroll,
  T_omnicron
};

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
  embuifs::deserializeFile(doc, T_mod_mgr_cfg);
  load_cfg(doc[label]);
  start();
}

void GenericModule::save(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, T_mod_mgr_cfg);
  getConfig(doc[label].to<JsonObject>());
  LOGD(T_Module, printf, "%s: writing cfg to file\n", label);
  embuifs::serialize2file(doc, T_mod_mgr_cfg);
}

String GenericModule::mkFileName(){
  // make file name
  String fname( "/" );
  fname += _def_config ? T_mod_mgr_cfg : label;
  fname += ".json";
  return fname;
}


// ****  GenericModuleProfiles methods


void GenericModuleProfiles::switchProfile(int idx){
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  // restore last used profile if specified one is wrong or < 0
  if (idx < 0 || idx > MAX_NUM_OF_PROFILES)
    _profilenum = doc[T_last_profile];
  else
    _profilenum = idx;

  LOGD(T_Module, printf, "%s switch profile:%d\n", label, _profilenum);
  JsonArray profiles = doc[T_profiles].as<JsonArray>();
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
    do {
      arr.add(empty);
    } while (s++ != MAX_NUM_OF_PROFILES);
  }

  // generate config to current profile cell
  JsonObject o = arr[_profilenum].to<JsonObject>();
  getConfig(o[T_cfg].to<JsonObject>());    // place config under {"cfg":{}} object

  doc[T_last_profile] = _profilenum;

  LOGD(T_Module, printf, "%s: writing cfg to file\n", label);
  embuifs::serialize2file(doc, mkFileName().c_str());
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
    LOGD(T_ModMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    return;
  }

  // check if it's a boot-up and need start all modules based on previous state in NVS
  if (!label){
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
    w = std::make_unique<TextScrollerWgdt>();
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

void ModuleManager::switchProfile(const char* label, int32_t idx){
  auto i = std::find_if(_modules.begin(), _modules.end(), MatchLabel<module_pt>(label));
  if (i != _modules.end())
    (*i)->switchProfile(idx);
}



// *** Running Text overlay 

TextScrollerWgdt::TextScrollerWgdt() : GenericModuleProfiles(T_txtscroll) {
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);

  _renderer = { (size_t)(this), [&](LedFB_GFX *gfx){ _scroll_line(gfx); } }; 

  set( 5000, TASK_FOREVER, [this](){ moduleRunner(); } );
  ts.addTask(*this);
}

TextScrollerWgdt::~TextScrollerWgdt(){
  stop();
/*
  if (_hdlr_lmp_change_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_change_evt);
    _hdlr_lmp_change_evt = nullptr;
  }
  if (_hdlr_lmp_state_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_state_evt);
    _hdlr_lmp_state_evt = nullptr;
  }
*/
}

void TextScrollerWgdt::load_cfg(JsonVariantConst cfg){
  LOGV(T_txtscroll, println, "Configure text scroller");
  _bitmapcfg.w                = cfg[T_width]    | DEF_BITMAP_WIDTH;
  _bitmapcfg.h                = cfg[T_height]   | DEF_BITMAP_HEIGHT;
  _bitmapcfg.x                = cfg[T_x1pos];
  _bitmapcfg.y                = cfg[T_y1pos]    | DEF_BITMAP_YOFFSET;
  _bitmapcfg.font_index       = cfg[T_font1];
  _bitmapcfg.baseline_shift_y = cfg[T_offset];
  _bitmapcfg.color            = cfg[T_color1]   | DEFAULT_TEXT_COLOR;
  _bitmapcfg.alpha_bg         = cfg[T_alpha_b]  | DEF_OVERLAY_ALPHA;

  _scrollrate = cfg[T_rate] | 10;

  // grab a lock on bitmap canvas
  std::lock_guard<std::mutex> lock(mtx);
  _textmask = std::make_unique<Arduino_Canvas_Mono>(_bitmapcfg.w, _bitmapcfg.h, nullptr);
  _textmask->begin();
  _textmask->setUTF8Print(true);
  _textmask->setTextWrap(false);
  _textmask->setFont(fonts.at(_bitmapcfg.font_index));
  //_textmask->setTextBound();
  //_textmask_clk->setRotation(2);

  _weathercfg.city_id = cfg[T_cityid].as<unsigned>();

  if (cfg[T_apikey].is<const char*>())
    _weathercfg.apikey =  cfg[T_apikey].as<const char*>();

  _weathercfg.refresh = (cfg[T_refresh] | 1) * 3600000;

  _last_redraw = millis();

  // weather update
  restart();
}

void TextScrollerWgdt::generate_cfg(JsonVariant cfg) const {
  cfg.clear();
  cfg[T_width]    = _bitmapcfg.w;
  cfg[T_height]   = _bitmapcfg.h;
  cfg[T_x1pos]    = _bitmapcfg.x;
  cfg[T_y1pos]    = _bitmapcfg.y;
  cfg[T_font1]    = _bitmapcfg.font_index;
  cfg[T_offset]   = _bitmapcfg.baseline_shift_y;
  cfg[T_color1]   = _bitmapcfg.color;
  cfg[T_alpha_b]  = _bitmapcfg.alpha_bg;
  cfg[T_rate]     = _scrollrate;

  //JsonObject weath = cfg[T_weather].isNull() ? cfg[T_weather].to<JsonObject>() : cfg[T_weather];
  //weath.clear();  // clear obj, I'll replace it's content

  // weather
  cfg[T_cityid] =  _weathercfg.city_id;
  if (_weathercfg.apikey.length())
    cfg[T_apikey] =  _weathercfg.apikey;
  cfg[T_refresh] = _weathercfg.refresh / 3600000;   // ms in hr
}

void TextScrollerWgdt::moduleRunner(){
  LOGV(T_txtscroll, printf, "pogoda %lu\n", getInterval());
  // this periodic runner needed only for weather update

  _getOpenWeather();
}

void TextScrollerWgdt::_scroll_line(LedFB_GFX *gfx){
  // if canvas can't be locked, skip this run
  std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
  if (!lock.try_lock())
    return;

  int32_t px_to_shift = (millis() - _last_redraw) * _scrollrate / 1000;
  _cur_offset -= px_to_shift;
  // добавляем ко времени последнего обновления столько интервалов заданной частоты на сколько пикселей мы продвинулись.
  // нужно оставить "хвосты" избыточного времени копиться до момента пока не набежит еще один високосный пиксель для сдвига
  _last_redraw += px_to_shift * 1000 / _scrollrate;
  

  _textmask->fillScreen(BLACK);
  _textmask->setCursor(_cur_offset, _bitmapcfg.h - _bitmapcfg.baseline_shift_y);
  _textmask->print(_txtstr.data());
  if (_cur_offset <  -1*_txt_pixlen)
    _cur_offset = _bitmapcfg.w;


  gfx->drawBitmap_bgfade(_bitmapcfg.x, _bitmapcfg.y, _textmask->getFramebuffer(), _bitmapcfg.w, _bitmapcfg.h, _bitmapcfg.color, _bitmapcfg.alpha_bg );
}

void TextScrollerWgdt::start(){
  // overlay rendering functor
  display.attachOverlay( _renderer );

  // enable timer
  enable();
  // request lamp's status to discover it's power state
  //EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::pwr));
}

void TextScrollerWgdt::stop(){
  disable();
  std::lock_guard<std::mutex> lock(mtx);
  display.detachOverlay(_renderer.id);
}

void TextScrollerWgdt::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
  //  return static_cast<TextScrollerWgdt*>(handler)->_lmpChEventHandler(base, id, event_data);
}
/*
void TextScrollerWgdt::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Power control
    case evt::lamp_t::pwron :
      LOGI(T_clock, println, "activate module");
      redraw = true;
      enable();
      break;
    case evt::lamp_t::pwroff :
      LOGI(T_clock, println, "suspend module");
      stop();
      break;
    default:;
  }
}
*/

void TextScrollerWgdt::_getOpenWeather(){
  if (!_weathercfg.apikey.length() || !_weathercfg.city_id) return;   // no API key - no weather

  // http://api.openweathermap.org/data/2.5/weather?id=1850147&units=metric&lang=ru&APPID=your_API_KEY>
  String url("http://api.openweathermap.org/data/2.5/weather?units=metric&lang=ru&id=");
  url += _weathercfg.city_id;
  url += "&APPID=";
  url += _weathercfg.apikey.c_str();

  HTTPClient http;
  http.begin(url);
  LOGV(T_txtscroll, printf, "get weather: %s\n", url.c_str());
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    LOGD(T_txtscroll, printf, "HTTP Weather response code:%d\n", code);

    // some HTTP error
    if (_weathercfg.retry){
      setInterval(getInterval() + DEF_WEATHER_RETRY);
    } else {
      _weathercfg.retry = true;
      setInterval(DEF_WEATHER_RETRY);
    }
    return;
  }

  JsonDocument doc;
  if ( deserializeJson(doc, *http.getStreamPtr()) != DeserializationError::Ok ) return;
  http.end();

  String pogoda;
  pogoda.reserve(100);
  pogoda = doc["name"].as<const char*>();
  pogoda += ": сейчас ";
  pogoda += doc[F("weather")][0][F("description")].as<const char*>();
  pogoda += ", ";

// Температура
  int t = int(doc["main"]["temp"].as<float>() + 0.5);
  if (t > 0)
    pogoda += "+";
  pogoda += t;

// Влажность
  pogoda += "°C, влажность:";
  pogoda += doc["main"]["humidity"].as<int>();
// Ветер
  pogoda += "%, ветер ";
  int deg = doc["wind"]["deg"];
  if( deg >22 && deg <=68 ) pogoda += "сев-вост.";
  else if( deg >68 && deg <=112 ) pogoda += "вост.";
  else if( deg >112 && deg <=158 ) pogoda += "юг-вост.";
  else if( deg >158 && deg <=202 ) pogoda += "юж.";
  else if( deg >202 && deg <=248 ) pogoda += "юг-зап.";
  else if( deg >248 && deg <=292 ) pogoda += "зап.";
  else if( deg >292 && deg <=338 ) pogoda += "сев-зап.";
  else pogoda += "сев.";
  int wind = int(doc["wind"]["speed"].as<float>() + 0.5);
  pogoda += wind;
  pogoda += " м/с";

  // sunrise/sunset
  pogoda += ", восх:";
  time_t sun = doc["sys"]["sunrise"].as<uint32_t>();
  pogoda += localtime(&sun)->tm_hour;
  pogoda += ":";
  pogoda += localtime(&sun)->tm_min;

  pogoda += ", закат:";
  sun = doc["sys"]["sunset"].as<uint32_t>();
  pogoda += localtime(&sun)->tm_hour;
  pogoda += ":";
  pogoda += localtime(&sun)->tm_min;

  // ths lock was meant for canvas, but let's use for string update also,
  // anyway string is also use when rendering text to bitmap
  std::lock_guard<std::mutex> lock(mtx);
  _txtstr = pogoda.c_str();

  // find text string width
  int16_t px, py; uint16_t pw;
  _textmask->getTextBounds(_txtstr.data(), 0, _bitmapcfg.h, &px, &py, &_txt_pixlen, &pw);

  // reset update
  _weathercfg.retry = false;
  setInterval(_weathercfg.refresh);
  LOGD(T_txtscroll, printf, "Weather update: %s\n", pogoda.c_str());
}



