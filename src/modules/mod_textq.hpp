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

#pragma once
#include "modules/mod_manager.hpp"
#include "display.hpp"
#include "canvas/Arduino_Canvas_Mono.h"


struct TextMessage {
    std::string msg;
    /**
     * @brief display counter
     * 0 - do not display
     * -1 - display forever
     */
    int32_t cnt;
    // interval in seconds between showing message again
    int32_t interval;
    // unique message id, if 0 - then not a unique message
    uint32_t id;

    // last displayed time
    uint32_t last_displayed{0};
    TextMessage() = default;
    explicit TextMessage(const char* m, int32_t cnt = 1, int32_t interval = 0, uint32_t id = 0) : msg(m), cnt(cnt), interval(interval), id(id) {}
    explicit TextMessage(std::string&& m, int32_t cnt = 1, int32_t interval = 0, uint32_t id = 0) : msg(m), cnt(cnt), interval(interval), id(id) {}
};

class TextScroll {
  // instance id
  uint8_t _id;

  bool _active{false};
  bool _load_next{true};

  TextBitMapCfg _bitmapcfg;
  std::unique_ptr<Arduino_Canvas_Mono> _textmask;

  std::shared_ptr<TextMessage> _current_msg;
  std::list< std::shared_ptr<TextMessage> > _msg_pool;

  int _cur_offset{0};
  // px per second
  int _scrollrate;
  uint32_t _last_redraw;
  uint16_t _txt_pixlen;

  overlay_cb_t _renderer;

  std::mutex mtx;


  // hook to check/update text scroller
  void _scroll_line(LedFB_GFX *gfx);

  // load next message from queue
  bool _load_next_msg();

public:
  TextScroll();
  //TextScroll(TextScroll&& rval) = default;
  ~TextScroll(){ stop(); }

  // load font face, size, etc...
  void load_cfg(JsonVariantConst cfg);

  // load messages array
  void load_msg(JsonArrayConst msg);

  void start();
  void stop();

  // purge current message queue
  void clear();

  void setID(uint8_t id){ _id = id; }
  uint8_t getID() const { return _id; }

  /**
   * @brief enque message
   * 
   * @param msg 
   * @param id 
   */
  void enqueueMSG(const TextMessage& msg, bool prepend = false);
  void enqueueMSG(TextMessage&& msg, bool prepend = false);

  /**
   * @brief find and update message in the queue with matching message ids
   * 
   * @param msg message
   * @param append - if true and unique message is not found in the queue, then appen message to the back if the queue
   */
  void updateMSG(const TextMessage& msg, bool enqueue = true);

};

class ModTextScroller : public GenericModule {

  std::list<TextScroll> _scrollers;

  bool _wifi_events_msg;
  uint8_t _wifi_events_stream{0};
  network_event_handle_t eid{0};

  static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  void _onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override {};

  // load class configuration into JsonObject
  void load_cfg(JsonVariantConst cfg) override;


public:
  ModTextScroller();
  ~ModTextScroller();

  void start() override {};
  void stop() override {};

  /**
   * @brief enqueue new message to the scroller instance with specified ID
   * 
   * @param msg 
   * @param scrolled_id - instance ID, if 0 - then any available instance
   */
  void enqueueMSG(const TextMessage& msg, uint8_t scroller_id = 0, bool prepend = false);
  void enqueueMSG(TextMessage&& msg, uint8_t scroller_id = 0, bool prepend = false);

  /**
   * @brief for specified scroller find and update message in the queue with matching message ids
   * 
   * @param msg 
   * @param scroller_id 
   * @param enqueue 
   */
  void updateMSG(const TextMessage& msg, uint8_t scroller_id = 0, bool enqueue = true);


  // EmbUI handlers

	/**
	 * @brief Construct an EmbUI page with module's state/configuration
	 * 
	 * @param interf 
	 * @param data 
	 * @param action 
	 */
	void mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action) override;

  /**
   * @brief apply basic configuration for module
   * i.e. event generators
   * @param interf 
   * @param data 
   * @param action 
   */
	void set_generic_options(Interface *interf, JsonObjectConst data, const char* action);

  // EmbUI API - remove configured scroller instance by it's stream_id
	void rm_instance(Interface *interf, JsonObjectConst data, const char* action);

  // EmbUI API - set configuration for the runner instance by it's stream_id
	void set_instance(Interface *interf, JsonObjectConst data, const char* action);

  // EmbUI API - send message to the queue
	void embui_send_msg(Interface *interf, JsonObjectConst data, const char* action);

  // EmbUI API - apply string appearance profile
	void embui_profile_apply(Interface *interf, JsonObjectConst data, const char* action);

  // EmbUI API - save string appearance profile
	void embui_profile_save(Interface *interf, JsonObjectConst data, const char* action);

private:

  // spawn a scroller based on json config
  void _spawn_scroller(JsonObjectConst scroller, JsonObjectConst text_profile);

  // removes active scroller from a pool by it's stream id
  void _kill_scroller(uint8_t stream_id);

};
