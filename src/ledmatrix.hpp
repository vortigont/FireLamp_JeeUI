/*
Copyright © 2023 Emil Muratov (https://github.com/vortigont/FireLamp_JeeUI)
Copyright © 2020 Dmytro Korniienko (https://github.com/DmytroKorniienko/FireLamp_EmbUI)
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
#include <FastLED.h>

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ? (WIDTH - x - 1) : x)
#define THIS_Y (MIRR_H ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define ROTATED_MATRIX
#define THIS_X (MIRR_V ? (HEIGHT - y - 1) : y)
#define THIS_Y (MIRR_H ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ? (WIDTH - x - 1) : x)
#define THIS_Y (MIRR_H ?  y : (HEIGHT - y - 1))

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define ROTATED_MATRIX
#define THIS_X (MIRR_V ? y : (HEIGHT - y - 1))
#define THIS_Y (MIRR_H ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ?  x : (WIDTH - x - 1))
#define THIS_Y (MIRR_H ? y : (HEIGHT - y - 1))

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define ROTATED_MATRIX
#define THIS_X (MIRR_V ? y : (HEIGHT - y - 1))
#define THIS_Y (MIRR_H ?  x : (WIDTH - x - 1))

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (MIRR_V ?  x : (WIDTH - x - 1))
#define THIS_Y (MIRR_H ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define ROTATED_MATRIX
#define THIS_X (MIRR_V ? (HEIGHT - y - 1) : y)
#define THIS_Y (MIRR_H ?  x : (WIDTH - x - 1))

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma warning "Wrong matrix parameters! Set to default"

#endif

constexpr uint16_t num_leds = WIDTH * HEIGHT;

namespace EffectMath_PRIVATE {
    typedef union {
    struct {
        bool MIRR_V:1; // отзрекаливание по V
        bool MIRR_H:1; // отзрекаливание по H
    };
    uint32_t flags; // набор битов для конфига
    } MATRIXFLAGS;

    extern MATRIXFLAGS matrixflags;
    extern CRGB leds[num_leds]; // основной буфер вывода изображения
    extern CRGB overrun;

    CRGB *getUnsafeLedsArray();
    uint32_t getPixelNumber(int16_t x, int16_t y);
}

using namespace EffectMath_PRIVATE;
