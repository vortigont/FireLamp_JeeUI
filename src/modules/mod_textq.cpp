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

#include "mod_textq.hpp"
#include "fonts.h"
#include "log.h"

#define DEF_BITMAP_WIDTH        64
#define DEF_BITMAP_HEIGHT       8
#define DEF_OVERLAY_ALPHA       32
#define DEF_BITMAP_YOFFSET      20
#define DEF_MAX_MGS_Q_LEN       25    // max messages in the queue

// *** Running Text overlay 

TextScroll::TextScroll(){
  //
  _renderer = { (size_t)(this), [&](LedFB_GFX *gfx){ _scroll_line(gfx); } };
}

void TextScroll::clear(){
  _msg_pool.clear();
  _load_next = true;
}

void TextScroll::load_cfg(JsonVariantConst cfg){
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
  
}

void TextScroll::start(){
  _active = true;
  _load_next = true;
  display.attachOverlay( _renderer );
}

void TextScroll::stop(){
  std::lock_guard<std::mutex> lock(mtx);
  display.detachOverlay(_renderer.id);
  _active = false;
  // if some message in progress now, push it back to the front of the queue
  if (_current_msg)
    _msg_pool.push_front(_current_msg);
}

void TextScroll::_scroll_line(LedFB_GFX *gfx){
  // check if new message must be loaded and we have anything to display
  if (_load_next && !_load_next_msg()) return;

  // if canvas can't be locked, skip this run
  std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
  if (!lock.try_lock())
    return;

  int32_t px_to_shift = (millis() - _last_redraw) * _scrollrate / 1000;
  _cur_offset -= px_to_shift;
  // дошла ли строка до конца?
  if (_cur_offset <  -1*_txt_pixlen){
    // decrement counter
    if (_current_msg->cnt > 0)
      --_current_msg->cnt;

    if (_current_msg->cnt){
      // need to redisplay the message again later, enqueue it
      _current_msg->last_displayed = millis();
      LOGV(T_txtscroll, printf, "requeue: %s\n", _current_msg->msg.c_str());
      _msg_pool.push_back(_current_msg);
    }

    _load_next = true;
    return;
    //_cur_offset = _bitmapcfg.w;
  }

  // добавляем ко времени последнего обновления столько интервалов заданной частоты на сколько пикселей мы продвинулись.
  // нужно оставить "хвосты" избыточного времени копиться до момента пока не набежит еще один високосный пиксель для сдвига
  _last_redraw += px_to_shift * 1000 / _scrollrate;
  
  // рисуем строку только если был сдвиг
  if (px_to_shift){
    _textmask->fillScreen(BLACK);
    _textmask->setCursor(_cur_offset, _bitmapcfg.h - _bitmapcfg.baseline_shift_y);
    _textmask->print(_current_msg->msg.data());
  }

  // draw overlay
  gfx->drawBitmap_bgfade(_bitmapcfg.x, _bitmapcfg.y, _textmask->getFramebuffer(), _bitmapcfg.w, _bitmapcfg.h, _bitmapcfg.color, _bitmapcfg.alpha_bg );
}

bool TextScroll::_load_next_msg(){
  for (auto i = _msg_pool.begin(); i != _msg_pool.end(); ++i){
    if (millis() - (*i)->last_displayed > (*i)->interval * 1000){
      // found a message that need to be displayed
      _current_msg = *i;
      // remove message from queue
      _msg_pool.erase(i);
      _load_next = false;
      // find text string width
      int16_t px, py; uint16_t ph;
      _textmask->getTextBounds(_current_msg->msg.c_str(), 0, _bitmapcfg.h, &px, &py, &_txt_pixlen, &ph);
      _last_redraw = millis();
      // reset string position to the right side of the canvas
      _cur_offset = _bitmapcfg.w;
      LOGD(T_txtscroll, printf, "load string: %s\n", _current_msg->msg.c_str());
      _load_next = false;
      return true;
    }
  }

  // have not found anything
  return false;
}

void TextScroll::load_msg(JsonArrayConst msg){
  for (auto m : msg){
    _msg_pool.emplace_back( std::make_shared<TextMessage>(m[T_msg].as<const char*>(), m[T_cnt].as<int32_t>(), m[T_interval].as<int32_t>()) );
  }
}

void TextScroll::enqueueMSG(const TextMessage& msg){
  if (_active && _msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    _msg_pool.emplace_back(std::make_shared<TextMessage>(msg));
}

void TextScroll::enqueueMSG(TextMessage&& msg){
  if (_active && _msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    _msg_pool.emplace_back(std::make_shared<TextMessage>(std::move(msg)));
}

void TextScroll::updateMSG(const TextMessage& msg, bool enqueue){
  if (!_active) return;

  for (auto m : _msg_pool){
    if (m->id == msg.id){
      (*m) = msg;
      return;
    }
  }
  // no messages found
  if (enqueue)
    enqueueMSG(msg);
}







ModTextScroller::ModTextScroller() : GenericModule(T_txtscroll, false){
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);
}

ModTextScroller::~ModTextScroller(){
  stop();
}

void ModTextScroller::load_cfg(JsonVariantConst cfg){
  JsonObjectConst queues = cfg["queues"];
  if (queues.isNull())
    return;

  LOGI(T_txtscroll, println, "loading text scrollers");
  for (JsonPairConst kv : queues){
    JsonObjectConst o = kv.value();
    if (o[T_enabled] == false)
      continue;

    LOGD(T_txtscroll, printf, "load scroller:%s\n", kv.key().c_str());
    // create new object
    TextScroll &t = _scrollers.emplace_back();
    t.setID(o[P_id]);
    // load string facing/size config
    t.load_cfg(cfg[T_profiles][o[T_profile].as<unsigned>()][T_cfg]);
    // load predefined messages
    t.load_msg(o[T_messages]);
    // start scroller
    t.start();
  }
}


/*
void ModTextScroller::generate_cfg(JsonVariant cfg) const {
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
*/
void ModTextScroller::moduleRunner(){
  // this periodic runner needed only for weather update
}


void ModTextScroller::start(){
  // enable timer
}

void ModTextScroller::stop(){

}

void ModTextScroller::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  //LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
  //  return static_cast<TextScrollerWgdt*>(handler)->_lmpChEventHandler(base, id, event_data);
}

void ModTextScroller::enqueueMSG(const TextMessage& msg, uint8_t scroller_id){
  for (auto &s : _scrollers){
    if (!scroller_id || (s.getID() == scroller_id)){
      s.enqueueMSG(msg);
      return;
    }
  }
}

void ModTextScroller::enqueueMSG(TextMessage&& msg, uint8_t scroller_id){
  for (auto &s : _scrollers){
    if (!scroller_id || (s.getID() == scroller_id)){
      s.enqueueMSG(std::move(msg));
      return;
    }
  }
}

void ModTextScroller::updateMSG(const TextMessage& msg, uint8_t scroller_id, bool enqueue){
  for (auto &s : _scrollers){
    if (!scroller_id || (s.getID() == scroller_id)){
      s.updateMSG(msg, enqueue);
      return;
    }
  }
}


/*
TextScrollerWgdt::TextScrollerWgdt() : GenericModuleProfiles(T_txtscroll) {
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);

  _renderer = { (size_t)(this), [&](LedFB_GFX *gfx){ _scroll_line(gfx); } }; 

  set( 5000, TASK_FOREVER, [this](){ moduleRunner(); } );
  ts.addTask(*this);
}

TextScrollerWgdt::~TextScrollerWgdt(){
  stop();
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
  enableIfNot();
  forceNextIteration();
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
*/

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

