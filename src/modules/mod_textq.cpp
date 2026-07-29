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
static constexpr const char* A_set_mod_txtscroll_profile_apply = "set_mod_txtscroll_profile_apply";
static constexpr const char* A_set_mod_txtscroll_profile_save = "set_mod_txtscroll_profile_save";


// *** Running Text overlay 

TextQRenderer::TextQRenderer(){
  render_cb.id = (size_t)(this);
  render_cb.callback = [&](LedFB_GFX *gfx){ render(gfx); };
}

void TextQRenderer::clear(){
  msg_pool.clear();
  load_next = true;
}

void TextQRenderer::load_cfg(JsonVariantConst cfg){
  //LOGV(T_txtscroll, println, "Configure text scroller");
  //serializeJsonPretty(cfg, Serial);
  bitmapcfg.w                = cfg[T_width]    | DEF_BITMAP_WIDTH;
  bitmapcfg.h                = cfg[T_height]   | DEF_BITMAP_HEIGHT;
  bitmapcfg.x                = cfg[T_x1pos];
  bitmapcfg.y                = cfg[T_y1pos]    | DEF_BITMAP_YOFFSET;
  bitmapcfg.font_index       = cfg[T_font1];
  bitmapcfg.baseline_shift_y = cfg[T_offset];
  bitmapcfg.color            = cfg[T_color1]   | DEFAULT_TEXT_COLOR;
  bitmapcfg.alpha_bg         = cfg[T_alpha_b]  | DEF_OVERLAY_ALPHA;

  // grab a lock on bitmap canvas
  std::lock_guard<std::mutex> lock(mtx);
  textmask = std::make_unique<Arduino_Canvas_Mono>(bitmapcfg.w, bitmapcfg.h, nullptr);
  textmask->begin();
  textmask->setUTF8Print(true);
  textmask->setTextWrap(false);
  textmask->setFont(fonts.at(bitmapcfg.font_index));
  //textmask->setTextBound();
  //textmask_clk->setRotation(2);
  
}

void TextQRenderer::start(){
  _active = true;
  load_next = true;
  display.attachOverlay( render_cb );
}

void TextQRenderer::stop(){
  std::lock_guard<std::mutex> lock(mtx);
  display.detachOverlay(render_cb.id);
  _active = false;
  // if some message in progress now, push it back to the front of the queue
  if (current_msg)
    msg_pool.push_front(current_msg);
}

bool TextQRenderer::load_next_msg(){
  for (auto i = msg_pool.begin(); i != msg_pool.end(); ++i){
    if (millis() - (*i)->last_displayed > (*i)->interval * 1000){
      // found a message that need to be displayed
      current_msg = *i;
      // remove message from queue
      msg_pool.erase(i);
      load_next = false;
      // find text string width
      int16_t px, py; uint16_t ph;
      textmask->getTextBounds(current_msg->msg.c_str(), 0, bitmapcfg.h, &px, &py, &txt_pixlen, &ph);
      last_redraw = millis();
      LOGD(T_txtscroll, printf, "load string: %s\n", current_msg->msg.c_str());
      load_next = false;
      return true;
    }
  }

  // have not found anything
  return false;
}

void TextQRenderer::load_msg(JsonArrayConst msg){
  for (auto m : msg){
    msg_pool.emplace_back( std::make_shared<TextMessage>(m[T_msg].as<const char*>(), m[T_cnt].as<int32_t>(), m[T_interval].as<int32_t>()) );
  }
}

void TextQRenderer::enqueueMSG(const TextMessage& msg, bool prepend){
  if (_active && msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    if (prepend)
      msg_pool.emplace_front(std::make_shared<TextMessage>(msg));
    else
      msg_pool.emplace_back(std::make_shared<TextMessage>(msg));
}

void TextQRenderer::enqueueMSG(TextMessage&& msg, bool prepend){
  LOGV(T_txtscroll, printf, "enqueueMSG:%s\n", msg.msg.c_str());
  if (_active && msg_pool.size() <= DEF_MAX_MGS_Q_LEN)
    if (prepend)
      msg_pool.emplace_front(std::make_shared<TextMessage>(std::move(msg)));
    else
      msg_pool.emplace_back(std::make_shared<TextMessage>(std::move(msg)));
}

void TextQRenderer::updateMSG(const TextMessage& msg, bool enqueue){
  if (!_active) return;

  for (auto m : msg_pool){
    if (m->id == msg.id){
      (*m) = msg;
      return;
    }
  }
  // no messages found
  if (enqueue)
    enqueueMSG(msg);
}


// Text scroller
void TextQScroller::render(LedFB_GFX *gfx){
  // check if new message must be loaded and we have anything to display
  if (load_next && !load_next_msg()) return;

  // if canvas can't be locked, skip this run
  std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
  if (!lock.try_lock())
    return;

  int32_t px_to_shift = (millis() - last_redraw) * _scrollrate / 1000;
  _cur_offset -= px_to_shift;
  // дошла ли строка до конца?
  if (_cur_offset <  -1*txt_pixlen){
    // decrement counter
    if (current_msg->cnt > 0)
      --current_msg->cnt;

    if (current_msg->cnt){
      // need to redisplay the message again later, enqueue it
      current_msg->last_displayed = millis();
      LOGV(T_txtscroll, printf, "requeue: %s\n", current_msg->msg.c_str());
      msg_pool.push_back(current_msg);
    }

    load_next = true;
    return;
    //_cur_offset = bitmapcfg.w;
  }

  // добавляем ко времени последнего обновления столько интервалов заданной частоты на сколько пикселей мы продвинулись.
  // нужно оставить "хвосты" избыточного времени копиться до момента пока не набежит еще один високосный пиксель для сдвига
  last_redraw += px_to_shift * 1000 / _scrollrate;
  
  // рисуем строку только если был сдвиг
  if (px_to_shift){
    textmask->fillScreen(BLACK);
    textmask->setCursor(_cur_offset, bitmapcfg.h - bitmapcfg.baseline_shift_y);
    textmask->print(current_msg->msg.data());
  }

  // draw overlay
  gfx->drawBitmap_bgfade(bitmapcfg.x, bitmapcfg.y, textmask->getFramebuffer(), bitmapcfg.w, bitmapcfg.h, bitmapcfg.color, bitmapcfg.alpha_bg );
}

bool TextQScroller::load_next_msg(){
  auto r = TextQRenderer::load_next_msg();
  if (r){
    // reset string position to the right side of the canvas
    _cur_offset = bitmapcfg.w;
  }
  return r;
}

void TextQScroller::load_cfg(JsonVariantConst cfg){
  _scrollrate = cfg[T_rate] | 10;
  TextQRenderer::load_cfg(cfg);
}



ModTextDisplay::ModTextDisplay() : GenericModule(T_txtscroll, false){
  // add EmbUI's handlers

  // set generic options
  embui.action.add(A_set_mod_txtscroll_generic, [this](Interface *interf, JsonVariantConst data, const char* action){ set_generic_options(interf, data, action); } );
  // remove specified scroller instance
  embui.action.add(A_set_mod_txtscroll_scroll_rm, [this](Interface *interf, JsonVariantConst data, const char* action){ rm_instance(interf, data, action); } );
  // set/add scroller instance options
  embui.action.add(A_set_mod_txtscroll_streamcfg, [this](Interface *interf, JsonVariantConst data, const char* action){ set_instance(interf, data, action); } );

  embui.action.add(A_set_mod_txtscroll_send, [this](Interface *interf, JsonVariantConst data, const char* action){ embui_send_msg(interf, data, action); } );

  embui.action.add(A_set_mod_txtscroll_profile_apply, [this](Interface *interf, JsonVariantConst data, const char* action){ embui_profile_apply(interf, data, action); } );

  embui.action.add(A_set_mod_txtscroll_profile_save, [this](Interface *interf, JsonVariantConst data, const char* action){ embui_profile_save(interf, data, action); } );

  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_change_evt);
  //esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_STATE_EVENTS, ESP_EVENT_ANY_ID, TextScrollerWgdt::_event_hndlr, this, &_hdlr_lmp_state_evt);
}

ModTextDisplay::~ModTextDisplay(){
  if (eid)
    WiFi.removeEvent(eid);

  embui.action.remove(A_set_mod_txtscroll_generic);
  embui.action.remove(A_set_mod_txtscroll_scroll_rm);
  embui.action.remove(A_set_mod_txtscroll_streamcfg);
  embui.action.remove(A_set_mod_txtscroll_send);
  embui.action.remove(A_set_mod_txtscroll_profile_apply);
  embui.action.remove(A_set_mod_txtscroll_profile_save);
  
  stop();
}

void ModTextDisplay::_spawn_instance(JsonObjectConst config, JsonObjectConst text_profile){
  LOGI(T_txtscroll, printf, "creating text render:%s\n", config[T_descr].as<const char*>());

  std::unique_ptr<TextQRenderer> t;

  // create new object
  switch (config[T_type].as<unsigned>()){
    //case 1:
    //  break;
    default:
      t = std::make_unique<TextQScroller>();
  }

  // a safety if something went wrong on creation
  if (!t){
    LOGI(T_txtscroll, printf, "err creating instance of %s\n", config[T_descr].as<const char*>());
    return;
  }

  t->setID(config[T_stream_id]);
  // load text facing/size config
  t->load_cfg(text_profile);
  // load predefined messages
  t->load_msg(config[T_messages]);

  // start scroller
  t->start();

  // put the obj into container
  _renders.emplace_back(std::move(t));

/*
  std::string m(scroller[T_descr].as<const char*>());
  m += " id:";
  m += std::to_string(t.getID());
  TextMessage msg(std::move(m));
  t->enqueueMSG(std::move(msg), true);
*/
}

void ModTextDisplay::load_cfg(JsonVariantConst cfg){
  JsonArrayConst queues = cfg[T_scrollers];
  if (queues.isNull())
    return;

  LOGI(T_txtscroll, printf, "loading text scrollers: %u", queues.size());
  for (JsonVariantConst o : queues){
    if (o[T_active] == false)
      continue;

    _spawn_instance(o, cfg[T_profiles][o[T_profile].as<unsigned>()][T_cfg]);
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
void ModTextDisplay::start(){
  // enable timer
}

void ModTextDisplay::stop(){

}
*/
void ModTextDisplay::_event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  //LOGV(T_clock, printf, "EVENT %s:%d\n", base, id);
  //if ( base == LAMP_CHANGE_EVENTS )
  //  return static_cast<TextScrollerWgdt*>(handler)->_lmpChEventHandler(base, id, event_data);
}

void ModTextDisplay::_onWiFiEvent(arduino_event_id_t event, arduino_event_info_t info){
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

void ModTextDisplay::enqueueMSG(const TextMessage& msg, uint8_t scroller_id, bool prepend){
  for (auto &s : _renders){
    if (!scroller_id || (s->getID() == scroller_id)){
      s->enqueueMSG(msg, prepend);
      return;
    }
  }
}

void ModTextDisplay::enqueueMSG(TextMessage&& msg, uint8_t scroller_id, bool prepend){
  for (auto &s : _renders){
    if (!scroller_id || (s->getID() == scroller_id)){
      s->enqueueMSG(std::move(msg), prepend);
      return;
    }
  }
}

void ModTextDisplay::updateMSG(const TextMessage& msg, uint8_t scroller_id, bool enqueue){
  for (auto &s : _renders){
    if (!scroller_id || (s->getID() == scroller_id)){
      s->updateMSG(msg, enqueue);
      return;
    }
  }
}

void ModTextDisplay::mkEmbUIpage(Interface *interf, JsonVariantConst data, const char* action){
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

void ModTextDisplay::set_generic_options(Interface *interf, JsonVariantConst data, const char* action){

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

void ModTextDisplay::rm_instance(Interface *interf, JsonVariantConst data, const char* action){
  int id = data;
  if (id < 1)
    return;

  _kill_instance(id);

  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  bool save = false;
  JsonArray queues = doc[T_scrollers];
  for (JsonArray::iterator it=queues.begin(); it!=queues.end(); ++it) {
    if ((*it)[T_stream_id] == id) {
      queues.remove(it);
      save = true;
      LOGD(T_txtscroll, printf, "erase stream: %d\n", id);
    }
  }
  // save file
  if (save)
    embuifs::serialize2file(doc, mkFileName().c_str());

  // load main module's page
  mkEmbUIpage(interf, {}, NULL);
}

void ModTextDisplay::set_instance(Interface *interf, JsonVariantConst data, const char* action){
  if (data[T_stream_id].isNull()) return;

  uint8_t stream_id = data[T_stream_id];
  bool active = data[T_active];

  // kill existing instance, if exist, I'll either spawn a new one with updated config or leave it dead
  _kill_instance(stream_id);

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
    embuifs::obj_merge(o, data);    // merge with saved messages

  embuifs::serialize2file(doc, mkFileName().c_str());

  if (active){
    // spawn new instance with updated config
    LOGI(T_txtscroll, printf, "spawn scroller: %u", stream_id);
    _spawn_instance(data, doc[T_profiles][data[T_profile].as<unsigned>()][T_cfg]);
  }

  // load main module's page
  mkEmbUIpage(interf, {}, NULL);
}

void ModTextDisplay::_kill_instance(uint8_t stream_id){
  // remove spawned instances
  std::erase_if(_renders, [stream_id](const std::unique_ptr<TextQRenderer> &s){ return s->getID() == stream_id; } );
}

void ModTextDisplay::embui_send_msg(Interface *interf, JsonVariantConst data, const char* action){
  JsonVariantConst v = data[P_text];
  if (v.is<const char*>() && !v.isNull()){
    TextMessage m(v.as<const char*>(), data[T_cnt] | 1, data[T_interval], data[P_id]);
    //LOGI(T_txtscroll, printf, "Add msg:%s\n", m.msg.c_str());
    if (data[T_update])
      updateMSG(std::move(m), data[T_stream_id]);
    else
      enqueueMSG(std::move(m), data[T_stream_id], data[T_prepend]);
  }
}

void ModTextDisplay::embui_profile_apply(Interface *interf, JsonVariantConst data, const char* action){
  for (auto &s : _renders){
    if (s->getID() == data[T_stream_id].as<uint8_t>()){
      s->load_cfg(data);
      return;
    }
  }
}

void ModTextDisplay::embui_profile_save(Interface *interf, JsonVariantConst data, const char* action){
  uint8_t stream_id = data[T_stream_id];
  if (!stream_id) return;   // stream id must be valid

  JsonDocument doc;
  embuifs::deserializeFile(doc, mkFileName().c_str());

  JsonArray queues = doc[T_scrollers];

  JsonObject o;
  for (auto instance = queues.begin(); instance != queues.end(); ++instance ){
    if ( stream_id == (*instance)[T_stream_id].as<uint8_t>() ){
      o = *instance;
      break;
    }
  }
  if (o.isNull())
    return;
  
  embuifs::obj_merge(o, data[T_scroller]);

  uint32_t idx = o[T_profile];
  embuifs::obj_deepmerge(doc[T_profiles][idx], data[T_profile]);

  //serializeJsonPretty(doc, Serial);

  embuifs::serialize2file(doc, mkFileName().c_str());

  // apply to current instance if any
  for (auto &s : _renders){
    if (s->getID() == stream_id){
      s->load_cfg(data[T_profile][T_cfg]);
      return;
    }
  }
}
