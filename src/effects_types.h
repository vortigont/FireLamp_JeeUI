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

#pragma once
#include "config.h"
#include <FastLED.h>
//#include LANG_FILE                //"text_res.h"
#include "color_palette.h"

//#define SF(s) __SFSTR(s)
//#define __SFSTR(s) #s

/**
 * @brief enumerated list of visual effects
 * 
 */
enum class effect_t : uint32_t {
empty = (0U),                              // пустой черный квадрат :)
whitelight,                               // Белый свет
colors,                                   // Смена цвета
attractor,                                // Притяжение
balls,                                    // Шары
bouncingballs,                            // Прыгающие мячики
bubbles,                                  // Бульбулятор (круги)
cells,                                    // Клеточки (Ёлки-палки)
cube,                                     // Блуждающий кубик
DNA,                                      // ДНК
drift,                                    // Дрифт
fairy,                                    // Фея
fire2012,                                 // Огонь 2012
fire2021,                                 // Огонь от Степко
fireflies,                                // Светлячки со шлейфом
fireveil,                                 // Огненная вуаль, Кометы, Парящие огни +
flags,                                    // Флаги
flock,                                    // Стая и Хищник
fountain,                                 // Источник
frizzles,                                 // Вьющийся цвет (с) Stepko
liquidlamp,                               // Лава-лампа
magma,                                    // Магма
matrix,                                   // Матрица
maze,                                     // Лабиринт
metaballs,                                // Метасферы
nexus,                                    // Nexus
noise3d,                                  // 3D-шум
oscillator,                               // Осциллятор 
paintball,                                // Пейнтбол
picasso,                                  // Пикассо
picasso4,                                 // Меташары
pile,                                     // Цветные Драже
pulse,                                    // Пульс
radialfire,                               // Радиальный Огонь (неопалимая купелина)
rainbow2d,                                // Радуга универсальная
rubik,                                    // Куб 2D, кубик-рубика
sinusoid,                                 // Синусоид
smokeballs,                               // Детские Сны (stepko)
smoker,                                   // Дым
sparkles,                                 // Конфетти
spballs,                                  // Н.Тесла
spiral,                                   // Спираль
starfall,                                 // Метель + Звездопад
stars,                                   // Звезды
starships,                               // Звездный Десант
tixyland,                                 // Тикси Ленд
twinkles,                                 // Мерцание
watercolors,                              // Акварель
waves,                                    // Волны
whirls                                    // Вихри
};



// effect index names
static constexpr const char* T_attractor = "attractor";
static constexpr const char* T_balls = "balls";
static constexpr const char* T_bouncingballs = "bouncingballs";
static constexpr const char* T_bubbles = "bubbles";
static constexpr const char* T_cells = "cells";
static constexpr const char* T_colors = "colors";
static constexpr const char* T_cube = "cube";
static constexpr const char* T_DNA = "DNA";
static constexpr const char* T_drift = "drift";
static constexpr const char* T_empty = "empty";
static constexpr const char* T_fairy = "fairy";
static constexpr const char* T_fire2012 = "fire2012";
static constexpr const char* T_fire2021 = "fire2021";
static constexpr const char* T_fireflies = "fireflies";
static constexpr const char* T_fireveil = "fireveil";
static constexpr const char* T_flags = "flags";
static constexpr const char* T_flock = "flock";
static constexpr const char* T_fountain = "fountain";
static constexpr const char* T_frizzles = "frizzles";
static constexpr const char* T_liquidlamp = "liquidlamp";
static constexpr const char* T_magma = "magma";
static constexpr const char* T_matrix = "matrix";
static constexpr const char* T_maze = "maze";
static constexpr const char* T_metaballs = "metaballs";
static constexpr const char* T_nexus = "nexus";
static constexpr const char* T_noise3d = "noise3d";
static constexpr const char* T_oscillator = "oscillator";
static constexpr const char* T_paintball = "paintball";
static constexpr const char* T_picasso = "picasso";
static constexpr const char* T_picasso4 = "picasso4";
static constexpr const char* T_pile = "pile";
static constexpr const char* T_pulse = "pulse";
static constexpr const char* T_radialfire = "radialfire";
static constexpr const char* T_rainbow2d = "rainbow2d";
static constexpr const char* T_rubik = "rubik";
static constexpr const char* T_sinusoid = "sinusoid";
static constexpr const char* T_smokeballs = "smokeballs";
static constexpr const char* T_smoker = "smoker";
static constexpr const char* T_sparkles = "sparkles";
static constexpr const char* T_spballs = "spballs";
static constexpr const char* T_spiral = "spiral";
static constexpr const char* T_starfall = "starfall";
static constexpr const char* T_stars = "stars";
static constexpr const char* T_starships = "starships";
static constexpr const char* T_tixyland = "tixyland";
static constexpr const char* T_twinkles = "twinkles";
static constexpr const char* T_watercolors = "watercolors";
static constexpr const char* T_waves = "waves";
static constexpr const char* T_whirls = "whirls";
static constexpr const char* T_whitelight = "whitelight";


// firmware defined static index names for each of available effects
static constexpr std::array<effect_t, 8> fw_effects_index = {
  effect_t::empty,
  effect_t::bouncingballs,
  effect_t::fire2021,
  effect_t::magma,
  effect_t::metaballs,
  effect_t::nexus,
  effect_t::radialfire,
  effect_t::smokeballs
};

/**
 * @brief firmware defined index label for each of available effects
 * @warning the order of elements in this array MUST match the order in effect_t enum!
 */
static constexpr std::array<const char*, 50> fw_effects_nameindex = {
T_empty,
T_whitelight,
T_colors,
T_attractor,
T_balls,
T_bouncingballs,
T_bubbles,
T_cells,
T_cube,
T_DNA,
T_drift,
T_fairy,
T_fire2012,
T_fire2021,
T_fireflies,
T_fireveil,
T_flags,
T_flock,
T_fountain,
T_frizzles,
T_liquidlamp,
T_magma,
T_matrix,
T_maze,
T_metaballs,
T_nexus,
T_noise3d,
T_oscillator,
T_paintball,
T_picasso,
T_picasso4,
T_pile,
T_pulse,
T_radialfire,
T_rainbow2d,
T_rubik,
T_sinusoid,
T_smokeballs,
T_smoker,
T_sparkles,
T_spballs,
T_spiral,
T_starfall,
T_stars,
T_starships,
T_tixyland,
T_twinkles,
T_watercolors,
T_waves,
T_whirls
};


#ifdef DISABLED_CODE

/** набор указателей на строки с именами эффектов
 * индекс элемента массива составляет id из EFF_ENUM
 * вместо несуществующих эффектов ставим указатель на 0-ю пустышку
 */
static constexpr std::array<const char*, 256> T_EFFNAMEID = {
  TEFF_blank, TEFF_001, TEFF_002, TEFF_003, TEFF_004, TEFF_005, TEFF_006, TEFF_007, TEFF_008, TEFF_009, TEFF_010, TEFF_011, TEFF_012, TEFF_013, TEFF_014, TEFF_015, // 0-15
  TEFF_016, TEFF_000, TEFF_018, TEFF_019, TEFF_020, TEFF_000, TEFF_022, TEFF_023, TEFF_024, TEFF_025, TEFF_026, TEFF_027, TEFF_028, TEFF_029, TEFF_000, TEFF_031, // 16-31
  TEFF_000, TEFF_033, TEFF_000, TEFF_035, TEFF_000, TEFF_037, TEFF_038, TEFF_000, TEFF_040, TEFF_041, TEFF_000, TEFF_000, TEFF_044, TEFF_045, TEFF_046, TEFF_047, // 32 - 47
  TEFF_048, TEFF_000, TEFF_050, TEFF_051, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_062, TEFF_063, // 48 - 63
  TEFF_064, TEFF_065, TEFF_066, TEFF_000, TEFF_000, TEFF_000, TEFF_070, TEFF_071, TEFF_072, TEFF_000, TEFF_074, TEFF_075, TEFF_000, TEFF_000, TEFF_000, TEFF_000, // 64 - 79
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
  TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000, TEFF_000  // 240 - 255
};

/**
 *  массив с версиями эффектов, порядковый номер элемента массива соответсвует ENUM эффекта.
 *  Четный номер версии означает, что у эффекта есть поддержка микрофона,
 *  нечетный - нет. 127 вариантов версий "должно хватить всем" (с) Джобс
 */
static constexpr uint8_t T_EFFVER[] = {
  0, 6, 6, 4, 4, 1, 3, 3, 3, 5, 3, 6, 5, 3, 5, 10, // 0-15
  1, 1, 3, 1, 3, 7, 5, 2, 1, 5, 5, 7, 7, 2, 1, 5, // 16-31
  5, 5, 5, 3, 7, 3, 7, 3, 3, 1, 4, 5, 7, 1, 1, 3, // 32 - 47
  3, 6, 6, 6, 4, 7, 5, 4, 1, 5, 6, 3, 1, 5, 7, 6, // 48 - 63
  5, 1, 1, 5, 4, 1,11, 1, 3, 1, 1, 3, 1, 1, 2, 2, // 64 - 79
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
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 240 - 255
};

/**
 * @brief manifest for effects requiring persistent framebuffer
 * i.e. such effects keeps data for next frame calculation in the framebufer
 * such as dim or blur data, etc...
 * 
 */
static constexpr std::array<bool, 256> eff_persistent_buff = {
  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 0, 0, 0, 1, 0, // 0-15
  0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0, // 16-31
  1, 0, 0, 0, 0, 0, 0, 0,  1, 1, 0, 0, 0, 0, 1, 1, // 32 - 47
  0, 0, 1, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 1, 0, 0, // 48 - 63
  0, 0, 1, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 64 - 79
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 80 - 95
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 96 - 111
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 112 - 127
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 128 - 143
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 144 - 159
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 160 - 175
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 176 - 191
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 192 - 207
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 208 - 223
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 224 - 239
  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, // 240 - 255
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
 * 
 * P.S. этот безумный бред нужно убрать
 */
#define COTNROLS_PREFIX "\"nb\":@nb@,\"name\":\"@name@\",\"ver\":@ver@,\"flags\":"

// Дефолтные контролы
static constexpr const char* E_DEFUI     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[]}";
static constexpr const char* E_DEFMICUI  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_1         = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"type\":48}, {\"id\":2,\"type\":48}]}"; // 3*16+0 для 2 контрола
// Общая конфигурация для эффектов только с 2-мя ползунками "Яркость" и "Скорость", пример - эффект"Тихий Океан"
static constexpr const char* E_2         = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}]}"; // 3*16+0 для 2 контрола
static constexpr const char* E_2_MIC     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}"; // 3*16+0 для 2 контрола
// Общая конфигурация для эффектов с 3-им ползунком для палитр
static constexpr const char* E_3PAL      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_3PAL_MIC  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":16},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
// Общая конфигурация для эффектов с 4-им ползунком для палитр
static constexpr const char* E_4PAL      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_4PAL_MIC      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":4,\"type\":16,\"val\":127,\"min\":0,\"max\":255,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";

static constexpr const char* E_AQUARIUM  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"name\":\"" DFTINTF_0D0 "\"},  {\"id\":3,\"val\":190,\"name\":\"" DFTINTF_0DA "\"},  {\"id\":4,\"min\":0,\"max\":2,\"name\":\"" DFTINTF_0E2 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_ARR       = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":6,\"name\":\"" DFTINTF_0E5 "\"}]}";
static constexpr const char* E_ATTRACT   = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0EE "\"},  {\"id\":3,\"val\":100,\"name\":\"" DFTINTF_0E9 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_BBALLS    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":32,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E1 "\"}]}";
static constexpr const char* E_BBUSH     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":50,\"max\":100,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"val\":7,\"max\":12,\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_112 "\"}]}";
static constexpr const char* E_BUTTERFLY = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":64,\"name\":\"" DFTINTF_0D9 "\"}, {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0DF "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E0 "\"}]}";
static constexpr const char* E_CELL      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":0,\"max\":6,\"name\":\"" DFTINTF_0D2 "\"}]}"; 
static constexpr const char* E_CLOCK     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":8,\"name\":\"" DFTINTF_0F7 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_CNOISE    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_108 "\"}]}";
static constexpr const char* E_COLORS    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":32,\"val\":\"2\",\"max\":3,\"name\":\"" DFTINTF_0E3 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_COMET     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":6,\"name\":\"" DFTINTF_00A "\"},  {\"id\":4,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":5,\"val\":6,\"max\":12,\"name\":\"" DFTINTF_104 "\"},  {\"id\":6,\"val\":32,\"max\":64,\"name\":\"" DFTINTF_0DE "\"}]}";
static constexpr const char* E_CUBE      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0F1 "\"},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_CUBE2D    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_100 "\"},  {\"id\":4,\"val\":4,\"max\":7,\"name\":\"" DFTINTF_10D "\"},  {\"id\":5,\"val\":4,\"max\":7,\"name\":\"" DFTINTF_10F "\"} ,{\"id\":6,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_110 "\"}]}";
static constexpr const char* E_DNA       = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":1,\"min\":0,\"max\":3,\"name\":\"" DFTINTF_0D2 "\"}, {\"id\":4,\"val\":10,\"min\":2,\"max\":16,\"name\":\"" DFTINTF_0DC "\"}, {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}]}";
static constexpr const char* E_DRAGE     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"val\":5,\"min\":1,\"max\":9,\"name\":\"" DFTINTF_0F6 "\"}, {\"id\":5,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F9 "\"}]}"; //DFTINTF_117
static constexpr const char* E_DRIFT     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"max\":4,\"name\":\"" DFTINTF_0FF "\"}]}";
static constexpr const char* E_F2012_MIC = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":64,\"max\":128,\"name\":\"" DFTINTF_088 "\"},  {\"id\":4,\"val\":6,\"max\":10,\"name\":\"" DFTINTF_084 "\"}, {\"id\":5,\"val\":0,\"max\":6,\"name\":\"" DFTINTF_0F6 "\"}, {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_F2018     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D7 "\"},  {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E4 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_FAIRY     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F9 "\"}]}";
static constexpr const char* E_FLAGS     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"min\":0,\"val\":0,\"max\":9,\"name\":\"" DFTINTF_00A "\"}, {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0E5 "\"} ]}";
static constexpr const char* E_FLOCK     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E8 "\"}]}";
static constexpr const char* E_FLYING    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":128,\"name\":\"" DFTINTF_0DE "\"},  {\"id\":4,\"val\":7,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_107 "\"}]}";
static constexpr const char* E_FOUNT     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0FA "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":10,\"name\":\"" DFTINTF_103 "\"}, {\"id\":5,\"type\":16,\"val\":127,\"min\":0,\"max\":255,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_FREQ      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"min\":1,\"val\":50,\"max\":255,\"name\":\"" DFTINTF_0DB "\"}, {\"id\":2,\"min\":1,\"val\":30,\"max\":255,\"name\":\"" DFTINTF_118 "\"}, {\"id\":3,\"min\":1,\"val\":1,\"max\":10,\"name\":\"" DFTINTF_00A "\"}, {\"id\":4,\"min\":1,\"val\":1,\"max\":4,\"name\":\"" DFTINTF_0D6 "\"}, {\"id\":5,\"min\":0,\"val\":1,\"max\":255,\"name\":\"" DFTINTF_0D3 "\"}, {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_108 "\"}, {\"id\":7,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F5 "\"}]}";
static constexpr const char* E_FWORK     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"val\":127,\"name\":\"" DFTINTF_0F2 "\"},  {\"id\":2,\"type\":48},  {\"id\":3,\"max\":8,\"val\":4,\"name\":\"" DFTINTF_10C "\"},  {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_109 "\"}]}";
static constexpr const char* E_LEAPERS   = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"max\":50,\"name\":\"" DFTINTF_0D8 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_LIGHT2    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"}]}";
static constexpr const char* E_LIGHTERS  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":16,\"max\":32,\"name\":\"" DFTINTF_0D5 "\"}, {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"}]}";
static constexpr const char* E_LIQLAM    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":17,\"name\":\"" DFTINTF_0F4 "\"},  {\"id\":4,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_0D6 "\"},  {\"id\":5,\"val\":0,\"min\":0,\"max\":4,\"name\":\"" DFTINTF_105 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_106 "\"}]}";
static constexpr const char* E_MATRIX    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":12,\"max\":32,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"val\":90,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":5,\"val\":20,\"max\":60,\"name\":\"" DFTINTF_10A "\"}]}";
static constexpr const char* E_MBL       = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_0F4 "\"},  {\"id\":4,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_0D6 "\"}]}";
static constexpr const char* E_MIRAGE     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"name\":\"" DFTINTF_11A "\"}]}";
static constexpr const char* E_MUNCH     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"val\":4,\"min\":0,\"max\":8,\"name\":\"" DFTINTF_10E "\"} ,{\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_NBAL      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":25,\"name\":\"" DFTINTF_0E6 "\"}]}";
static constexpr const char* E_NEXUS     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_NFIRE     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":50,\"max\":100,\"name\":\"" DFTINTF_088 "\"}, {\"id\":4,\"val\":7,\"max\":12,\"name\":\"" DFTINTF_084 "\"}, {\"id\":5,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}, {\"id\":6,\"val\":10,\"max\":20,\"name\":\"" DFTINTF_0F6 "\"}]}";
static constexpr const char* E_OSC       = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"name\":\"" DFTINTF_0DC "\"},  {\"id\":2,\"val\":160,\"min\":1,\"max\":255,\"name\":\"" DFTINTF_0FC "\"},  {\"id\":3,\"name\":\"" DFTINTF_0DB "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_PALMICUI  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_0FB "\"},  {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0F5 "\"}, {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E6 "\"}, {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
// размерность ползунка "Узор" должна быть MAX_PATTERN + 1 (patterns.h). При добавлении паттернов - менять и тут.
static constexpr const char* E_PATT      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"type\":48},  {\"id\":2,\"type\":48},  {\"id\":3,\"val\":0,\"min\":-32,\"max\":32,\"name\":\"" DFTINTF_0EA "\"},  {\"id\":4,\"val\":0,\"min\":-32,\"max\":32,\"name\":\"" DFTINTF_0EB "\"},  {\"id\":5,\"min\":0,\"max\":38,\"val\":0,\"name\":\"" DFTINTF_0EC "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"},  {\"id\":7,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_116 "\"}]}";
static constexpr const char* E_PICAS     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":5,\"val\":1,\"max\":4,\"name\":\"" DFTINTF_00A "\"}]}";
static constexpr const char* E_POLAR     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"val\":1,\"max\":14,\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_POPCORN   = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":16,\"max\":32,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0DE "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_112 "\"}]}";
static constexpr const char* E_PRIZMATA  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"name\":\"" DFTINTF_0DE "\"}, {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_PULS      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D6 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_PUZZLES   = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"min\":4,\"max\":8,\"name\":\"" DFTINTF_10D "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_RADAR255  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0D1 "\"}, {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"}]}";
static constexpr const char* E_SHAD      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"name\":\"" DFTINTF_0ED "\"},  {\"id\":3,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0E4 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_SINUS     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[ {\"id\":3,\"max\":255,\"name\":\"" DFTINTF_0DB "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":3,\"name\":\"" DFTINTF_00A "\"}]}";
static constexpr const char* E_SMOKBALLS = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"}, {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}, {\"id\":5,\"val\":240,\"name\":\"" DFTINTF_0DB "\"}]}";
static constexpr const char* E_SMOKER    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"val\":160,\"max\":255,\"name\":\"" DFTINTF_0D0 "\"}, {\"id\":3,\"val\":85,\"max\":255,\"name\":\"" DFTINTF_0DA "\"}]}";
static constexpr const char* E_SNAKE     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"},  {\"id\":4,\"val\":4,\"max\":16,\"name\":\"" DFTINTF_0FE "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_0D4 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_111 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_SNOW      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"val\":\"1\",\"max\":3,\"name\":\"" DFTINTF_0F5 "\"}]}";
static constexpr const char* E_SPARCLES  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"type\":16,\"max\":10,\"name\":\"" DFTINTF_0D0 "\"},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_SPBALS    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":3,\"max\":6,\"val\":3,\"name\":\"" DFTINTF_0D5 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_SSHIPS    = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"val\":8,\"max\":15,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"val\":0,\"min\":0,\"max\":8,\"name\":\"" DFTINTF_117 "\"}, {\"id\":5,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_STARFAIL  = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":5,\"max\":10,\"name\":\"" DFTINTF_0EF "\"} , {\"id\":4,\"max\":3,\"name\":\"" DFTINTF_0FD "\"} ,{\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_108 "\"}]}"; 
//static constexpr const char* E_TETRIS_CLK = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48}, {\"id\":3,\"val\":2,\"max\":3,\"name\":\"" DFTINTF_088 "\"}, {\"id\":4,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_24hrs "\"}]}";
static constexpr const char* E_TLAND     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":25,\"min\":0,\"max\":35,\"name\":\"" DFTINTF_0D2 "\"},  {\"id\":4,\"val\":160,\"min\":0,\"name\":\"" DFTINTF_101 "\"},  {\"id\":5,\"val\":0,\"min\":0,\"name\":\"" DFTINTF_113 "\"},  {\"id\":6,\"max\":4,\"name\":\"" DFTINTF_114 "\"}]}";
static constexpr const char* E_WAVES     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":4,\"max\":8,\"name\":\"" DFTINTF_0E7 "\"},  {\"id\":4,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_084 "\"}]}";
static constexpr const char* E_WCOLOR      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"min\":1,\"max\":8,\"val\":4,\"name\":\"" DFTINTF_0EF "\"}, {\"id\":4,\"type\":2,\"val\":0,\"name\":\"" DFTINTF_0F9 "\"}]}"; 
static constexpr const char* E_WHITE     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":1,\"name\":\"" DFTINTF_0D7 "\"}, {\"id\":3,\"name\":\"" DFTINTF_0E6 "\"}, {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";
static constexpr const char* E_WRAIN     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":25,\"max\":45,\"name\":\"" DFTINTF_0EF "\"},  {\"id\":4,\"val\":4,\"min\":0,\"max\":" SF(FASTLED_PALETTS_COUNT) ",\"name\":\"" DFTINTF_100 "\"},  {\"id\":5,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_102 "\"},  {\"id\":6,\"type\":2,\"val\":1,\"name\":\"" DFTINTF_10B "\"},  {\"id\":7,\"max\":8,\"name\":\"" DFTINTF_115 "\"}]}";
// Инженерный
//static constexpr const char* E_TEST      = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":2,\"type\":48},  {\"id\":3,\"val\":8,\"max\":16,\"name\":\"" DFTINTF_0D5 "\"}]}";
static constexpr const char* E_TEST2     = "{" COTNROLS_PREFIX SF(SET_ALL_EFFFLAGS) ",\"ctrls\":[{\"id\":3,\"name\":\"" DFTINTF_084 "\"},  {\"id\":4},  {\"id\":5},  {\"id\":6},  {\"id\":7,\"type\":18,\"val\":1,\"name\":\"" DFTINTF_020 "\"}]}";

/** набор указателей на строки с UI-конфигом для эффектов по-умолчанию
 * индекс элемента массива составляет id из EFF_ENUM
 * вместо несуществующих эффектов или эффектов с дефолтными ползунками ставим указатель на базовый конфиг
 * не обязательно каждому эффекту лепить свой конфиг, можно ставить указатели на эффекты с похожими контролами
 */
static constexpr std::array<const char *, 256> T_EFFUICFG = {
  E_DEFUI, E_WHITE,   E_COLORS,   E_DEFMICUI, E_SPARCLES, E_4PAL, E_STARFAIL,   E_MATRIX,  E_WCOLOR,    E_LIGHT2,    E_CUBE,   E_PULS,  E_4PAL,   E_BBUSH,    E_2,        E_PALMICUI, // 0-15
  E_CELL,  E_PUZZLES, E_TLAND,    E_SPBALS,   E_3PAL,     E_WRAIN, E_FAIRY,      E_FOUNT,  E_BBALLS,    E_SINUS,     E_4PAL,   E_3PAL,  E_COMET,  E_4PAL_MIC, E_PRIZMATA, E_FLOCK,    // 16-31
  E_3PAL,  E_DRIFT,   E_POPCORN,  E_4PAL,     E_RADAR255, E_WAVES, E_F2012_MIC,  E_FAIRY,  E_4PAL,      E_DNA,       E_F2018,  E_CLOCK, E_CUBE2D, E_NFIRE,    E_PICAS,    E_SSHIPS,   // 32 - 47
  E_FLAGS, E_LEAPERS, E_3PAL_MIC, E_3PAL_MIC, E_AQUARIUM, E_FWORK, E_2,          E_MUNCH,  E_CNOISE,    E_BUTTERFLY, E_SHAD,   E_PATT,  E_ARR,    E_NBAL,     E_ATTRACT,  E_SNAKE,    // 48 - 63
  E_NEXUS, E_DEFUI,   E_DEFUI,    E_POLAR,    E_2_MIC,    E_NEXUS, E_SMOKBALLS,  E_MBL,    E_LIQLAM,    E_3PAL,      E_SMOKER, E_NFIRE, E_MIRAGE, E_FREQ,     E_OSC,      E_DEFUI,    // 64 - 79
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
  E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI, E_DEFUI  // 240 - 255
};

#endif // DISABLED_CODE

// ==== Константы для эффектов ====

// Эффекты на базе "3D Noise"
#define NOISE_SCALE_AMP        (58.)                // амплификатор шкалы (влияет на машстаб "пятен" эффекта, большие пятна выглядят красивее чем куча мелких)
#define NOISE_SCALE_ADD        (8.)                 // корректор шкалы

#define BALLS_AMOUNT           (7U)                 // максимальное количество "шариков"
#define LIGHTERS_AM            (32U)                // светлячки
#define AVAILABLE_BOID_COUNT   (10U)                // стая, кол-во птиц

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
#define e_com_TAILSPEED        (500)                // скорость смещения хвоста
#define e_com_BLUR             (12U)                // размытие хвоста
#define e_com_3DCOLORSPEED     (3U)                 // скорость случайного изменения цвета (0й - режим)

// ------------- светлячки со шлейфом -------------
//#define BALLS_AMOUNT         (7U)                 // максимальное количество "шариков"
#define CLEAR_PATH             (1U)                 // очищать путь
#define TRACK_STEP             (70U)                // длина хвоста шарика (чем больше цифра, тем хвост короче)

// ------------- метель -------------
#define SNOW_DENSE             (60U)                 // плотность снега
#define SNOW_TAIL_STEP         (100U)                // длина хвоста
#define SNOW_SATURATION        (0U)                  // насыщенность (от 0 до 255)

// ------------- звездопад -------------
#define STAR_DENSE             (60U)                 // плотность комет
#define STAR_TAIL_STEP         (100U)                // длина хвоста кометы
#define STAR_SATURATION        (150U)                // насыщенность кометы (от 0 до 255)


// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// (c) SottNick
// refactored by Vortigont
#define CUBE2D_MAX_SIZE 7       // максимальный размер обрабатываемых квадратов (AxA)
#define CUBE2D_PAUSE_FRAMES 20   // число кадров паузы между трансформациями

// ----------- Эффект "Звезды" адаптация SottNick
#define CENTER_DRIFT_SPEED 6U         // скорость перемещения плавающего центра возникновения звёзд
#define STAR_BLENDER 128U             // хз что это

// ---- Эффект "Мотыльки"
// (с) Сотнег, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
#define BUTTERFLY_MAX_COUNT           (64U) // максимальное количество мотыльков
#define BUTTERFLY_FIX_COUNT           (28U) // количество мотыльков для режима, конда бегунок Масштаб регулирует цвет

// ***************** НАСТРОЙКИ ГЕНЕРАЦИИ ЛАБИРИНТА *****************
#define MAZE_GAMEMODE 0        // режим игры: 0 - видим весь лабиринт, 1 - видим вокруг себя часть
#define MAZE_FOV 3             // область видимости в режиме игры 1
#define MAZE_SHIFT 0                   // (1 да / 0 нет) смещение лабиринта (чтобы не видеть нижнюю и левую стену)
