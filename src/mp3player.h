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
#pragma once

#include "HardwareSerial.h"
#include "DFMiniMp3.h"
#include "ts.h"
#include "evtloop.h"

#define MP3_SERIAL Serial1
#define DFPLAYER_DEFAULT_VOL  12
#define DFPLAYER_JSON_CFG_JSIZE 4096


class MP3PlayerController {

  struct Flags {
    bool ready:1;                 // закончилась ли инициализация
    bool eff_playtrack:1;         // режим проигрывания треков эффектов
    bool eff_looptrack:1;         // зацикливать дорожку эффекта
    bool mute:1;                  // Player's DAC disabled
    bool isplaying:1;             // воспроизводится ли сейчас песня или эффект
    bool looptrack:1;             // if current track is looped (cmd has been sent to player)
    bool alarm:1;                 // alarm sound is playing
  };

    Flags flags{};

    HardwareSerial& _serial;
    Task _tPeriodic; // периодический опрос плеера
    uint8_t _volume = DFPLAYER_DEFAULT_VOL;
    DfMp3_StatusState _state = DfMp3_StatusState_Idle;

  esp_event_handler_instance_t _lmp_ch_instance = nullptr;
  esp_event_handler_instance_t _lmp_set_instance = nullptr;

  static void event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  // change events handler
  void _lmpChEventHandler(esp_event_base_t base, int32_t id, void* data);

  // set events handler

  void _lmpSetEventHandler(esp_event_base_t base, int32_t id, void* data);

  /**
   * @brief initialze player instance
   * 
   */
  void init();

  /**
   * @brief send a delayed command to loop currently played track
   * 
   */
  void _loop_current_track();

public:
  MP3PlayerController(HardwareSerial& serial, DfMp3Type type = DfMp3Type::origin, uint32_t ackTimeout = DF_ACK_TIMEOUT);
  // d-tor
  ~MP3PlayerController();

  // Player instance
  DFMiniMp3 *dfp = nullptr;

  /**
   * @brief initialize player
   * 
   */
  void begin(int8_t rxPin, int8_t txPin, DfMp3Type type, uint32_t ackTimeout = DF_ACK_TIMEOUT);

  void begin(int8_t rxPin, int8_t txPin);


  void subscribe();

  void unsubscribe();

  void loop();

  // play/advertise current time
  void playTime(int track);

  bool isReady(){ return flags.ready; }

  /**
   * @brief play effect melody
   * 
   * @param effnb 
   */
  void playEffect(uint32_t effnb);

  /**
   * @brief play alarm melody
   * 
   * @param track 
   */
  void playAlarm(int track);

  /**
   * @brief set/unset playing effect sounds
   * 
   * @param value 
   */
  void setPlayEffects(bool value);

  /**
   * @brief Set/uset loop effect's track
   * otherwise player will play next track on end
   * 
   * @param value 
   */
  void setLoopEffects(bool value);

  uint8_t getVolume() const { return _volume; }
  void setVolume(uint8_t vol);
};
