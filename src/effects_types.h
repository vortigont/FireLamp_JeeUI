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
#include <FastLED.h>
#include "color_palette.h"

/**
 * @brief enumerated list of visual effects
 * 
 */
enum class effect_t : uint32_t {
empty = (0U),                             // пустой черный квадрат :)
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
picassoBalls,                             // Пикассо Меташары
picassoShapes,                            // Пикассо фигуры
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
stars,                                    // Звезды
starships,                                // Звездный Десант
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
static constexpr const char* T_picassoBalls = "picassoBalls";
static constexpr const char* T_picassoShapes = "picassoShapes";
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
static constexpr std::array<effect_t, 15> fw_effects_index = {
  effect_t::empty,
  effect_t::bouncingballs,
  effect_t::fireveil,
  effect_t::fire2012,
  effect_t::liquidlamp,
  effect_t::magma,
  effect_t::metaballs,
  effect_t::nexus,
  effect_t::picassoBalls,
  effect_t::picassoShapes,
  effect_t::radialfire,
  effect_t::smokeballs,
  effect_t::whirls,
  effect_t::noise3d,
  effect_t::flock
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
T_picassoBalls,
T_picassoShapes,
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


// ==== Константы для эффектов ====

#define BALLS_AMOUNT           (7U)                 // максимальное количество "шариков"
#define LIGHTERS_AM            (32U)                // светлячки

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
