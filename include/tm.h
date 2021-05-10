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

#ifdef TM1637_CLOCK
#include "TM1637.h"
#include "misc.h"
#include "main.h"
#include "config.h"
#include "enc.h"

#ifndef TM_TIME_DELAY
  #define TM_TIME_DELAY 3U
#endif


static TM1637 tm1637(TM_CLK_PIN, TM_DIO_PIN);


extern void tm_setup();
extern void tm_loop();
extern void tm_setted();  // Проверка, установлено ли время
extern void splitIp(String str, String dlm, String dest[]);  // Функция разделителя по указателю
extern String formatIp(String inArr[], String dlm);    // Функция форматирования
uint8_t& getSetDelay();  // Задержка, для отображения с других плагинов
void tmDisplay(String val, bool over = true, bool pd = false, uint8_t offst = 0); // Функция для вывода данных с других плагинов
void tmDisplay(int val, bool over = true, bool pd = false, uint8_t offst = 0);
void tmDisplay(float val, bool over = true, bool pd = false, uint8_t offst = 0);

#endif
#endif
