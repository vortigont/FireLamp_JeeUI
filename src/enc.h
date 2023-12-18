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
#include "config.h"

#include "effects_types.h"

// Опциональные настройки (показаны по умолчанию)
//#define EB_FAST 30     // таймаут быстрого поворота, мс
//#define EB_DEB 80      // дебаунс кнопки, мс
//#define EB_HOLD 1000   // таймаут удержания кнопки, мс
//#define EB_STEP 500    // период срабатывания степ, мс
//#define EB_CLICK 400   // таймаут накликивания, мс

#include "EncButton.h"

#ifndef ENC_EXIT_TIMEOUT
#define ENC_EXIT_TIMEOUT 3U   // Таймаут выхода из настроек эффекта. Приблизительно равно (EXIT_TIMEOUT * 10 секунд)
#endif

#ifndef ENC_STRING_EFFNUM_DELAY
#define ENC_STRING_EFFNUM_DELAY 17
#endif

#ifndef ENC_SW
#define ENC_SW                  (14)                        // D5 Пин кнопки стандартного энкодера
#endif
#ifndef ENC_DT
#define ENC_DT                  (5)                         // D1 Пин DT энкодера
#endif
#ifndef ENC_CLK
#define ENC_CLK                 (4)                         // D2 Пин CLK энкодера
#endif

void IRAM_ATTR isrEnc();
void interrupt();
void noInterrupt();

void isTurn();
void isClick();
void isHolded();
void myClicks();
//void myStep();
void encSetBri(int val);
void encSetEffect(int val);
void encSetDynCtrl(int val);
//void encDisplay(uint16_t value, String type = "");
//void encDisplay(float value);
//void encDisplay(String str);
void resetTimers();
void exitSettings();
void encSendString(String str, CRGB color, bool force = true, uint8_t delay = 40U);
void encSendStringNumEff(String str, CRGB color);
bool validControl(const CONTROL_TYPE ctrlCaseType);

void enc_setup(); 
void encLoop();
uint8_t getEncTxtDelay();
void setEncTxtDelay(const uint8_t speed);
CRGB getEncTxtColor();
void setEncTxtColor(const CRGB color);

void toggleDemo();
//void toggleGBright();
void toggleMic();
void toggleAUX();
void sendTime();
void sendIP();

extern EncButton<EB_CALLBACK, ENC_DT, ENC_CLK, ENC_SW> enc;
