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
#include "widgets.hpp"
#include "EmbUI.h"
#include "log.h"


// размеры шрифтов при выводе времени

// Adfruit fonts
#include "Fonts/Org_01.h"                   // классный мелкий квадратный шрифт 5х5 /english/
//#include "Fonts/Picopixel.h"                // 3x5  /english/
//#include "Fonts/TomThumb.h"                 // 3x5 скругленный /english/
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

static constexpr const char* T_Widget = "Widget";

//static const GFXfont* fonts[] = {&FreeMono9pt7b, &FreeMonoBold9pt7b, &FreeSans9pt7b, &FreeSansOblique9pt7b, &FreeSerif9pt7b,
//  &FreeSerifBold9pt7b, &FreeSerifItalic9pt7b, &Org_01, &Picopixel, &TomThumb};

// array of available fonts
static constexpr std::array<const GFXfont*, 7> fonts = {&FreeSerif9pt8b, &FreeSerifBold9pt8b, &Cooper6pt8b, &Cooper8pt8b, &CrystalNormal8pt8b, &CrystalNormal10pt8b, &Org_01};


// *****
// EmbUI handlers

void ui_page_widgets(Interface *interf, const JsonObject *data, const char* action){
  interf->json_frame_interface();
     interf->json_section_uidata();
        interf->uidata_pick( "lampui.pages.widgets" );
    interf->json_frame_flush();
  interf->json_frame_value(informer.getConfig(T_clock), true);
  interf->json_frame_flush();
}


void set_widget_cfg(Interface *interf, const JsonObject *data, const char* action){
  // only one widget for now - clock, later need to make a selector via widget's label

  if (!data || !(*data).size()) return;   // call with no data

  informer.setConfig(std::string_view (action).substr(9).data(), *data);  // set_wdgt_
}

void register_widgets_handlers(){
  embui.action.add(A_ui_page_widgets, ui_page_widgets);                   // меню: переход на страницу "Виджеты"
  embui.action.add(A_set_widget, set_widget_cfg);                         // set widget configuration
}



// ****  GenericWidget methods

GenericWidget::GenericWidget(const char* wlabel, unsigned periodic) : label(wlabel) {
  set( periodic, TASK_FOREVER, [this](){ widgetRunner(); } );
  ts.addTask(*this);
}

JsonVariant GenericWidget::load_cfg_from_NVS(const char* lbl){
  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);

  // it does not matter if config file does not exist or requested object is missing
  // we should anyway call load_cfg to let derived class implement any default values configuration
  embuifs::deserializeFile(doc, T_widgets_cfg);
  return doc[lbl];
}

JsonVariant GenericWidget::getConfig() const {
  LOGD(T_Widget, printf, "getConfig for widget:%s\n", label);

  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);
  //JsonObject cfg = doc.to<JsonObject>();
  JsonVariant cfg( doc.createNestedObject(label) );

  generate_cfg(cfg);
  return cfg;
}

void GenericWidget::setConfig(JsonVariantConst cfg){
  LOGD(T_Widget, printf, "%s: setConfig()\n", label);

  // apply supplied configuration to widget 
  load_cfg(cfg);
  // save supplied config to NVS
  save(cfg);
}

void GenericWidget::load(JsonVariantConst cfg){
  load_cfg(cfg);
  start();
}

void GenericWidget::save(){
  save(getConfig());
}

void GenericWidget::save(JsonVariantConst cfg){
  // save supplied config to persistent storage
  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);
  embuifs::deserializeFile(doc, T_widgets_cfg);

  // get/created nested object for specific widget
  JsonVariant dst = doc[label].isNull() ? doc.createNestedObject(label) : doc[label];
  JsonObjectConst o = cfg.as<JsonObjectConst>();

  for (JsonPairConst kvp : o)
      dst[kvp.key()] = kvp.value();

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
  // temporary flag, until I implement it outside of wdget's configuration
  cfg[T_enabled] = true; 
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
  }
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

// ****  Widget Manager methods

void WidgetManager::start(const char* label){
  LOGD(T_Widget, printf, "WM::start widget: %s\n", label ? label : "ALL");
  if (label){
    // check if such widget is already spawned
    auto i = std::find_if(_widgets.cbegin(), _widgets.cend(), MatchLabel<widget_pt>(label));
    if ( i != _widgets.cend() )
      return;
  }

  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);

  // it does not matter if config file does not exist or requested object is missing
  // we should anyway call load_cfg to let derived class implement any default values configuration
  embuifs::deserializeFile(doc, T_widgets_cfg);

  JsonObject obj = doc.as<JsonObject>();

  for (JsonPair kvp : obj){
    // check if widget is enabled at all
    JsonVariant v = kvp.value();
    if(!v[T_enabled])
      continue;

    // if only specific widget needs to be started, check if it's label match 
    if (label && std::string_view(kvp.key().c_str()).compare(label) !=0)
      continue;

    // spawn a new widget based on label and loaded configuration
    _spawn(kvp.key().c_str(), v);
  }
}

void WidgetManager::stop(const char* label){
//  if (clock)
//    delete clock.release();

}

JsonVariant WidgetManager::getConfig(const char* widget_label){
  LOGV(T_Widget, printf, "WM::getConfig for: %s\n", widget_label);

  auto i = std::find_if(_widgets.begin(), _widgets.end(), MatchLabel<widget_pt>(widget_label));
  if ( i != _widgets.end() ) {
    return (*i)->getConfig();
  }

  // widget instance is not created, try to load from config
  return GenericWidget::load_cfg_from_NVS(widget_label);
}

void WidgetManager::setConfig(const char* widget_label, JsonVariantConst cfg){
  LOGD(T_Widget, printf, "WM setConfig for: %s\n", widget_label);

  auto i = std::find_if(_widgets.begin(), _widgets.end(), MatchLabel<widget_pt>(widget_label));
  if ( i == _widgets.end() ) {
    LOGV(T_Widget, println, "WM widget does not exist, spawn a new one");
    // such widget does not exist currently, spawn a new one with supplied config and store cfg to NVS
    _spawn(widget_label, cfg, true);
    return;
  }

  // apply and save widget's configuration
  (*i)->setConfig(cfg);
  // if widget was disabled, then remove its instance from container
  if (!cfg[T_enabled]){
    LOGD(T_Widget, printf, "WM delete widget instance: %s\n", widget_label);
    _widgets.erase(i);
  }
}

void WidgetManager::_spawn(const char* widget_label, JsonVariantConst cfg, bool persistent){
  LOGD(T_Widget, printf, "spawn: %s\n", widget_label);
  // spawn a new widget based on label
//  std::unique_ptr<GenericWidget> w;

  if(std::string_view(widget_label).compare(T_clock) == 0){
    auto w = std::make_unique<ClockWidget>();
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
  }

// some other widgets to be done
}



// ****
// Widgets Manager instance
WidgetManager informer;

