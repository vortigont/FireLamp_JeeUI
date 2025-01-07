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

//#include <format>
#include "weather.hpp"
#include "components.hpp"
#include "modules/mod_textq.hpp"
#include "HTTPClient.h"
#include <MD5Builder.h>
#include "EmbUI.h"
#include "log.h"

#define DEF_WEATHER_RETRY       5000

ModWeatherSource::ModWeatherSource() : GenericModuleProfiles(T_weather){
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);
  _msg_id = std::rand();

  set( 5000, TASK_FOREVER, [this](){ _getOpenWeather(); } );
  ts.addTask(*this);
}

ModWeatherSource::~ModWeatherSource(){
  ts.deleteTask(*this);
}

void ModWeatherSource::_getOpenWeather(){
  // no API key, city - no weather updates
  if (!_weathercfg.apikey.length() || !_weathercfg.city_id) { disable(); return; }
  
  // no WiFi connection - skip update
  if (!WiFi.isConnected()){
    return;
    LOGW(T_txtscroll, println, "no WiFi, skip update");
  }

  auto scroller = zookeeper.getModulePtr(T_txtscroll);
  if (!scroller)
    return;

  // http://api.openweathermap.org/data/2.5/weather?id=1850147&units=metric&lang=ru&APPID=your_API_KEY>
  String url;
  url.reserve(128);
  url += "http://api.openweathermap.org/data/2.5/weather?units=metric&lang=ru&id=";
  url += _weathercfg.city_id;
  url += "&APPID=";
  url += _weathercfg.apikey.c_str();

  LOGD(T_weather, printf, "update t: %lu\n", getInterval()/1000);

  //  TextMessage m1("Обновление погоды");
  //  static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m1), _scroller_id);

  HTTPClient http;
  http.begin(url.c_str());
  LOGV(T_weather, printf, "fetch: %s\n", url.c_str());
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    std::string m("Ошибка обновления погоды, HTTP:");
    m += std::to_string(code);
    LOGE(T_txtscroll, println, m.c_str());

    // report error
    TextMessage msg(std::move(m));
    static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(msg), _scroller_id);

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

  std::string pogoda;
  pogoda.reserve(128);
  pogoda += doc[T_name].as<const char*>();
  pogoda += ": сейчас ";
  pogoda += doc[T_weather][0][T_description].as<const char*>();
  pogoda += ", ";

// Температура
  int t = int(doc[P_main]["temp"].as<float>() + 0.5);
  if (t > 0)
    pogoda += "+";
  pogoda += std::to_string(t);

// Влажность
  pogoda += "°C, влажность:";
  pogoda += std::to_string(doc[P_main]["humidity"].as<int>());
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
  int wind = int(doc["wind"][T_speed].as<float>() + 0.5);
  pogoda += std::to_string(wind);
  pogoda += " м/с";

  // sunrise/sunset
  pogoda += ", восх:";
  time_t sun = doc["sys"]["sunrise"].as<uint32_t>();
  pogoda += std::to_string(localtime(&sun)->tm_hour);
  pogoda += ":";
  if (localtime(&sun)->tm_min < 10)
    pogoda += static_cast<char>(0x30);  // '0'
  pogoda += std::to_string(localtime(&sun)->tm_min);

  pogoda += ", закат:";
  sun = doc["sys"]["sunset"].as<uint32_t>();
  pogoda += std::to_string(localtime(&sun)->tm_hour);
  pogoda += ":";
  if (localtime(&sun)->tm_min < 10)
    pogoda += static_cast<char>(0x30);  // '0'
  pogoda += std::to_string(localtime(&sun)->tm_min);

  LOGI(T_weather, println, pogoda.c_str());

  // update message
  TextMessage m2(std::move(pogoda), _repeat_cnt, _repeat_interval, _msg_id);
  static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m2), _scroller_id);

  // reset update timer
  _weathercfg.retry = false;
  setInterval(_weathercfg.refresh);
}

void ModWeatherSource::load_cfg(JsonVariantConst cfg){
  LOGD(T_weather, println, "Configure weather source");

  _weathercfg.city_id = cfg[T_cityid].as<unsigned>();

  if (cfg[T_apikey].is<const char*>())
    _weathercfg.apikey =  cfg[T_apikey].as<const char*>();

  _weathercfg.refresh = (cfg[T_refresh] | 1) * 3600000;

  _scroller_id = cfg[T_destination];
  _repeat_cnt = cfg[T_repeat] | -1;
  _repeat_interval = cfg[T_interval];

  // weather update
  enableIfNot();
  forceNextIteration();
}

void ModWeatherSource::generate_cfg(JsonVariant cfg) const {
  cfg.clear();

  // weather
  cfg[T_cityid] =  _weathercfg.city_id;
  if (_weathercfg.apikey.length())
    cfg[T_apikey] =  _weathercfg.apikey;
  cfg[T_refresh] = _weathercfg.refresh / 3600000;   // ms in hr
  cfg[T_destination] = _scroller_id;

  cfg[T_repeat] = _repeat_cnt;
  cfg[T_interval] = _repeat_interval;
}


// NarodMon

ModNarodMonSource::ModNarodMonSource() : GenericModule(T_narodmon, false){
  _msg_id = std::rand();

  MD5Builder md5;
  md5.begin();
  md5.add(embui.macid());
  md5.calculate();
  _sourceCfg.uuid = md5.toString();

  set( 5000, TASK_FOREVER, [this](){ getData(); } );
  ts.addTask(*this);
}

ModNarodMonSource::~ModNarodMonSource(){
  ts.deleteTask(*this);
}

void ModNarodMonSource::load_cfg(JsonVariantConst cfg){
  LOGD(T_narodmon, println, "Configure NarodMon source");

  if (cfg[T_apikey].is<const char*>())
    _sourceCfg.apikey =  cfg[T_apikey].as<const char*>();

  if (cfg[T_sensor_id].is<const char*>())
    _sourceCfg.sensorid =  cfg[T_sensor_id].as<const char*>();

  if (cfg[P_lang].is<const char*>())
    _sourceCfg.lang =  cfg[P_lang].as<const char*>();

  _sourceCfg.refresh = (cfg[T_refresh] | 10) * 60000;

  _scroller_id = cfg[T_destination];
  _repeat_cnt = cfg[T_repeat] | -1;
  _repeat_interval = cfg[T_interval];

  // weather update
  enableIfNot();
  forceNextIteration();
}

void ModNarodMonSource::generate_cfg(JsonVariant cfg) const {
  cfg.clear();

  if (_sourceCfg.apikey.length())
    cfg[T_apikey] =  _sourceCfg.apikey;

  cfg[T_sensor_id] =  _sourceCfg.sensorid;
  cfg[T_refresh] = _sourceCfg.refresh / 60000;   // ms in min

  cfg[T_destination] = _scroller_id;
  cfg[T_repeat] = _repeat_cnt;
  cfg[T_interval] = _repeat_interval;
}

void ModNarodMonSource::getData(){
  // no API key - no weather updates
  if (!_sourceCfg.apikey.length()) { disable(); return; }
  
  // no WiFi connection - skip update
  if (!WiFi.isConnected()){
    return;
    LOGW(T_txtscroll, println, "no WiFi, skip update");
  }

  auto scroller = zookeeper.getModulePtr(T_txtscroll);

  // no text destination available
  if (!scroller)
    return;

  //TextMessage m1("Обновление NarodMon");
  //static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m1), _scroller_id);

  JsonDocument doc;
  JsonObject o = doc.to<JsonObject>();

  o[T_cmd] = "sensorsOnDevice";
  o["api_key"] = _sourceCfg.apikey;
  o["uuid"] = _sourceCfg.uuid;
  o[P_id] = _sourceCfg.sensorid;
  o[P_lang] = _sourceCfg.lang;

  std::string buffer;
  serializeJson(doc, buffer);

  HTTPClient http;
  http.begin("http://narodmon.ru/api");
  http.addHeader(asyncsrv::T_Content_Type, asyncsrv::T_application_json);
  //LOGV(T_weather, printf, "fetch: %s\n", url.c_str());

  int code = http.POST((uint8_t*)buffer.c_str(), buffer.length());
  if (code != HTTP_CODE_OK) {
    buffer = "Ошибка обновления погоды, HTTP:";
    buffer += std::to_string(code);
    LOGE(T_txtscroll, println, buffer.c_str());

    // report error
    TextMessage msg(std::move(buffer));
    static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(msg), _scroller_id);

    // some HTTP error
    if (_retry){
      setInterval(getInterval() + DEF_WEATHER_RETRY);
    } else {
      _retry = true;
      setInterval(DEF_WEATHER_RETRY);
    }
    return;
  }

  if ( deserializeJson(doc, *http.getStreamPtr()) != DeserializationError::Ok ) return;
  http.end();

  buffer.clear();
  buffer += doc[T_name].as<const char*>();
  buffer += (char)0x20; // space

  JsonArray arr = doc[T_sensors];
  for (auto s: arr){
    buffer += s[T_name].as<const char*>();
    buffer += ": ";
    buffer += s[P_value].as<String>().c_str();
    buffer += s[T_unit].as<const char*>();
    buffer += ", ";
  }

  LOGI(T_narodmon, println, buffer.c_str());

  // update message
  TextMessage m2(std::move(buffer), _repeat_cnt, _repeat_interval, _msg_id);
  static_cast<ModTextScroller*>(scroller)->updateMSG(std::move(m2), _scroller_id);

  // reset update timer
  _retry = false;
  setInterval(_sourceCfg.refresh);
}

