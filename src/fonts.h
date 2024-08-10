/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (vortigont)

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

#include <array>
#include "clib/u8g2.h"

/*
  List of used U8G2 fonts

// Numeric only fonts
u8g2_font_fewture_tn                - 14x14 https://github.com/olikraus/u8g2/wiki/fntgrptulamide
u8g2_font_7x14B_tn                  - 7x14  https://github.com/olikraus/u8g2/wiki/fntgrpx11

// latin fonts
u8g2_font_tiny_simon_tr             - 3x7   https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2
u8g2_font_doomalpha04_tr            - 13x13 https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2#font-pictures
u8g2_font_greenbloodserif2_tr       - 15x16 https://github.com/olikraus/u8g2/wiki/fntgrpbitfontmaker2#font-pictures


// cyrillic fonts
u8g2_font_5x8_t_cyrillic            - 5x8 Lat/Cyrillic font
u8g2_font_8x13_t_cyrillic           - 8x13 Lat/Cyrillic font  https://github.com/olikraus/u8g2/wiki/fntgrpx11
u8g2_font_unifont_t_cyrillic        - 16x16 Lat/Cyrillic font https://github.com/olikraus/u8g2/wiki/fntgrpunifont
*/

// array of available U8G2 fonts
static constexpr std::array<const uint8_t*, 20> fonts = {
  u8g2_font_5x8_t_cyrillic,
  u8g2_font_8x13_t_cyrillic,
  u8g2_font_unifont_t_cyrillic,
  u8g2_font_fewture_tn,
  u8g2_font_7x14B_tn,
  u8g2_font_tiny_simon_tr,
  u8g2_font_doomalpha04_tr,
  u8g2_font_greenbloodserif2_tr,
  u8g2_font_freedoomr25_tn,
  u8g2_font_logisoso20_tn,
  u8g2_font_logisoso42_tn,
  u8g2_font_mystery_quest_32_tn,
  u8g2_font_mystery_quest_48_tn,
  u8g2_font_maniac_tn,
  u8g2_font_lucasarts_scumm_subtitle_o_tn,
  u8g2_font_bubble_tn,
  u8g2_font_moosenooks_tr,
  u8g2_font_osr29_tn,
  u8g2_font_osb21_tn,
  u8g2_font_osb29_tn
};

// array of available Adafruit fonts
//static constexpr std::array<const GFXfont*, 8> fonts = {&FreeSerif9pt8b, &FreeSerifBold9pt8b, &Cooper6pt8b, &Cooper8pt8b, &CrystalNormal8pt8b, &CrystalNormal10pt8b, &Org_01, &TomThumb};
