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

/*
  !!! ВНОСИТЬ ИЗМЕНЕНИЯ В ЭТОЙ ФАЙЛ НЕЛЬЗЯ !!!
  скопируйте "user_config.h.default" в "user_config.h" и вносите изменения в файл-копию

  !!! NEVER EVER CHANGE THIS FILE !!!
  copy "user_config.h.default" to "user_config.h" and adjust to your needs
*/


#pragma once
#if defined CUSTOM_CFG
# include CUSTOM_CFG
#else
# if defined __has_include
#  if __has_include("user_config.h")
#   include "user_config.h"
#  endif
# endif
#endif
// required for esp32 IDF macro
#include "Arduino.h"

//-----------------------------------
//#define LAMP_DEBUG                                          // режим отладки, можно также включать в platformio.ini
//-----------------------------------
#ifndef LANG_FILE
#define LANG_FILE                  "text_res-RUS.h"           // Языковой файл по дефолту
#endif

#ifndef MIC_POLLRATE
#define MIC_POLLRATE          (50U)                         // как часто опрашиваем микрофон, мс
#endif

#ifndef HIGH_MAP_FREQ
#define HIGH_MAP_FREQ         (20000U)                      // верхняя граница слышимого диапазона, используется для мапинга, дефолтное и общепринятое значение 20000Гц
#endif

#define FAST_ADC_READ                                       // использовать полный диапазон звуковых частот, если закомментировано, то будет до 5кГц, но сэкономит память и проще обсчитать...
#ifdef FAST_ADC_READ
#ifndef SAMPLING_FREQ
#define SAMPLING_FREQ         (18000U*2U)
#endif
#else
#define SAMPLING_FREQ         (5000U*2U)
#endif

#ifndef LOW_FREQ_MAP_VAL
#define LOW_FREQ_MAP_VAL      (35U)                         // Граница логарифмически приведенных значений выше которой уже идут средние частоты (микрофон) ~150Гц
#endif

#ifndef HI_FREQ_MAP_VAL
#define HI_FREQ_MAP_VAL       (188U)                        // Граница логарифмически приведенных значени выше которой уже идут высокие частоты (микрофон) ~4.5кГц
#endif

#ifndef MIN_PEAK_LEVEL
#define MIN_PEAK_LEVEL        (50U)                         // Минимальный амплитудный уровень, для эффектов зависящих от микрофона
#endif


#ifndef FADE_STEPTIME
#define FADE_STEPTIME         (50U)                         // default time between fade steps, ms
#endif
#ifndef FADE_TIME
#define FADE_TIME             (2000U)                       // Default fade time, ms
#endif
#ifndef FADE_MININCREMENT
#define FADE_MININCREMENT     (2U)                          // Minimal increment for fading steps
#endif
#ifndef FADE_LOWBRTFRACT
#define FADE_LOWBRTFRACT      (10U)                         // доля от максимальной яркости, до которой работает затухание при смене эффектов. Если текущая яркость ниже двойной доли, то затухание будет достигать нуля
#endif

#ifndef MAX_FPS
#define MAX_FPS               (50U)                         // Максимальное число обсчитываемых и выводимых кадров в секунду
#endif

#define EFFECTS_RUN_TIMER   (uint16_t)(1000 / MAX_FPS)     // период обработки эффектов - при 10 это 10мс, т.е. 1000/10 = 100 раз в секунду, при 20 = 50 раз в секунду, желательно использовать диапазон 10...40

#ifndef DEFAULT_DEMO_TIMER
  #define DEFAULT_DEMO_TIMER  (60U)                         // интервал смены демо по-умолчанию
#endif


#ifndef CFG_AUTOSAVE_TIMEOUT
#define CFG_AUTOSAVE_TIMEOUT       (60*1000U)                   // таймаут сохранения конфигурации эффекта, по умолчанию - 60 секунд
#endif
