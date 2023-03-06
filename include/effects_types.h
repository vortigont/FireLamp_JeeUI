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

#ifndef __EFFECTS_TYPES_H
#define __EFFECTS_TYPES_H

#include "config.h"
#include <FastLED.h>
#include LANG_FILE                //"text_res.h"
#include "color_palette.h"

#define SF(s) __SFSTR(s)
#define __SFSTR(s) #s
//#define SF(...) #__VA_ARGS__


/*
 * Aurora: https://github.com/pixelmatix/aurora
 * Copyright (c) 2014 Jason Coon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static const char PGidxtemplate[] PROGMEM  = "%s{\"n\":%d,\"f\":%d}";

/**
 * типы/определения для палитр
 */
typedef const TProgmemRGBPalette16 PGMPalette;

// все установленные биты для EFFFLAGS
#define SET_ALL_EFFFLAGS 3

typedef enum : uint8_t {
EFF_NONE = (0U),                              // Специальный служебный эффект, не комментировать и индекс не менять константу!
EFF_WHITE_COLOR,                              // Белый свет
EFF_COLORS,                                   // Смена цвета
EFF_RAINBOW_2D,                               // Радуга универсальная
EFF_SPARKLES,                                 // Конфетти
EFF_PILE,                                     // Цветные Драже
EFF_SNOWSTORMSTARFALL,                        // Метель + Звездопад
EFF_MATRIX,                                   // Матрица
EFF_WATERCOLORS,                              // Акварель
EFF_LIGHTER_TRACES,                           // Светлячки со шлейфом
// 10
EFF_CUBE,                                     // Блуждающий кубик
EFF_PULSE,                                    // Пульс
EFF_EVERYTHINGFALL,                           // Водо/огне/лава/радуга/хренопад
EFF_FIRE,                                     // Огонь
EFF_PAINTBALL,                                // Пейнтбол
EFF_3DNOISE,                                  // 3D-шум
EFF_CELL,                                     // Клеточки
EFF_PUZZLES,                                  // Пятнашки
EFF_T_LEND,                                   // Тикси Ленд
EFF_SPBALS,                                   // Н.Тесла
//20
EFF_OSCIL,                                    // Осцилятор 
EFF_WRAIN,                                    // Шторм 
EFF_FAIRY,                                    // Фея
EFF_FOUNT,                                    // Источник
EFF_BBALS,                                    // Прыгающие мячики
EFF_SINUSOID3,                                // Синусоид
EFF_METABALLS,                                // Метасферы
EFF_SPIRO,                                    // Спираль
EFF_COMET,                                    // Кометы, Парящие огни +
EFF_CIRCLES,                                  // Бульбулятор
//30
EFF_PRIZMATA,                                 // Призмата
EFF_FLOCK,                                    // Стая и Хищник
EFF_SWIRL,                                    // Водоворот
EFF_DRIFT,                                    // Дрифт
EFF_POPCORN,                                  // Попкорн
EFF_TWINKLES,                                 // Мерцание
EFF_RADAR,                                    // Радар
EFF_WAVES,                                    // Волны
EFF_FIRE2012,                                 // Огонь 2012
EFF_BENGALL,                                  // Бенгальские Огни
//40
EFF_BALLS,                                    // Шары
EFF_DNA,                                      // ДНК
EFF_FIRE2018,                                 // Огонь 2018
EFF_RINGS,                                    // Кодовый замок
EFF_CUBE2,                                    // Куб 2D
EFF_MAGMA,                                    // Магма
EFF_PICASSO,                                  // Пикассо
EFF_STARSHIPS,                                // Звездный Десант
EFF_FLAGS,                                    // Флаги
EFF_LEAPERS,                                  // Прыгуны
//50
EFF_WHIRL,                                    // Вихри
EFF_STAR,                                     // Звезды
EFF_AQUARIUM,                                 // Аквариум
EFF_FIREWORKS,                                // Фейерверк
EFF_PACIFIC,                                  // Тихий Океан
EFF_MUNCH,                                    // Вышиванка
EFF_NOISE,                                    // Цветной шум
EFF_BUTTERFLY,                                // Мотыльки
EFF_SHADOWS,                                  // Тени
EFF_PATTERNS,                                 // Узоры
//60
EFF_ARROWS,                                   // Стрелки
EFF_NBALLS,                                   // Дикие шарики
EFF_ATTRACT,                                  // Притяжение
EFF_SNAKE,                                    // Змейки by kDn
EFF_NEXUS,                                    // Nexus
EFF_MAZE,                                     // Лабиринт
EFF_FRIZZLES,                                 // Вьющийся цвет (с) Stepko
EFF_POLARL,                                   // Северное Сияние
EFF_FLOWER,                                   // Цветение
EFF_TEST,                                     // Змеиный Остров
//70
EFF_SMOKEBALLS,                               // Детские Сны (stepko)
EFF_PICASSO4,                                 // Меташары
EFF_LIQUIDLAMP,                               // Лаваламп 
EFF_RACER,                                    // Космо-Гонщик
EFF_SMOKER,                                   // Дым
EFF_FIRE2021,                                 // Огонь от Степко
EFF_MIRAGE,                                   // Мираж
EFF_TIME = (250U)                             // Часы (служебный, смещаем в конец)
#ifdef MIC_EFFECTS
,EFF_VU = (254U)                              // Частотный анализатор (служебный, смещаем в конец)
,EFF_OSC = (255U)                             // Осциллограф
#endif
} EFF_ENUM;

/** набор указателей на строки с именами эффектов
 * индекс элемента массива составляет id из EFF_ENUM
 * вместо несуществующих эффектов ставим указатель на 0-ю пустышку
 */
static const char* const T_EFFNAMEID[] PROGMEM = {
  TEFF_000, TEFF_001, TEFF_002, TEFF_003, TEFF_004, TEFF_005, TEFF_006, TEFF_007, TEFF_008, TEFF_009, TEFF_010, TEFF_011, TEFF_012, TEFF_013, TEFF_014, TEFF_015, // 0-15
  TEFF_016, TEFF_017, TEFF_018, TEFF_019, TEFF_020, TEFF_021, TEFF_022, TEFF_023, TEFF_024, TEFF_025, TEFF_026, TEFF_027, TEFF_028, TEFF_029, TEFF_030, TEFF_031, // 16-31
  TEFF_032, TEFF_033, TEFF_034, TEFF_035, TEFF_036, TEFF_037, TEFF_038, TEFF_039, TEFF_040, TEFF_041, TEFF_042, TEFF_043, TEFF_044, TEFF_045, TEFF_046, TEFF_047, // 32 - 47
  TEFF_048, TEFF_049, TEFF_050, TEFF_051, TEFF_052, TEFF_053, TEFF_054, TEFF_055, TEFF_056, TEFF_057, TEFF_058, TEFF_059, TEFF_060, TEFF_061, TEFF_062, TEFF_063, // 48 - 63
  TEFF_064, TEFF_065, TEFF_066, TEFF_067, TEFF_068, TEFF_069, TEFF_070, TEFF_071, TEFF_072, TEFF_073, TEFF_074, TEFF_075, TEFF_076, TEFF_000, TEFF_000, TEFF_000, // 64 - 79
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 80 - 95
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 96 - 111
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 112 - 127
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 128 - 143
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 144 - 159
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 160 - 175
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 176 - 191
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 192 - 207
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 208 - 223
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 224 - 39
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_250, TEFF_000, TEFF_000, TEFF_000, TEFF_254, TEFF_255  // 240 - 255
};

/**
 *  массив с версиями эффектов, порядковый номер элемента массива соответсвует ENUM эффекта.
 *  Четный номер версии означает, что у эффекта есть поддержка микрофона,
 *  нечетный - нет. 127 вариантов версий "должно хватить всем" (с) Джобс
 */
static const uint8_t T_EFFVER[] PROGMEM = {
  0, 6, 6, 4, 4, 1, 3, 3, 3, 5, 3, 6, 5, 3, 5, 10, // 0-15
  1, 1, 3, 1, 3, 7, 5, 2, 1, 5, 5, 7, 7, 2, 1, 5, // 16-31
  5, 5, 5, 3, 7, 3, 4, 3, 3, 1, 4, 5, 7, 1, 1, 3, // 32 - 47
  1, 6, 6, 6, 4, 7, 5, 4, 1, 5, 6, 3, 1, 5, 7, 6, // 48 - 63
  5, 1, 1, 5, 4, 1, 7, 1, 3, 1, 1, 1, 1, 0, 0, 0, // 64 - 79
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80 - 95
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 96 - 111
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 112 - 127
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 128 - 143
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 144 - 159
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 160 - 175
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 176 - 191
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 192 - 207
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 208 - 223
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 224 - 239
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0, 2, 2, // 240 - 255
};


/* младшие 4 бита отдаются на выбор контрола, что даст 16 различных типов контроллов
* старшие 4 бита используются как набор управляющих кодов, к примеру - отображать при включенном микрофоне, при выключенном и т.д., тоже 16 вариантов
* https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/post-67459
*/
typedef enum : uint8_t {ALWAYS=0,ISMICON,ISMICOFF,HIDE} CONTROL_CASE; // старшие 4 бита
typedef enum : uint8_t {RANGE=0,EDIT,CHECKBOX} CONTROL_TYPE; // младшие 4 бита

/**
 * Набор статических строк, определяющих UI эффекта если он отличается от базового
 *
 *  GUI для эффекта по-умолчанию
 * Полный формат для пользовательского (id=0...7) параметра имеет вид: {"id":0,"type":0,"name":"Параметр","val":"127","min":"1","max":"255","step":"1"}
 * если какой-то из параметров стандартный из этого списка, то его можно опустить и не указывать
 * @nb@ - будет заменен на реальный номер эффекта, @name@ - на дефолтное имя эффекта, @ver@ - версия
 * https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/post-48813
 * https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/post-48848
 */

#define COTNROLS_PREFIX "\"nb\":@nb@,\"name\":\"@name@\",\"ver\":@ver@,\"flags\":"

// Дефолтные контролы
static const char E_DEFUI[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[]}";
// Добавил ко всем конфигам чекбокс {Микрофон} с индексом 7. Ввиду модифированного поведения функции myLamp.isMicOnOff(), он будет появляться только если глобально
// микрофон включен, и эффект обладает зависимостью от микрофона в принципе.
static const char E_DEFMICUI[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
// Общая конфигурация для эффектов с 3-им ползунком для палитр
static const char E_3PAL[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_3PAL_MIC[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":16},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";

// Общая конфигурация для эффектов только с 2-мя ползунками "Яркость" и "Скорость", пример - эффект"Тихий Океан"
static const char E_2[]         PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}]}"; // 3*16+0 для 2 контрола
static const char E_2_MIC[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}"; // 3*16+0 для 2 контрола
// Общая конфигурация для эффектов с 4-им ползунком для палитр
static const char E_4PAL[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_4PAL_MIC[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":4,\"type\":16,\"val\":127,\"min\":0,\"max\":255,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";

static const char E_PUZZLES[]   PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"min\":4,\"max\":8,\"name\":\"" DFTINTF_10D "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_SPARCLES[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":16,\"max\":10,\"name\":\"" DFTINTF_0D0 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_LIGHTERS[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":16,\"max\":32,\"name\":\"" DFTINTF_0D5 "\"}, {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"}]}";
static const char E_RADAR255[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D1 "\"}, {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"}]}";
static const char E_WHITE[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"name\":\"" DFTINTF_0D7 "\"}, {\"id\":3,\"name\":\"" DFTINTF_0E6 "\"}, {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_LEAPERS[]   PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":50,\"name\":\"" DFTINTF_0D8 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_BUTTERFLY[] PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":64,\"name\":\"" DFTINTF_0D9 "\"}, {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0DF "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E0 "\"}]}";
static const char E_PULS[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D6 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_BBALLS[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":32,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E1 "\"}]}";
static const char E_PRIZMATA[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"name\":\"" DFTINTF_0DE "\"}, {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_AQUARIUM[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"name\":\"" DFTINTF_0D0 "\"},  {\"id\":3,\"val\":190,\"name\":\"" DFTINTF_0DA "\"},  {\"id\":4,\"min\":0,\"max\":2,\"name\":\"" DFTINTF_0E2 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_OSC[]       PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"name\":\"" DFTINTF_0DC "\"},  {\"id\":2,\"val\":160,\"min\":1,\"max\":255,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":3,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
// размерность ползунка "Узор" должна быть MAX_PATTERN + 1 (patterns.h). При добавлении паттернов - менять и тут.
static const char E_PATT[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"type\":48},  {\"id\":2,\"type\":48},  {\"id\":3,\"val\":0,\"min\":-32,\"max\":32,\"name\":\"" DFTINTF_0EA "\"},  {\"id\":4,\"val\":0,\"min\":-32,\"max\":32,\"name\":\"" DFTINTF_0EB "\"},  {\"id\":5,\"min\":0,\"max\":38,\"val\":0,\"name\":\"" DFTINTF_0EC "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"},  {\"id\":7,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_116 "\"}]}";
static const char E_SHAD[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0ED "\"},  {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E4 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_ARR[]       PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":6,\"name\":\"" DFTINTF_0E5 "\"}]}";
static const char E_F2018[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D7 "\"},  {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E4 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_NBAL[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":25,\"name\":\"" DFTINTF_0E6 "\"}]}";
static const char E_ATTRACT[]   PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0EE "\"},  {\"id\":3,\"val\":100,\"name\":\"" DFTINTF_0E9 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_FLOCK[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E8 "\"}]}";
static const char E_WAVES[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":8,\"name\":\"" DFTINTF_0E7 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_MUNCH[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"val\":4,\"min\":0,\"max\":8,\"name\":\"" DFTINTF_10E "\"} ,{\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_DRIFT[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"max\":4,\"name\":\"" DFTINTF_0FF "\"}]}";
static const char E_CUBE2D[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_100 "\"},  {\"id\":4,\"val\":4,\"max\":7,\"name\":\"" DFTINTF_10D "\"},  {\"id\":5,\"val\":4,\"max\":7,\"name\":\"" DFTINTF_10F "\"} ,{\"id\":6,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_110 "\"}]}";
static const char E_LIGHT2[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"}]}";
static const char E_CUBE[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0F1 "\"},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_STARFAIL[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"} , {\"id\":4,\"max\":3,\"name\":\"" DFTINTF_0FD "\"} ,{\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_108 "\"}]}"; 
static const char E_SNAKE[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"val\":4,\"max\":16,\"name\":\"" DFTINTF_0FE "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_111 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_NEXUS[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_FWORK[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"val\":127,\"name\":\"" DFTINTF_0F2 "\"},  {\"id\":2,\"type\":48},  {\"id\":3,\"max\":8,\"val\":4,\"name\":\"" DFTINTF_10C "\"},  {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_109 "\"}]}";
static const char E_MATRIX[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":12,\"max\":32,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"val\":90,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":5,\"val\":20,\"max\":60,\"name\":\"" DFTINTF_10A "\"}]}";
static const char E_NFIRE[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":50,\"max\":100,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"val\":7,\"max\":12,\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}]}";
static const char E_BBUSH[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":50,\"max\":100,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"val\":7,\"max\":12,\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_112 "\"}]}";
static const char E_POPCORN[]   PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":16,\"max\":32,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0DE "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_112 "\"}]}";
static const char E_MBL[]       PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_0F4 "\"},  {\"id\":4,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_0D6 "\"}]}";
static const char E_LIQLAM[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":17,\"name\":\"" DFTINTF_0F4 "\"},  {\"id\":4,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_0D6 "\"},  {\"id\":5,\"val\":0,\"min\":0,\"max\":4,\"name\":\"" DFTINTF_105 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_106 "\"}]}";
static const char E_F2012_MIC[] PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":64,\"max\":128,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"val\":6,\"max\":10,\"name\":\"" DFTINTF_084 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_DNA[]       PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":1,\"min\":0,\"max\":3,\"name\":\"" DFTINTF_0D2 "\"}, {\"id\":4,\"val\":10,\"min\":2,\"max\":16,\"name\":\"" DFTINTF_0DC "\"}, {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}]}";
static const char E_SNOW[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"val\":\"1\",\"max\":3,\"name\":\"" DFTINTF_0F5 "\"}]}";
static const char E_SMOKBALLS[] PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_PALMICUI[]  PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_0FB "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F5 "\"}, {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E6 "\"}, {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_COLORS[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":32,\"val\":\"2\",\"max\":3,\"name\":\"" DFTINTF_0E3 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_TLAND[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":25,\"min\":0,\"max\":35,\"name\":\"" DFTINTF_0D2 "\"},  {\"id\":4,\"val\":160,\"min\":0,\"name\":\"" DFTINTF_101 "\"},  {\"id\":5,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_113 "\"},  {\"id\":6,\"max\":4,\"name\":\"" DFTINTF_114 "\"}]}";
static const char E_FLYING[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":128,\"name\":\"" DFTINTF_0DE "\"},  {\"id\":4,\"val\":7,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_107 "\"}]}";
static const char E_WRAIN[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":25,\"max\":45,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"val\":4,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_100 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_102 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_10B "\"},  {\"id\":7,\"max\":8,\"name\":\"" DFTINTF_115 "\"}]}";
static const char E_CLOCK[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":8,\"name\":\"" DFTINTF_0F7 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_COMET[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":6,\"name\":\"" DFTINTF_00A "\"},  {\"id\":4,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":5,\"val\":6,\"max\":12,\"name\":\"" DFTINTF_104 "\"},  {\"id\":6,\"val\":32,\"max\":64,\"name\":\"" DFTINTF_0DE "\"}]}";
static const char E_CELL[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":0,\"max\":6,\"name\":\"" DFTINTF_0D2 "\"}]}"; 
static const char E_FAIRY[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F9 "\"}]}";
static const char E_FOUNT[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0FA "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":10,\"name\":\"" DFTINTF_103 "\"}, {\"id\":5,\"type\":16,\"val\":127,\"min\":0,\"max\":255,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static const char E_POLAR[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"val\":1,\"max\":14,\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_SMOKER[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"max\":255,\"name\":\"" DFTINTF_0D0 "\"}, {\"id\":3,\"val\":85,\"max\":255,\"name\":\"" DFTINTF_0DA "\"}]}";
static const char E_DRAGE[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"val\":5,\"min\":1,\"max\":9,\"name\":\"" DFTINTF_0F6 "\"}, {\"id\":5,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F9 "\"}]}"; //DFTINTF_117
static const char E_PICAS[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":5,\"val\":1,\"max\":4,\"name\":\"" DFTINTF_00A "\"}]}";
static const char E_SSHIPS[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"val\":8,\"max\":15,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":8,\"name\":\"" DFTINTF_117 "\"}, {\"id\":5,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static const char E_FLAGS[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"min\":0,\"val\":0,\"max\":10,\"name\":\"" DFTINTF_00A "\"}]}";
static const char E_FREQ[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"min\":1,\"val\":50,\"max\":255,\"name\":\"" DFTINTF_0DB "\"}, {\"id\":2,\"min\":1,\"val\":30,\"max\":255,\"name\":\"" DFTINTF_118 "\"}, {\"id\":3,\"min\":1,\"val\":1,\"max\":10,\"name\":\"" DFTINTF_00A "\"}, {\"id\":4,\"min\":1,\"val\":1,\"max\":4,\"name\":\"" DFTINTF_0D6 "\"}, {\"id\":5,\"min\":0,\"val\":1,\"max\":255,\"name\":\"" DFTINTF_0D3 "\"}, {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_108 "\"}, {\"id\":7,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F5 "\"}]}";
static const char E_CNOISE[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_108 "\"}]}";
static const char E_SINUS[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[ {\"id\":3,\"max\":255,\"name\":\"" DFTINTF_0DB "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":3,\"name\":\"" DFTINTF_00A "\"}]}";
static const char E_1[]         PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"type\":48}, {\"id\":2,\"type\":48}]}"; // 3*16+0 для 2 контрола
static const char E_MIRAGE[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"name\":\"" DFTINTF_11A "\"}]}";
static const char E_WCOLOR[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":1,\"max\":8,\"val\":4,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}]}"; 
static const char E_SPBALS[]    PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":3,\"max\":6,\"val\":3,\"name\":\"" DFTINTF_0D5 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
// Инженерный
//static const char E_TEST[]      PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"}]}";
static const char E_TEST2[]     PROGMEM = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"name\":\"" DFTINTF_084 "\"},  {\"id\":4},  {\"id\":5},  {\"id\":6},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";

/** набор указателей на строки с UI-конфигом для эффектов по-умолчанию
 * индекс элемента массива составляет id из EFF_ENUM
 * вместо несуществующих эффектов или эффектов с дефолтными ползунками ставим указатель на базовый конфиг
 * не обязательно каждому эффекту лепить свой конфиг, можно ставить указатели на эффекты с похожими контролами
 */
static const char* const T_EFFUICFG[] PROGMEM = {
  E_DEFUI, E_WHITE,   E_COLORS,   E_DEFMICUI, E_SPARCLES, E_4PAL, E_STARFAIL,   E_MATRIX,  E_WCOLOR,    E_LIGHT2,    E_CUBE,   E_PULS,  E_4PAL,   E_BBUSH,    E_2,        E_PALMICUI, // 0-15
  E_CELL,  E_PUZZLES, E_TLAND,    E_SPBALS,   E_3PAL,     E_WRAIN, E_FAIRY,      E_FOUNT,  E_BBALLS,    E_SINUS,     E_4PAL,   E_3PAL,  E_COMET,  E_4PAL_MIC, E_PRIZMATA, E_FLOCK, // 16-31
  E_3PAL,  E_DRIFT,   E_POPCORN,  E_4PAL,     E_RADAR255, E_WAVES, E_F2012_MIC,  E_FAIRY,  E_4PAL,      E_DNA,       E_F2018,  E_CLOCK, E_CUBE2D, E_NFIRE,    E_PICAS,    E_SSHIPS, // 32 - 47
  E_FLAGS, E_LEAPERS, E_3PAL_MIC, E_3PAL_MIC, E_AQUARIUM, E_FWORK, E_2,          E_MUNCH,  E_CNOISE,    E_BUTTERFLY, E_SHAD,   E_PATT,  E_ARR,    E_NBAL,     E_ATTRACT,  E_SNAKE, // 48 - 63
  E_NEXUS, E_DEFUI,   E_DEFUI,    E_POLAR,    E_2_MIC,    E_NEXUS, E_SMOKBALLS,  E_MBL,    E_LIQLAM,    E_3PAL,      E_SMOKER, E_NFIRE, E_MIRAGE, E_DEFUI,    E_DEFUI,    E_DEFUI, // 64 - 79
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 80 - 95
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 96 - 111
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 112 - 127
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 128 - 143
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 144 - 159
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 160 - 175
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 176 - 191
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 192 - 207
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 208 - 223
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, // 224 - 39
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_3PAL,  E_1,     E_DEFUI, E_DEFUI, E_FREQ,  E_OSC  // 240 - 255
};

/**
 * константы/определения
 */
#define MIN_RANGE 1     // заложим дейфан пока нет динамических ползунков
#define MAX_RANGE 255   // заложим дейфан пока нет динамических ползунков


#if  576U < (WIDTH * HEIGHT)
  #define BIGMATRIX
#endif

static const uint8_t gamma_exp[] PROGMEM = {
0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   7,   7,
7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  12,  12,
12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  26,  27,  28,
28,  29,  30,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,
39,  40,  41,  42,  43,  44,  44,  45,  46,  47,  48,  49,  50,  51,  52,
53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,
68,  70,  71,  72,  73,  74,  75,  77,  78,  79,  80,  82,  83,  84,  85,
87,  89,  91,  92,  93,  95,  96,  98,  99,  100, 101, 102, 105, 106, 108,
109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 125, 126, 128, 130, 131,
133, 135, 136, 138, 140, 142, 143, 145, 147, 149, 151, 152, 154, 156, 158,
160, 162, 164, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187,
190, 192, 194, 196, 198, 200, 202, 204, 207, 209, 211, 213, 216, 218, 220,
222, 225, 227, 229, 232, 234, 236, 239, 241, 244, 246, 249, 251, 253, 254, 255};

// ==== Константы для эффектов ====

// Эффекты на базе "3D Noise"
#define NOISE_SCALE_AMP        (58.)                // амплификатор шкалы (влияет на машстаб "пятен" эффекта, большие пятна выглядят красивее чем куча мелких)
#define NOISE_SCALE_ADD        (8.)                 // корректор шкалы

#define BALLS_AMOUNT           (7U)                 // максимальное количество "шариков"
#define LIGHTERS_AM            (32U)                // светлячки
#define NUM_LAYERS             (1U)                 // The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS2            (2U)                 // The coordinates for 3 16-bit noise spaces.
#define AVAILABLE_BOID_COUNT   (10U)                // стая, кол-во птиц

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
#define e_com_TAILSPEED        (500)                // скорость смещения хвоста
#define e_com_BLUR             (12U)                // размытие хвоста
#define e_com_3DCOLORSPEED     (3U)                 // скорость случайного изменения цвета (0й - режим)

// ------------- светлячки со шлейфом -------------
//#define BALLS_AMOUNT         (7U)                 // максимальное количество "шариков"
#define CLEAR_PATH             (1U)                 // очищать путь
#define TRACK_STEP             (70U)                // длина хвоста шарика (чем больше цифра, тем хвост короче)

// --------------------------- эффект мячики ----------------------
#define bballsMaxNUM_BALLS     (WIDTH-1)             // максимальное количество мячиков прикручено при адаптации для бегунка Масштаб
#define bballsGRAVITY          (-9.7f)               // Downward (negative) acceleration of gravity in m/s^2
#define bballsH0               (2)                   // Starting height, in meters, of the ball (strip length)
#define bballsVImpact0         (EffectMath::sqrt(-2 * bballsGRAVITY * bballsH0))

// ------------- метель -------------
#define SNOW_DENSE             (60U)                 // плотность снега
#define SNOW_TAIL_STEP         (100U)                // длина хвоста
#define SNOW_SATURATION        (0U)                  // насыщенность (от 0 до 255)

// ------------- звездопад -------------
#define STAR_DENSE             (60U)                 // плотность комет
#define STAR_TAIL_STEP         (100U)                // длина хвоста кометы
#define STAR_SATURATION        (150U)                // насыщенность кометы (от 0 до 255)

// ============= DRIFT / ДРИФТ ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift.h
#define CENTER_max  max(WIDTH / 2, HEIGHT / 2) // Наибольшее значение центра
#define WIDTH_steps  256U / WIDTH   // диапазон значений приходящихся на 1 пиксель ширины матрицы
#define HEIGHT_steps 256U / HEIGHT // диапазон значений приходящихся на 1 пиксель высоты матрицы



// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// (c) SottNick
// refactored by Vortigont
#define CUBE2D_MAX_SIZE 7       // максимальный размер обрабатываемых квадратов (AxA)
#define CUBE2D_PAUSE_FRAMES 20   // число кадров паузы между трансформациями
#endif

// ----------- Эффект "Звезды" адаптация SottNick
#define CENTER_DRIFT_SPEED 6U         // скорость перемещения плавающего центра возникновения звёзд
#define STAR_BLENDER 128U             // хз что это

//---------- Эффект "Фейерверк" адаптация kostyamat
//https://gist.github.com/jasoncoon/0cccc5ba7ab108c0a373

#define MODEL_BORDER (HEIGHT - 4U)  // как далеко за экран может вылетить снаряд, если снаряд вылетает за экран, то всышка белого света (не особо логично)
#define MODEL_WIDTH  (MODEL_BORDER + WIDTH  + MODEL_BORDER) // не трогать, - матиматика
#define MODEL_HEIGHT (MODEL_BORDER + HEIGHT + MODEL_BORDER) // -//-
#define PIXEL_X_OFFSET ((MODEL_WIDTH  - WIDTH ) / 2) // -//-
#define PIXEL_Y_OFFSET ((MODEL_HEIGHT - HEIGHT) / 2) // -//-

#define SPARK 8U // максимальное количество снарядов
#define NUM_SPARKS WIDTH // количество разлитающихся петард (частей снаряда)
const saccum78 gGravity = 10;
const fract8  gBounce = 127;
const fract8  gDrag = 255;


// ---- Эффект "Мотыльки"
// (с) Сотнег, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
#define BUTTERFLY_MAX_COUNT           (64U) // максимальное количество мотыльков
#define BUTTERFLY_FIX_COUNT           (28U) // количество мотыльков для режима, конда бегунок Масштаб регулирует цвет

//------------ Эффект "Змеиный Остров"
#define SNAKE_LENGTH  (HEIGHT / 2U)
#define SNAKE2_LENGTH (HEIGHT / 2U + WIDTH / 4U)
#define MAX_SNAKES    (16U) 

// SottNick константы
//константы размера матрицы вычисляется только здесь и не меняется в эффектах
const uint8_t CENTER_X_MINOR =  (WIDTH / 2) -  ((WIDTH - 1) & 0x01); // центр матрицы по ИКСУ, сдвинутый в меньшую сторону, если ширина чётная
const uint8_t CENTER_Y_MINOR = (HEIGHT / 2) - ((HEIGHT - 1) & 0x01); // центр матрицы по ИГРЕКУ, сдвинутый в меньшую сторону, если высота чётная
const uint8_t CENTER_X_MAJOR =   WIDTH / 2  + (WIDTH % 2);           // центр матрицы по ИКСУ, сдвинутый в большую сторону, если ширина чётная
const uint8_t CENTER_Y_MAJOR =  HEIGHT / 2  + (HEIGHT % 2);          // центр матрицы по ИГРЕКУ, сдвинутый в большую сторону, если высота чётная


// ***************** НАСТРОЙКИ ГЕНЕРАЦИИ ЛАБИРИНТА *****************
#define GAMEMODE 0        // режим игры: 0 - видим весь лабиринт, 1 - видим вокруг себя часть
#define FOV 3             // область видимости в режиме игры 1

// размеры лабиринта ДОЛЖНЫ БЫТЬ НЕЧЁТНЫЕ независимо от размеров матрицы!
// при SHIFT 1 размер лабиринта можно ставить на 1 длиннее матрицы (матрица 16х16 лабиринт 17х17)
#if (WIDTH % 2 == 0)
#define MAZE_WIDTH (WIDTH-1)      // ширина лабиринта
#else
#define MAZE_WIDTH WIDTH          // ширина лабиринта
#endif

#if (HEIGHT % 2 == 0)
#define MAZE_HEIGHT (HEIGHT-1)    // высота лабиринта
#else
#define MAZE_HEIGHT HEIGHT        // высота лабиринта
#endif
#define MAZE_SHIFT 0                   // (1 да / 0 нет) смещение лабиринта (чтобы не видеть нижнюю и левую стену)

// VU-meter
#define SAMPLES           256U          // Must be a power of 8
/*#if WIDTH > 20
  # if WIDTH & 01        
    #define NUM_BANDS WIDTH/2 + 1
  #else
    #define NUM_BANDS WIDTH/2
  #endif
#else
  #define NUM_BANDS WIDTH
#endif*/
#define TOP            (HEIGHT - 1)                // Don't allow the bars to go offscreen
//#define BAR_WIDTH      (WIDTH  / (NUM_BANDS - 1))  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
