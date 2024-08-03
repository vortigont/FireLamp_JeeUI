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
#include <string_view>
#include "mod_manager.hpp"
#include "EmbUI.h"
#include "nvs_handle.hpp"
#include "HTTPClient.h"
#include "log.h"


// размеры шрифтов при выводе времени
/*
// Adafruit fonts
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
*/

#define CLOCK_DEFAULT_YOFFSET   14          // default offset for clock module
#define DEF_BITMAP_WIDTH        64
#define DEF_BITMAP_HEIGHT       8
#define DEF_OVERLAY_ALPHA       32
#define DEF_BITMAP_YOFFSET      20
#define DEF_WEATHER_RETRY       5000


/*
  List of used U8G2 fonts

// Numeric only fonts
u8g2_font_fewture_tn                - 14x14 https://github.com/olikraus/u8g2/wiki/fntgrptulamide
u8g2_font_7x14B_tn                  - 7x14  https://github.com/olikraus/u8g2/wiki/fntgrpx11

// latin fonts
u8g2_font_tiny_simon_tr             - 3x7   https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2
u8g2_font_doomalpha04_tr            - 13x13 https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2#font-pictures
u8g2_font_greenbloodserif2_tr       - 15x16 https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2#font-pictures


// cyrillic fonts
u8g2_font_5x8_t_cyrillic            - 5x8 Lat/Cyrillic font
u8g2_font_8x13_t_cyrillic           - 8x13 Lat/Cyrillic font  https://github.com/olikraus/u8g2/wiki/fntgrpx11
u8g2_font_unifont_t_cyrillic        - 16x16 Lat/Cyrillic font https://github.com/olikraus/u8g2/wiki/fntgrpunifont
*/

// array of available U8G2 fonts
static constexpr std::array<const uint8_t*, 20> fonts = {
  u8g2_font_5x8_t_cyrillic,
  u8g2_font_8x13_t_cyrillic,
  u8g2_font_unifont_t_cyrillic,
  u8g2_font_fewture_tn,
  u8g2_font_7x14B_tn,
  u8g2_font_tiny_simon_tr,
  u8g2_font_doomalpha04_tr,
  u8g2_font_greenbloodserif2_tr,
  u8g2_font_freedoomr25_tn,
  u8g2_font_logisoso20_tn,
  u8g2_font_logisoso42_tn,
  u8g2_font_mystery_quest_32_tn,
  u8g2_font_mystery_quest_48_tn,
  u8g2_font_maniac_tn,
  u8g2_font_lucasarts_scumm_subtitle_o_tn,
  u8g2_font_bubble_tn,
  u8g2_font_moosenooks_tr,
  u8g2_font_osr29_tn,
  u8g2_font_osb21_tn,
  u8g2_font_osb29_tn
};

// array of available Adafruit fonts
//static constexpr std::array<const GFXfont*, 8> fonts = {&FreeSerif9pt8b, &FreeSerifBold9pt8b, &Cooper6pt8b, &Cooper8pt8b, &CrystalNormal8pt8b, &CrystalNormal10pt8b, &Org_01, &TomThumb};


// array with all available module names (labels) we can run
static constexpr std::array<const char*, 3> wdg_list = {T_clock, T_alrmclock, T_txtscroll};



// *****
// EmbUI handlers



// ****  GenericModule methods

GenericModule::GenericModule(const char* wlabel, unsigned periodic) : label(wlabel) {
  set( periodic, TASK_FOREVER, [this](){ moduleRunner(); } );
  ts.addTask(*this);
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
/*
void GenericModule::save(JsonVariantConst cfg){
  // save supplied config to persistent storage
  JsonDocument doc;
  embuifs::deserializeFile(doc, T_mod_mgr_cfg);

  // get/created nested object for specific module
  JsonVariant dst = doc[label].isNull() ? doc[label].to<JsonObject>() : doc[label];
  JsonObjectConst o = cfg.as<JsonObjectConst>();

  for (JsonPairConst kvp : o){
    dst[kvp.key()] = kvp.value();
  }

  embuifs::serialize2file(doc, T_mod_mgr_cfg);
}
*/



// ****  GenericModuleProfiles methods

String GenericModuleProfiles::_mkFileName(){
  // make file name
  String fname( "/" );
  fname += label;
  fname += ".json";
  return fname;
}

void GenericModuleProfiles::switchProfile(int idx){
  JsonDocument doc;
  embuifs::deserializeFile(doc, _mkFileName().c_str());

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
  embuifs::deserializeFile(doc, _mkFileName().c_str());

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
  embuifs::serialize2file(doc, _mkFileName().c_str());
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


// *** ClockModule
ClockModule::ClockModule() : GenericModuleProfiles(T_clock, TASK_SECOND) {
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, ClockModule::_event_hndlr, this, &_hdlr_lmp_change_evt));
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, ClockModule::_event_hndlr, this, &_hdlr_lmp_state_evt));
  clk.cb.id = (size_t)&clk;   // make unique id for clock overlay
  date.cb.id = (size_t)&date; // make unique id for date overlay
}

ClockModule::~ClockModule(){
  display.detachOverlay( clk.cb.id );
  display.detachOverlay( date.cb.id );

  if (_hdlr_lmp_change_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_change_evt);
    _hdlr_lmp_change_evt = nullptr;
  }

  if (_hdlr_lmp_state_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, _hdlr_lmp_state_evt);
    _hdlr_lmp_state_evt = nullptr;
  }
}

void ClockModule::load_cfg(JsonVariantConst cfg){
  // grab a lock on bitmap canvas
  std::lock_guard<std::mutex> lock(mtx);

  // try to detach any existing overlay
  display.detachOverlay(clk.cb.id);

  // clk
  clk.x = cfg[T_x1pos];
  clk.y = cfg[T_y1pos];
  clk.w = cfg[T_clkw] | 16;
  clk.h = cfg[T_clkh] | 8;
  clk.mixer = static_cast<ovrmixer_t>( cfg[T_mixer].as<unsigned>() );
  LOGV(T_Display, printf, "ovr mix:%u\n", e2int(clk.mixer));
  clk.baseline_shift_x = cfg[T_x1offset];
  clk.baseline_shift_y = cfg[T_y1offset];
  clk.font_index = cfg[T_font1];
  clk.seconds_font_index = cfg[T_font2];
  clk.show_seconds = cfg[T_seconds];
  clk.twelwehr = cfg[T_tm_12h];
  clk.color_txt = cfg[T_color1] | DEFAULT_TEXT_COLOR;
  clk.color_bg = cfg[T_color2];
  clk.alpha_tx = cfg[T_alpha_t] | 128;
  clk.alpha_bg = cfg[T_alpha_b] | 128;
  clk.eff_num  = cfg[V_effect_idx] | -1;    // buy default '-1' for no change

/*
  // temporary object to calculate bitmap size
  Arduino_Canvas_Mono helper(8, 1, nullptr);
  helper.setTextWrap(false);

  int16_t x,y;

  helper.setFont(fonts[clk.font_index]);
  helper.getTextBounds(clk.show_seconds ? "00:69:69" : "69:88", 0, 0, &x, &y, &clk.maxW, &clk.maxH);
  ++clk.maxH;
  LOGD(T_Module, printf, "time canvas font:%u, clr:%u, bounds: %u, %u\n", clk.font_index, clk.color_txt, clk.maxW, clk.maxH);
*/

  _textmask_clk = std::make_unique<Arduino_Canvas_Mono>(clk.w, clk.h, nullptr);
  _textmask_clk->begin();
  _textmask_clk->setTextColor(65535); // draw with 'white' although it will be a mask on '1's
  _textmask_clk->setTextWrap(false);
  _textmask_clk->fillScreen(0);

  //texture_ovr_cb_t clkovr { [&](LedFB_GFX *gfx){ gfx->fadeBitmap(clk.x, clk.y, _textmask_clk->getFramebuffer(), 48, 16, clk.color, 64); } }; 

  switch (clk.mixer){
    case ovrmixer_t::alphablend :
      clk.cb.callback = [&](LedFB_GFX *gfx){
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (!lock.try_lock()) return;
        gfx->drawBitmap_alphablend( clk.x, clk.y, _textmask_clk->getFramebuffer(), clk.w, clk.h,
                                    LedFB_GFX::colorCRGB(clk.color_txt), clk.alpha_tx,
                                    LedFB_GFX::colorCRGB(clk.color_bg), clk.alpha_bg);
        lock.unlock();
      };
      LOGV(T_Display, println, "Use alpha blend mixer");
      break;

    case ovrmixer_t::color_scale :
      clk.cb.callback = [&](LedFB_GFX *gfx){
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (!lock.try_lock()) return;
        gfx->drawBitmap_scale_colors( clk.x, clk.y, _textmask_clk->getFramebuffer(), clk.w, clk.h,
                                      LedFB_GFX::colorCRGB(clk.color_txt), LedFB_GFX::colorCRGB(clk.color_bg));
        lock.unlock();
      };
      LOGV(T_Display, println, "Use color scale mixer");
      break;

    default :
      clk.cb.callback = [&](LedFB_GFX *gfx){
        std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
        if (!lock.try_lock()) return;
        gfx->drawBitmap_bgfade( clk.x, clk.y, _textmask_clk->getFramebuffer(), clk.w, clk.h,
                                      LedFB_GFX::colorCRGB(clk.color_txt), clk.alpha_tx);
        lock.unlock();
      };
      LOGV(T_Display, println, "Use bg dim mixer");
  }

  //clk.cb.callback = [&](LedFB_GFX *gfx){ gfx->blendBitmap(clk.x, clk.y, _textmask_clk->getFramebuffer(), clk.w, clk.h, clk.color_txt, clk.alpha_tx, clk.color_bg, clk.alpha_bg); };
  //clk.cb.callback = [&](LedFB_GFX *gfx){ gfx->drawBitmap_scale_colors(clk.x, clk.y, _textmask_clk->getFramebuffer(), clk.w, clk.h, LedFB_GFX::colorCRGB(clk.color_txt), LedFB_GFX::colorCRGB(clk.color_bg)); };
  //CRGB cf = LedFB_GFX::colorCRGB(clk.color_txt);
  //CRGB cb = LedFB_GFX::colorCRGB(clk.color_bg);
  //LOGV(T_Display, printf, "Clk colors: %u/%u front:%u,%u,%u, back:%u,%u,%u\n", clk.color_txt, clk.color_bg, cf.r, cf.g, cf.b, cb.r, cb.g, cb.b);
  //LOGV(T_Display, printf, "clk overlay: %u\n", (size_t)&clk);
  display.attachOverlay( clk.cb );

  // date
  date_show = cfg[P_date];
  date.x = cfg[T_x2pos];
  date.y = cfg[T_y2pos] | 8;
  date.w = cfg[T_datew] | 16;
  date.h = cfg[T_dateh] | 8;
  date.baseline_shift_x = cfg[T_x2offset];
  date.baseline_shift_y = cfg[T_y2offset];
  date.color = cfg[T_color3] | DEFAULT_TEXT_COLOR;
  date.font_index = cfg[T_font3];
  date.alpha_bg = cfg[T_alpha_b2];
  if (cfg[T_datefmt])
    date.datefmt = cfg[T_datefmt].as<const char*>();


  // detach existing date overlay
  display.detachOverlay(date.cb.id);

  if (date_show){
    _textmask_date = std::make_unique<Arduino_Canvas_Mono>(date.w, date.h, nullptr);
    _textmask_date->begin();
    _textmask_date->setTextWrap(false);

    date.cb.callback = [&](LedFB_GFX *gfx){
      std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
      if (!lock.try_lock()) return;
      gfx->drawBitmap_bgfade(date.x, date.y, _textmask_date->getFramebuffer(), date.w, date.h, date.color, date.alpha_bg);
      lock.unlock();
    };
    LOGV(T_Display, printf, "date overlay: %u\n", (size_t)&date);
    display.attachOverlay( date.cb );
  } else {
    // check if need to release existing date bitmap
    if (_textmask_date){
      _textmask_date.reset();
    }
  }

  redraw = true;

  // switch effect if defined
  if (clk.eff_num > -1)
    EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::effSwitchTo), &clk.eff_num, sizeof(clk.eff_num));
}

void ClockModule::generate_cfg(JsonVariant cfg) const {
  // clk
  cfg[T_x1pos] = clk.x;
  cfg[T_y1pos] = clk.y;
  cfg[T_clkw] = clk.w;
  cfg[T_clkh] = clk.h;
  cfg[T_mixer] = e2int(clk.mixer);
  cfg[T_x1offset] = clk.baseline_shift_x;
  cfg[T_y1offset] = clk.baseline_shift_y;
  cfg[T_font1] = clk.font_index;
  cfg[T_font2] = clk.seconds_font_index;
  cfg[T_seconds] = clk.show_seconds;
  cfg[T_tm_12h] = clk.twelwehr;
  cfg[T_color1] = clk.color_txt;
  cfg[T_color2] = clk.color_bg;
  cfg[T_alpha_t] = clk.alpha_tx;
  cfg[T_alpha_b] = clk.alpha_bg;
  cfg[V_effect_idx] = clk.eff_num;

  // date
  cfg[P_date] = date_show;
  cfg[T_x2pos] = date.x;
  cfg[T_y2pos] = date.y;
  cfg[T_datew] = date.w;
  cfg[T_dateh] = date.h;
  cfg[T_x2offset] = date.baseline_shift_x;
  cfg[T_y2offset] = date.baseline_shift_y;
  cfg[T_color3] = date.color;
  cfg[T_alpha_b2] = date.alpha_bg;
  cfg[T_font3] = date.font_index;
  cfg[T_datefmt] = date.datefmt;
}

void ClockModule::moduleRunner(){
  // make sure that we have screen to write to
//  if (!getOverlay()){
//    LOGW(T_Module, println, "No overlay available");
//    return;
//  }

  std::time_t now;
  std::time(&now);
  std::tm *tm = std::localtime(&now);

  // force redraw on a date shift, i.e. ntp adjustment, etc
  if (now - last_date>60) redraw = true;

  // check if I need to refresh clock on display
  if (clk.show_seconds || tm->tm_sec == 0 || redraw )
    _print_clock(tm);

  // check if I need to refresh date on display once in an hour
  if (date_show && ( redraw || (tm->tm_min == 0 && tm->tm_sec == 0 ) ) )
    _print_date(tm);

  last_date = now;
  redraw = false;
}

void ClockModule::_print_clock(std::tm *tm){
  // grab a lock on bitmap canvas
  std::lock_guard<std::mutex> lock(mtx);

  _textmask_clk->fillScreen(0);
  char result[std::size("20:00")];

  std::strftime(result, std::size(result), clk.twelwehr ? "%I:%M" : "%R", tm);    // "%R" equivalent to "%H:%M"
  // put a space inplace of a leading zero when in 24h mode
  if (tm->tm_hour < 10 || (clk.twelwehr && (tm->tm_hour % 12 < 10)) )
    result[0] = 0x20;

  _textmask_clk->setFont(fonts[clk.font_index]);

  //_textmask_clk->getTextBounds(result, clk.x, clk.y, &x, &y, &w, &h);
  _textmask_clk->setCursor(clk.baseline_shift_x, clk.h - clk.baseline_shift_y);

  _textmask_clk->print(result);

  if (clk.show_seconds){
    _textmask_clk->setFont(fonts[clk.seconds_font_index]);
    std::strftime(result, std::size(result), ":%S", tm);
    //LOGV(T_Module, printf, "fill sec bounds: %d, %d, %u, %u\n", clk.scursor_x, clk.scursor_y-h+1, clk.smaxW-w, h);
    //LOGV(T_Module, printf, "sec: %s, font:%u, clr:%u, bounds: %d, %d, %u, %u\n", result, clk.seconds_font_index, clk.color, clk.scursor_x, clk.scursor_y, clk.smaxW, h);
    _textmask_clk->print(result);
  }
}

void ClockModule::_print_date(std::tm *tm){
  std::lock_guard<std::mutex> lock(mtx);
  _textmask_date->fillScreen(0);
  char result[20];

  std::strftime(result, std::size(result), date.datefmt.c_str(), tm);
  _textmask_date->setFont(fonts[date.font_index]);
  _textmask_date->setCursor(date.baseline_shift_x, date.h - date.baseline_shift_y);

  _textmask_date->print(result);
  //LOGD(T_Module, printf, "Date: %s, font:%u, clr:%u, bounds: %d %d %u %u\n", result, date.font_index, date.color, x, y, date.maxW, h);
}

void ClockModule::start(){
  // request lamp's status to discover it's power state
  EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::pwr));
}

void ClockModule::stop(){
  disable();
  //releaseOverlay();
}

void ClockModule::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
    return static_cast<ClockModule*>(handler)->_lmpChEventHandler(base, id, event_data);

  //if ( base == LAMP_STATE_EVENTS )
  //  return static_cast<ClockModule*>(handler)->_lmpChEventHandler(base, id, event_data);
}

void ClockModule::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
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

// **** AlarmClock

AlarmClock::AlarmClock() : GenericModule(T_alrmclock, TASK_SECOND) {
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

void AlarmClock::moduleRunner(){
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
  if (tm->tm_hour >= _cuckoo.on && tm->tm_hour <= _cuckoo.off)
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
    String l("wdgt_profile_");
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
    String s(A_set_mod_state, std::size_t(A_set_mod_state)-1 );   // chop '*' out of "set_modstate_*"
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

TextScrollerWgdt::TextScrollerWgdt() : GenericModuleProfiles(T_txtscroll, 5000) {
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);

  _renderer = { (size_t)(this), [&](LedFB_GFX *gfx){ _scroll_line(gfx); } }; 
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



