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

#include "mod_clock.hpp"
#include "fonts.h"
#include "EmbUI.h"

static constexpr const char* A_set_mod_alrm           = "set_mod_alrm";                   // set alarm module item's configuration


// *** ClockModule
ClockModule::ClockModule() : GenericModuleProfiles(T_clock) {
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, ClockModule::_event_hndlr, this, &_hdlr_lmp_change_evt));
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, ClockModule::_event_hndlr, this, &_hdlr_lmp_state_evt));
  clk.cb.id = (size_t)&clk;   // make unique id for clock overlay
  date.cb.id = (size_t)&date; // make unique id for date overlay
  set( TASK_SECOND, TASK_FOREVER, [this](){ moduleRunner(); } );
  ts.addTask(*this);
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
  // run Task scheduler
  enable();
  // request lamp's status to discover it's power state
  EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::pwr));
}

void ClockModule::stop(){
  // stop Task scheduler
  disable();
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

AlarmClock::AlarmClock() : GenericModule(T_alrmclock) {
  esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd),
    [](void* self, esp_event_base_t base, int32_t id, void* data){ static_cast<AlarmClock*>(self)->_lmpChEventHandler(base, id, data); }, this, &_hdlr_lmp_change_evt
  );
//  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, AlarmClock::_event_hndlr, this, &_hdlr_lmp_state_evt));
  set( TASK_SECOND, TASK_FOREVER, [this](){ moduleRunner(); } );
  ts.addTask(*this);

  // add EmbUI's handler to set Alarm's config
  embui.action.add(A_set_mod_alrm, [this](Interface *interf, JsonObjectConst data, const char* action){ setAlarmItem(data); } );
}

AlarmClock::~AlarmClock(){
  if (_hdlr_lmp_change_evt){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::fadeEnd), _hdlr_lmp_change_evt);
    _hdlr_lmp_change_evt = nullptr;
  }

  embui.action.remove(A_set_mod_alrm);
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

void AlarmClock::setAlarmItem(JsonVariantConst cfg){
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

void AlarmClock::mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action){
  String key(T_ui_pages_module_prefix);
  key += label;
  // load Module's structure from a EmbUI's UI data
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( key.c_str() );
/*
  // Main frame MUST be flushed before sending other ui_data sections
  interf->json_frame_flush();

  interf->json_frame_interface();
  interf->json_section_uidata();
*/
  interf->uidata_pick("lampui.sections.mod_alarm.hdr");
  for (int i = 0; i !=_alarms.size(); ++i){
    String idx(i);
    interf->uidata_pick( "lampui.sections.mod_alarm.item", NULL, idx.c_str() );
  }
//  interf->json_frame_flush();
  // prepare an object with alarms setups, loaded via js from WebUI
  interf->json_frame_jscall("alarm_items_load");

  JsonDocument doc;
  getConfig(doc.to<JsonObject>());
  interf->json_object_add(doc);

  // otherwise just show a message that no config could be set w/o activating the widget
  //interf->uidata_pick("lampui.sections.mod_alarm.msg_inactive");

  interf->json_frame_flush();
}
