/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

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

#include "tm1637display.hpp"
#include "char_const.h"
#include "timeProcessor.h"
#include "log.h"

// String welcome_banner = "FIRE_START"; // Список букв для вывода A Bb Cc Dd Ee F G Hh Ii J K Ll m Nn Oo P q r S t U v w x Y Z
/* Указывать можно в любом регистре, разделять лучше нижним подчеркиванием "_", если поставить пробел,
то слова разделятся и будут отображаться по очереди, например сначала заскроллится "FIRE",
дойдет до конца, потухнет и только тогда появится "START"*/

#define SCROOL_DELAY  200


TMDisplay::~TMDisplay(){
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _evt_ch_hndlr);
  esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, _evt_set_hndlr);
  WiFi.removeEvent(eid);
};

void TMDisplay::init() {
  begin();
  clearScreen();

  _wrkr.set(TASK_SECOND, TASK_FOREVER, [this](){ _loop(); });
  ts.addTask(_wrkr);
  _wrkr.enable();

  // Set WiFi event handlers
  eid = WiFi.onEvent( [this](WiFiEvent_t event, WiFiEventInfo_t info){ _onWiFiEvent(event, info); } );

  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, TMDisplay::event_hndlr, this, &_evt_ch_hndlr));
  ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, TMDisplay::event_hndlr, this, &_evt_set_hndlr));

  LOG(println, "tm1637 initialized");
}

void TMDisplay::_loop(){
  if (timer){
    // we are displaying something, wait for timeout
    --timer;
    return;
  }

  // just run the clock
  _showClock();
}

void TMDisplay::_scrool(){
  //LOG(print, "TM _scrl:"); LOG(println, (unsigned)getAnimation());

  if (getAnimation() == Animation::SCROLL_LEFT){
    getAnimator()->scrollLeft(SCROOL_DELAY, 1);
  } else {
    if (repeat){
      // repeat scroll text
      --repeat;
      getAnimator()->scrollLeft(SCROOL_DELAY, 1);
      return;
    } else {
      // end of scrolling, switch to _loop
      _wrkr.setCallback([this](){ _loop(); });
      _wrkr.setInterval(TASK_SECOND);
    }
  }
}

void TMDisplay::_showClock(){
/*
// todo fix this missing method
  if(TimeProcessor::getInstance().isDirtyTime()) {      // Светим --:--, если не подтянулось время с инета или не было настроено вручную
    auto d =  (showColon) ? DisplayDigit().setG().setDot() : DisplayDigit().setG();
    const uint8_t rawBuffer[4] = {d, d, d, d};
    displayRawBytes(rawBuffer, 4);
  } else {
*/
  const tm* t = localtime(TimeProcessor::now());
  char dispTime[6];            // Массив для сбора времени
  sprintf (dispTime,
            clk_lzero ? "%02u%s%02u" : "%u%s%02u",
            clk_12h ? t->tm_hour % 12 : t->tm_hour,
            showColon ? "." : "",
            t->tm_min);

  clk_lzero ? display(dispTime) : (( (clk_12h ? t->tm_hour%12 : t->tm_hour) < 10) ? display(dispTime, true, false, 1) : display(dispTime));
  showColon=!showColon;
}

void TMDisplay::event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
  //LOG(printf, "TMDisplay::event_hndlr %s:%d\n", base, id);
  reinterpret_cast<TMDisplay*>(handler_args)->_event_picker(base, id, event_data);
}

void TMDisplay::_event_picker(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
  // Power control
    case evt::lamp_t::pwron :
      setBrightness(brtOn);
      _addscroll(T_On);
      //display(T_On, true, true);
      //timer = 2;
      break;
    case evt::lamp_t::pwroff :
      setBrightness(brtOff);
      _addscroll(T_Off);
      //display(T_Off, true, true);
      //timer = 2;
      break;

    default:;
  }

  // pick only SET events
  if (base == LAMP_SET_EVENTS){
    switch (static_cast<evt::lamp_t>(id)){
    // Brightness control
      case evt::lamp_t::brightness_nofade :
      case evt::lamp_t::brightness : {
        String s("Br.");
        unsigned b = *((unsigned*) data);
        if (b<10) s.concat((char)0x20); // append space
        s += b;
        display(s);
        timer = 2;
        break;
      }

      default:;
    }
  }

}

void TMDisplay::_onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info){
    switch (event){
      case SYSTEM_EVENT_STA_GOT_IP: {
        String ip("IP Addr ");
        ip.concat(WiFi.localIP().toString());
        ip.replace( (char)0x2e, (char)0x6f ); // replace '.' with 'o'
        _addscroll(ip.c_str(), 1);
        break;
      }
      default:;
    }
}

void TMDisplay::_addscroll(const char* t, int rpt){
  if (getAnimation() == Animation::SCROLL_LEFT){
    getAnimator()->concat(" ");  // add space sepparator
    getAnimator()->concat(t);
  } else {
    clearScreen();
    display(t)->scrollLeft(SCROOL_DELAY, 1);
    repeat = rpt;
    _wrkr.setInterval(SCROOL_DELAY);
    _wrkr.setCallback([this](){ _scrool(); });
  }
}

void TMDisplay::brightness(uint8_t b, bool lampon){
  if (b > TM_BRIGHTNESS_MAX) b = TM_BRIGHTNESS_MAX;
  if (b == 0 && lampon) b = 1;      // make sure brightness will never be 0 for "On" mode
  lampon ? brtOn = b : brtOff = b;
  setBrightness(b);
}
