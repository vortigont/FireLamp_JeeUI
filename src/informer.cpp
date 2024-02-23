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
#include "informer.hpp"
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

static constexpr const char* T_Informer = "Informer";

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




GenericGFXWidget::GenericGFXWidget(const char* wlabel, LedFB_GFX* display, unsigned periodic) : label(wlabel), screen(display) {
  set(periodic, TASK_FOREVER,
    [this](){ widgetRunner(); } //,
//    [this](){ onStart(); },
//    [this](){ onStop(); }
  );
}

//GenericGFXWidget::~GenericGFXWidget(){}

void GenericGFXWidget::_deserialize_cfg(){
  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);
  if (!embuifs::deserializeFile(doc, T_widgets_cfg)) return;

  // skip config with empty objects, i.e. new file or new widget
  if ( doc[label].isNull() ) return;

  JsonVariantConst cfg( doc[label] );
  // load config via derivated method
  load_cfg(cfg);
}

JsonVariant GenericGFXWidget::getConfig(){
  LOGD(T_Informer, printf, "getConfig for widget: %s\n", label);

  DynamicJsonDocument doc(WIDGETS_CFG_JSIZE);

  JsonVariant cfg( doc.createNestedObject(label) );

  generate_cfg(cfg);
  return cfg;
}

void GenericGFXWidget::setConfig(JsonVariantConst cfg){
  LOGD(T_Informer, printf, "setConfig for widget: %s\n", label);

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

void GenericGFXWidget::begin(){
  _deserialize_cfg();
  ts.addTask(*this);
  enable();
}


// ***
// *** ClockWidget

void ClockWidget::load_cfg(JsonVariantConst cfg){
  // clk
  clk.x = cfg[T_x1offset];
  clk.y = cfg[T_y1offset];
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

  screen->setTextWrap(false);
  date.fresh = false;
  screen->fillScreen(CRGB::Black);
  ready = true;
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
  if (!ready){
    LOGD(T_Informer, println, "****** RUN WHEN NOT INIT!!!!");
    return;
  }


  screen->setTextColor(clk.color);
  screen->setFont(fonts[clk.font_index]);
  screen->setCursor(clk.x, clk.y);

  std::time_t now;
  std::time(&now);
  std::tm *tm = std::localtime(&now);

  constexpr size_t buffsize = sizeof("2024-02-23");
  char result[buffsize];
  std::strftime(result, buffsize, "%H:%M", tm);

  int16_t x,y;
  uint16_t w,h;
  screen->getTextBounds(result, clk.x, clk.y, &x, &y, &w, &h);
  screen->fillRect(x,y, w,h, 0);

  //_screen->fillScreen(CRGB::Black);
  screen->print(result);
  LOGD(T_Informer, printf, "time: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.font_index, clk.color, x,y,w,h);

  if (clk.show_seconds){
    screen->setFont(fonts[clk.seconds_font_index]);
    std::strftime(result, buffsize, ":%S", tm);
    screen->getTextBounds(result, screen->getCursorX(), screen->getCursorY(), &x, &y, &w, &h);
    screen->fillRect(x,y, w,h, 0);
    screen->print(result);
  }

  // check if I need to refresh date on display
  if (!date.show || (date.fresh && (tm->tm_hour != 0) && (tm->tm_min != 0)) ) return;

  // print date
  std::strftime(result, buffsize, "%F", tm);
  screen->setFont(fonts[date.font_index]);
  screen->setTextColor(date.color);
  screen->setCursor(date.x, date.y);
  screen->getTextBounds(result, date.x, date.y, &x, &y, &w, &h);
  screen->fillRect(x,y, w,h, 0);
  screen->print(result);
  date.fresh = true;

  LOGD(T_Informer, printf, "date: %s, font:%u, clr:%u, bounds: %d %d %u %u\n", result, date.font_index, date.color, x,y,w,h);
}


void WidgetManager::_overlay_buffer(bool activate) {
  if (activate && !_overlay){
    LOGD(T_Informer, println, "obtain overlay");
    _overlay = display.getOverlay();  // obtain overlay buffer
    _screen = new LedFB_GFX(_overlay);
    _screen->setRotation(2);            // adafruit coordinates are different from LedFB, so need to rotate it
  } else{
    LOGD(T_Informer, println, "release overlay");
    delete(_screen);
    _screen = nullptr;
    _overlay.reset();
  }
}

void WidgetManager::start(){
  _overlay_buffer(true);

  if (!clock)
    clock = std::make_unique<ClockWidget>(_screen);

  clock->begin();
}

void WidgetManager::stop(){
  if (clock)
    delete clock.release();

  _overlay_buffer(false);
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

