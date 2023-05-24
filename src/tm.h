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

#ifndef _TM_H
#define _TM_H

#include "TM1637.h"
#include "config.h"

#ifndef TM_TIME_DELAY
  #define TM_TIME_DELAY 3U
#endif

#ifndef TM_BRIGHTNESS
  #define TM_BRIGHTNESS 7U //яркость дисплея, 0..7
#endif
#ifndef TM_SHOW_BANNER
  #define TM_SHOW_BANNER 0
#endif

class TMCLOCK : public TM1637 {
public:
  TMCLOCK(uint8_t clkPin, uint8_t dataPin) : TM1637 (clkPin, dataPin) {};
  uint8_t& getSetDelay();  // Задержка, для отображения с других плагинов
  void tm_setup();
  void tm_loop();
  void showIp();
  uint8_t getIpShow() {return ipShow;}
  void setIpShow()  {ipShow = 17;}
private:
  String splittedIp[5] = {};
  bool showPoints;
  uint8_t tmDelayTime;
  uint8_t ipShow;
  #if TM_SHOW_BANNER
  bool bannerShowed;
  void showBanner();
  #endif
  void switchShowPoints(){showPoints=!showPoints;};
  void splitIp(String str, String dlm, String dest[]);  // Функция разделителя по указателю
  String formatIp(String inArr[], String dlm);    // Функция форматирования
};

extern TMCLOCK *tm1637;
#endif