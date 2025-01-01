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
#include "EmbUI.h"
#include "log.h"

#define DEF_BITMAP_WIDTH        64
#define DEF_BITMAP_HEIGHT       8
#define DEF_OVERLAY_ALPHA       32
#define DEF_BITMAP_YOFFSET      20
#define DEF_MAX_MGS_Q_LEN       25    // max messages in the queue


static constexpr const char* A_set_mod_txtscroll_generic = "set_mod_txtscroll_generic";
static constexpr const char* A_get_mod_txtscroll_scroll_edit  = "get_mod_txtscroll_scroll_edit";
static constexpr const char* A_set_mod_txtscroll_scroll_rm = "set_mod_txtscroll_scroll_rm";
static constexpr const char* A_set_mod_txtscroll_streamcfg = "set_mod_txtscroll_streamcfg";
static constexpr const char* A_set_mod_txtscroll_send = "set_mod_txtscroll_send";

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
  //LOGV(T_txtscroll, println, "Configure text scroller");
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

void TextScroll::enqueueMSG(const TextMessage& msg, bool prepend){
  if (_active && _msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    if (prepend)
      _msg_pool.emplace_front(std::make_shared<TextMessage>(msg));
    else
      _msg_pool.emplace_back(std::make_shared<TextMessage>(msg));
}

void TextScroll::enqueueMSG(TextMessage&& msg, bool prepend){
  LOGV(T_txtscroll, printf, "enqueueMSG:%s\n", msg.msg.c_str());
  if (_active && _msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    if (prepend)
      _msg_pool.emplace_front(std::make_shared<TextMessage>(std::move(msg)));
    else
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
  // add EmbUI's handlers

  // set generic options
  embui.action.add(A_set_mod_txtscroll_generic, [this](Interface *interf, JsonObjectConst data, const char* action){ set_generic_options(interf, data, action); } );
  // remove specified scroller instance
  embui.action.add(A_set_mod_txtscroll_scroll_rm, [this](Interface *interf, JsonObjectConst data, const char* action){ rm_instance(interf, data, action); } );
  // set/add scroller instance options
  embui.action.add(A_set_mod_txtscroll_streamcfg, [this](Interface *interf, JsonObjectConst data, const char* action){ set_instance(interf, data, action); } );

  embui.action.add(A_set_mod_txtscroll_send, [this](Interface *interf, JsonObjectConst data, const char* action){ embui_send_msg(interf, data, action); } );
  
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);
}

ModTextScroller::~ModTextScroller(){
  if (eid)
    WiFi.removeEvent(eid);

  embui.action.remove(A_set_mod_txtscroll_generic);
  embui.action.remove(A_set_mod_txtscroll_scroll_rm);
  embui.action.remove(A_set_mod_txtscroll_streamcfg);
  embui.action.remove(A_set_mod_txtscroll_send);
  stop();
}

void ModTextScroller::_spawn_scroller(JsonObjectConst scroller, JsonObjectConst text_profile){
  LOGD(T_txtscroll, printf, "load scroller:%s\n", scroller[T_descr].as<const char*>());
  // create new object
  TextScroll &t = _scrollers.emplace_back();
  t.setID(scroller[T_stream_id]);
  // load string facing/size config
  t.load_cfg(text_profile);
  // load predefined messages
  t.load_msg(scroller[T_messages]);

  std::string m(scroller[T_descr].as<const char*>());
  m += " id:";
  m += std::to_string(t.getID());
  TextMessage msg(std::move(m));

  t.enqueueMSG(std::move(msg), true);

  // start scroller
  t.start();
}

void ModTextScroller::load_cfg(JsonVariantConst cfg){
  JsonArrayConst queues = cfg[T_scrollers];
  if (queues.isNull())
    return;

  LOGI(T_txtscroll, printf, "loading text scrollers: %u", queues.size());
  for (JsonVariantConst o : queues){
    if (o[T_active] == false)
      continue;

    _spawn_scroller(o, cfg[T_profiles][o[T_profile].as<unsigned>()][T_cfg]);
  }

  _wifi_events_msg = cfg[P_wifi];
  _wifi_events_stream = cfg[T_stream_id];

  // register/unregister wifi event handler
  if (_wifi_events_msg && !eid){
    // Set WiFi event handlers
    eid = WiFi.onEvent( [this](WiFiEvent_t event, WiFiEventInfo_t info){ _onWiFiEvent(event, info); } );
    LOGV(T_txtscroll, println, "monitor WiFi events");
  } else if (!_wifi_events_msg && eid){
    WiFi.removeEvent(eid);
  }
}
/*
void ModTextScroller::start(){
  // enable timer
}

void ModTextScroller::stop(){

}
*/
void ModTextScroller::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  //LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
  //  return static_cast<TextScrollerWgdt*>(handler)->_lmpChEventHandler(base, id, event_data);
}

void ModTextScroller::_onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info){
  // message WiFi events to scrollers
  switch (event){
    case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
      std::string m("WiFi connected SSID:");
      m += WiFi.SSID().c_str();
      m += " ip:";
      m += WiFi.localIP().toString().c_str();

      TextMessage msg(std::move(m));
      // let's use magic id for this so not to flood the queue in case of frequent reconnects
      msg.id = 0xdeadbeef;
      updateMSG(std::move(msg), _wifi_events_stream);
      break;
    }
    default:;
  }
}

void ModTextScroller::enqueueMSG(const TextMessage& msg, uint8_t scroller_id, bool prepend){
  for (auto &s : _scrollers){
    if (!scroller_id || (s.getID() == scroller_id)){
      s.enqueueMSG(msg, prepend);
      return;
    }
  }
}

void ModTextScroller::enqueueMSG(TextMessage&& msg, uint8_t scroller_id, bool prepend){
  for (auto &s : _scrollers){
    if (!scroller_id || (s.getID() == scroller_id)){
      s.enqueueMSG(std::move(msg), prepend);
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

void ModTextScroller::mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action){
  String key(T_ui_pages_module_prefix);
  key += label;
  // load Module's structure from a EmbUI's UI data
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( key.c_str() );

  // call js function that will get txtscroll.json and create a list of objects on a page
  interf->json_frame_jscall("txtscroller_mk_page_main");
  interf->json_frame_flush();
}

void ModTextScroller::set_generic_options(Interface *interf, JsonObjectConst data, const char* action){

  // apply setting to running instance
  load_cfg(data);

  // merge with config on FS
  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  // reset doc to object if deserialization failed
  if (doc.isNull())
    doc.to<JsonObject>();

  JsonObject cfg = doc.as<JsonObject>();

  embuifs::obj_merge(cfg, data);

  LOGD(T_txtscroll, printf, "writing cfg to file: %s\n", mkFileName().c_str());
  embuifs::serialize2file(doc, mkFileName().c_str());
}

void ModTextScroller::rm_instance(Interface *interf, JsonObjectConst data, const char* action){
  int id = data[P_value].as<int>();
  if (id < 1)
    return;

  _kill_scroller(id);

  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  bool save = false;
  JsonArray queues = doc[T_scrollers];
  for (JsonArray::iterator it=queues.begin(); it!=queues.end(); ++it) {
    if ((*it)[action] == id) {
      queues.remove(it);
      save = true;
      LOGD(T_txtscroll, printf, "erase sream: %d", id);
    }
  }
  // save file
  if (save)
    embuifs::serialize2file(doc, mkFileName().c_str());

  // load main module's page
  mkEmbUIpage(interf, {}, NULL);
}

void ModTextScroller::set_instance(Interface *interf, JsonObjectConst data, const char* action){
  if (data[T_stream_id].isNull()) return;

  uint8_t stream_id = data[T_stream_id];
  bool active = data[T_active];

  // kill existing instance, if exist, I'll either spawn a new one with updated config or leave it dead
  _kill_scroller(stream_id);
  //auto i = std::find_if(_scrollers.begin(), _scrollers.end(), [stream_id](const TextScroll &t){ return stream_id == t.getID(); });
  //if (i != _scrollers.end() && !active) _scrollers.erase(i);

  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  JsonArray queues = doc[T_scrollers];

  // this does not work
  //auto instance = std::find_if(queues.begin(), queues.end(), [stream_id](const JsonVariant &t){ return stream_id == t[T_stream_id].as<uint8_t>(); });
  JsonObject o;
  for (auto instance = queues.begin(); instance != queues.end(); ++instance ){
    if ( stream_id == (*instance)[T_stream_id].as<uint8_t>() ){
      o = *instance;
      break;
    }
  }

  if (o.isNull())
    queues.add(data);
  else
    embuifs::obj_deepmerge(o, data);    // merge with saved messages

  embuifs::serialize2file(doc, mkFileName().c_str());

  if (active){
    // spawn new instance with updated config
    LOGI(T_txtscroll, printf, "spawn scroller: %u", stream_id);
    _spawn_scroller(data, doc[T_profiles][data[T_profile].as<unsigned>()][T_cfg]);
  }

  // load main module's page
  mkEmbUIpage(interf, {}, NULL);
}

void ModTextScroller::_kill_scroller(uint8_t stream_id){
  // remove spawned instances
  std::erase_if(_scrollers, [stream_id](const TextScroll &s){ return s.getID() == stream_id; } );
}

void ModTextScroller::embui_send_msg(Interface *interf, JsonObjectConst data, const char* action){
  JsonVariantConst v = data[P_text];
  if (v.is<const char*>() && !v.isNull()){
    TextMessage m(v.as<const char*>(), data[T_cnt], data[T_interval]);
    //LOGI(T_txtscroll, printf, "Add msg:%s\n", m.msg.c_str());
    enqueueMSG(std::move(m), data[T_stream_id], data[T_prepend]);
  }
}
