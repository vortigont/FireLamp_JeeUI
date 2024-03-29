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

#ifndef MIC_PIN
 #ifdef ESP8266
  #define MIC_PIN               (A0)                          // ESP8266 Analog Pin ADC0 = A0
 #elif defined CONFIG_IDF_TARGET_ESP32
  #define MIC_PIN               (GPIO_NUM_34)                 // ESP32 Analog Pin
 #elif defined CONFIG_IDF_TARGET_ESP32C3
  #define MIC_PIN               (GPIO_NUM_2)                  // ESP32c3 Analog Pin
 #elif defined CONFIG_IDF_TARGET_ESP32S3                                                                                                                      
  #define MIC_PIN               (GPIO_NUM_34)                 // ESP32-S3 Analog Pin
 #endif
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

// DFPlayer
//#ifndef MP3_SERIAL_TIMEOUT
//#define MP3_SERIAL_TIMEOUT    (300U)                       // 300мс по умолчанию, диапазон 200...1000, подбирается экспериментально, не желательно сильно повышать
//#endif
//#ifndef DFPLAYER_START_DELAY
//#define DFPLAYER_START_DELAY  (500U)                       // 500мс по умолчанию, диапазон 10...1000, подбирается экспериментально, не желательно сильно повышать, безусловная задержка до инициализации
//#endif


#ifndef DS18B20_PIN
#define DS18B20_PIN           (13)                        // D7 Пин подключения датчика DS18b20. При использовании энкодара, датчик можно назначить на пин кнопки (SW) энкодера. И поставить резистор подтяжки к +3.3в.
#endif
#ifndef DS18B_READ_DELAY
#define DS18B_READ_DELAY      (10U)                       // Секунд - периодичность опроса датчика. Чаще не надо, возможно лучше реже. С учетом теплоемкости датчика, воздуха и подложки матрицы - смысла нет
#endif
#ifndef COOLER_PIN
#define COOLER_PIN            (-1)                        // Пин для управления вентилятором охлаждения лампы. (-1) если управление вентилятором не нужно. 
#endif
#ifndef COOLER_PIN_TYPE
#define COOLER_PIN_TYPE       (0U)                        // 0-дискретный вкл\выкл, 1 - ШИМ (для 4-х пиновых вентиляторов). Убедитесь, что вывод COOLER_PIN поддерживает PWM.
#endif
#ifndef COOLING_FAIL
#define COOLING_FAIL          (6U)                        // Количество циклов DS18B_READ_DELAY. Если за это время снизить температуру до TEMP_DEST (Дискретный режим выхода вентилятора), или TEMP_MAX (ШИМ),  
#endif
#ifndef CURRENT_LIMIT_STEP
#define CURRENT_LIMIT_STEP    (0U)
#endif
#ifndef TEMP_DEST
#define TEMP_DEST             (50U)
#endif



#ifndef OFF_BRIGHTNESS
#define OFF_BRIGHTNESS          (2U)                        // яркость вывода текста в случае выключенной лампы
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

#ifndef SPEED_ADJ
#define SPEED_ADJ             (1.0)                         // Поправка скорости эффектов относительно размеров метрицы.
#endif

#define EFFECTS_RUN_TIMER   (uint16_t)(1000 / MAX_FPS)     // период обработки эффектов - при 10 это 10мс, т.е. 1000/10 = 100 раз в секунду, при 20 = 50 раз в секунду, желательно использовать диапазон 10...40

#ifndef DEFAULT_DEMO_TIMER
  #define DEFAULT_DEMO_TIMER  (60U)                         // интервал смены демо по-умолчанию
#endif


// настройки времени
#ifndef HTTPTIME_SYNC_INTERVAL
 #define HTTPTIME_SYNC_INTERVAL    (4)                           // интервал синхронизации времени по http, час
#endif

#ifndef CFG_AUTOSAVE_TIMEOUT
#define CFG_AUTOSAVE_TIMEOUT       (60*1000U)                   // таймаут сохранения конфигурации эффекта, по умолчанию - 60 секунд
#endif

#ifndef TEXT_OFFSET
#define TEXT_OFFSET           (4U)                          // высота, на которой бежит текст (от низа матрицы)
#endif
#ifndef LET_WIDTH
#define LET_WIDTH             (5U)                          // ширина буквы шрифта
#endif
#ifndef LET_HEIGHT
#define LET_HEIGHT            (8U)                          // высота буквы шрифта
#endif
#ifndef LET_SPACE
#define LET_SPACE             (1U)                          // пропуск между символами (кол-во пикселей)
#endif
#ifndef LETTER_COLOR
#define LETTER_COLOR          (CRGB::White)                 // цвет букв по умолчанию
#endif
#ifndef DEFAULT_TEXT_SPEED
#define DEFAULT_TEXT_SPEED    (100U)                        // скорость движения текста, в миллисекундах - меньше == быстрее
#endif
#ifndef FADETOBLACKVALUE
#define FADETOBLACKVALUE      (222U)                        // степень затенения фона под текстом, до 255, чем больше число - тем больше затенение.
#endif

// --- РАССВЕТ -------------------------
#ifndef DAWN_BRIGHT
#define DAWN_BRIGHT           (200U)                        // максимальная яркость рассвета (0-255)
#endif
#ifndef DAWN_TIMEOUT
#define DAWN_TIMEOUT          (1U)                          // сколько рассвет светит после времени будильника, минут
#endif
//#define PRINT_ALARM_TIME                                    // нужен ли вывод времени для будильника, если пустая строка в событии будильника


#ifndef TM_BRIGHTNESS
  #define TM_BRIGHTNESS 7U //яркость дисплея, 0..7
#endif
