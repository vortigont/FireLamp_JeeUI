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
#include "config.h"
#include "mp3player.h"
#include "log.h"
#include "char_const.h"

// which serial to use for esp32
#define MP3_SERIAL_SPEED    9600  //DFPlayer Mini suport only 9600-baud
#define MP3_SERIAL_TIMEOUT  350   //average DFPlayer response timeout 200msec..300msec for YX5200/AAxxxx chip & 350msec..500msec for GD3200B/MH2024K chip

#define MP3_EFF_FOLDER       3     // folder with effects track

#define MP3_LOOPTRACK_CMD_DELAY 5000

MP3PlayerController::MP3PlayerController(HardwareSerial& serial, DfMp3Type type, uint32_t ackTimeout) : _serial(serial) {
  dfp = new DFMiniMp3(serial, type, ackTimeout);
}

MP3PlayerController::~MP3PlayerController(){
  unsubscribe();
  dfp->stop();
  delete dfp; dfp = nullptr;
}


void MP3PlayerController::begin(int8_t rxPin, int8_t txPin){
  LOGD(T_DFPlayer, println, "Initializing...");
  _serial.begin(MP3_SERIAL_SPEED, SERIAL_8N1, rxPin, txPin);

  // event poller
  _tPeriodic.set(100, TASK_FOREVER, [this](){
      loop();
    });

  ts.addTask(_tPeriodic);
  _tPeriodic.enableDelayed();

  // event bus subsribe
  subscribe();

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

  // this will (probably) make a player to reply with state packet and we can understand that it's on-line
  //dfp->getTotalTrackCount();
  dfp->reset();
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

  // Register the handler for task iteration event; need to pass instance handle for later unregistration.
  if (!_lmp_ch_instance)
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, MP3PlayerController::event_hndlr, this, &_lmp_ch_instance));

  if (!_lmp_set_instance)
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, MP3PlayerController::event_hndlr, this, &_lmp_set_instance));

  //ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), EBTN_EVENTS, ESP_EVENT_ANY_ID, ButtonEventHandler::event_hndlr, this, &_btn_einstance));
}

void MP3PlayerController::unsubscribe(){
  if (_lmp_ch_instance){
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, _lmp_ch_instance));
    _lmp_ch_instance = nullptr;
  }

  if (_lmp_set_instance){
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(evt::get_hndlr(), LAMP_SET_EVENTS, ESP_EVENT_ANY_ID, _lmp_set_instance));
    _lmp_set_instance = nullptr;
  }

  //ESP_ERROR_CHECK(esp_event_handler_instance_unregister_with(_loop, EBTN_EVENTS, ESP_EVENT_ANY_ID, _btn_einstance));
};

void MP3PlayerController::event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data){
  LOGD(T_DFPlayer, printf, "EVENT %s:%d\n", base, id);
  if ( base == LAMP_CHANGE_EVENTS )
    return static_cast<MP3PlayerController*>(handler)->_lmpChEventHandler(base, id, event_data);

  if ( base == LAMP_SET_EVENTS )
    return static_cast<MP3PlayerController*>(handler)->_lmpSetEventHandler(base, id, event_data);
}

void MP3PlayerController::_lmpChEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Power control
/*
    case evt::lamp_t::pwron :
      LOGI(T_DFPlayer, println, T_Notification);
      dfp->playFolderTrack(2, 1);
      break;
    case evt::lamp_t::pwroff :
      LOGI(T_DFPlayer, println, T_Notification);
      dfp->playFolderTrack(2, 1);
      break;
*/
    case evt::lamp_t::effSwitchTo :
      if (flags.eff_playtrack)
        playEffect(*reinterpret_cast<uint32_t*>(data));
      break;
  }
}

void MP3PlayerController::_lmpSetEventHandler(esp_event_base_t base, int32_t id, void* data){
  switch (static_cast<evt::lamp_t>(id)){
    // Volume control
    case evt::lamp_t::mp3vol :
      LOGI(T_DFPlayer, println, T_mp3vol);
      setVolume(*reinterpret_cast<int*>(data));
      break;
    case evt::lamp_t::mp3mute :
      LOGI(T_DFPlayer, println, "mute");
      dfp->disableDac();
      flags.mute = true;
      break;
    case evt::lamp_t::mp3unmute :
      LOGI(T_DFPlayer, println, "unmute");
      dfp->enableDac();
      flags.mute = true;
      break;
  }
}


/*
void MP3PlayerController::restartSound(){
  isplayname = false;
  int currentState = dfp->getStatus();
  LOG(printf_P,PSTR("DFplayer: readState()=%d, mp3mode=%d, alarm=%d\n"), currentState, mp3mode, alarm);
  if(currentState == 512 || currentState == -1 || currentState == 0){ // странное поведение, попытка фикса https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/page-312#post-75394
    Task *_t = new Task(
        200,
        TASK_ONCE, [this](){
          if(isOn() || (ready && alarm)){
            if(alarm){
              ReStartAlarmSound((ALARM_SOUND_TYPE)tAlarm);
            } else if(!mp3mode && effectmode){
              if(cur_effnb>0)
                playEffect(cur_effnb, soundfile); // начать повтороное воспроизведение в эффекте
            } else if(mp3mode) {
              cur_effnb++;
              if(cur_effnb>mp3filescount)
                cur_effnb=1;
              playMp3Folder(cur_effnb);
            }
          }
          isplaying = true; },
        &ts, false, nullptr, nullptr, true);
    _t->enableDelayed();
  }
}
*/
/*
void MP3PlayerController::printSatusDetail(){
  uint8_t type = readType();
  int value = read();

  switch (type) {
    case TimeOut:
      LOG(println, "DFplayer: Time Out!");
      if(isAlarm()){
        isplaying = false;
        restartSound();
      }
      break;
    case WrongStack:
      LOG(println, "DFplayer: Stack Wrong!");
      break;
    case DFPlayerCardInserted:
      LOG(println, "DFplayer: Card Inserted!");
      ready = true;
      setVolume(cur_volume); // в случае перетыкания карты или сборса плеера - восстановим громкость
      break;
    case DFPlayerCardRemoved:
      LOG(println, "DFplayer: Card Removed!");
      ready = false;
      break;
    case DFPlayerCardOnline:
      LOG(println, "DFplayer: Card Online!");
      setVolume(cur_volume); // в случае перетыкания карты или сборса плеера - восстановим громкость
      break;
    //case DFPlayerFeedBack:  // этот кейс добавлен для нормальной работы с некоторыми версиями DFPlayer - поправлено в библиотеке, требуется проверка
    case DFPlayerPlayFinished:
     {
        LOG(printf_P, PSTR("DFplayer: Number: %d Play Finished!\n"), value);
        if(restartTimeout+5000<millis() && !isadvert){ // c момента инициализации таймаута прошло более 5 секунд (нужно чтобы не прерывало вывод времени в режиме без звука)
          isplaying = false;
          if(!iscancelrestart)
            restartSound();
          else {
            iscancelrestart = false;
            restartTimeout = millis();
          }
        }
      }
      break;
    case DFPlayerError:
      LOG(print, "DFPlayerError:");
      switch (value) {
        case Busy:
          LOG(println, "Card not found");
          break;
        case Sleeping:
          LOG(println, "Sleeping");
          break;
        case SerialWrongStack:
          LOG(println, "Get Wrong Stack");
          break;
        case CheckSumNotMatch:
          LOG(println, "Check Sum Not Match");
          break;
        case FileIndexOut:
          LOG(println, "File Index Out of Bound");
          break;
        case FileMismatch:
          LOG(println, "Cannot Find File");
          if(isplayname) // только для случая когда нет файла с именем эффекта, если нет самой озвучки эффекта, то не рестартуем
            restartSound();
          isplaying = false;
          break;
        case Advertise:
          LOG(println, "In Advertise");
          isplaying = false;
          isadvert = false;
          // возникла ошибка с минутами или будильником, попробуем еще раз
          if((restartTimeout+10000<millis() && timeSoundType == TIME_SOUND_TYPE::TS_VER1) || isAlarm()){ // c момента инициализации таймаута прошло более 10 секунд, избавляюсь от зацикливания попыток
            restartTimeout=millis();
            if(isAlarm())
              restartSound();
            if(timeSoundType == TIME_SOUND_TYPE::TS_VER1 && nextAdv){
              Task *_t = new Task(
                  2.5 * TASK_SECOND + 300,
                  TASK_ONCE, [this](){ playAdvertise(nextAdv); },
                  &ts, false, nullptr, nullptr, true);
              _t->enableDelayed();
            }
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
*/

void MP3PlayerController::playTime(int hours, int minutes){
  // do not play anything if player is on Mute
  if (flags.mute) return;

  if( dfp->getStatus().state == DfMp3_StatusState_Playing ){
    dfp->playAdvertisement(100*hours+minutes);
  } else {
    flags.looptrack  = false;
    dfp->playFolderTrack16(0, 100*hours+minutes);
  }
}

void MP3PlayerController::playEffect(uint32_t effnb){
  // do not play anything if player is on Mute
  if (flags.mute){
   return;
   LOGD(T_DFPlayer, println, "suppress play due to mute flag");
  }
  LOGI(T_DFPlayer, printf, "effect folder:%u, track:%u, effnb:%u\n", MP3_EFF_FOLDER, effnb%256, effnb);
  dfp->playFolderTrack(MP3_EFF_FOLDER, effnb%256);
  _state = DfMp3_StatusState_Playing;
  // for looping current track player must be in play mode already playing
  // it needs a delay between starting playback and sending repeat command
  if (flags.eff_looptrack){
    Task *t = new Task(MP3_LOOPTRACK_CMD_DELAY, TASK_ONCE,
        [this](){
          dfp->setRepeatPlayCurrentTrack(true);
          flags.looptrack = true;
        },
        &ts, false, nullptr, nullptr, true );
    t->enableDelayed();
  }
}

void MP3PlayerController::setVolume(uint8_t vol) {
  _volume=vol;
  dfp->setVolume(vol);
}


/*
void MP3PlayerController::playFolder0(int filenb) {
  LOG(printf_P, PSTR("DFplayer: playLargeFolder filenb: %d\n"), filenb);
  playLargeFolder(0x00, filenb);
}

void MP3PlayerController::playAdvertise(int filenb) {
  LOG(printf_P, PSTR("DFplayer: Advertise filenb: %d\n"), filenb);
  advertise(filenb);
  isadvert = true;
  Task *_t = new Task(
      6.66 * TASK_SECOND,
      TASK_ONCE, [this](){
        LOG(println, "DFplayer: isadvert = false");
        isadvert = false; // через 6.66 секунд снимим флаг, шаманство!
      },
      &ts, false, nullptr, nullptr, true);
  _t->enableDelayed();
}




void MP3PlayerController::playName(uint16_t effnb)
{
  isplayname = true;
  LOG(printf_P, PSTR("DFplayer: playName, effnb:%d\n"), effnb%256);
  playFolder(2, effnb%256);
}

void MP3PlayerController::StartAlarmSoundAtVol(ALARM_SOUND_TYPE val, uint8_t vol){
  LOG(printf_P, PSTR("DFplayer: StartAlarmSoundAtVol at %d\n"), vol);
  setTempVolume(vol);
  tAlarm = val;
  Task *_t = new Task(300, TASK_ONCE, nullptr, &ts, false, nullptr, [this](){
    ReStartAlarmSound((ALARM_SOUND_TYPE)tAlarm);
  }, true);
  _t->enableDelayed();
}

void MP3PlayerController::ReStartAlarmSound(ALARM_SOUND_TYPE val){
  isplaying = true;
  LOG(printf_P, PSTR("DFplayer: ReStartAlarmSound %d\n"), val);
  switch(val){
    case ALARM_SOUND_TYPE::AT_FIRST :
      playFolder(1,1);
      break;
    case ALARM_SOUND_TYPE::AT_SECOND :
      playFolder(1,2);
      break;
    case ALARM_SOUND_TYPE::AT_THIRD :
      playFolder(1,3);
      break;
    case ALARM_SOUND_TYPE::AT_FOURTH :
      playFolder(1,4);
      break;
    case ALARM_SOUND_TYPE::AT_FIFTH :
      playFolder(1,5);
      break;
    case ALARM_SOUND_TYPE::AT_RANDOM : {
      randomSeed(millis());
      int soundfile = random(5)+1;
      LOG(printf_P, PSTR("DFplayer: Random alarm %d\n"), soundfile);
      playFolder(1,soundfile);
      break;
    }
    case ALARM_SOUND_TYPE::AT_RANDOMMP3 : {
      randomSeed(millis());
      int soundfile = random(mp3filescount)+1;
      LOG(printf_P, PSTR("DFplayer: Random alarm %d\n"), soundfile);
      playMp3Folder(soundfile);
      break;
    }
    default:
    break;
  }
}

void MP3PlayerController::setTempVolume(uint8_t vol) {
  if(ready){
    int tcnt = 5;
    do {
      tcnt--;
      if(readVolume()!=vol)
        volume(vol);
    } while(!readType() && tcnt);
  }
  LOG(printf_P, PSTR("DFplayer: Set volume: %d\n"), vol);
}

void MP3PlayerController::setIsOn(bool val, bool forcePlay) {
  on = val;

  if(!forcePlay){
    iscancelrestart = true;
    restartTimeout = millis();
  }

  if(!on){
    stop();
    isplaying = false;
    iscancelrestart = true;
    restartTimeout = millis();
  } else if(forcePlay && (effectmode || mp3mode))
    playEffect(cur_effnb, soundfile);

  if(tPeriodic && on)
    return;

  if (!on){
    delete tPeriodic;
    tPeriodic = nullptr;
    return;
  }
  
    tPeriodic = new Task(1.21 * TASK_SECOND, TASK_FOREVER, std::bind(&MP3PlayerController::handle,this), &ts, false, nullptr, nullptr, true); // "ленивый" опрос - раз в 1.21 сек (стараюсь избежать пересеченией с произнесением времени)
    tPeriodic->enableDelayed();
}
*/