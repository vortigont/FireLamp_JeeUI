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

#include "mp3player.h"
#include "char_const.h"
#include "EmbUI.h"           // EmbUI's header
#include "nvs_handle.hpp"
#include "log.h"

// which serial to use for esp32
#define MP3_SERIAL_SPEED    9600  //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  350   //average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

#define MP3_EFF_FOLDER            3     // folder with effects track
#define MP3_NOTIFICATION_FOLDER   1     // folder with cuckoo and alarm files

#define MP3_LOOPTRACK_CMD_DELAY 5000

#define MP3_CUCKOO_FILES_INCREMENT  10    // file names increment
#define MP3_ALARM_DURATION          60    // how long to repeat alarm playback (sec)


MP3PlayerController::MP3PlayerController(HardwareSerial& serial, DfMp3Type type, uint32_t ackTimeout) : _serial(serial) {
  dfp = new DFMiniMp3(serial, type, ackTimeout);
}

MP3PlayerController::~MP3PlayerController(){
  unsubscribe();
  dfp->stop();
  delete dfp; dfp = nullptr;
}


void MP3PlayerController::begin(int8_t rxPin, int8_t txPin){
  LOGD(T_DFPlayer, printf, "Initializing at rx:%d, tx:%d", rxPin, txPin);
  _serial.begin(MP3_SERIAL_SPEED, SERIAL_8N1, rxPin, txPin);

  dfp->onPlayFinished( [this](DfMp3_PlaySources source, uint16_t track){
    LOGI(T_DFPlayer, printf, "playback end #%u\n", track);
    // ignore "play end event if loop is active"
    if (!flags.looptrack)
      _state = DfMp3_StatusState_Idle;
  } );

  dfp->onPlaySource( [this](DfMp3_SourceEvent event, DfMp3_PlaySources source)
    {
      LOGI(T_DFPlayer, printf, "on-line: %u\n", source);
      // set vol on first online event
      if (!flags.ready){
        flags.ready = true;
        dfp->setVolume(_volume);
      }
      _state = DfMp3_StatusState_Idle;
    }
  );

  dfp->onError( [](uint16_t errorCode){  LOGW(T_DFPlayer, printf, "Error: %u\n", errorCode); } );


  // event bus subsribe
  if (!_lmp_ch_events)
    subscribe();

  // restore opts state from NVS
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READONLY, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->get_item(T_mp3vol, _volume);
    handle->get_item(T_mp3mute, _mute);
  }

  // event poller
  _tPeriodic.set(100, TASK_FOREVER, [this](){ loop(); });

  ts.addTask(_tPeriodic);
  _tPeriodic.enableDelayed();


  // this will (probably) make a player to reply with state packet and we can understand that it's on-line
  //dfp->getTotalTrackCount();
  Task* t = new (std::nothrow) Task(TASK_SECOND, TASK_ONCE, [this](){ dfp->reset(); }, &ts, false, nullptr, nullptr, true);

  if (t)
    t->enableDelayed();

}

// this method will recreate MP3Player object
void MP3PlayerController::begin(int8_t rxPin, int8_t txPin, DfMp3Type type, uint32_t ackTimeout){
  if (dfp) delete dfp;
  dfp = new DFMiniMp3(_serial, type, ackTimeout);
  begin(rxPin, txPin);
}

void MP3PlayerController::loop(){
  dfp->loop();
}

void MP3PlayerController::subscribe(){

  // Register the handler for change notification events; need to pass instance handle for later unregistration.
  if (!_lmp_ch_events)
    esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, MP3PlayerController::event_hndlr, this, &_lmp_ch_events);

  if (!_lmp_set_events)
    esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, MP3PlayerController::event_hndlr, this, &_lmp_set_events);

  if (!_lmp_get_events){
    esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_GET_EVENTS, e2int(evt::lamp_t::mp3state), MP3PlayerController::event_hndlr, this, &_lmp_get_events);
  }
}

void MP3PlayerController::unsubscribe(){
  if (_lmp_ch_events){
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _lmp_ch_events));
    _lmp_ch_events = nullptr;
  }

  if (_lmp_set_events){
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, _lmp_set_events));
    _lmp_set_events = nullptr;
  }

  if (_lmp_get_events){
    esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_GET_EVENTS, e2int(evt::lamp_t::mp3state), _lmp_set_events);
    _lmp_get_events = nullptr;
  }
};

void MP3PlayerController::event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  LOGD(T_DFPlayer, printf, "EVENT %s:%d\n", base, id);
  if ( base == LAMP_CHANGE_EVENTS )
    return static_cast<MP3PlayerController*>(handler)->_lmpChEventHandler(base, id, event_data);

  if ( base == LAMP_SET_EVENTS )
    return static_cast<MP3PlayerController*>(handler)->_lmpSetEventHandler(base, id, event_data);

  if ( base == LAMP_GET_EVENTS )
    return static_cast<MP3PlayerController*>(handler)->_lmpGetEventHandler(base, id, event_data);
}

void MP3PlayerController::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Power control
/*
    case evt::lamp_t::pwron :
      LOGI(T_DFPlayer, println, T_Notification);
      dfp->playFolderTrack(2, 1);
      break;
*/
    case evt::lamp_t::pwroff :
      dfp->stop();
      //dfp->playFolderTrack(2, 1);
      break;
    case evt::lamp_t::effSwitchTo :
      if (flags.eff_playtrack)
        playEffect(*reinterpret_cast<uint32_t*>(data));
      break;
    case evt::lamp_t::cockoo :
      playTime(*reinterpret_cast<int*>(data));
      break;
    case evt::lamp_t::alarmTrigger :
      playAlarm(*reinterpret_cast<int*>(data));
      break;
    // stop alarm playback
    case evt::lamp_t::alarmStop :
      dfp->stop();
      flags.alarm = false;
      break;
    default:;
  }
}

void MP3PlayerController::_lmpSetEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Volume control
    case evt::lamp_t::mp3vol :
      setVolume(*reinterpret_cast<int32_t*>(data));
      break;
    case evt::lamp_t::mp3volstep :
      setVolume( constrain(_volume + *reinterpret_cast<int*>(data), DFPLAYER_MIN_VOL, DFPLAYER_MAX_VOL) );
      break;
    case evt::lamp_t::mp3mute :
      setSilent(true);
      break;
    case evt::lamp_t::mp3unmute :
      setSilent(false);
      break;
    default:;
  }
}

void MP3PlayerController::_lmpGetEventHandler(esp_event_base_t base, int32_t id, void* data){
  // here I simply publish current player state
  Interface interf(&embui.feeders);
  interf.json_frame_value();
  interf.value(T_mp3vol, _volume);
  interf.value(T_mp3mute, _mute);
  interf.json_frame_flush();
}

void MP3PlayerController::playTime(int track){
  // do not play anything if player is on Mute
  if (_mute) return;

  // check for talking clock
  if (track == 1){
    std::time_t now;
    std::time(&now);
    std::tm *tm = std::localtime(&now);
    track = 100*tm->tm_hour + tm->tm_min;   // формируем индекс файла

    if( dfp->getStatus().state == DfMp3_StatusState_Playing ){
      dfp->playAdvertisement(track);
      LOGD(T_DFPlayer, printf, "adv time:%d\n", track);
    } else {
      flags.looptrack  = false;
      dfp->setRepeatPlayCurrentTrack(false);
      dfp->playFolderTrack16(0, track);
      LOGD(T_DFPlayer, printf, "play time:%d\n", track);
    }
    return;
  }

  // играть "кукушки" только если плеер простаивает
  if( dfp->getStatus().state == DfMp3_StatusState_Idle ){
    dfp->playFolderTrack16(1, track + MP3_CUCKOO_FILES_INCREMENT);
    LOGD(T_DFPlayer, printf, "play cockoo:%d\n", track + MP3_CUCKOO_FILES_INCREMENT);
  }
}

void MP3PlayerController::playEffect(uint32_t effnb){
  // do not play anything if player is on Mute
  if (_mute){
   return;
   LOGD(T_DFPlayer, println, "suppress play due to mute flag");
  }
  LOGI(T_DFPlayer, printf, "effect: folder:%u, track:%u, effnb:%u\n", MP3_EFF_FOLDER, effnb%256, effnb);
  dfp->playFolderTrack(MP3_EFF_FOLDER, effnb%256);
  _state = DfMp3_StatusState_Playing;
  // for looping current track player must be in play mode already playing
  // it needs a delay between starting playback and sending repeat command
  if (flags.eff_looptrack){
    _loop_current_track();
  }
}

void MP3PlayerController::playAlarm(int track){
  // will force play event if player is on Mute

  LOGI(T_DFPlayer, printf, "Alarm: folder:%u, track:%u\n", MP3_NOTIFICATION_FOLDER, track+100);
  dfp->playFolderTrack16(MP3_NOTIFICATION_FOLDER, track+100);
  _loop_current_track();
  _state = DfMp3_StatusState_Playing;
  flags.alarm = true;
  // scheduled alarm playback stop
  Task *t = new Task(MP3_ALARM_DURATION * TASK_SECOND, TASK_ONCE,
      [this](){
        if (flags.alarm){
          dfp->stop();
          flags.alarm = false;
        }
      },
      &ts, false, nullptr, nullptr, true );
  t->enableDelayed();
}

void MP3PlayerController::setVolume(int32_t vol) {
  _volume=vol;
  dfp->setVolume(vol);
  LOGI(T_DFPlayer, printf, "volume: %u\n", _volume);
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->set_item(T_mp3vol, _volume);
  }
  // post sound volume event update
  EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::mp3vol), &vol, sizeof(vol) );
}

void MP3PlayerController::setLoopEffects(bool value){
  flags.eff_looptrack = value;
  dfp->setRepeatPlayCurrentTrack(value);
  LOGI(T_DFPlayer, printf, "track loop: %u\n", value);
}

void MP3PlayerController::setPlayEffects(bool value){
  flags.eff_playtrack = value;
  if (!value)
    dfp->stop();
}

void MP3PlayerController::_loop_current_track(){
    Task *t = new Task(MP3_LOOPTRACK_CMD_DELAY, TASK_ONCE,
        [this](){
          dfp->setRepeatPlayCurrentTrack(true);
          flags.looptrack = true;
        },
        &ts, false, nullptr, nullptr, true );
    t->enableDelayed();
}

void MP3PlayerController::setSilent(bool m){
  if(_mute == m) return;
  _mute = m;

  if (_mute){
    LOGI(T_DFPlayer, println, "mute");
    dfp->stop();
    // disabling DAC works only untill next track is played, so it is useless
    // dfp->disableDac();
    _mute = true;
  } else {
    LOGI(T_DFPlayer, println, "unmute");
    //dfp->enableDac();
    _mute = false;
  }

  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle(T_lamp, NVS_READWRITE, &err);

  if (err == ESP_OK) {
    //LOGD(T_WdgtMGR, printf, "Err opening NVS handle: %s\n", esp_err_to_name(err));
    handle->set_item(T_mp3mute, _mute);
  }
}