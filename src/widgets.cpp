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

#include <ctime>
#include <string_view>
#include "widgets.hpp"
#include "EmbUI.h"
#include "nvs_handle.hpp"
#include "log.h"


// размеры шрифтов при выводе времени

// Adfruit fonts
#include "Fonts/Org_01.h"                   // классный мелкий квадратный шрифт 5х5 /english/
//#include "Fonts/Picopixel.h"                // proportional up to 5x5 3x5  /english/
#include "Fonts/TomThumb.h"                 // 3x5 /english/
//#include "Fonts/FreeSerif9pt7b.h"           // "01:33:30" влезает время целиком
//#include "Fonts/FreeSerifBold9pt7b.h"       // "01:35:5 " толстый аккуратный шрифт, последня цифра не всегда влезает /english/

  //#include "Fonts/FreeMono9pt7b.h"           // моноширинный тонкий шрифт оставляет много свободного места между символами, влезает "01:09:"
  //#include "Fonts/FreeMonoBold9pt7b.h"       // "01:11:"
  //#include "Fonts/FreeSans9pt7b.h"           // "01:33:2", некрасивые тонкие точки в ":"
  //#include "Fonts/FreeSansOblique9pt7b.h"    // "01:33:2" очень тонкий шрифт, курсив 
  //#include "Fonts/FreeSerifItalic9pt7b.h"     // "01:33:30" влезает время целиком, курсив /english/


// Russian fonts
#include "FreeSerif9.h"                        // "01:35:5" не лезет последний символ
#include "FreeSerifBold9.h"                    // "01:35:5" не лезет последний символ

#include "Cooper6.h"                        // "01:35:5" не лезет последний символ
#include "Cooper8.h"                        // "01:35:5" не лезет последний символ
//#include "CrystalNormal6.h"                 // "01:33:30" - тонкий, 1 пиксель, занимает чуть больше половины экрана
#include "CrystalNormal8.h"                 // "01:33:30" - тонкий, 1-2 пикселя, занимает пол экрана в высоту, и почти весь в ширину
#include "CrystalNormal10.h"                // "01:35:5" пушка! не влезает последний символ

#define WIDGETS_CFG_JSIZE   4096

#define CLOCK_DEFAULT_YOFFSET   14          // default offset for clock widget

//static const GFXfont* fonts[] = {&FreeMono9pt7b, &FreeMonoBold9pt7b, &FreeSans9pt7b, &FreeSansOblique9pt7b, &FreeSerif9pt7b,
//  &FreeSerifBold9pt7b, &FreeSerifItalic9pt7b, &Org_01, &Picopixel, &TomThumb};

// array of available fonts
static constexpr std::array<const GFXfont*, 8> fonts = {&FreeSerif9pt8b, &FreeSerifBold9pt8b, &Cooper6pt8b, &Cooper8pt8b, &CrystalNormal8pt8b, &CrystalNormal10pt8b, &Org_01, &TomThumb};


// *****
// EmbUI handlers

// start/stop widget EmbUI command
void set_widget_onoff(Interface *interf, const JsonObject *data, const char* action){
  if (!data || !(*data).size()) return;   // call with no data
  bool state = (*data)[action];
  //set_wdgtena_*
  std::string_view lbl = std::string_view (action).substr(12);
  // start / stop widget
  state ? informer.start(lbl.data()) : informer.stop(lbl.data());
}

// set widget's configuration from WebUI
void set_widget_cfg(Interface *interf, const JsonObject *data, const char* action){
  if (!data || !(*data).size()) return;   // call with no data
  // "set_wdgt_*" - action mask
  informer.setConfig(std::string_view (action).substr(9).data(), *data);  // set_wdgt_
}

void set_alrm_item(Interface *interf, const JsonObject *data, const char* action){
  AlarmClock* ptr = reinterpret_cast<AlarmClock*>( informer.getWidgetPtr(T_alrmclock) );
  if (!ptr) return;
  ptr->setAlarmItem((*data));
}

void register_widgets_handlers(){
  embui.action.add(A_set_widget_onoff, set_widget_onoff);                 // start/stop widget
  embui.action.add(A_set_wcfg_alrm, set_alrm_item);                       // set alarm item
  embui.action.add(A_set_widget, set_widget_cfg);                         // set widget configuration (this wildcard should be the last one)
}



// ****  GenericWidget methods

GenericWidget::GenericWidget(const char* wlabel, unsigned periodic) : label(wlabel) {
  set( periodic, TASK_FOREVER, [this](){ widgetRunner(); } );
  ts.addTask(*this);
}

void GenericWidget::load_cfg_from_NVS(JsonObject obj, const char* lbl){
  JsonDocument doc;
  // it does not matter if config file does not exist or requested object is missing
  // we should anyway call load_cfg to let derived class implement any default values configuration
  embuifs::deserializeFile(doc, T_widgets_cfg);
  JsonObjectConst o = doc[lbl];
  for (JsonPairConst kvp : o){
    obj[kvp.key()] = kvp.value();
  }
}

void GenericWidget::getConfig(JsonObject obj) const {
  LOGD(T_Widget, printf, "getConfig for widget:%s\n", label);

  generate_cfg(obj);
}

void GenericWidget::setConfig(JsonVariantConst cfg){
  LOGD(T_Widget, printf, "%s: setConfig()\n", label);

  // apply supplied configuration to widget 
  load_cfg(cfg);
  // save supplied config to NVS
  save(cfg);
}

void GenericWidget::load(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, T_widgets_cfg);
  load_cfg(doc[label]);
  start();
}

void GenericWidget::load(JsonVariantConst cfg){
  load_cfg(cfg);
  start();
}

void GenericWidget::save(){
  JsonDocument doc;
  embuifs::deserializeFile(doc, T_widgets_cfg);
  getConfig(doc[label].to<JsonObject>());
  LOGD(T_Widget, printf, "%s: writing cfg to file\n", label);
  embuifs::serialize2file(doc, T_widgets_cfg);
}

void GenericWidget::save(JsonVariantConst cfg){
  // save supplied config to persistent storage
  JsonDocument doc;
  embuifs::deserializeFile(doc, T_widgets_cfg);

  // get/created nested object for specific widget
  JsonVariant dst = doc[label].isNull() ? doc[label].to<JsonObject>() : doc[label];
  JsonObjectConst o = cfg.as<JsonObjectConst>();

  for (JsonPairConst kvp : o){
    dst[kvp.key()] = kvp.value();
  }

  embuifs::serialize2file(doc, T_widgets_cfg);
}


// ****  GenericGFXWidget methods

bool GenericGFXWidget::getOverlay(){
  if (screen) return true;
  auto overlay = display.getOverlay();  // obtain overlay buffer
  if (!overlay) return false;   // failed to allocate overlay, i.e. display configuration has not been done yet
  LOGD(T_Widget, printf, "%s obtain overlay\n", label);
  screen = new LedFB_GFX(overlay);
  screen->setRotation(2);            // adafruit coordinates are different from LedFB, so need to rotate it
  return true;
}

void GenericGFXWidget::releaseOverlay(){
  LOGD(T_Widget, printf, "%s release overlay\n", label);
  delete(screen);
  screen = nullptr;
  //overlay.reset();
}



// *** ClockWidget
ClockWidget::ClockWidget() : GenericGFXWidget(T_clock, TASK_SECOND) {
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, ClockWidget::_event_hndlr, this, &_hdlr_lmp_change_evt));
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, ClockWidget::_event_hndlr, this, &_hdlr_lmp_state_evt));
}

ClockWidget::~ClockWidget(){
  if (_hdlr_lmp_change_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_change_evt);
    _hdlr_lmp_change_evt = nullptr;
  }
  if (_hdlr_lmp_state_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_state_evt);
    _hdlr_lmp_state_evt = nullptr;
  }
}

void ClockWidget::load_cfg(JsonVariantConst cfg){
  // clk
  clk = {};
  clk.x = cfg[T_x1offset];
  clk.y = cfg[T_y1offset] | CLOCK_DEFAULT_YOFFSET;     // if not defined, then set y offset to default value
  clk.font_index = cfg[T_font1];
  clk.seconds_font_index = cfg[T_font2];
  clk.show_seconds = cfg[T_seconds];
  clk.twelwehr = cfg[T_tm_12h];
  clk.color = cfg[T_color1] | DEFAULT_TEXT_COLOR;

  // date
  date = {};
  date.show = cfg[P_date];
  date.x = cfg[T_x2offset];
  date.y = cfg[T_y2offset];
  date.color = cfg[T_color2];
  date.font_index = cfg[T_font3] | DEFAULT_TEXT_COLOR;

  if (!screen) return;  // overlay is not loaded yet
  screen->setTextWrap(false);
  screen->fillScreen(CRGB::Black);
  redraw = true;
}

void ClockWidget::generate_cfg(JsonVariant cfg) const {
  // clk
  cfg[T_x1offset] = clk.x;
  cfg[T_y1offset] = clk.y;
  cfg[T_font1] = clk.font_index;
  cfg[T_font2] = clk.seconds_font_index;
  cfg[T_seconds] = clk.show_seconds;
  cfg[T_tm_12h] = clk.twelwehr;
  cfg[T_color1] = clk.color;

  // date
  cfg[P_date] = date.show;
  cfg[T_x2offset] = date.x;
  cfg[T_y2offset] = date.y;
  cfg[T_color2] = date.color;
  cfg[T_font3] = date.font_index;
}

void ClockWidget::widgetRunner(){
  // make sure that we have screen to write to
  if (!getOverlay()){
    LOGW(T_Widget, println, "No overlay available");
    return;
  }

  std::time_t now;
  std::time(&now);
  std::tm *tm = std::localtime(&now);

  // force redraw on a date shift, i.e. ntp adjustment, etc
  if (now - last_date>60) redraw = true;

  // check if I need to refresh clock on display
  if (clk.show_seconds || tm->tm_sec == 0 || redraw )
    _print_clock(tm);

  // check if I need to refresh date on display once in an hour
  if (date.show && ( redraw || (tm->tm_min == 0 && tm->tm_sec == 0 ) ) )
    _print_date(tm);

  last_date = now;
  redraw = false;
}

void ClockWidget::_print_clock(std::tm *tm){
  char result[std::size("20:23")];

  int16_t x,y;
  uint16_t w,h;
  // print minutes only at :00 or stale screen
  if (tm->tm_sec == 0 || redraw){
    std::strftime(result, std::size(result), clk.twelwehr ? "%I:%M" : "%R", tm);    // "%R" equivalent to "%H:%M"
    // put a space inplace of a leading zero
    if (tm->tm_hour < 10)
      result[0] = 0x20;

    screen->setTextColor(clk.color);
    screen->setFont(fonts[clk.font_index]);
    screen->setCursor(clk.x, clk.y);

    screen->getTextBounds(result, clk.x, clk.y, &x, &y, &w, &h);
    clk.maxW = std::max( clk.maxW, static_cast<uint16_t>(w + (x-clk.x)) );   // (x-clk.x) is the offset from cursolr position to the nearest dot of the printed text
    LOGV(T_Widget, printf, "fill time bounds: %d, %d, %u, %u\n", x, y, clk.maxW, h);
    screen->fillRect(clk.x, y, clk.maxW, h, 0);
    // для шрифта 3х5 откусываем незначащий ноль что бы текст влез на матрицу 16х16. Коряво, но люди просят.
    if (clk.font_index == 7 && tm->tm_hour%12 < 10){
      std::string_view s(result);
      s.remove_prefix(1);
      screen->print(s.data());
    } else
      screen->print(result);
    // save cursor for seconds printing
    clk.scursor_x = screen->getCursorX();
    clk.scursor_y = screen->getCursorY();
    LOGV(T_Widget, printf, "time: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.font_index, clk.color, x, y, clk.maxW, h);
  }

  if (clk.show_seconds){
    screen->setFont(fonts[clk.seconds_font_index]);
    screen->setCursor(clk.scursor_x, clk.scursor_y);
    std::strftime(result, std::size(result), ":%S", tm);
    screen->getTextBounds(result, clk.scursor_x, clk.scursor_x, &x, &y, &w, &h);
    clk.smaxW = std::max(clk.smaxW, static_cast<uint16_t>(clk.scursor_x+w));
    LOGV(T_Widget, printf, "fill sec bounds: %d, %d, %u, %u\n", clk.scursor_x, clk.scursor_y-h+1, clk.smaxW-w, h);
    screen->fillRect(clk.scursor_x, clk.scursor_y-h+1, clk.smaxW-clk.scursor_x, h, 0);
    LOGV(T_Widget, printf, "sec: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.seconds_font_index, clk.color, clk.scursor_x, clk.scursor_y, clk.smaxW, h);
    screen->print(result);
  }
}

void ClockWidget::_print_date(std::tm *tm){
  char result[std::size("2024-02-23")];

  std::strftime(result, std::size(result), "%F", tm);
  //std::strftime(result, buffsize, "%d %b, %a", tm);
  screen->setFont(fonts[date.font_index]);
  screen->setTextColor(date.color);
  screen->setCursor(date.x, date.y);

  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(result, date.x, date.y, &x, &y, &w, &h);
  date.maxW = std::max(date.maxW, w);

  screen->fillRect(x,y, date.maxW,h, 0);
  screen->print(result);

  LOGD(T_Widget, printf, "Date: %s, font:%u, clr:%u, bounds: %d %d %u %u\n", result, date.font_index, date.color, x, y, date.maxW, h);
}

void ClockWidget::start(){
  // request lamp's status to discover it's power state
  EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::pwr));
}

void ClockWidget::stop(){
  disable();
  releaseOverlay();
}

void ClockWidget::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
    return static_cast<ClockWidget*>(handler)->_lmpChEventHandler(base, id, event_data);

  //if ( base == LAMP_STATE_EVENTS )
  //  return static_cast<ClockWidget*>(handler)->_lmpChEventHandler(base, id, event_data);
}

void ClockWidget::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Power control
    case evt::lamp_t::pwron :
      LOGI(T_clock, println, "activate widget");
      redraw = true;
      enable();
      break;
    case evt::lamp_t::pwroff :
      LOGI(T_clock, println, "suspend widget");
      stop();
      break;
    default:;
  }
}

// **** AlarmClock

AlarmClock::AlarmClock() : GenericWidget(T_alrmclock, TASK_SECOND) {
  esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd),
    [](void* self, esp_event_base_t base, int32_t id, void* data){ static_cast<AlarmClock*>(self)->_lmpChEventHandler(base, id, data); }, this, &_hdlr_lmp_change_evt
  );
//  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, AlarmClock::_event_hndlr, this, &_hdlr_lmp_state_evt));
}

AlarmClock::~AlarmClock(){
  if (_hdlr_lmp_change_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), _hdlr_lmp_change_evt);
    _hdlr_lmp_change_evt = nullptr;
  }
}

void AlarmClock::load_cfg(JsonVariantConst cfg){
  // Cucoo
  _cuckoo.hr = cfg[T_cockoo_hr];
  _cuckoo.hhr = cfg[T_cockoo_hhr];
  _cuckoo.quater = cfg[T_quarter];
  _cuckoo.on = cfg[T_on];
  _cuckoo.off = cfg[T_off] | 24;

  // Alarm
  JsonArrayConst al = cfg[T_event];
  // quit if config is empty
  if (!al.size())
    return;

  size_t cnt{0};
  for (JsonVariantConst e : al){
    if (cnt == _alarms.size()) return;  // array overflow
    _alarms.at(cnt).active = e[T_active];
    _alarms.at(cnt).type = static_cast<alarm_t>(e[T_type].as<int>());
    _alarms.at(cnt).hr = e[T_hr];
    _alarms.at(cnt).min = e[T_min];
    _alarms.at(cnt).track = e[T_snd];
    // sunrise
    _alarms.at(cnt).rise_on = e[T_riseOn];
    _alarms.at(cnt).sunrise_startBr = e[T_startBr];
    _alarms.at(cnt).sunrise_endBr = e[T_endBr];
    _alarms.at(cnt).sunrise_offset = e[T_offset];
    _alarms.at(cnt).sunrise_duration = e[T_duration];
    _alarms.at(cnt).sunrise_eff = e[V_effect_idx];
    // dusk
    _alarms.at(cnt).dusk_on = e[T_duskOn];
    _alarms.at(cnt).dusk_pwroff = e[T_duskPrwOff];
    _alarms.at(cnt).dusk_startBr = e[T_duskStartBr];
    _alarms.at(cnt).dusk_endBr = e[T_duskEndBr];
    _alarms.at(cnt).dusk_duration = e[T_duskDuration];
    _alarms.at(cnt).dusk_eff = e[T_duskEff];

    LOGD(T_alrmclock, printf, "#%u active:%u, %u:%u, track:%d\n", cnt, _alarms.at(cnt).active, _alarms.at(cnt).hr, _alarms.at(cnt).min, _alarms.at(cnt).track );
    ++cnt;
  }
}

void AlarmClock::generate_cfg(JsonVariant cfg) const {
  LOGD(T_alrmclock, println, "serializing alarm config");
  // Cucoo
  cfg[T_cockoo_hr] = _cuckoo.hr;
  cfg[T_cockoo_hhr] = _cuckoo.hhr;
  cfg[T_quarter] = _cuckoo.quater;
  cfg[T_on] = _cuckoo.on;
  cfg[T_off] = _cuckoo.off;

  // serialize _alarm array into Jdoc array of objects
  JsonArray arr = cfg[T_event].isNull() ? cfg[T_event].to<JsonArray>() : cfg[T_event];
  arr.clear();  // clear array, I'll replace it's content
  for (auto &e : _alarms){
    JsonObject obj = arr.add<JsonObject>();
    obj[T_active] = e.active;
    obj[T_type] = static_cast<uint16_t>(e.type);
    obj[T_hr] = e.hr;
    obj[T_min] = e.min;
    obj[T_snd] = e.track;
    // sunrise
    obj[T_riseOn]   = e.rise_on;
    obj[T_startBr] = e.sunrise_startBr;
    obj[T_endBr] = e.sunrise_endBr;
    obj[T_offset] = e.sunrise_offset;
    obj[T_duration] = e.sunrise_duration;
    obj[V_effect_idx] = e.sunrise_eff;
    // dusk
    obj[T_duskOn] = e.dusk_on;
    obj[T_duskPrwOff] = e.dusk_pwroff;
    obj[T_duskStartBr] = e.dusk_startBr;
    obj[T_duskEndBr] = e.dusk_endBr;
    obj[T_duskDuration] = e.dusk_duration;
    obj[T_duskEff] = e.dusk_eff;
  }
}

void AlarmClock::widgetRunner(){
  std::time_t now;
  std::time(&now);
  std::tm *tm = std::localtime(&now);

  // skip non 00 seconds
  if (tm->tm_sec) return;

  _sunrise_check();
  // reset time to now
  std::time(&now);
  tm = std::localtime(&now);

  // iterate alarms
  for (auto &e : _alarms){
    if (!e.active || tm->tm_hour != e.hr || tm->tm_min != e.min) continue;  // skip disabled or alarms not matching current time

    // skip weekend alarms if today is not one of the weekend days
    if ( e.type == alarm_t::weekends && (tm->tm_wday != 0 && tm->tm_wday != 6) ) continue;

    // skip workday alarms if today is one of the weekend days
    if ( e.type == alarm_t::workdays && (tm->tm_wday == 0 || tm->tm_wday == 6) ) continue;

    // else it must be either an alarm_t::onetime or alarm_t::daily alarm, so I can trigger it (if audio track is not zero)
    if (e.track)
      EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::alarmTrigger), &e.track, sizeof(int));

    // check if dusk is activated for triggered alarm
    if (e.dusk_on){
      LOGD(T_alrmclock, println, "initiate dusk fade-out");
      evt::gradual_fade_t f{e.dusk_startBr, e.dusk_endBr, e.dusk_duration * 60000};

      // switch effect
      if (e.dusk_eff)
        EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::effSwitchTo), &e.dusk_eff, sizeof(e.dusk_eff));

      // power-on lamp's effect engine
      EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwronengine));

      // run gradual fade
      EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::gradualFade), &f, sizeof(f));

      _fade_await = e.dusk_pwroff;
    }

    // if it was one-time alarm, disable it and save config
    if (e.type == alarm_t::onetime){
      e.active = false;
      save();
    }

    // if one of the alarms was triggered, then I can quit here,
    // no need to check for other alarms or call for Cuckoo clock
    return;
  }

  // cockoo clock
  if (tm->tm_hour >= _cuckoo.on && tm->tm_hour < _cuckoo.off)
    _cockoo_events(tm);

}

void AlarmClock::_cockoo_events(std::tm *tm){
  int t{0};
  if (_cuckoo.hr && tm->tm_min == 0){
    t = _cuckoo.hr;
    LOGV(T_clock, println, "hr cuckoo cmd");
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::cockoo), &t, sizeof(int));
    return;
  }

  if (_cuckoo.hhr && tm->tm_min == 30){
    t = _cuckoo.hhr;
    LOGV(T_clock, println, "hhr cuckoo cmd");
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::cockoo), &t, sizeof(int));
    return;
  }
  if (_cuckoo.quater && (tm->tm_min == 15 || tm->tm_min == 45 )){
    t = _cuckoo.quater;
    LOGV(T_clock, println, "quarter cuckoo cmd");
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::cockoo), &t, sizeof(int));
  }
}

void AlarmClock::setAlarmItem(JsonVariant cfg){
  size_t idx = cfg[T_idx];
  LOGD(T_alrmclock, printf, "set alarm config for: %u\n", idx);
  if (idx >= _alarms.size() ) return;
  _alarms.at(idx).active = cfg[T_active];
  _alarms.at(idx).type = static_cast<alarm_t>(cfg[T_type].as<int>());
  _alarms.at(idx).hr = cfg[T_hr];
  _alarms.at(idx).min = cfg[T_min];
  _alarms.at(idx).track = cfg[T_snd];
  // sunrise
  _alarms.at(idx).rise_on = cfg[T_riseOn];
  _alarms.at(idx).sunrise_startBr = cfg[T_startBr];
  _alarms.at(idx).sunrise_endBr = cfg[T_endBr];
  _alarms.at(idx).sunrise_offset = cfg[T_offset];
  _alarms.at(idx).sunrise_duration = cfg[T_duration];
  _alarms.at(idx).sunrise_eff = cfg[V_effect_idx];
  // dusk
  _alarms.at(idx).dusk_on = cfg[T_duskOn];
  _alarms.at(idx).dusk_pwroff = cfg[T_duskPrwOff];
  _alarms.at(idx).dusk_startBr = cfg[T_duskStartBr];
  _alarms.at(idx).dusk_endBr = cfg[T_duskEndBr];
  _alarms.at(idx).dusk_duration = cfg[T_duskDuration];
  _alarms.at(idx).dusk_eff = cfg[T_duskEff];

  save();
}

void AlarmClock::_sunrise_check(){
  std::time_t now;

  for (auto &e : _alarms){
    if (!e.active || !e.rise_on) continue;  // skip disabled alarms or no sunrise

    std::time(&now);
    // сдвигаем время вперёд для расчета начала рассвета по будильнику
    now += e.sunrise_offset * 60;   // min to sec
    std::tm *tm = std::localtime(&now);

    if (tm->tm_hour != e.hr || tm->tm_min != e.min) continue;

    // skip weekend alarms if today is not one of the weekend days
    if ( e.type == alarm_t::weekends && (tm->tm_wday != 0 && tm->tm_wday != 6) ) continue;

    // skip workday alarms if today is one of the weekend days
    if ( e.type == alarm_t::workdays && (tm->tm_wday == 0 || tm->tm_wday == 6) ) continue;

    // если все проверки прошли, значит сейчас время рассвета для одного из будильников
    evt::gradual_fade_t f{e.sunrise_startBr, e.sunrise_endBr, e.sunrise_duration * 60000};

    LOGD(T_alrmclock, println, "initiate sun rise");

    // switch effect
    if (e.sunrise_eff)
      EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::effSwitchTo), &e.sunrise_eff, sizeof(e.sunrise_eff));

    // power-on lamp
    EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwronengine));

    // run gradual fade
    EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::gradualFade), &f, sizeof(f));

    // дальше не проверяем
    return;
  }

}

void AlarmClock::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
  // currently only fade event is tracked, send powerdown command on fade end
  if (_fade_await)
    EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwroff));

  _fade_await = false;
}


// ****  Widget Manager methods

void WidgetManager::start(const char* label){
  LOGD(T_WdgtMGR, printf, "start: %s\n", label ? label : "ALL");
  if (label){
    // check if such widget is already spawned
    auto i = std::find_if(_widgets.cbegin(), _widgets.cend(), MatchLabel<widget_pt>(label));
    if ( i != _widgets.cend() )
      return;
  }

  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_widgets, NVS_READWRITE, &err);

  if (err != ESP_OK) {
    // if NVS handle is unavailable then just quit
    LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    return;
  }

  JsonDocument doc;
  // it does not matter if config file does not exist or requested object is missing
  // we should anyway call load_cfg to let derived class implement any default values configuration
  embuifs::deserializeFile(doc, T_widgets_cfg);
  JsonObject obj = doc.as<JsonObject>();

  for (JsonPair kvp : obj){
    // check if widget is enabled at all
    JsonVariant v = kvp.value();
    // if only specific widget needs to be started, check if it's label match 
    if (label && std::string_view(kvp.key().c_str()).compare(label) == 0){
      // spawn a new widget based on label and loaded configuration
      _spawn(kvp.key().c_str(), v);
      // save "activated" flag to eeprom
      handle->set_item(kvp.key().c_str(), 1UL);
      return;
    }

    // if no label given, then check for NVS state key and start widget if activated
    if (!label){
      uint32_t state = 0; // value will default to 0, if not yet set in NVS
      handle->get_item(kvp.key().c_str(), state);
      LOGD(T_WdgtMGR, printf, "Boot state for %s: %u\n", kvp.key().c_str(), state);
      // if saved state is >0 then widget is active, we can restore it
      if (state)
        _spawn(kvp.key().c_str(), v);
    }
  }
}

void WidgetManager::stop(const char* label){
  if (!label) return;
  // check if such widget is already spawned
  auto i = std::find_if(_widgets.cbegin(), _widgets.cend(), MatchLabel<widget_pt>(label));
  if ( i != _widgets.cend() ){
    LOGI(T_WdgtMGR, printf, "deactivate %s\n", label);
    _widgets.erase(i);
    // remove state flag from NVS
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_widgets, NVS_READWRITE);
    handle->erase_item(label);
  }
}

void WidgetManager::getConfig(JsonObject obj, const char* label){
  LOGV(T_WdgtMGR, printf, "getConfig for: %s\n", label);

  auto i = std::find_if(_widgets.begin(), _widgets.end(), MatchLabel<widget_pt>(label));
  if ( i != _widgets.end() ) {
    return (*i)->getConfig(obj);
  }

  // widget instance is not created, try to load from FS config
  GenericWidget::load_cfg_from_NVS(obj, label);
}

void WidgetManager::setConfig(const char* label, JsonVariantConst cfg){
  LOGD(T_WdgtMGR, printf, "setConfig for: %s\n", label);

  auto i = std::find_if(_widgets.begin(), _widgets.end(), MatchLabel<widget_pt>(label));
  if ( i == _widgets.end() ) {
    LOGV(T_WdgtMGR, println, "widget does not exist, spawn a new one");
    // such widget does not exist currently, spawn a new one with supplied config and store cfg to NVS
    _spawn(label, cfg, true);
    return;
  }

  // apply and save widget's configuration
  (*i)->setConfig(cfg);
}

void WidgetManager::_spawn(const char* label, JsonVariantConst cfg, bool persistent){
  LOGD(T_Widget, printf, "WM spawn: %s\n", label);
  // spawn a new widget based on label
  std::unique_ptr<GenericWidget> w;

  if(std::string_view(label).compare(T_clock) == 0){
    w = std::make_unique<ClockWidget>();
  } else if(std::string_view(label).compare(T_alrmclock) == 0){
    w = std::make_unique<AlarmClock>();
  } else
    return;   // no such widget exist

  if (cfg.isNull()){
    // ask widget to read it's config from NVS, if any
    LOGV(T_Widget, println, "WM call widget load cfg from NVS");
    w->load();
  } else {
    if (persistent){
      w->setConfig(cfg);
      w->start();
    } else
      w->load(cfg);
  }

  _widgets.emplace_back(std::move(w));

// some other widgets to be done
}

void WidgetManager::getWidgetsState(Interface *interf) const {
  if (!_widgets.size()) return;

  interf->json_frame_value();
  // generate values 
  for ( auto i = _widgets.cbegin(); i != _widgets.cend(); ++i){
    String s(A_set_widget_onoff, 12 );   // truncate '*' "set_wdgtena_*"
    interf->value( const_cast<char*>( (s + (*i)->getLabel()).c_str() ), true);
  }
  // not needed
  //interf->json_frame_flush();
}

GenericWidget* WidgetManager::getWidgetPtr(const char* label){
  if (!_widgets.size()) return nullptr;
  auto i = std::find_if(_widgets.begin(), _widgets.end(), MatchLabel<widget_pt>(label));
  if ( i == _widgets.end() )
    return nullptr;

  return (*i).get();
}


// ****
// Widgets Manager instance
WidgetManager informer;

