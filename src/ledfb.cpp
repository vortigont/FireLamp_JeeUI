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

#include "ledfb.hpp"
#ifdef XY_EXTERN
#include "XY.h"
#endif

#ifdef MATRIXx4
  #include "matrix4.h"
#endif

// ************* НАСТРОЙКА МАТРИЦЫ *****
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (cfg.vmirror ? (WIDTH - x - 1) : x)
#define THIS_Y (cfg.hmirror ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X (cfg.vmirror ? (HEIGHT - y - 1) : y)
#define THIS_Y (cfg.hmirror ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X (cfg.vmirror ? (WIDTH - x - 1) : x)
#define THIS_Y (cfg.hmirror ?  y : (HEIGHT - y - 1))

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (cfg.vmirror ? y : (HEIGHT - y - 1))
#define THIS_Y (cfg.hmirror ? (WIDTH - x - 1) : x)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (cfg.vmirror ?  x : (WIDTH - x - 1))
#define THIS_Y (cfg.hmirror ? y : (HEIGHT - y - 1))

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (cfg.vmirror ? y : (HEIGHT - y - 1))
#define THIS_Y (cfg.hmirror ?  x : (WIDTH - x - 1))

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (cfg.vmirror ?  x : (WIDTH - x - 1))
#define THIS_Y (cfg.hmirror ? (HEIGHT - y - 1) : y)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X (cfg.vmirror ? (HEIGHT - y - 1) : y)
#define THIS_Y (cfg.hmirror ?  x : (WIDTH - x - 1))

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma warning "Wrong matrix parameters! Set to default"

#endif

static CRGB blackhole;              // Kostyamat's invisible pixel :) current effects code can't live w/o it

uint32_t LedFB::transpose(uint16_t x, uint16_t y){
#if defined(XY_EXTERN)
    return pgm_read_dword(&XYTable[y * WIDTH + x]);
#elif defined(MATRIXx4)
    return matrix4_XY(x, y);
#else
    // default substitutions
    if (MATRIX_TYPE || (THIS_Y % 2 == 0))                     // если чётная строка
    {
        return (THIS_Y * SEGMENTS * _WIDTH + THIS_X);
    }
    else                                                      // если нечётная строка
    {
        return (THIS_Y * SEGMENTS * _WIDTH + _WIDTH - THIS_X - 1);
    }
#endif
}

CRGB& LedFB::at(size_t i){ return i < fb->size() ? fb->at(i) : blackhole; };
