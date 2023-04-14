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
#include "effectmath.h"

// обертка для blur2d из FastLED
// используется встроенный блер, так что необходимости в данной функции более нет, отключено
uint16_t XY(uint8_t x, uint8_t y) {return 0;}

namespace EffectMath {

/*
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
*/

// lookup table for fast atan func
static const PROGMEM float LUT[102] = {
     0,           0.0099996664, 0.019997334, 0.029991005, 0.039978687,
     0.049958397, 0.059928156,  0.069885999, 0.079829983, 0.089758173,
     0.099668652, 0.10955953,   0.11942893,  0.12927501,  0.13909595,
     0.14888994,  0.15865526,   0.16839015,  0.17809294,  0.18776195,
     0.19739556,  0.20699219,   0.21655031,  0.22606839,  0.23554498,
     0.24497867,  0.25436807,   0.26371184,  0.27300870,  0.28225741,
     0.29145679,  0.30060568,   0.30970293,  0.31874755,  0.32773849,
     0.33667481,  0.34555557,   0.35437992,  0.36314702,  0.37185606,
     0.38050637,  0.38909724,   0.39762798,  0.40609807,  0.41450688,
     0.42285392,  0.43113875,   0.43936089,  0.44751999,  0.45561564,
     0.46364760,  0.47161558,   0.47951928,  0.48735857,  0.49513325,
     0.50284320,  0.51048833,   0.51806855,  0.52558380,  0.53303409,
     0.54041952,  0.54774004,   0.55499572,  0.56218672,  0.56931317,
     0.57637525,  0.58337301,   0.59030676,  0.59717667,  0.60398299,
     0.61072594,  0.61740589,   0.62402308,  0.63057774,  0.63707036,
     0.64350110,  0.64987046,   0.65617871,  0.66242629,  0.66861355,
     0.67474097,  0.68080884,   0.68681765,  0.69276786,  0.69865984,
     0.70449406,  0.71027100,   0.71599114,  0.72165483,  0.72726268,
     0.73281509,  0.73831260,   0.74375558,  0.74914461,  0.75448018,
     0.75976276,  0.76499283,   0.77017093,  0.77529752,  0.78037310,
     0.78539819,  0.79037325};


//--------------------------------------
// blur1d: one-dimensional blur filter. Spreads light to 2 line neighbors.
// blur2d: two-dimensional blur filter. Spreads light to 8 XY neighbors.
//
//           0 = no spread at all
//          64 = moderate spreading
//         172 = maximum smooth, even spreading
//
//         173..255 = wider spreading, but increasing flicker
//
//         Total light is NOT entirely conserved, so many repeated
//         calls to 'blur' will also result in the light fading,
//         eventually all the way to black; this is by design so that
//         it can be used to (slowly) clear the LEDs to black.
void blur2d( LedFB &leds, fract8 blur_amount)
{
    EffectMath::blurRows(leds, blur_amount);
    EffectMath::blurColumns(leds, blur_amount);
}

// blurRows: perform a blur1d on every row of a rectangular matrix
void blurRows( LedFB &leds, fract8 blur_amount)
{
/*    for( uint8_t row = 0; row < height; ++row) {
        CRGB* rowbase = leds + (row * width);
        blur1d( rowbase, width, blur_amount);
    }
*/
    // blur rows same as columns, for irregular matrix
    uint8_t keep = 255 - blur_amount;
    uint8_t seep = blur_amount >> 1;
    for( uint8_t row = 0; row < leds.cfg.h(); row++) {
        CRGB carryover = CRGB::Black;
        for( uint8_t i = 0; i < leds.cfg.w(); i++) {
            CRGB cur = leds.pixel(i,row);
            CRGB part = cur;
            part.nscale8( seep);
            cur.nscale8( keep);
            cur += carryover;
            if( i) leds.pixel(i-1,row) += part;
            leds.pixel(i,row) = cur;
            carryover = part;
        }
    }
}

// blurColumns: perform a blur1d on each column of a rectangular matrix
void blurColumns(LedFB &leds, fract8 blur_amount)
{
    // blur columns
    uint8_t keep = 255 - blur_amount;
    uint8_t seep = blur_amount >> 1;
    for( uint8_t col = 0; col < leds.cfg.w(); ++col) {
        CRGB carryover = CRGB::Black;
        for( uint8_t i = 0; i < leds.cfg.h(); ++i) {
            CRGB cur = leds.pixel(col,i);
            CRGB part = cur;
            part.nscale8( seep);
            cur.nscale8( keep);
            cur += carryover;
            if( i) leds.pixel(col,i-1) += part;
            leds.pixel(col,i) = cur;
            carryover = part;
        }
    }
} 

// ******** общие мат. функции переиспользуются в другом эффекте
uint8_t mapsincos8(bool map, uint8_t theta, uint8_t lowest, uint8_t highest) {
  uint8_t beat = map ? sin8(theta) : cos8(theta);
  return lowest + scale8(beat, highest - lowest);
}

void MoveFractionalNoise(bool _scale, const uint8_t noise3d[][WIDTH][HEIGHT], int8_t amplitude, float shift) {
  uint8_t zD, zF;
  CRGB *leds = getUnsafeLedsArray(); // unsafe
  CRGB ledsbuff[num_leds];
  uint16_t _side_a = _scale ? HEIGHT : WIDTH;
  uint16_t _side_b = _scale ? WIDTH : HEIGHT;

  for(uint8_t i=0; i<NUM_LAYERS; i++)
    for (uint16_t a = 0; a < _side_a; a++) {
      uint8_t _pixel = _scale ? noise3d[i][0][a] : noise3d[i][a][0];
      int16_t amount = ((int16_t)(_pixel - 128) * 2 * amplitude + shift * 256);
      int8_t delta = ((uint16_t)fabs(amount) >> 8) ;
      int8_t fraction = ((uint16_t)fabs(amount) & 255);
      for (uint8_t b = 0 ; b < _side_b; b++) {
        if (amount < 0) {
          zD = b - delta; zF = zD - 1;
        } else {
          zD = b + delta; zF = zD + 1;
        }
        CRGB PixelA = CRGB::Black  ;
        if ((zD >= 0) && (zD < _side_b))
          PixelA = _scale ? getPixel(zD%WIDTH, a%HEIGHT) : getPixel(a%WIDTH, zD%HEIGHT);

        CRGB PixelB = CRGB::Black ;
        if ((zF >= 0) && (zF < _side_b))
          PixelB = _scale ? getPixel(zF%WIDTH, a%HEIGHT) : getPixel(a%WIDTH, zF%HEIGHT);
        uint16_t x = _scale ? b : a;
        uint16_t y = _scale ? a : b;
        ledsbuff[getPixelNumber(x%WIDTH, y%HEIGHT)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));   // lerp8by8(PixelA, PixelB, fraction );
      }
    }
  memcpy(leds, ledsbuff, sizeof(CRGB)* num_leds);
}

uint8_t ceil8(uint8_t a, uint8_t b){ return a/b + !!(a%b); }

/* kostyamat добавил
функция увеличения яркости  */
CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter)
{
  CRGB incrementalColor = color;
  incrementalColor.nscale8( howMuchBrighter);
  return color + incrementalColor;
}

/* kostyamat добавил
 функция уменьшения яркости */
CRGB makeDarker( const CRGB& color, fract8 howMuchDarker )
{
  CRGB newcolor(color);
  newcolor.nscale8( 255 - howMuchDarker);
  return newcolor;
}

float randomf(float min, float max)
{
  return fmap(random(1024), 0, 1023, min, max);
}

/* kostyamat добавил
 функция возвращает true, если float
 ~= целое (первая цифра после запятой == 0)
bool isInteger(float val) {
    float val1;
    val1 = val - (int)val;
    if ((int)(val1 * 10) == 0)
        return true;
    else
        return false;
}
 */

// Функция создает вспышки в разных местах матрицы, параметр 0-255. Чем меньше, тем чаще.
void addGlitter(LedFB &leds, uint8_t chanceOfGlitter){
  if ( random8() < chanceOfGlitter) leds.at(random16(leds.size())) += CRGB::Gray;
}

// Функция создает разноцветные конфетти в разных местах матрицы, параметр 0-255. Чем меньше, тем чаще.
void confetti(LedFB &leds, byte density) {
  for (byte i=0; i < num_leds/256; i++)
    if ( random8() < density)
      leds[random16(leds.size())] = random(32, 16777216);
//      if (RGBweight(mx.fb->data(), idx) < 32) mx[idx] = random(32, 16777216);
}

/*
//gamma correction function
// pretty cloce to much mem-cheap fastled's dim8*
void gammaCorrection()
{
  byte r, g, b;
  for (uint16_t i = 0; i < num_leds; i++)
  {
    r = mx[i].r;
    g = mx[i].g;
    b = mx[i].b;
    mx[i].r = pgm_read_byte(gamma_exp + r);
    mx[i].g = pgm_read_byte(gamma_exp + g);
    mx[i].b = pgm_read_byte(gamma_exp + b);
  }
}
*/

/*
uint32_t getPixColor(uint32_t thisSegm) // функция получения цвета пикселя по его номеру
{
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel < num_leds ) 
    return (((uint32_t)mx[thisPixel].r << 16) | ((uint32_t)mx[thisPixel].g << 8 ) | (uint32_t)mx[thisPixel].b);
  else return 0;
  //else return (((uint32_t)overrun.r << 16) | ((uint32_t)overrun.g << 8 ) | (uint32_t)overrun.b);
}
*/

void drawPixelXY(int16_t x, int16_t y, const CRGB &color) // функция отрисовки точки по координатам X Y
{
  if (x<0 || y<0 || x>maxWidthIndex || y>maxHeightIndex) return; // skip out of canvas drawing
  getPixel(x,y) = color;
}

void wu_pixel(uint32_t x, uint32_t y, CRGB col, LedFB &fb) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  // extract the fractional parts and derive their inverses
  uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  #undef WU_WEIGHT
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t xn = (x >> 8) + (i & 1); uint16_t yn = (y >> 8) + ((i >> 1) & 1);
    CRGB clr = fb.pixel(xn,yn);
    clr.r = qadd8(clr.r, (col.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (col.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (col.b * wu[i]) >> 8);

    fb.pixel(xn, yn) = clr;
  }
}

CRGB colorsmear(const CRGB &col1, const CRGB &col2, byte l) {
  CRGB temp1 = col1;
  nblend(temp1, col2, l);
  return temp1;
}

void sDrawPixelXYF(float x, float y, const CRGB &color) {
  byte ax = byte(x);
  byte xsh = (x - byte(x)) * 255;
  byte ay = byte(y);
  byte ysh = (y - byte(y)) * 255;
  CRGB colP1 = colorsmear(color, CRGB(0, 0, 0), xsh);
  CRGB col1 = colorsmear(colP1, CRGB(0, 0, 0), ysh);
  CRGB col2 = colorsmear(CRGB(0, 0, 0), color, xsh);
  CRGB col3 = colorsmear(CRGB(0, 0, 0),colP1, ysh);
  CRGB col4 = colorsmear(CRGB(0, 0, 0),col2, ysh);

  getPixel(ax, ay) += col1;
  getPixel(ax+1, ay) += col2;
  getPixel(ax, ay+1) += col3;
  getPixel(ax+1, ay+1) += col4;
}

void sDrawPixelXYF_X(float x, int16_t y, const CRGB &color) {
  byte ax = byte(x);
  byte xsh = (x - byte(x)) * 255;
  CRGB col1 = colorsmear(color, CRGB(0, 0, 0), xsh);
  CRGB col2 = colorsmear(CRGB(0, 0, 0), color, xsh);
  getPixel(ax, y) += col1;
  getPixel(ax + 1, y) += col2;
}

void sDrawPixelXYF_Y(int16_t x, float y, const CRGB &color) {
  byte ay = byte(y);
  byte ysh = (y - byte(y)) * 255;
  CRGB col1 = colorsmear(color, CRGB(0, 0, 0), ysh);
  CRGB col2 = colorsmear(CRGB(0, 0, 0), color, ysh);
  getPixel(x, ay) += col1;
  getPixel(x, ay+1) += col2; 
}

void drawPixelXYF(float x, float y, const CRGB &color, LedFB &fb, uint8_t darklevel)
{
  if (x<-1 || y<-1 || x>fb.cfg.w() || y>fb.cfg.h()) return; // skip out of canvas drawing, allow 1 px tradeoff
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                  WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    // тут нам, ИМХО, незачем гонять через прокладки, и потом сдвигать регистры. А в случае сегмента подразумевается, 
    // что все ЛЕД в одном сегменте одинакового цвета, и достаточно получить цвет любого из них.
    CRGB clr(fb.pixel(xn, yn));
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    if (darklevel > 0) getPixel(xn, yn) = makeDarker(clr, darklevel);
    else fb.pixel(xn, yn) = clr;
  }
#undef WU_WEIGHT
}

void drawPixelXYF_X(float x, int16_t y, const CRGB &color, uint8_t darklevel)
{
  if (x<-1.0 || y<-1 || x>((float)WIDTH) || y>((float)HEIGHT)) return;

  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, ix = 255 - xx;
  // calculate the intensities for each affected pixel
  uint8_t wu[2] = {ix, xx};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int8_t i = 1; i >= 0; i--) {
    int16_t xn = x + (i & 1);
    CRGB clr = getPixel(xn, y);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    if (darklevel > 0) getPixel(xn, y) = makeDarker(clr, darklevel);
    else getPixel(xn, y) = clr;
  }
}

void drawPixelXYF_Y(int16_t x, float y, const CRGB &color, uint8_t darklevel)
{
  if (x<-1 || y<-1.0 || x>((float)WIDTH) || y>((float)HEIGHT)) return;

  // extract the fractional parts and derive their inverses
  uint8_t yy = (y - (int)y) * 255, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[2] = {iy, yy};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (int8_t i = 1; i >= 0; i--) {
    int16_t yn = y + (i & 1);
    CRGB clr = getPixel(x, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    if (darklevel > 0) getPixel(x, yn) = makeDarker(clr, darklevel);
    else getPixel(x, yn) = clr;
  }
}

CRGB getPixColorXYF(float x, float y)
{
  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  CRGB clr=CRGB::Black;
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    if(!i){
      clr = getPixel(xn, yn);
    } else {
      CRGB tmpColor = getPixel(xn, yn);
      clr.r = qadd8(clr.r, (tmpColor.r * wu[i]) >> 8);
      clr.g = qadd8(clr.g, (tmpColor.g * wu[i]) >> 8);
      clr.b = qadd8(clr.b, (tmpColor.b * wu[i]) >> 8);
    }
  }
  return clr;
  #undef WU_WEIGHT
}

CRGB getPixColorXYF_X(float x, int16_t y)
{
  if (x<-1.0 || y<-1.0 || x>((float)WIDTH) || y>((float)HEIGHT)) return CRGB::Black;

  // extract the fractional parts and derive their inverses
  uint8_t xx = (x - (int)x) * 255, ix = 255 - xx;
  // calculate the intensities for each affected pixel
  uint8_t wu[2] = {ix, xx};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  CRGB clr=CRGB::Black;
  for (int8_t i = 1; i >= 0; i--) {
      int16_t xn = x + (i & 1);
      if(i){
        clr = getPixel(xn, y);
      } else {
        CRGB tmpColor = getPixel(xn, y);
        clr.r = qadd8(clr.r, (tmpColor.r * wu[i]) >> 8);
        clr.g = qadd8(clr.g, (tmpColor.g * wu[i]) >> 8);
        clr.b = qadd8(clr.b, (tmpColor.b * wu[i]) >> 8);
      }
  }
  return clr;
}

CRGB getPixColorXYF_Y(int16_t x, float y)
{
  if (x<-1 || y<-1.0 || x>((float)WIDTH) || y>((float)HEIGHT)) return CRGB::Black;

  // extract the fractional parts and derive their inverses
  uint8_t yy = (y - (int)y) * 255, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[2] = {iy, yy};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  CRGB clr=CRGB::Black;
  for (int8_t i = 1; i >= 0; i--) {
      int16_t yn = y + (i & 1);
      if(i){
        clr = getPixel(x, yn);
      } else {
        CRGB tmpColor = getPixel(x, yn);
        clr.r = qadd8(clr.r, (tmpColor.r * wu[i]) >> 8);
        clr.g = qadd8(clr.g, (tmpColor.g * wu[i]) >> 8);
        clr.b = qadd8(clr.b, (tmpColor.b * wu[i]) >> 8);
      }
  }
  return clr;
}

/*!
   @brief    Write a line.  Bresenham's algorithm - thx wikpedia
   https://github.com/adafruit/Adafruit-GFX-Library
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color CRGB Color to draw with
*/
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const CRGB &color, LedFB &fb) {
  // discard lines that for sure goes out of canvas,
  // the rest will be caught on pixel access level 
  if (x0<0 && x1<0) return;
  if (y0<0 && y1<0) return;
  if (x0>maxWidthIndex && x1>maxWidthIndex) return;
  if (y0>maxHeightIndex && y1>maxHeightIndex) return;

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }

  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      fb.pixel(y0, x0) = color;
    } else {
      fb.pixel(x0, y0) = color;
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void drawLineF(float x1, float y1, float x2, float y2, const CRGB &color, LedFB &fb){
  // discard lines that for sure goes out of canvas,
  // the rest will be caught on pixel access level 
  if (x1<0 && x2<0) return;
  if (y1<0 && y2<0) return;
  if (x1>fb.cfg.maxWidthIndex() && x2>fb.cfg.maxWidthIndex()) return;
  if (y1>fb.cfg.maxHeightIndex() && y2>fb.cfg.maxHeightIndex()) return;

  float deltaX = fabs(x2 - x1);
  float deltaY = fabs(y2 - y1);
  float error = deltaX - deltaY;

  float signX = x1 < x2 ? 0.5 : -0.5;
  float signY = y1 < y2 ? 0.5 : -0.5;

  while (x1 != x2 || y1 != y2) { // (true) - а я то думаю - "почему функция часто вызывает вылет по вачдогу?" А оно вон оно чё, Михалычь!
    if ((signX > 0. && x1 > x2 + signX) || (signX < 0. && x1 < x2 + signX))
      break;
    if ((signY > 0. && y1 > y2 + signY) || (signY < 0. && y1 < y2 + signY))
      break;
    drawPixelXYF(x1, y1, color, fb);
    float error2 = error;
    if (error2 > -deltaY)
    {
      error -= deltaY;
      x1 += signX;
      }
      if (error2 < deltaX) {
          error += deltaX;
          y1 += signY;
      }
  }
}

void drawSquareF(float x, float y, float leg, CRGB color, LedFB &fb) {
  drawLineF(x+leg,y+leg,x+leg,y-leg,color, fb);
  drawLineF(x+leg,y-leg,x-leg,y-leg,color, fb);
  drawLineF(x-leg,y-leg,x-leg,y+leg,color, fb);
  drawLineF(x-leg,y+leg,x+leg,y+leg,color, fb);
}

void drawCircle(int x0, int y0, int radius, const CRGB &color, LedFB &fb){
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    fb.pixel(x0, y0) = color;
    return;
  }

  while (a >= b)  {
    fb.pixel(a + x0, b + y0) = color;
    fb.pixel(b + x0, a + y0) = color;
    fb.pixel(-a + x0, b + y0) = color;
    fb.pixel(-b + x0, a + y0) = color;
    fb.pixel(-a + x0, -b + y0) = color;
    fb.pixel(-b + x0, -a + y0) = color;
    fb.pixel(a + x0, -b + y0) = color;
    fb.pixel(b + x0, -a + y0) = color;
    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

void drawCircleF(float x0, float y0, float radius, const CRGB &color, LedFB &fb, float step){
  float a = radius, b = 0.;
  float radiusError = step - a;

  if (radius <= step*2) {
    drawPixelXYF(x0, y0, color, fb);
    return;
  }

  while (a >= b)  {
      drawPixelXYF(a + x0, b + y0, color, fb, 50);
      drawPixelXYF(b + x0, a + y0, color, fb, 50);
      drawPixelXYF(-a + x0, b + y0, color, fb, 50);
      drawPixelXYF(-b + x0, a + y0, color, fb, 50);
      drawPixelXYF(-a + x0, -b + y0, color, fb, 50);
      drawPixelXYF(-b + x0, -a + y0, color, fb, 50);
      drawPixelXYF(a + x0, -b + y0, color, fb, 50);
      drawPixelXYF(b + x0, -a + y0, color, fb, 50);

    b+= step;
    if (radiusError < 0.)
      radiusError += 2. * b + step;
    else
    {
      a-= step;
      radiusError += 2 * (b - a + step);
    }
  }
}

void fill_circleF(float cx, float cy, float radius, CRGB col, LedFB &fb) {
  int8_t rad = radius;
  for (float y = -radius; y < radius; y += (fabs(y) < rad ? 1 : 0.2)) {
    for (float x = -radius; x < radius; x += (fabs(x) < rad ? 1 : 0.2)) {
      if (x * x + y * y < radius * radius)
        drawPixelXYF(cx + x, cy + y, col, fb, 0);
    }
  }
}

//uint16_t RGBweight (CRGB *leds, uint16_t idx) {return (leds[idx].r + leds[idx].g + leds[idx].b);}

void nightMode(LedFB &ledarr){
    for (auto &i : ledarr){
        i.r = dim8_lin(i.r); //dim8_video
        i.g = dim8_lin(i.g);
        i.b = dim8_lin(i.b);
    }
}


uint32_t getPixelNumberBuff(uint16_t x, uint16_t y, uint8_t W , uint8_t H) // получить номер пикселя в буфере по координатам
{

  uint16_t _THIS_Y = y;
  uint16_t _THIS_X = x;
  
  if ((_THIS_Y % 2 == 0) || MATRIX_TYPE)                     // если чётная строка
  {
      return ((uint32_t)_THIS_Y * SEGMENTS * W + _THIS_X);
  }
  else                                                      // если нечётная строка
  {
      return ((uint32_t)_THIS_Y * SEGMENTS * W + W - _THIS_X - 1);
  }

}

/*  some other funcs depends on this */
CRGB &getPixel(uint16_t x, uint16_t y){
  return mx.pixel(x,y);
  // Все, что не попадает в диапазон WIDTH x HEIGHT отправляем в "невидимый" светодиод.
//  if (y > getmaxHeightIndex() || x > getmaxWidthIndex())
//      return overrun;
//  return leds[getPixelNumber(x,y)];
}

float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max){
  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}
double dmap(const double x, const double in_min, const double in_max, const double out_min, const double out_max){
  return (out_max - out_min) * (x - in_min) / (in_max - in_min) + out_min;
}

float distance(float x1, float y1, float x2, float y2){
    float dx = x2 - x1, dy = y2 - y1;
    return EffectMath::sqrt((dx * dx) + (dy * dy));
}

float sqrt(float x){
  union{
      int i;
      float x;
  } u;

  u.x = x;
  // u.i = (1<<29) + (u.i >> 1) - (1<<22);
  // u.i = 0x20000000 + (u.i >> 1) - 0x400000;
  u.i = (u.i >> 1) + 0x1FC00000;
  return u.x;
}

float tan2pi_fast(float x) {
  float y = (1 - x*x);
  return x * (((-0.000221184 * y + 0.0024971104) * y - 0.02301937096) * y + 0.3182994604 + 1.2732402998 / y);
  //float y = (1 - x*x);
  //return x * (-0.0187108 * y + 0.31583526 + 1.27365776 / y);
}

float atan2_fast(float y, float x)
{
  //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  //Volkan SALMA

  const float ONEQTR_PI = PI / 4.0;
  const float THRQTR_PI = 3.0 * PI / 4.0;
  float r, angle;
  float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
  if ( x < 0.0f )
  {
      r = (x + abs_y) / (abs_y - x);
      angle = THRQTR_PI;
  }
  else
  {
      r = (x - abs_y) / (x + abs_y);
      angle = ONEQTR_PI;
  }
  angle += (0.1963f * r * r - 0.9817f) * r;
  if ( y < 0.0f )
      return( -angle );     // negate if in quad III or IV
  else
      return( angle );
}

float atan_fast(float x){
  /*
  A fast look-up method with enough accuracy
  */
  if (x > 0) {
      if (x <= 1) {
      int index = round(x * 100);
      return LUT[index];
      } else {
      float re_x = 1 / x;
      int index = round(re_x * 100);
      return (M_PI_2 - LUT[index]);
      }
  } else {
      if (x >= -1) {
      float abs_x = -x;
      int index = round(abs_x * 100);
      return -(LUT[index]);
      } else {
      float re_x = 1 / (-x);
      int index = round(re_x * 100);
      return (LUT[index] - M_PI_2);
      }
  }
}

float mapcurve(const float x, const float in_min, const float in_max, const float out_min, const float out_max, float (*curve)(float,float,float,float)){
  if (x <= in_min) return out_min;
  if (x >= in_max) return out_max;
  return curve((x - in_min), out_min, (out_max - out_min), (in_max - in_min));
}

float linear(float t, float b, float c, float d) { return c * t / d + b; }

float InQuad(float t, float b, float c, float d) { t /= d; return c * t * t + b; }

float OutQuad(float t, float b, float c, float d) { t /= d; return -c * t * (t - 2) + b; }

float InOutQuad(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return c / 2 * t * t + b;
  --t;
  return -c / 2 * (t * (t - 2) - 1) + b;
}

float InCubic(float t, float b, float c, float d) { t /= d; return c * t * t * t + b; }

float OutCubic(float t, float b, float c, float d) { t = t / d - 1; return c * (t * t * t + 1) + b; }

float InQuart(float t, float b, float c, float d) { t /= d; return c * t * t * t * t + b; }

float OutQuart(float t, float b, float c, float d) { t = t / d - 1; return -c * (t * t * t * t - 1) + b; }

float InOutCubic(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return c / 2 * t * t * t + b;
  t -= 2;
  return c / 2 * (t * t * t + 2) + b;
}

float InOutQuart(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return c / 2 * t * t * t * t + b;
  t -= 2;
  return -c / 2 * (t * t * t * t - 2) + b;
}

float InQuint(float t, float b, float c, float d) { t /= d; return c * t * t * t * t * t + b; }

float fixed_to_float(int input){ return ((float)input / (float)(1 << 16)); }

int float_to_fixed(float input){ return (int)(input * (1 << 16)); }

float OutQuint(float t, float b, float c, float d) {
  t = t / d - 1;
  return c * (t * t * t * t * t + 1) + b;
}

float InExpo(float t, float b, float c, float d) { return (t==0) ? b : c * powf(2, 10 * (t/d - 1)) + b; }
float OutExpo(float t, float b, float c, float d) { return (t==d) ? b+c : c * (-powf(2, -10 * t/d) + 1) + b; }
float InOutExpo(float t, float b, float c, float d);
float InCirc(float t, float b, float c, float d) { t /= d; return -c * (sqrt(1 - t * t) - 1) + b; }
float OutCirc(float t, float b, float c, float d) { t = t / d - 1; return c * sqrt(1 - t * t) + b; }

float InOutQuint(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return  c / 2 * t * t * t * t * t + b;
  t -= 2;
  return c / 2 * (t * t * t * t * t + 2) + b;
}

float InOutExpo(float t, float b, float c, float d) {
  if (t==0) return b;
  if (t==d) return b + c;
  t /= d / 2;
  if (t < 1) return c/2 * powf(2, 10 * (t - 1)) + b;
  --t;
  return c/2 * (-powf(2, -10 * t) + 2) + b;
}

float InOutCirc(float t, float b, float c, float d) {
  t /= d / 2;
  if (t < 1) return -c/2 * (sqrt(1 - t*t) - 1) + b;
  t -= 2;
  return c/2 * (sqrt(1 - t*t) + 1) + b;
}

} // namespace EffectMath

void Boid::update() {
  // Update velocity
  velocity += acceleration;
  // Limit speed
  velocity.limit(maxspeed);
  location += velocity;
  // Reset acceleration to 0 each cycle
  acceleration *= 0;
}

void Boid::repelForce(PVector obstacle, float radius) {
  //Force that drives boid away from obstacle.

  PVector futPos = location + velocity; //Calculate future position for more effective behavior.
  PVector dist = obstacle - futPos;
  float d = dist.mag();

  if (d <= radius) {
    PVector repelVec = location - obstacle;
    repelVec.normalize();
    if (d != 0) { //Don't divide by zero.
      // float scale = 1.0 / d; //The closer to the obstacle, the stronger the force.
      repelVec.normalize();
      repelVec *= (maxforce * 7);
      if (repelVec.mag() < 0) { //Don't let the boids turn around to avoid the obstacle.
        repelVec.y = 0;
      }
    }
    applyForce(repelVec);
  }
}

void Boid::flock(Boid boids [], uint8_t boidCount) {
  PVector sep = separate(boids, boidCount);   // Separation
  PVector ali = align(boids, boidCount);      // Alignment
  PVector coh = cohesion(boids, boidCount);   // Cohesion
  // Arbitrarily weight these forces
  sep *= 1.5;
  ali *= 1.0;
  coh *= 1.0;
  // Add the force vectors to acceleration
  applyForce(sep);
  applyForce(ali);
  applyForce(coh);
}

PVector Boid::separate(Boid boids [], uint8_t boidCount) {
  PVector steer = PVector(0, 0);
  int count = 0;
  // For every boid in the system, check if it's too close
  for (int i = 0; i < boidCount; i++) {
    Boid other = boids[i];
    if (!other.enabled)
      continue;
    float d = location.dist(other.location);
    // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
    if ((d > 0) && (d < desiredseparation)) {
      // Calculate vector pointing away from neighbor
      PVector diff = location - other.location;
      diff.normalize();
      diff /= d;        // Weight by distance
      steer += diff;
      count++;            // Keep track of how many
    }
  }
  // Average -- divide by how many
  if (count > 0) {
    steer /= (float) count;
  }

  // As long as the vector is greater than 0
  if (steer.mag() > 0) {
    // Implement Reynolds: Steering = Desired - Velocity
    steer.normalize();
    steer *= maxspeed;
    steer -= velocity;
    steer.limit(maxforce);
  }
  return steer;
}

PVector Boid::align(Boid boids [], uint8_t boidCount) {
  PVector sum = PVector(0, 0);
  int count = 0;
  for (int i = 0; i < boidCount; i++) {
    Boid other = boids[i];
    if (!other.enabled)
      continue;
    float d = location.dist(other.location);
    if ((d > 0) && (d < neighbordist)) {
      sum += other.velocity;
      count++;
    }
  }
  if (count > 0) {
    sum /= (float) count;
    sum.normalize();
    sum *= maxspeed;
    PVector steer = sum - velocity;
    steer.limit(maxforce);
    return steer;
  }
  else {
    return PVector(0, 0);
  }
}

PVector Boid::cohesion(Boid boids [], uint8_t boidCount) {
  PVector sum = PVector(0, 0);   // Start with empty vector to accumulate all locations
  int count = 0;
  for (int i = 0; i < boidCount; i++) {
    Boid other = boids[i];
    if (!other.enabled)
      continue;
    float d = location.dist(other.location);
    if ((d > 0) && (d < neighbordist)) {
      sum += other.location; // Add location
      count++;
    }
  }
  if (count > 0) {
    sum /= count;
    return seek(sum);  // Steer towards the location
  }
  else {
    return PVector(0, 0);
  }
}

PVector Boid::seek(PVector target) {
  PVector desired = target - location;  // A vector pointing from the location to the target
  // Normalize desired and scale to maximum speed
  desired.normalize();
  desired *= maxspeed;
  // Steering = Desired minus Velocity
  PVector steer = desired - velocity;
  steer.limit(maxforce);  // Limit to maximum steering force
  return steer;
}

void Boid::arrive(PVector target) {
  PVector desired = target - location;  // A vector pointing from the location to the target
  float d = desired.mag();
  // Normalize desired and scale with arbitrary damping within 100 pixels
  desired.normalize();
  if (d < 4) {
    float m = map(d, 0, 100, 0, maxspeed);
    desired *= m;
  }
  else {
    desired *= maxspeed;
  }

  // Steering = Desired minus Velocity
  PVector steer = desired - velocity;
  steer.limit(maxforce);  // Limit to maximum steering force
  applyForce(steer);
}

void Boid::wrapAroundBorders() {
  if (location.x < 0) location.x = WIDTH - 1;
  if (location.y < 0) location.y = HEIGHT - 1;
  if (location.x >= WIDTH) location.x = 0;
  if (location.y >= HEIGHT) location.y = 0;
}


void Boid::avoidBorders() {
  PVector desired = velocity;

  if (location.x < 8) desired = PVector(maxspeed, velocity.y);
  if (location.x >= WIDTH - 8) desired = PVector(-maxspeed, velocity.y);
  if (location.y < 8) desired = PVector(velocity.x, maxspeed);
  if (location.y >= HEIGHT - 8) desired = PVector(velocity.x, -maxspeed);

  if (desired != velocity) {
    PVector steer = desired - velocity;
    steer.limit(maxforce);
    applyForce(steer);
  }

  if (location.x < 0) location.x = 0;
  if (location.y < 0) location.y = 0;
  if (location.x >= WIDTH) location.x = WIDTH - 1;
  if (location.y >= HEIGHT) location.y = HEIGHT - 1;
}

bool Boid::bounceOffBorders(float bounce) {
  bool bounced = false;

  if (location.x >= WIDTH) {
    location.x = WIDTH - 1;
    velocity.x *= -bounce;
    bounced = true;
  }
  else if (location.x < 0) {
    location.x = 0;
    velocity.x *= -bounce;
    bounced = true;
  }

  if (location.y >= HEIGHT) {
    location.y = HEIGHT - 1;
    velocity.y *= -bounce;
    bounced = true;
  }
  else if (location.y < 0) {
    location.y = 0;
    velocity.y *= -bounce;
    bounced = true;
  }

  return bounced;
}