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
#include "NetworkEvents.h"
#include "WiFiType.h"


// default time a single static text message is displayed (seconds)
#define TEXTQSTATIC_DEF_DISPLAY_TIME_SEC  5

struct TextMessage {
  std::string msg;
  /**
   * @brief display counter
   * now many times the mesasge should be displayed repetitively
   * 0 - do not display (discard message)
   * -1 - display forever
   */
  int32_t cnt;

  /**
   * @brief minimum interval between repetitive message display (in seconds)
   * 0 - repeat as soon as possible
   * 
   */
  int32_t interval;

  /**
   * @brief max duration time to display one message (in seconds)
   * could be treaded in differnet way depending on implementation class
   * 0 - default value, depending on renderer implementation
   * 
   */
  int32_t duration;

  // unique message id, if 0 - then not a unique message
  uint32_t id;

  // last displayed time (counted in millis())
  uint32_t last_displayed{0};
  TextMessage() = default;
  explicit TextMessage(const char* m, int32_t cnt = 1, int32_t interval = 0, int32_t duration = 0, uint32_t id = 0) : msg(m), cnt(cnt), interval(interval), duration(duration), id(id) {}
  explicit TextMessage(std::string&& m, int32_t cnt = 1, int32_t interval = 0, int32_t duration = 0, uint32_t id = 0) : msg(m), cnt(cnt), interval(interval), duration(duration), id(id) {}
};


/**
 * @brief A generic class that maintains a TextMessage queue and renders text messages
 * via overlay mask
 * @note this object on instantiation attaches to globaly accessible "LEDDisplay display" object instance and detaches on destruction
 * 
 */
class TextQRenderer {
  // instance id
  uint8_t _id;
  bool _active{false};

public:
  TextQRenderer();
  //TextScroll(TextScroll&& rval) = default;
  virtual ~TextQRenderer(){ stop(); }

  // load font face, size, etc...
  virtual void load_cfg(JsonVariantConst cfg);

  // load messages array
  virtual void load_msg(JsonArrayConst msg);

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
   * @param enqueue - if true and unique message is not found in the queue, then appen message to the back if the queue
   */
  void updateMSG(const TextMessage& msg, bool enqueue = true);

protected:
  // 2D overlay texture
  std::unique_ptr<Arduino_Canvas_Mono> textmask;
  // 2D overlay texture callback object
  overlay_cb_t render_cb;
  TextBitMapCfg bitmapcfg;

  std::mutex mtx;

  std::list< std::shared_ptr<TextMessage> > msg_pool;
  std::shared_ptr<TextMessage> current_msg;

  bool load_next{true};

  // hook to renderer function that draws text to display
  virtual void render(LedFB_GFX *gfx) = 0;

  // load next message from queue
  virtual bool load_next_msg();

  // checks if message display counter has not expired yet and msg must be requeued again
  // otherwise discard message
  // this method sets `load_next` flag
  void requeue_counter();
};

/**
 * @brief Class renders a scrolling text from a message pool to bitmap overlay
 */
class TextQScroller : public TextQRenderer {

public:
  TextQScroller() = default;

  void load_cfg(JsonVariantConst cfg) override;

private:
  // full length of a text in pixels if drawn on canvas
  uint16_t _txt_pixlen;
  int _cur_offset{0};
  // px per second
  int _scrollrate;
  // last time text was redrawn to canvas
  uint32_t _last_redraw;

  void render(LedFB_GFX *gfx) override;
  bool load_next_msg() override;
};

/**
 * @brief Class renders a scrolling text from a message pool to bitmap overlay
 * how it treats TextMessage options:
 * cnt - as-is - how many times to repeat displaying text
 * interval - as-is - min time between same message redisplayed
 * duration - 0 treated as default min display time of TEXTQSTATIC_DEF_DISPLAY_TIME_SEC seconds
 * 
 * alignment options:
 * halign - horizontal alignment of text on canvas, <0 - from left border, 0 - center, >0 - offset from right border
 * valign - vertical alignment of text on canvas, <0 - offset from top, 0 - center, >0 - offset from bottom
 */
class TextQStatic : public TextQRenderer {

public:
  TextQStatic() = default;

  void load_cfg(JsonVariantConst cfg) override;

private:
  // text alignment in on a canvas
  int32_t _h_align, _v_align;
  // text string width,height if printed on canvas
  uint16_t _w, _h;

  void render(LedFB_GFX *gfx) override;
  bool load_next_msg() override;

  void _draw_text_on_canvas();
};


/**
 * @brief Pluggable module - holds a container for various text queue renderers
 * manages renderers and it's configs
 * @note the object IDs for TextQRenderer instances are:
 * 0 - for TextQScroller (default)
 * 1 - for TextQStatic
 */
class ModTextDisplay : public GenericModule {

  std::list< std::unique_ptr<TextQRenderer> > _renders;

  bool _wifi_events_msg;
  uint8_t _wifi_events_stream{0};
  network_event_handle_t eid{0};

  static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  void _onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override {};

  // load class configuration from a JsonObject
  void load_cfg(JsonVariantConst cfg) override;


public:
  ModTextDisplay();
  ~ModTextDisplay();

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
	void mkEmbUIpage(Interface *interf, JsonVariantConst data, const char* action) override;

  /**
   * @brief apply basic configuration for module
   * i.e. event generators
   * @param interf 
   * @param data 
   * @param action 
   */
	void set_generic_options(Interface *interf, JsonVariantConst data, const char* action);

  // EmbUI API - remove configured scroller instance by it's stream_id
	void rm_instance(Interface *interf, JsonVariantConst data, const char* action);

  // EmbUI API - set configuration for the runner instance by it's stream_id
	void set_instance(Interface *interf, JsonVariantConst data, const char* action);

  // EmbUI API - send message to the queue
	void embui_send_msg(Interface *interf, JsonVariantConst data, const char* action);

  // EmbUI API - apply string appearance profile
	void embui_profile_apply(Interface *interf, JsonVariantConst data, const char* action);

  // EmbUI API - save string appearance profile
	void embui_profile_save(Interface *interf, JsonVariantConst data, const char* action);

private:

  // spawn a scroller based on json config
  void _spawn_instance(JsonObjectConst config, JsonObjectConst text_profile);

  // removes active scroller from a pool by it's stream id
  void _kill_instance(uint8_t stream_id);

};
