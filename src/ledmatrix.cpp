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

#include "ledmatrix.hpp"

#ifdef MATRIXx4
  #include "matrix4.h"
#endif

namespace EffectMath_PRIVATE {
    MATRIXFLAGS matrixflags;
    CRGB leds[num_leds]; // основной буфер вывода изображения
    CRGB overrun;
    
    CRGB *getUnsafeLedsArray(){return leds;}

    // ключевая функция с подстройкой под тип матрицы, использует MIRR_V и MIRR_H
    uint32_t getPixelNumber(int16_t x, int16_t y) // получить номер пикселя в ленте по координатам
    {
    #if defined(XY_EXTERN)
        uint16_t i = (y * WIDTH) + x;
        uint16_t j = pgm_read_dword(&XYTable[i]);
        return j;
    #elif defined(MATRIXx4)
      return matrix4_XY(x, y);
    #else
        // хак с макроподстановкой, пусть живет пока
        #define MIRR_H matrixflags.MIRR_H
        #define MIRR_V matrixflags.MIRR_V
        
        if ((THIS_Y % 2 == 0) || MATRIX_TYPE)                     // если чётная строка
        {
            return ((uint32_t)THIS_Y * SEGMENTS * _WIDTH + THIS_X);
        }
        else                                                      // если нечётная строка
        {
            return ((uint32_t)THIS_Y * SEGMENTS * _WIDTH + _WIDTH - THIS_X - 1);
        }
    
        #undef MIRR_H
        #undef MIRR_V
    #endif
    }
}
