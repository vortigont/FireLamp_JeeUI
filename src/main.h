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
#ifndef __MAIN_H_
#define __MAIN_H_

#if __cplusplus >= 201703L
#define register // keyword 'register' is banned with c++17
#endif

#include "config.h"

#ifdef ESP_USE_BUTTON
#include "buttons.h"
#endif
// TM1637 class
#include "tm.h"

#ifdef ENCODER
  #include "enc.h"
#endif

#ifdef RTC
  #include "rtc.h"
#endif

#ifdef USE_STREAMING
  #include "ledStream.h"
#endif

#define LAMPFW_VERSION_MAJOR     3
#define LAMPFW_VERSION_MINOR     4
#define LAMPFW_VERSION_REVISION  0

#define LAMPFW_VERSION_VALUE     (MAJ, MIN, REV) ((MAJ) << 16 | (MIN) << 8 | (REV))

/* make version as integer for comparison */
#define LAMPFW_VERSION           LAMPFW_VERSION_VALUE(LAMPFW_VERSION_MAJOR, LAMPFW_VERSION_MINOR, LAMPFW_VERSION_REVISION)

/* make version as string, i.e. "2.6.1" */
#define LAMPFW_VERSION_STRING    TOSTRING(LAMPFW_VERSION_MAJOR) "." TOSTRING(LAMPFW_VERSION_MINOR) "." TOSTRING(LAMPFW_VERSION_REVISION)


// глобальные переменные для работы с ними в программе

#ifdef ESP_USE_BUTTON
extern Buttons *myButtons;
#endif
#ifdef MP3PLAYER
#include "mp3player.h"
extern MP3PlayerDevice *mp3;
#endif

#ifdef EMBUI_USE_MQTT
//void mqttConnect();
void sendData();
#endif

//void create_parameters();
//void sync_parameters();

#endif
