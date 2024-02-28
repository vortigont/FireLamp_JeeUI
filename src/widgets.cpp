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

  interf->json_frame_value(informer.getConfig(NULL), true);
  interf->json_frame_flush();

  // call setter with no data, it will publish existing config values if any
  //getset_dfplayer_device(interf, nullptr, NULL);
  //getset_dfplayer_opt(interf, nullptr, NULL);

}

void set_widget_cfg(Interface *interf, const JsonObject *data, const char* action){
  // only one widget for now - clock, later need to make a selector via widget's label

  if (!data || !(*data).size()) return;   // call with no data

  informer.setConfig(action, *data);
}



// ****  GenericWidget methods

GenericWidget::GenericWidget(const char* wlabel, unsigned periodic) : label(wlabel) {
  set(periodic, TASK_FOREVER, [this](){ widgetRunner(); }
  );
}

void GenericWidget::_deserialize_cfg(){
  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);

  // it does not matter if config file does not exist or requested object is missing
  // we should anyway call load_cfg to let derived class implement any default values configuration
  embuifs::deserializeFile(doc, T_widgets_cfg);
  JsonVariantConst cfg( doc[label] );
  // load config via derivated method
  load_cfg(cfg);
}

JsonVariant GenericWidget::getConfig(){
  LOGD(T_Widget, print, "getConfig for widget:"); LOGD(T_Widget, println, label);

  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);

  JsonVariant cfg( doc.createNestedObject(label) );

  generate_cfg(cfg);
  return cfg;
}

void GenericWidget::setConfig(JsonVariantConst cfg){
  LOGD(T_Widget, printf, "setConfig for widget: %s\n", label);

  // apply supplied configuration to widget 
  load_cfg(cfg);

  // save supplied config to persistent storage
  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);
  embuifs::deserializeFile(doc, T_widgets_cfg);

  // get/created nested object for specific widget
  JsonVariant dst = doc[label].isNull() ? doc.createNestedObject(label) : doc[label];
  JsonObjectConst o(cfg);

  for (JsonPairConst kvp : o)
      dst[kvp.key()] = kvp.value();

  embuifs::serialize2file(doc, T_widgets_cfg);
}

void GenericWidget::begin(){
  LOGI(T_Widget, printf, "Start widget %s", label);
  _deserialize_cfg();
  ts.addTask(*this);
  enable();
}


// ****  GenericGFXWidget methods

bool GenericGFXWidget::getOverlay(){
  if (overlay) return true;
  LOGD(T_Widget, printf, "%s obtain overlay", label);
  overlay = display.getOverlay();  // obtain overlay buffer
  if (!overlay) return false;   // failed to allocate overlay, i.e. display configuration has not been done yet
  screen = new LedFB_GFX(overlay);
  screen->setRotation(2);            // adafruit coordinates are different from LedFB, so need to rotate it
  return true;
}

void GenericGFXWidget::releaseOverlay(){
  LOGD(T_Widget, printf, "%s release overlay", label);
  delete(screen);
  screen = nullptr;
  overlay.reset();
}



// *** ClockWidget

void ClockWidget::load_cfg(JsonVariantConst cfg){
  // clk
  clk.x = cfg[T_x1offset];
  clk.y = cfg[T_y1offset] | CLOCK_DEFAULT_YOFFSET;     // if not defined, then set y offset to default value
  clk.font_index = cfg[T_font1];
  clk.seconds_font_index = cfg[T_font2];
  clk.show_seconds = cfg[T_seconds];
  clk.twelwehr = cfg[T_tm_12h];
  clk.color = cfg[T_color1];

  // date
  date.show = cfg[T_enabled];
  date.x = cfg[T_x2offset];
  date.y = cfg[T_y2offset];
  date.color = cfg[T_color2];
  date.font_index = cfg[T_font3];

  if (!screen) return;  // overlay is not loaded yet
  screen->setTextWrap(false);
  clk.fresh = false;
  date.fresh = false;
  screen->fillScreen(CRGB::Black);
}

void ClockWidget::generate_cfg(JsonVariant cfg){
  // clk
  cfg[T_x1offset] = clk.x;
  cfg[T_y1offset] = clk.y;
  cfg[T_font1] = clk.font_index;
  cfg[T_font2] = clk.seconds_font_index;
  cfg[T_seconds] = clk.show_seconds;
  cfg[T_tm_12h] = clk.twelwehr;
  cfg[T_color1] = clk.color;

  // date
  cfg[T_enabled] = date.show;
  cfg[T_x2offset] = date.x;
  cfg[T_y2offset] = date.y;
  cfg[T_color2] = date.color;
  cfg[T_font3] = date.font_index;
}

void ClockWidget::widgetRunner(){
/*
  if (!ready){
    LOGE(T_Widget, println, "****** RUN WHEN NOT INIT!!!!");
    return;
  }
*/
  // make sure that we have screen to write to
  if (!getOverlay()) {
    LOGE(T_Widget, println, "No overlay available");
    return;
  }

  std::time_t now;
  std::time(&now);
  std::tm *tm = std::localtime(&now);

  // check if I need to refresh clock on display
  if (clk.show_seconds || !clk.fresh || (now - last_date>60) )
    _print_clock(tm);

  // check if I need to refresh date on display
  // I will refresh it at least one minute just in case the date will change due to ntp events or else
  if (date.show && ( !date.fresh ||  (now - last_date>86400) ) )
    _print_date(tm);
}

void ClockWidget::_print_clock(std::tm *tm){
  //LOGI(T_Widget, println, "sec");
  screen->setTextColor(clk.color);
  screen->setFont(fonts[clk.font_index]);
  screen->setCursor(clk.x, clk.y);

  constexpr size_t buffsize = sizeof("20:23");
  char result[buffsize];
  std::strftime(result, buffsize, clk.twelwehr ? "%I:%M" : "%R", tm);    // "%R" equivalent to "%H:%M"

  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(result, clk.x, clk.y, &x, &y, &w, &h);
  clk.minX = std::min(clk.minX, x);
  clk.minY = std::min(clk.minY, y);
  clk.maxW = std::max(clk.maxW, w);
  clk.maxH = std::max(clk.maxH, h);
  screen->fillRect(clk.minX, clk.minY, clk.maxW, clk.maxH, 0);

  //_screen->fillScreen(CRGB::Black);
  screen->print(result);
  LOGV(T_Widget, printf, "time: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.font_index, clk.color, clk.minX, clk.minY, clk.maxW, clk.maxH);

  if (clk.show_seconds){
    screen->setFont(fonts[clk.seconds_font_index]);
    std::strftime(result, buffsize, ":%S", tm);
    screen->getTextBounds(result, screen->getCursorX(), screen->getCursorY(), &x, &y, &w, &h);
    clk.sminX = std::min(clk.sminX, x);
    clk.sminY = std::min(clk.sminY, y);
    clk.smaxW = std::max(clk.smaxW, w);
    clk.smaxH = std::max(clk.smaxH, h);
    screen->fillRect(clk.sminX,clk.sminY, clk.smaxW,clk.smaxH, 0);
    screen->print(result);
  }

  clk.fresh = true;
  LOGV(T_Widget, printf, "sec: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.seconds_font_index, clk.color, clk.minX, clk.minY, clk.maxW, clk.maxH);
}

void ClockWidget::_print_date(std::tm *tm){
  //LOGI(T_Widget, println, "sec");
  constexpr size_t buffsize = sizeof("2024-02-23");
  //constexpr size_t buffsize = sizeof("23 Oct, Sun blah");
  char result[buffsize];

  std::strftime(result, buffsize, "%F", tm);
  //std::strftime(result, buffsize, "%d %b, %a", tm);
  screen->setFont(fonts[date.font_index]);
  screen->setTextColor(date.color);
  screen->setCursor(date.x, date.y);

  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(result, date.x, date.y, &x, &y, &w, &h);
  date.minX = std::min(date.minX, x);
  date.minY = std::min(date.minY, y);
  date.maxW = std::max(date.maxW, w);
  date.maxH = std::max(date.maxH, h);

  screen->fillRect(date.minX,date.minY, date.maxW,date.maxH, 0);
  screen->print(result);
  date.fresh = true;

  LOGV(T_Widget, printf, "Date: %s, font:%u, clr:%u, bounds: %d %d %u %u\n", result, date.font_index, date.color, date.minX, date.minY, date.maxW, date.maxH);
}

// ****  Widget Manager methods


void WidgetManager::start(){
  if (!clock)
    clock = std::make_unique<ClockWidget>();

  clock->begin();
}

void WidgetManager::stop(){
  if (clock)
    delete clock.release();

}

void WidgetManager::register_handlers(){
  embui.action.add(A_ui_page_widgets, ui_page_widgets);                   // меню: переход на страницу "Виджеты"
  embui.action.add(A_set_widget, set_widget_cfg);                         // set widget configuration
}

JsonVariant WidgetManager::getConfig(const char* widget_label){
  //if (std::string_view wdgt(widget_label).compare(T_w_clock) == 0)

  return clock->getConfig();
  
}

void WidgetManager::setConfig(const char* widget_label, JsonVariantConst cfg){
  // pick which widget we should configure
  // for now only clock
  clock->setConfig(cfg);
}



// ****
// Widgets Manager instance
WidgetManager informer;

