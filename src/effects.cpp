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

#include "patterns.h"
#include "effects.h"
#include "log.h"   // LOG macro

#define OBSOLETE_CODE

#define CENTER_X_MINOR (fb->w()/2 -  (fb->maxWidthIndex() & 0x01)) // центр матрицы по ИКСУ, сдвинутый в меньшую сторону, если ширина чётная
#define CENTER_Y_MINOR (fb->h()/2 -  (fb->maxHeightIndex() & 0x01)) // центр матрицы по ИГРЕКУ, сдвинутый в меньшую сторону, если высота чётная
#define CENTER_X_MAJOR (fb->w()/2 + !!(fb->w()%2))          // центр матрицы по ИКСУ, сдвинутый в большую сторону, если ширина чётная
#define CENTER_Y_MAJOR (fb->h()/2 + !!(fb->h()%2))          // центр матрицы по ИГРЕКУ, сдвинутый в большую сторону, если высота чётная

#if !defined (OBSOLETE_CODE)
// ------------- Эффект "Конфетти" --------------
bool EffectSparcles::run(){
  if (dryrun(3.0))
    return false;
  return sparklesRoutine();
}

// !--
void EffectSparcles::setControl(size_t idx, int32_t value){
  if(_val->getId()==3) eff = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSparcles::sparklesRoutine()
{
  fb->fade(map(scale, 1, 255, 50, 1));

  CHSV currentHSV;

  for (uint8_t i = 0; i < (uint8_t)round(2.5 * (speed / 255.0) + 1); i++) {
    uint8_t x = random8(0U, fb->w());
    uint8_t y = random8(0U, fb->h());

    if (!fb->at(x, y)) {
        currentHSV = CHSV(random8(1U, 255U), random8(192U, 255U), random8(192U, 255U));
      fb->at(x, y) = currentHSV;
    }
  }
  return true;
}

// ------ Эффект "Белая Лампа"
// ------------- белый свет (светится горизонтальная полоса по центру лампы; масштаб - высота центральной горизонтальной полосы; скорость - регулировка от холодного к тёплому; яркость - общая яркость) -------------
bool EffectWhiteColorStripe::run(){
  return whiteColorStripeRoutine();
}

// !--
void EffectWhiteColorStripe::setControl(size_t idx, int32_t value){
  if(_val->getId()==3) shift = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWhiteColorStripe::whiteColorStripeRoutine()
{
  fb->clear();

  byte _scale = scale;
  byte _speed = speed;

    if(_scale < 126){
        uint8_t centerY = fb->maxHeightIndex() / 2U;
        for (int16_t y = centerY; y >= 0; y--)
        {
          int br = (MAX_BRIGHTNESS)-constrain(map(_scale,126,1,1,15)*(centerY-y)*((centerY-y)/(fb->h()*(0.0005*brightness))),1,MAX_BRIGHTNESS); if(br<0) br=0;
          int _shift = map(shift,1,255,-centerY,centerY);

            CRGB color = CHSV(
              45U,                                                        // определяем тон
              map(_speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
              (y == centerY ? (MAX_BRIGHTNESS) : br));                        // определяем яркость для центральной вертикальной полосы (или двух) яркость всегда равна MAX_BRIGHTNESS
                                                                          // для остальных вертикальных полос яркость равна либо MAX_BRIGHTNESS, либо вычисляется по br

          for (int16_t x = 0U; x < (int16_t)fb->w(); x++)
          {
            fb->at(x, y + _shift) = color;                // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
            fb->at(x, (fb->h() - y + _shift) - 1) = color; // при нечётной - одна, но дважды
          }
        }
    } else if(_scale > 128){
        uint8_t centerX = fb->maxWidthIndex() / 2U;
        for (int16_t y = 0U; y < (int16_t)fb->h(); y++){
          for (int16_t x = centerX; x >= 0; x--){
            int br = (MAX_BRIGHTNESS)-constrain(map(_scale,128,255,1,15)*(centerX-x)*((centerX-x)/(fb->w()*(0.0005*brightness))),1,MAX_BRIGHTNESS); if(br<0) br=0;
            int _shift = map(shift,1,255,-centerX,centerX);

            CRGB color = CHSV(
              45U,                                                        // определяем тон
              map(_speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
              (x == centerX ? (MAX_BRIGHTNESS) : br));                        // определяем яркость для центральной вертикальной полосы (или двух) яркость всегда равна MAX_BRIGHTNESS
                                                                          // для остальных вертикальных полос яркость равна либо MAX_BRIGHTNESS, либо вычисляется по br

            fb->at(x + _shift, y) = color;                // при чётной ширине матрицы максимально яркими отрисуются 2 центральных вертикальных полосы
            fb->at((fb->w() - x + _shift) - 1, y) = color;  // при нечётной - одна, но дважды
          }
        }
    }
    else {
        for (int16_t y = 0; y < (int16_t)fb->h(); y++){
          for (int16_t x = 0; x < (int16_t)fb->w(); x++){
            CRGB color = CHSV(
              45U,                                                  // определяем тон
              map(_speed, 0U, 255U, 0U, 170U),                      // определяем насыщенность
              (MAX_BRIGHTNESS));
            fb->at(x, y) = color;                                   // 127 - заливка полная
          }
        }
    }
  return true;
}

// --------------------------- эффект пульс ----------------------
// Stefan Petrick's PULSE Effect mod by PalPalych for GyverLamp

// !++
void EffectPulse::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.05, 1.0) * getBaseSpeedFactor();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPulse::run() {
  // if (dryrun(3.0))
  //   return false;

  // EVERY_N_SECONDS(3){
  //   LOG(printf_P,PSTR("speed: %d, speedFactor: %5.2f, pulse_step: %5.2f\n"), speed, speedFactor, pulse_step);
  // }

  CRGBPalette16 palette;
  CRGB _pulse_color;
  uint8_t _pulse_delta = 0;

  palette = RainbowColors_p;
  uint8_t _scale = scale;

  if (pulse_step <= currentRadius) {
    for (uint8_t i = 0; i < pulse_step; i++ ) {
      uint8_t _dark = qmul8( 2U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
      if (_scale == 1) {            // 1 - случайные диски
        _pulse_hue = pulse_hue;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 17) {    // 2...17 - перелив цвета дисков
        _pulse_delta = (17U - _scale) ;
        _pulse_color = CHSV(_pulse_hueall, 255U, _dark);

      } else if (_scale <= 33) {    // 18...33 - выбор цвета дисков
        _pulse_hue = (_scale - 18U) * 16U ;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 50) {    // 34...50 - дискоцветы
        _pulse_hue += (_scale - 33U) * 0.5 ;
        _pulse_color = CHSV(_pulse_hue, 255U, _dark);

      } else if (_scale <= 67) {    // 51...67 - пузыри цветы
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
         _pulse_hue += (68U - _scale) * 7U ;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);

      } else if (_scale < 83) {     // 68...83 - выбор цвета пузырей
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_hue = (_scale - 68U) * 16U ;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);

      } else if (_scale < 100) {    // 84...99 - перелив цвета пузырей
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_delta = (_scale - 85U)  ;
        _pulse_color = CHSV(_pulse_hueall, _sat, _dark);

      } else { // 100 - случайные пузыри
        uint8_t _sat =  qsub8( 255U, cos8 (128U / (pulse_step + 1U) * (i + 1U))) ;
        _pulse_hue = pulse_hue;
        _pulse_color = CHSV(_pulse_hue, _sat, _dark);
      }
      EffectMath::drawCircle(centerX, centerY, i, _pulse_color, fb);
    }
  } else {
    fb->fade(7);
    centerX = random8(fb->w() - 5U) + 3U;
    centerY = random8(fb->h() - 5U) + 3U;
    _pulse_hueall += _pulse_delta;
    pulse_hue = random8(0U, 255U);
    currentRadius = random8(3U, 9U);
    pulse_step = 0;
  }
  pulse_step+=speedFactor;
  EffectMath::blur2d(fb, 10);
  return true;
}

// радуги 2D
// ------------- радуга вертикальная/горизонтальная ----------------
bool EffectRainbow::run(){
  // коэф. влияния замаплен на скорость, 4 ползунок нафиг не нужен
  hue += (6.0 * (speed / 255.0) + 0.05 ); // скорость смещения цвета зависит от кривизны наклна линии, коэф. 6.0 и 0.05

    twirlFactor = EffectMath::fmap((float)scale, 85, 170, 8.3, 24);      // на сколько оборотов будет закручена матрица, [0..3]

  if(scale<85){
    return rainbowHorVertRoutine(false);
  } else if (scale>170){
    return rainbowHorVertRoutine(true);
  } else {
    return rainbowDiagonalRoutine();
  }
}

bool EffectRainbow::rainbowHorVertRoutine(bool isVertical)
{
  for (uint8_t i = 0U; i < (isVertical?fb->w():fb->h()); i++)
  {
    for (uint8_t j = 0U; j < (isVertical?fb->h():fb->w()); j++)
    {
      CHSV thisColor = CHSV((hue + i * scale), 255, 255);
      fb->at((isVertical?i:j), (isVertical?j:i)) = thisColor;
    }
  }
  return true;
}

// ------------- радуга диагональная -------------
bool EffectRainbow::rainbowDiagonalRoutine()
{
  for (uint8_t i = 0U; i < fb->w(); i++)
  {
    for (uint8_t j = 0U; j < fb->h(); j++)
    {
      CRGB thisColor = CHSV((uint8_t)(hue + ((float)fb->w() / (float)fb->h() * i + j * twirlFactor) * ((float)255 / (float)fb->maxDim())), 255, 255);
      fb->at(i, j) = thisColor;
    }
  }
  return true;
}

// ------------- цвета -----------------
void EffectColors::load(){
    fb->fill(CHSV(scale, 255U, 55U));
}

bool EffectColors::run(){
  return colorsRoutine();
}

// void EffectColors::setscl(const byte _scl){
//   EffectCalc::setscl(_scl);
//   modeColor = scale;
// }

// !--
void EffectColors::setControl(size_t idx, int32_t value){
  if(_val->getId()==2) modeColor = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectColors::colorsRoutine()
{
  unsigned int delay = (speed==1)?4294967294:255-speed+1; // на скорости 1 будет очень долгое ожидание)))

  ihue = (speed==1)?scale:ihue;
  step=(step+1)%(delay+1);

  if(step!=delay) {

    // выключен микрофон
    switch(mode){
      case 1:
        if(!step){
          if(!modeColor){
            modeColor = scale;
            fb->fill(CHSV(modeColor, 255U, 255U));
          }
          else {
            modeColor = 0;
            fb->clear();
          }          
        }
        break;
      case 2:
        fb->fill(CHSV(ihue, 255U, 255U));
        break;
      case 3:
        if(!step){
          if(scale<=127)
            modeColor = ~modeColor;
          else
            modeColor = modeColor ? 0 : map(scale,128,255,1,255);
          fb->fill(CHSV(modeColor, 255U, 255U));
        }
        break;
      default:
        fb->fill(CHSV(ihue, 255U, 255U));      
    }

  } else {
    ihue += scale; // смещаемся на следущий
  }
  return true;
}

// ------------- Эффект "New Матрица" ---------------
// !--
void EffectMatrix::setControl(size_t idx, int32_t value)
{
  if(_val->getId()==1) _speed = EffectMath::fmap((float)EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.06, 0.4)*getBaseSpeedFactor();
  else if(_val->getId()==3){
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    lighters.assign( map(_scale, 1, 32, 1, fb->w()), Lighter() );
    lighters.shrink_to_fit();
    load();
  }
  else if(_val->getId()==4) _hue = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) gluk = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)

  if (_hue == 1) {
    randColor = true;
    white = false;
  } else if (_hue == 255) {
    white = true;
    randColor = false;
  } else {
    randColor = false;
    white = false;
  }
  return String();
}

bool EffectMatrix::run(){
  return matrixRoutine();
}

void EffectMatrix::load(){
  randomSeed(micros());
  for (auto &i : lighters){
    i.posX = random(0, fb->w());
    i.posY = EffectMath::randomf(fb->h() - fb->h() /2, fb->h());
    i.spdX = 1;
    i.spdY = (float)random(10, 20) / 10.0f;
    i.color = hue;
    i.light = random(196,255);
  }
}

bool EffectMatrix::matrixRoutine()
{
  
  fb->dim(map(speed, 1, 255, 252, 240));
  
  CHSV color;

  for (auto &i : lighters){
    i.posY -= i.spdY * _speed;

    if (white) {
      color = rgb2hsv_approximate(CRGB::Gray);
      color.val = i.light;
    } else if (randColor) {
      EVERY_N_MILLIS(600 * speedFactor / _speed) {
        hue = random(1, 250);
      }
      color = CHSV(hue, 255, i.light);
    } else {
      color = CHSV(_hue, 255, i.light);
    }


    EffectMath::drawPixelXYF_Y(i.posX, i.posY, color, fb);

    count += speedFactor;

    if (gluk > 1 and (uint8_t)count%2 == 0) 
      if (random8() < gluk * 2) {
        i.posX = i.posX + random(-1, 2);
        i.light = random(196,255);
      }

    if(i.posY < -1) {
      i.posX = random(0, fb->maxWidthIndex());
      i.posY = EffectMath::randomf(fb->h() - fb->h() /2, fb->h());
      i.spdY = EffectMath::randomf(1.5, 2.5);
      i.light = random(127U, 255U);
      i.color = hue;
    }
  }

  return true;
}

// ------------- звездопад/метель -------------
bool EffectStarFall::run(){
  return snowStormStarfallRoutine();
}

void EffectStarFall::load(){
  randomSeed(millis());
  for (auto &i : lighters){
    i.posX = random(-fb->w(), fb->w());
    i.posY = random(fb->maxHeightIndex(), fb->h() + 4);
    i.spdX = EffectMath::randomf(-1, 1);  // X
    i.spdY = EffectMath::randomf(1, 2);   // Y
    i.color = random(0U, 255U);
    i.light = 255;
  }
}

// !++
void EffectStarFall::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) _speed = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.25, .5)*getBaseSpeedFactor();
  else if(_val->getId()==3){
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    lighters.assign( map(_scale, 1, 10, LIGHTERS_MAX/8, LIGHTERS_MAX), Lighter() );
    lighters.shrink_to_fit();
    load();
  }
  else if(_val->getId()==4) effId = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) isNew = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectStarFall::snowStormStarfallRoutine(){
  fb->dim(255 - (effId == 2 ? 70 : 60) * _speed);
  CHSV color;
  for (auto &i : lighters){
    //color = CHSV((effId > 1 ? lightersColor[i] : 255), (effId > 1 ? light[i] : 0), (effId > 1 ? 255 : light[i]));
    switch (effId)
    {
    case 1:
      color = CHSV(127, 0, i.light);
      break;
    case 2:
      if (i.light > 10) { byte tmp = i.light - 10 * _speed; color = CHSV(i.color, 255 - i.light, tmp); i.light=tmp; }
      else color = rgb2hsv_approximate( CRGB::Black);
      break;
    default:
      color = CHSV(i.color, 255, i.light);
    }

    if (isNew) {
      i.posX -= _speed * (effId == 1 ? lighters[0].spdX : i.spdX);
      i.posY -= 1 * _speed;
    } else {
      i.posX += i.spdX * (_speed / 2);
      i.posY -= i.spdY * (_speed / 2);
    }

    EffectMath::drawPixelXYF(i.posX, i.posY, color, fb, 0);

    if(i.posY < -1) {
      if (isNew) {
        i.posX = random(-fb->w(), fb->w());
        i.posY = effId > 1 ? random(fb->h() / 2, fb->h() + 4) : random(fb->maxHeightIndex(), fb->h() + 4);
        i.spdX = EffectMath::randomf(-1, 1);  // X
        i.spdY = EffectMath::randomf(1, 2);   // Y
      } else {
        i.posX = (float)random(-(fb->w() * 10 - 2), (fb->w() * 10 - 2)) / 10.0f;
        i.posY = random(fb->h(), fb->h() + 4);
        i.spdX = (float)random(15, 25) / 10.0f;   // X
        i.spdY = i.spdX; // Y
      }
      i.color = random(0U, 255U);
      i.light = random(127U, 255U);
    }
  }
  return true;
}

// ------------- светлячки --------------
//#define LIGHTERS_AM           (100U)
void EffectLighters::load(){
  randomSeed(micros());
  for (auto &i : lighters){
    i.posX = static_cast<float>(random(0, fb->w()));
    i.posY = static_cast<float>(random(0, fb->h()));

    i.spdX = random(-fb->w()/2, fb->w()/2);
    i.spdY = random(-fb->h()/2, fb->h()/2);

    i.color = random(0U, 255U);
    i.light = random(1U, 3U)*127U;
  }
}

// !++
void EffectLighters::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 4096.0f + 0.005f)*getBaseSpeedFactor();
  else if(_val->getId()==3) {
    lighters.assign(EffectCalc::setDynCtrl(_val).toInt(), Lighter());
    lighters.shrink_to_fit();
    load();
  }
  else if(_val->getId()==4) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectLighters::run(){
  fb->fade(50);

  EVERY_N_MILLIS(333)   // randomize lighters motion
  {
    for (auto &i : lighters){
      i.spdX += random(-fb->w()/4, fb->w()/4) + random(0, 10)/10.0;
      i.spdY += random(-fb->h()/4, fb->h()/4) + random(0, 10)/10.0;

      i.spdX = fmod(i.spdX, 21);
      i.spdY = fmod(i.spdY, 21);

      i.light = random(255U-(lighters.size()*8), 255U);
      if(!lighters.size())
        i.light = 127;
    }
  }

  int c = 0;  // some color shifter
  for (auto &i : lighters){
    // EVERY_N_SECONDS(1) { LOG.printf_P("S0:%d S1:%d P0:%3.2f P1:%3.2f, cnt:%3.2f\n", lightersSpeed[0U][i], lightersSpeed[1U][i],lightersPos[0U][i],lightersPos[1U][i],speedFactor); }
    i.posX += i.spdX*speedFactor;
    i.posY += i.spdY*speedFactor;

    if (i.posX < 0) i.posX = (float)fb->maxWidthIndex();
    if (i.posX > fb->maxWidthIndex()) i.posX = 0.0f;

    if (i.posY < 0.0f){
      i.posY = fb->maxHeightIndex();
      i.spdY = -i.spdY/10.0;
      i.spdX = -i.spdX/10.0;
    }
    if (i.posY > fb->maxHeightIndex()){
      i.posY = 0;
      i.spdY = -i.spdY/10.0;
      i.spdX = -i.spdX/10.0;
    }
    //LOG(printf, "Draw P0:%3.2f P1:%3.2f S0:%3.2f S1:%3.2f\n", lightersPos[0U][i], lightersPos[1U][i], lightersSpeed[0U][i], lightersSpeed[1U][i]);

    if (subPix)
      EffectMath::drawPixelXYF(i.posX, i.posY, CHSV(i.color, 255U-(++c*2), i.light), fb, 0);
    else
      fb->at(static_cast<int>(i.posX), static_cast<int>(i.posY)) = CHSV(i.color, 255U-(++c*2), i.light);
  }
  return true;
}

// ------------- светлячки со шлейфом -------------
// !++
void EffectLighterTracers::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.01, .1)*getBaseSpeedFactor();
  else if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectLighterTracers::load(){
  for (uint8_t j = 0U; j < _AMOUNT; j++)
  {
    int8_t sign;
    // забиваем случайными данными
    coord[j][0U] = (float)fb->w() / 2.0f;
    random(0, 2) ? sign = 1 : sign = -1;
    vector[j][0U] = ((float)random(40, 150) / 10.0f) * sign;
    coord[j][1U] = (float)fb->h() / 2;
    random(0, 2) ? sign = 1 : sign = -1;
    vector[j][1U] = ((float)random(40, 150) / 10.0f) * sign;
    light[j] = 127;
    //ballColors[j] = random(0, 9) * 28;
  }
}


bool EffectLighterTracers::run(){

  return lighterTracersRoutine();
}

bool EffectLighterTracers::lighterTracersRoutine()
{

  fb->fade(map(speed, 1, 255, 6, 55)); // размер шлейфа должен сохранять размер, не зависимо от скорости

  // движение шариков
  uint8_t maxBalls = cnt;
  for (uint8_t j = 0U; j < maxBalls; j++)
  {
    ballColors[j] = (maxBalls-j) * _AMOUNT + j;

    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i] * speedFactor;
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0.0f;
        vector[j][i] = -vector[j][i];
      }
    }

    if ((uint16_t)coord[j][0U] > fb->maxWidthIndex())
    {
      coord[j][0U] = fb->maxWidthIndex();
      vector[j][0U] = -vector[j][0U];
    }
    if ((uint16_t)coord[j][1U] > fb->maxHeightIndex())
    {
      coord[j][1U] = fb->maxHeightIndex();
      vector[j][1U] = -vector[j][1U];
    }
    EVERY_N_MILLIS(random16(256, 1024)) {
      if (light[j] == 127)
        light[j] = 255;
      else light[j] = 127;
    }
    EffectMath::drawPixelXYF(coord[j][0U], coord[j][1U], CHSV(ballColors[j], 200U, 255U), fb);
  }
  EffectMath::blur2d(fb, 5);
  return true;
}

// ------------- пейнтбол -------------
// !++
void EffectLightBalls::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) speedFactor = (float)EffectCalc::setDynCtrl(_val).toInt() /255.0 +0.1;
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectLightBalls::run()
{

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  EffectMath::blur2d(fb, dim8_raw(beatsin8(3,64,100)));

  // Use two out-of-sync sine waves
  uint16_t  i = beatsin16( 79 * speedFactor, 0, 255); //91
  uint16_t  j = beatsin16( 67 * speedFactor, 0, 255); //109
  uint16_t  k = beatsin16( 53 * speedFactor, 0, 255); //73
  uint16_t  m = beatsin16( 97 * speedFactor, 0, 255); //123

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis() / (scale /16 + 1);

  fb->at( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 29, 200U, 255U);
  fb->at( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 41, 200U, 255U);
  fb->at( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 37, 200U, 255U);
  fb->at( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 53, 200U, 255U);

  return true;
}

// ------------- эффект "блуждающий кубик" -------------
// !++
void EffectBall::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) {
    speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.02, 0.15) * getBaseSpeedFactor();
  }
  else if(_val->getId()==2) {
    EffectCalc::setDynCtrl(_val).toInt();
    if (scale <= 85)
      ballSize = map(scale, 1, 85, 1U, max((uint8_t)min(fb->w(),fb->h()) / 3, 1));
    else if (scale > 85 and scale <= 170)
      ballSize = map(scale, 170, 86, 1U, max((uint8_t)min(fb->w(),fb->h()) / 3, 1));
    else
      ballSize = map(scale, 171, 255, 1U, max((uint8_t)min(fb->w(),fb->h()) / 3, 1));
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectBall::load() {
  palettesload();
  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] = i? float(fb->w() - ballSize) / 2 : float(fb->h() - ballSize) / 2;
    vectorB[i] = 7.;
    if (random(0, 2)) vectorB[i] = -vectorB[i];
    ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255));
  }
}

bool EffectBall::run() {
// каждые 5 секунд коррекция направления
  EVERY_N_MILLISECONDS(map(speed, 1, 255, 6000, 3000)) {
    //LOG(println,ballSize);
    for (uint8_t i = 0U; i < 2U; i++) {
      float correct = EffectMath::randomf(0, 2);
      
      if (fabs(vectorB[i]) <= 4)      // слишком разогнались, будем тормозить
        flag[i] = true;
      if (fabs(vectorB[i]) >= 10)     // оттормозились, будем разгоняться
        flag[i] = false;

      if (flag[i])
        vectorB[i] += vectorB[i] > 0 ? correct : -correct;    // постепенно разганяем
      else 
        vectorB[i] += vectorB[i] > 0 ? -correct : correct;    // постепенно тормозим

      if (random8() < 85)  vectorB[i] = -vectorB[i];          // резко меняем направление
    }
  }

  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] += vectorB[i] * speedFactor;
    if ((int8_t)coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255)); //CHSV(random(1, 250), random(200, 255), 255);
    }
  }
  if ((int8_t)coordB[0U] > (int8_t)(fb->w() - ballSize))
  {
    coordB[0U] = (fb->w() - ballSize);
    vectorB[0U] = -vectorB[0U];
    ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255));
  }
  if ((int8_t)coordB[1U] > (int8_t)(fb->h() - ballSize))
  {
    coordB[1U] = (fb->h() - ballSize);
    vectorB[1U] = -vectorB[1U];
    ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255));
  }

  if (scale <= 85)  // при масштабе до 85 выводим кубик без шлейфа
    fb->clear();
  else if (scale > 85 and scale <= 170)
    fb->fade(255 - map(speed, 1, 255, 245, 200)); // выводим кубик со шлейфом, длинна которого зависит от скорости.
  else
    fb->fade(255 - map(speed, 1, 255, 253, 248)); // выводим кубик с длинным шлейфом, длинна которого зависит от скорости.

  for (uint8_t i = 0; i < ballSize; i++)
  {
    for (uint8_t j = 0; j < ballSize; j++)
    {
      EffectMath::drawPixelXYF(coordB[0U] + (float)i, coordB[1U] + (float)j, ballColor, fb, 0);
    }
  }
  return true;
}
#endif  //  OBSOLETE_CODE

// ----------- Эффекты "Лава, Зебра, etc"
void Effect3DNoise::fillNoiseLED()
{
  for (size_t h = 0; h != _noise.h(); ++h)
  {
    int32_t hoffset = scale * h;
    for (size_t w = 0; w != _noise.w(); ++w)
    {
      int32_t woffset = scale * w;

      // new data in a noise map
      uint8_t data = qsub8(inoise8(_x + hoffset, _y + woffset, _z), 16);
      data = qadd8(data, scale8(data, 39));

      //if (dataSmoothing)
      //  data = scale8( _noise.at(w, h), dataSmoothing) + scale8( data, 256 - dataSmoothing);

      _noise.at(w, h) = data;

      // draw
      uint8_t index = _noise.at( w, h );
      uint8_t bri =   _noise.at( _noise.w() - w - 1, _noise.h() - h - 1 );
      // if this palette is a 'loop', add a slowly-changing base value
      if ( _cycleColor)
        index += ihue;

      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( _blur && bri > 127)
        bri = 255;
      else
        bri = dim8_raw( bri * 2);

      fb->at(w, h) = ColorFromPalette( *curPalette, index, bri);
    }
  }
  
  // apply slow drift to X and Y, just for visual variation.
  _x += speed / 8;  //* 0.125; // 1/8
  _y += speed / 16; //* 0.0625; // 1/16
  _z += speed;
/*
  for (uint8_t y = 0; y != fb->h(); y++){
    for (uint8_t x = 0; x != fb->w(); x++){
      uint8_t index = _noise.at( x % _noise.w(), y % _noise.h() );
      uint8_t bri =   _noise.at( y % _noise.w(), x % _noise.h() );
      // if this palette is a 'loop', add a slowly-changing base value
      if ( _cycleColor)
        index += ihue;

      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( _blur && bri > 127)
        bri = 255;
      else
        bri = dim8_raw( bri * 2);

      fb->at(x, y) = ColorFromPalette( *curPalette, index, bri);
    }
  }
*/
  ++ihue;
}

void Effect3DNoise::fillnoise8()
{
  for (size_t i = 0; i != _noise.w(); ++i)
  {
    int32_t ioffset = scale * i;
    for (size_t j = 0; j != _noise.h(); ++j)
    {
      int32_t joffset = scale * j;
      _noise.at(i, j) = inoise8(_x + ioffset, _y + joffset, _z);
    }
  }
  _z += speed;
}

void Effect3DNoise::load(){
  palettesload();
  fillnoise8();
}

void Effect3DNoise::setControl(size_t idx, int32_t value) {
  switch (idx){
    // 0 - move speed
    //case 0:
    //  speed = NOISE_SCALE_AMP * value / 512;
    //  break;

    // 1 - scale expected range 
    //case 1:
    //  scale = NOISE_SCALE_AMP * value / 255 + NOISE_SCALE_ADD;
    //  break;

    // 2 - Palletes
    //curPalette = &ZeebraColors_p;

    // 3 - blur pallete
    case 3:
      _blur = value;
      break;

    // 4 - cycle color
    case 4:
      _cycleColor = value;
      break;

    // 5 - smoothing
    //case 5:
    //  dataSmoothing = value;
    //  break;

    default:
      EffectCalc::setControl(idx, value);
      fillnoise8();
  }
}

bool Effect3DNoise::run(){
  fillNoiseLED();
  return true;
}


//----- Эффект "Прыгающие Мячики"
//  BouncingBalls2014 is a program that lets you animate an LED strip
//  to look like a group of bouncing balls
//  Daniel Wilson, 2014
//  https://github.com/githubcdr/Arduino/blob/master/bouncingballs/bouncingballs.ino
//  With BIG thanks to the FastLED community!
//  адаптация от SottNick
// перевод на субпиксельную графику kostyamat
bool EffectBBalls::run(){
  return bBallsRoutine();
}

void EffectBBalls::load(){
  fb->clear();
  balls.assign(scale, Ball());

  //ColorFromPalette(*curPalette, color * 9);
  randomSeed(millis());
  int i = 0;
  for (auto &bball : balls){
    bball.color = random(0, 255);
    int xx = fb->w()/(balls.size()+1) * (++i);
    bball.x = xx;
    //LOG(printf_P, PSTR("Ball n:%d x:%d\n"), i, xx);
    bball.vimpact = EffectMath::sqrt(-2 * EffectBBalls_gravity * EffectBBalls_dropH) + EffectMath::randomf( - 2., 2.);                   // And "pop" up at vImpact0
    bball.cor = 0.9 - float(i) / pow(balls.size(), 2);
    if (halo){
      bball.brightness = 200;
    } else if ( i && bball.x == balls[i-1].x){      // skip 1st interation
      bball.brightness = balls[i-1].brightness + 32;
    }
  }
}

void EffectBBalls::setControl(size_t idx, int32_t value){
  switch (idx){
    // case0 - move speed expected range 1500-700

    // scale expected range 0-25 maps to 1-width/3 pixels
    case 1:
      scale = map(value, 0, 25, 1, fb->w()/3);
      break;

    // Halo boolean
    case 2:
      halo = value;
      break;

    // 3 ring radius - raw, expect 1 to about width/2
    case 3:
      _radius = value;
      break;

    // 4 fade - raw 0-255
    case 4:
      _fade = value;
      break;

    default:
      EffectCalc::setControl(idx, value);
  }
}

bool EffectBBalls::bBallsRoutine()
{
  // resize must be done inside this routine to provide thread-safety for controls change
  if (balls.size() != scale) load();

  if (_fade)
    fb->dim(_fade);
  else
    fb->clear();

  hue += (float)speed/ 1024;
  for (auto &bball : balls){
    bballsTCycle =  millis() - bball.tlast;     // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    bballsHi = 0.55 * EffectBBalls_gravity * pow( (float)bballsTCycle / speed , 2) + bball.vimpact * (float)bballsTCycle / speed;

    if (bballsHi < 0) {
      // If the ball crossed the threshold of the "ground," put it back on the ground
      bball.tlast = millis();
      //bballsHi = 0.0f;
      bball.vimpact = bball.cor * bball.vimpact ;   // and recalculate its new upward velocity as it's old velocity * COR

      //if ( bball.vimpact < 0.01 ) bball.vimpact = EffectMath::sqrt(-2 * EffectBBalls_gravity * EffectBBalls_dropH);  // If the ball is barely moving, "pop" it back up at vImpact0
      if ( bball.vimpact < 0.1 ) // сделал, чтобы мячики меняли свою прыгучесть и положение каждый цикл
      {
        bball.cor = 0.90 - (EffectMath::randomf(0., 9.)) / pow(EffectMath::randomf(4., 9.), 2.); // сделал, чтобы мячики меняли свою прыгучесть каждый цикл
        bball.shift = bball.cor >= 0.85;                             // если мячик максимальной прыгучести, то разрешаем ему сдвинуться
        bball.vimpact = EffectMath::sqrt(-2 * EffectBBalls_gravity * EffectBBalls_dropH);
      }
    }

    bball.pos = bballsHi * (float)fb->maxHeightIndex() / EffectBBalls_dropH;       // Map "h" to a "pos" integer index position on the LED strip
    if (halo)
      bball.pos += _radius;

    if (bball.shift > 0.0f && bball.pos >= (float)fb->maxHeightIndex() - .5) {                  // если мячик получил право, то пускай сдвинется на максимальной высоте 1 раз
      bball.shift = 0.0f;
      if (bball.color % 2 == 0) {                                       // чётные налево, нечётные направо
        if (bball.x < 0) bball.x = (fb->maxWidthIndex());
        else --bball.x;
      } else {
        if (bball.x > fb->maxWidthIndex()) bball.x = 0;
        else ++bball.x;
      }
    }
  }

  // Adjust balls brightness
  for (auto bball = balls.begin(); bball != balls.end(); ++bball){
    if (halo){ // если ореол включен
    // fb->h() -  invert Y axis to match display's coordinates
      EffectMath::drawCircleF(bball->x, fb->h() - bball->pos, _radius, CHSV(bball->color + (byte)hue, 225, bball->brightness), fb);
    } else {
      if (bball == balls.begin()){
        bball->brightness = 156;
        EffectMath::drawPixelXYF_Y(bball->x, fb->h() - bball->pos, CHSV(bball->color + (byte)hue, 255, bball->brightness), fb, 5);
        continue;    // skip first iteration
      } 
      // попытка создать объем с помощью яркости. Идея в том, что шарик на переднем фоне должен быть ярче, чем другой,
      // который движится в том же Х. И каждый следующий ярче предыдущего.
      bball->brightness = bball->x == std::prev(bball)->x ? bball->brightness + 32 : 156;
      EffectMath::drawPixelXYF_Y(bball->x, fb->h() - bball->pos, CHSV(bball->color + (byte)hue, 255, bball->brightness), fb, 5);
    }
  }

  return true;
}

#if !defined (OBSOLETE_CODE)
// ***** SINUSOID3 / СИНУСОИД3 *****
/*
  Sinusoid3 by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  read more about the concept: https://www.youtube.com/watch?v=mubH-w_gwdA
*/
// !++
void EffectSinusoid3::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) e_s3_speed = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.033, 1) * getBaseSpeedFactor();
  else if(_val->getId()==2) _scale = map8(EffectCalc::setDynCtrl(_val).toInt(),50,150);
  else if(_val->getId()==3) e_s3_size = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 3, 9);
  else if(_val->getId()==4) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSinusoid3::run() {
  float time_shift = millis()&0xFFFFF; // на больших значениях будет страннео поведение, поэтому уменьшаем точность, хоть и будет иногда срыв картинки, но в 18 минут, так что - хрен с ним

  
switch (type) {
    case 0: //Sinusoid I
      for (uint8_t y = 0; y < fb->h(); y++) {
        for (uint8_t x = 0; x < fb->w(); x++) {
          CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 134.3447 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 170.3884 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.b = ~v;
          fb->at(x, y) = color;
        }
      }
      break;
    case 1: //it's not sinusoid II
      for (uint8_t y = 0; y < fb->h(); y++) {
        for (uint8_t x = 0; x < fb->w(); x++) {
		  CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (float(0.001 * time_shift * e_s3_speed) + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 68.8107 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 65.534 * time_shift))) / 32767.0;
          v = 127 * (((float)(0.001 * time_shift * e_s3_speed)) + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.g = ~v;
		  fb->at(x, y) = color;
        }
      }
      break;
    case 2: //Sinusoid III
      for (uint8_t y = 0; y < fb->h(); y++) {
        for (uint8_t x = 0; x < fb->w(); x++) {
          CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 68.8107 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 65.534 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.g = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 134.3447 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 170.3884 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.b = ~v;
          fb->at(x, y) = color;
        }
      }
      break;
    case 3: //Sinusoid IV
      for (uint8_t y = 0; y < fb->h(); y++) {
        for (uint8_t x = 0; x < fb->w(); x++) {
          CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy))) + (time_shift * e_s3_speed * 100)) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 68.8107 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 65.534 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy))) + (time_shift * e_s3_speed * 100)) / 32767.0);
          color.g = ~v;
          
          cx = (y - semiHeightMajor); // + float(e_s3_size * (sin16(e_s3_speed * 134.3447 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor); // + float(e_s3_size * (cos16(e_s3_speed * 170.3884 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy))) + (time_shift * e_s3_speed * 100)) / 32767.0);
          color.b = ~v;
          fb->at(x, y) = color;
        }
      }
      break;
  }
  return true;
}
#endif  // OBSOLETE_CODE


/*
 ***** METABALLS / МЕТАСФЕРЫ *****
Metaballs proof of concept by Stefan Petrick 
https://gist.github.com/StefanPetrick/170fbf141390fafb9c0c76b8a0d34e54
*/
void EffectMetaBalls::setControl(size_t idx, int32_t value){
  switch (idx){
    // speed
    case 0:
      speedFactor = EffectMath::fmap(value, 1, 10, 0.1, 1);
      break;

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectMetaBalls::load(){
  palettesload();
}

bool EffectMetaBalls::run()
{
  // get some 3 random moving points
  unsigned long t = millis() * speedFactor;
  // get some 3 random moving points
  uint8_t x1 = beatsin88(23 * 256 * speedFactor, 0, fb->maxWidthIndex());//V1
  uint8_t y1 = beatsin88(28 * 256 * speedFactor, 0, fb->maxHeightIndex());

  //uint8_t x1 = inoise8(t, 12355, 85) / hormap;// V2
  //uint8_t y1 = inoise8(t, 5, 685) / vermap;

  uint8_t x2 = inoise8(t, 25355, 685) / hormap;
  uint8_t y2 = inoise8(t, 355, 11685) / vermap;

  uint8_t x3 = inoise8(t, 55355, 6685) / hormap;
  uint8_t y3 = inoise8(t, 25355, 22685) / vermap;

  for (uint8_t y = 0; y < fb->h(); y++) {
    for (uint8_t x = 0; x < fb->w(); x++) {

      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      // calculate distances of the 3 points from actual pixel
      // and add them together with weightening
      uint8_t  dx =  abs(x - x1);
      uint8_t  dy =  abs(y - y1);
      uint8_t dist = 2 * EffectMath::sqrt((dx * dx) + (dy * dy));

      dx =  abs(x - x2);
      dy =  abs(y - y2);
      dist += EffectMath::sqrt((dx * dx) + (dy * dy));

      dx =  abs(x - x3);
      dy =  abs(y - y3);
      dist += EffectMath::sqrt((dx * dx) + (dy * dy));

      // inverse result
      byte color = scale*4 / (dist==0?1:dist);

      // map color between thresholds
      if (color > 0 and color < 60) {
        fb->at(x, y) = ColorFromPalette(*curPalette, color * 9);
      } else {
        fb->at(x, y) = ColorFromPalette(*curPalette, 0);
      }
      // show the 3 points, too
      fb->at(x1, y1) = CRGB(255, 255, 255);
      fb->at(x2, y2) = CRGB(255, 255, 255);
      fb->at(x3, y3) = CRGB(255, 255, 255);
    }
  }
  return true;
}

#if !defined (OBSOLETE_CODE)
// ***** Эффект "Спираль"     ****
/*
 * Aurora: https://github.com/pixelmatix/aurora
 * https://github.com/pixelmatix/aurora/blob/sm3.0-64x64/PatternSpiro.h
 * Copyright (c) 2014 Jason Coon
 * Неполная адаптация SottNick
 */
void EffectSpiro::load(){
  palettesload();    // подгружаем дефолтные палитры
}

//!++
void EffectSpiro::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.75, 3);
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSpiro::run() {
  // страхуемся от креша
  if (curPalette == nullptr) {
    return false;
  }

  bool change = false;
  uint8_t spirooffset = 256 / spirocount;

  //fb->dim(254U - palettescale);
  //fb->dim(250-speed_factor*7);
  uint8_t dim = beatsin8(16. / speedFactor, 5, 10);
  fb->dim(254 - dim);

  for (int i = 0; i < spirocount; i++) {
    uint8_t  x = EffectMath::mapsincos8(MAP_SIN, spirotheta1 + i * spirooffset, spirominx, spiromaxx);
    uint8_t  y = EffectMath::mapsincos8(MAP_COS, spirotheta1 + i * spirooffset, spirominy, spiromaxy);
    uint8_t x2 = EffectMath::mapsincos8(MAP_SIN, spirotheta2 + i * spirooffset, x - spiroradiusx, x + spiroradiusx);
    uint8_t y2 = EffectMath::mapsincos8(MAP_COS, spirotheta2 + i * spirooffset, y - spiroradiusy, y + spiroradiusy);
    CRGB color = ColorFromPalette(*curPalette, (spirohueoffset + i * spirooffset), 128U);
    fb->at(x2, y2) += color;

    if(x2 == spirocenterX && y2 == spirocenterY) change = true; // в центре могут находится некоторое время
  }

  spirotheta2 += speedFactor * 2;
  spirotheta1 += speedFactor;
  spirohueoffset += speedFactor;

  if (change && !spirohandledChange) { // меняем кол-во спиралей
    spirohandledChange = true;

    if (spirocount >= fb->w() || spirocount == 1)
      spiroincrement = !spiroincrement;

    if (spiroincrement) {
      if(spirocount >= 4)
        spirocount *= 2;
      else
        spirocount += 1;
    } else {
      if(spirocount > 4)
        spirocount /= 2;
      else
          spirocount -= 1;
    }

    spirooffset = 256 / spirocount;
  }

  // сброс спустя время, чтобы счетчик спиралей не менялся скачками
  if(spirohandledChange){
    if(internalCnt==25) { // спустя 25 кадров
      spirohandledChange = false;
      internalCnt=0;
    } else {
      internalCnt++;
    }
  }

  EffectMath::blur2d(fb, 32);
  return true;
}
#endif  // OBSOLETE_CODE

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
// ***** Парящий огонь, Кровавые Небеса, Радужный Змей и т.п.
// базис (c) Stefan Petrick
//!++
void EffectFireVeil::setControl(size_t idx, int32_t value) {
  switch (idx){
    // 0 - move speed ~30-255

    // 1 - effect switcher
    case 1:
      _effId = value;
      load();
      break;

    // 2 - movenoise shift
    case 2:
      _shift = value;
      break;

    // 3 - _colorId 0-255
    case 3:
      _colorId = value;
      break;

    // 4 - smooth 0-255
    case 4:
      _discrete = value;
      break;

    // 5 - blur 0-255
    case 5:
      _blur = value;
      break;

    // 6 - amplitude 0-255
    case 6:
      _amplitude = value;
      break;

    // 7 - noise smooth 100-255
    case 7:
      eNs_noisesmooth = value;
      break;

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectFireVeil::drawFillRect2_fast(int32_t x1, int32_t y1, int32_t x2, int32_t y2, CRGB color)
{
  for (int32_t xP = x1; xP <= x2; xP++){
    for (int32_t yP = y1; yP <= y2; yP++){
      fb->at(xP, yP) += color;
    }
  }
}

void EffectFireVeil::moveFractionalNoise(bool direction, int8_t amplitude, int32_t shift) {
  int16_t zD, zF;
  uint16_t _side_a = direction ? fb->h() : fb->w();
  uint16_t _side_b = direction ? fb->w() : fb->h();

  for (auto &i : noise3d.map)
    for (uint16_t a = 0; a < _side_a; a++) {
      uint8_t _pixel = direction ? i.at(0,a) : i.at(a,0);
      int16_t amount = (_pixel - 128) * 2 * amplitude + shift;
      uint8_t delta = abs(amount) >> 8;
      uint8_t fraction = abs(amount) & 255;
      for (uint16_t b = 0 ; b < _side_b; b++) {
        if (amount < 0) {
          zD = b - delta; zF = zD - 1;
        } else {
          zD = b + delta; zF = zD + 1;
        }
        //Serial.printf("zD: %d, zF:%d\n", zD, zF);
        CRGB pixelA(CRGB::Black);
        if ((zD >= 0) && (zD < _side_b))
          pixelA = direction ? fb->at(zD%fb->w(), a%fb->h()) : fb->at(a%fb->w(), zD%fb->h());

        CRGB pixelB(CRGB::Black);
        if ((zF >= 0) && (zF < _side_b))
          pixelB = direction ? fb->at(zF%fb->w(), a%fb->h()) : fb->at(a%fb->w(), zF%fb->h());
        uint16_t x = direction ? b : a;
        uint16_t y = direction ? a : b;
        fb->at(x, y) = pixelA.nscale8(ease8InOutApprox(255 - fraction)) + pixelB.nscale8(ease8InOutApprox(fraction));   // lerp8by8(PixelA, PixelB, fraction );
        //Serial.printf("x:%d, y:%d, r:%u g:%u b:%u\n", x, y, result.at(x, y).r, result.at(x, y).g, result.at(x, y).b);
      }
    }
}

void EffectFireVeil::load() {
  for (auto &i : noise3d.opt){
    i.e_x = random16();
    i.e_y = random16();
    i.e_z = random16();
    i.e_scaleX = 6000;
    i.e_scaleY = 6000;
  }
}

bool EffectFireVeil::run(){
  switch (_effId)
  {
  case 1 :
    return rainbowComet3Routine();
    break;
  case 2 :
    return firelineRoutine();
    break;
  case 3 :
    return fractfireRoutine();
    break;
  case 4 :
    return stringsRoutine();
    break;
//  case 5 :
//    return flsnakeRoutine();
//    break;
  default:
    return rainbowCometRoutine();
  }
}

bool EffectFireVeil::stringsRoutine() {
  if (_colorId == 0) {
    ++count;
    // cycle hue if color is 0
    if (count%2 == 0){
      ++hue;
      --hue2;
    } 
  } else
    hue = hue2 = _colorId;

  CRGB color;
  // shift saturation/v over time
  hsv2rgb_spectrum(CHSV(hue, (_colorId == 255) ? 0 : beatsin8(speed, 220, 255, 0, 180), beatsin8(speed / 2, 64, 255)), color);

  for (size_t i = 0; i != fb->h(); ++i){
    auto k = 2 * 256 / fb->w();  // take 1/2 of width proportion
    auto t = beatsin8(_discrete * speed, 0, fb->w()/2);
    auto n = quadwave8(t + count) / k;
    auto n2 = quadwave8(t) / k;

    fb->at(n + fb->w()/2, i) = color;

    hsv2rgb_spectrum(CHSV(hue, (_colorId == 255) ? 0 : beatsin8(speed, 220, 255, 0, 180), beatsin8(speed / 2, 64, 255)), color);
    fb->at(n2 + fb->w()/4, i) = color;
  }

    // скорость движения по массиву noise
    noise3d.opt[0].e_x += 1000 * speed; //1000;
    noise3d.opt[0].e_y += 1000 * speed; //1000;
    noise3d.opt[0].e_z += 1000 * speed; //1000;
    noise3d.opt[0].e_scaleX = 500 * _discrete;//12000;
    noise3d.opt[0].e_scaleY = 500 * _discrete;

    // fill noise map
    noise3d.fillNoise(eNs_noisesmooth);

    moveFractionalNoise(MOVE_X, _amplitude, _shift);
    moveFractionalNoise(MOVE_Y, _amplitude, -1 * _shift);

    if (_blur)
      EffectMath::blur2d(fb, _blur);
  return true;
}

bool EffectFireVeil::firelineRoutine(){
  if (_colorId == 0){
    ++count;
    // cycle hue if color is 0
    if (count%2 == 0){
      ++hue;
      --hue2;
    } 
  } else
    hue = hue2 = _colorId;

  for (uint16_t i = 1; i < fb->w(); i += 2) {
    fb->at( i, fb->h()/2) += CHSV(hue + i * 2 , 255, 255);
  }
  // Noise
  int32_t beat2 = beatsin88(3 * speed, 5, 30);
  noise3d.opt[0].e_x += 12 * speed; // 3000;
  noise3d.opt[0].e_y += 12 * speed; // 3000;
  noise3d.opt[0].e_z += 12 * speed; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * _discrete; // 8000
  noise3d.opt[0].e_scaleY = 1333 * _discrete; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_Y, _amplitude, _shift);
  moveFractionalNoise(MOVE_X, _amplitude, beat2);
  if (_blur)
    EffectMath::blur2d(fb, _blur);    // < -- размытие хвоста

  return true;
}

bool EffectFireVeil::fractfireRoutine() {
  if (_colorId == 0) {
    count ++;
    if (count%2 == 0) hue ++;
  }
    else hue = _colorId;

  for (uint8_t i = 1; i < fb->w(); i += 2) {
    fb->at(i, fb->maxHeightIndex()) += CHSV(hue + i * 2, _colorId == 255 ? 64 : 255, 255);
  }
  // Noise
  int32_t beat = beatsin88(5 * speed, 50, 100);
  noise3d.opt[0].e_y += 12 * speed; // 3000;
  noise3d.opt[0].e_z += 12 * speed; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * _discrete; // 8000;
  noise3d.opt[0].e_scaleY = 1333 * _discrete; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_Y, _amplitude, beat);
  moveFractionalNoise(MOVE_X, _amplitude, _shift);
  //EffectMath::blur2d(fb, 32); // нужно ли размытие?
  return true;
}

/*
bool EffectFireVeil::flsnakeRoutine() {
  // if(!isDebug()) 
    fb->dim(_blur); 
  // else fb->clear();
  
  ++count;
  if (_colorId == 1 or _colorId == 255) {
    if (count%2 == 0) ++hue;
  }
  else hue = _colorId;

  for (uint16_t y = 2; y < fb->maxHeightIndex(); y += 5) {
    for (uint16_t x = 2; x < fb->maxWidthIndex(); x += 5) {
      fb->at(x, y) += CHSV(x * y + hue, _colorId == 255 ? 64 : 255, 255);
      fb->at(x + 1, y) += CHSV((x + 4) * y + hue, _colorId == 255 ? 64 : 255, 255);
      fb->at(x, y + 1) += CHSV(x * (y + 4) + hue, _colorId == 255 ? 64 : 255, 255);
      fb->at(x + 1, y + 1) += CHSV((x + 4) * (y + 4) + hue, _colorId == 255 ? 64 : 255, 255);
    }
  }
  // Noise
  noise3d.opt[0].e_x += 12 * speed; // 3000;
  noise3d.opt[0].e_y += 12 * speed; // 3000;
  noise3d.opt[0].e_z += 12 * speed; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * _discrete; // 8000
  noise3d.opt[0].e_scaleY = 1333 * _discrete; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_X, 5);
  moveFractionalNoise(MOVE_Y, 5);
  return true;
}
*/

bool EffectFireVeil::rainbowCometRoutine()
{ // Rainbow Comet by PalPalych
/*
  Follow the Rainbow Comet Efect by PalPalych
  Speed = tail dispersing
  Scale = 0 - Random 3d color
          1...127 - time depending color
          128...254 - selected color
          255 - white
*/


  CRGB _eNs_color;
  if (_colorId == 255) {
    _eNs_color= CRGB::White;
  } else if (_colorId == 0) {
    _eNs_color = CHSV(noise3d.lxy(0,0,0) * e_com_3DCOLORSPEED , 255, 255);
  } else if (_colorId >0 && _colorId < 128) {
    _eNs_color = CHSV(millis() / ((uint16_t)_colorId + 1U) * 4 + 10, 255, 255);
  } else {
    _eNs_color = CHSV((_colorId - 128) * 2, 255, 255);
  }

  drawFillRect2_fast(fb->w() / 2, fb->h() / 2, fb->w() / 2 + 1, fb->h() / 2 + 1, _eNs_color);

  // Noise
  noise3d.opt[0].e_x += 3000 * speed; // 3000;
  noise3d.opt[0].e_y += 3000 * speed; // 3000;
  noise3d.opt[0].e_z += 3000 * speed; // 3000;
  noise3d.opt[0].e_scaleX = 667 * _discrete; // 8000
  noise3d.opt[0].e_scaleY = 667 * _discrete; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);
  //noise3d.printmap();
  moveFractionalNoise(MOVE_X, _amplitude, _shift);
  moveFractionalNoise(MOVE_Y, _amplitude, -1 * _shift);

  if (_blur)
    EffectMath::blur2d(fb, _blur);    // < -- размытие хвоста

  return true;
}

bool EffectFireVeil::rainbowComet3Routine()
{ 
  CHSV color = rgb2hsv_approximate(CRGB::Green);
  if (_colorId == 0){
    --count;
    if (!count){
      count = speed;
      hue++;
    } 
    color.hue += hue;
  } else
    color.hue += _colorId;

  EffectMath::drawPixelXYF(fb->w() / 2, fb->h() / 2, color, fb, 0); // одна комета  по центру

  color = rgb2hsv_approximate(CRGB::Red);
  if (_colorId == 0)
    color.hue += hue;
  else
  color.hue += _colorId;

  auto xx = fb->w() / 2 + sin8( millis() / 10 / speed) / 22;
  auto yy = fb->h() / 2 + cos8( millis() / 10 / speed) / 22;
  EffectMath::drawPixelXYF(xx, yy, color, fb, 0);

  color = rgb2hsv_approximate(CRGB::Blue);
  if (_colorId == 0)
    color.hue += hue;
  else
  color.hue += _colorId;

  xx = fb->w() / 2 + sin8( millis() / 12 / speed) / 32;
  yy = fb->h() / 2 + sin8( millis() / 7 / speed) / 32;
  EffectMath::drawPixelXYF(xx, yy, color, fb, 0);

  noise3d.opt[0].e_x += 3000 * speed;
  noise3d.opt[0].e_y += 3000 * speed;
  noise3d.opt[0].e_z += 3000 * speed;
  noise3d.opt[0].e_scaleX = 667 * _discrete; // 4000;
  noise3d.opt[0].e_scaleY = 667 * _discrete; // 4000;
  noise3d.fillNoise(eNs_noisesmooth);
  moveFractionalNoise(MOVE_X, _amplitude, _shift);
  moveFractionalNoise(MOVE_Y, _amplitude, -1 * _shift);

  if (_blur)
    EffectMath::blur2d(fb, _blur);

  return true;
}

#if !defined (OBSOLETE_CODE)
// ============= ЭФФЕКТ СТАЯ ===============
// Адаптация от (c) SottNick
void EffectFlock::load(){
  palettesload();    // подгружаем дефолтные палитры
  Boid::spawn(boids, fb->w(), fb->h());
  for (auto &b : boids){
    b.maxspeed = 0.380 * speedFactor + 0.380 / 2;
    b.maxforce = 0.015 * speedFactor + 0.015 / 2;
  }
  predator = Boid( random8(0,fb->w()), random(0, fb->h()) );
  predator.maxspeed = 0.385 * speedFactor + 0.385 / 2;
  predator.maxforce = 0.020 * speedFactor + 0.020 / 2;
  predator.neighbordist = 8.0;
  predator.desiredseparation = 0.0;
}

// !++
void EffectFlock::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) {
    speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 196.0)*getBaseSpeedFactor();
    for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++)
      {
        //boids[i] = Boid(15, 15);
        boids[i].maxspeed = 0.380 * speedFactor + 0.380 / 2;
        boids[i].maxforce = 0.015 * speedFactor + 0.015 / 2;
      }
    if (predatorPresent)
      {
        predator.maxspeed = 0.385 * speedFactor + 0.385 / 2;
        predator.maxforce = 0.020 * speedFactor + 0.020 / 2;
        //predator.neighbordist = 8.0;
        //predator.desiredseparation = 0.0;
      }
  } else if(_val->getId()==4) predatorPresent = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFlock::run(){
  if (curPalette == nullptr) {
    return false;
  }
  return flockRoutine();
}

bool EffectFlock::flockRoutine() {
  hueoffset += (speedFactor/5.0+0.1);

  fb->fade(map(speed, 1, 255, 220, 10));

  bool applyWind = random(0, 255) > 240;
  if (applyWind) {
    wind.x = Boid::randomf() * .015 * speedFactor + .015 / 2;
    wind.y = Boid::randomf() * .015 * speedFactor + .015 / 2;
  }

  CRGB color = ColorFromPalette(*curPalette, hueoffset, 170);
  for (auto &boid : boids){
    if (predatorPresent) {
          // flee from predator
          boid.repelForce(predator.location, 8);
        }
    boid.run(boids);
    boid.wrapAroundBorders(fb->w(), fb->h());
    PVector location = boid.location;
    EffectMath::drawPixelXYF(location.x, location.y, color, fb);
    if (applyWind) {
          boid.applyForce(wind);
          applyWind = false;
        }
  }
  if (predatorPresent) {
    predator.run(boids);
    predator.wrapAroundBorders(fb->w(), fb->h());
    color = ColorFromPalette(*curPalette, hueoffset + 128, 255);
    PVector location = predator.location;
    EffectMath::drawPixelXYF(location.x, location.y, color, fb);
  }

  return true;
}


// ------------------------------ ЭФФЕКТ МЕРЦАНИЕ ----------------------
// (c) SottNick
#define TWINKLES_SPEEDS 4     // всего 4 варианта скоростей мерцания
#define TWINKLES_MULTIPLIER 24 // слишком медленно, если на самой медленной просто по единичке добавлять

void EffectTwinkles::load(){
  palettesload();    // подгружаем дефолтные палитры
  setup();
}

void EffectTwinkles::setup()
{
  //randomSeed(millis());
  for (auto i = ledsbuff.begin(); i != ledsbuff.end(); ++i ){
    if (random(0,255) < tnum) {                                // чем ниже tnum, тем чаще будут заполняться элементы лампы
      i->r = random8();                           // оттенок пикселя
      i->g = random8(1, TWINKLES_SPEEDS * 2 + 1); // скорость и направление (нарастает 1-4 или угасает 5-8)
      i->b = random8();                           // яркость
    }
    else
      *i = 0; // всё выкл
  }
}

// !++
void EffectTwinkles::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 1, 8);//((float)TWINKLES_MULTIPLIER * (float)EffectCalc::setDynCtrl(_val).toInt() / 380.0);
  else if(_val->getId()==2) { tnum = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 5, 132); setup(); } 
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectTwinkles::twinklesRoutine(){
  if (curPalette == nullptr) {
    return false;
  }

  for (auto i = ledsbuff.begin(); i != ledsbuff.end(); ++i ){
    if (i->b == 0){
      if (random(0,255) < tnum && thue > 0)
      {                                                         // если пиксель ещё не горит, зажигаем каждый ХЗй
        i->r = random8();                            // оттенок пикселя
        i->g = random8(1, TWINKLES_SPEEDS * 2 + 1);  // скорость и направление (нарастает 1-4, но не угасает 5-8)
        i->b = i->g;                      // яркость
        thue--;                                                 // уменьшаем количество погасших пикселей
      }
    } else if (i->g <= TWINKLES_SPEEDS) { // если нарастание яркости
      if (i->b > 255U - i->g - speedFactor)
      { // если досигнут максимум
        i->b = 255U;
        i->g += TWINKLES_SPEEDS;
      } else
        i->b += i->g + speedFactor;
    } else { // если угасание яркости
      if (i->b <= i->g - TWINKLES_SPEEDS + speedFactor)
      {                      // если досигнут минимум
        i->b = 0; // всё выкл
        thue++;              // считаем количество погасших пикселей
      }
      else
        i->b -= i->g + TWINKLES_SPEEDS - speedFactor;
    }
  }

  for (int i = 0; i != ledsbuff.size(); ++i )
    fb->at(i) = ledsbuff.at(i).b ? ColorFromPalette(*curPalette, ledsbuff.at(i).r, ledsbuff.at(i).b) : CRGB::Black;

  EffectMath::blur2d(fb, 32); // так они не только разгороються, но и раздуваються. Красивше :)
  return true;
}

// ============= WAVES /  ВОЛНЫ ===============
// Prismata Loading Animation
// v1.0 - Updating for GuverLamp v1.7 by SottNick 11.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternWave.h
// Адаптация от (c) SottNick
void EffectWaves::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectWaves::run(){
  //fpsmeter();
  return wavesRoutine();
}

// !++
void EffectWaves::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.25, 1)*getBaseSpeedFactor();
  else if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWaves::wavesRoutine() {
  if (curPalette == nullptr) {
    return false;
  }
  
  fb->dim(255 - 10 * speedFactor); // димирование зависит от скорости, чем быстрее - тем больше димировать
  EffectMath::blur2d(fb, 20); // @Palpalych советует делать размытие. вот в этом эффекте его явно не хватает... (есть сабпиксель, он сам размывает)
  
  float n = 0;
  for (float i = 0.0; i < (_scale <=4 ? fb->w() : fb->h()); i+= 0.5) {
    n = (float)quadwave8(i * 4 + waveTheta) / (256.0 / ((float)(_scale <=4 ? fb->h() : fb->w()) -1));
    switch (_scale) {
      case 1: // одна волна горизонтально, справа на лево 
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 2: // две волны горизонтально, справа на лево
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF(i, (float)fb->maxHeightIndex() - n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 3: // одна волна горизонтально, слева на право 
        EffectMath::drawPixelXYF((float)fb->maxWidthIndex() - i, n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 4: // две волны горизонтально, слева на право
        EffectMath::drawPixelXYF((float)fb->maxWidthIndex() - i, n, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb->maxWidthIndex() - i, (float)fb->maxHeightIndex() - n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 5: // одна волна вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 6: // две волны вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb->maxWidthIndex() - n, i, ColorFromPalette(*curPalette, whue + i), fb);
      break;
      case 7: // одна волна верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)fb->maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 8: // две волны верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)fb->maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb->maxWidthIndex() - n, (float)fb->maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
      break;
    }
  }
  waveTheta += 5.0 * speedFactor;
  whue += 2.5 * speedFactor;

  return true;
}
#endif  // OBSOLETE_CODE

// ============= FIRE 2012 /  ОГОНЬ 2012 ===============
// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
/*
 * Эффект "Огонь 2012"
 */
void EffectFire2012::load(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&WhiteBlackColors_p);
  palettes.push_back(&WoodFireColors_p);

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(millis());
}

void EffectFire2012::setControl(size_t idx, int32_t value){
  switch (idx){
    // 0 - speed - range 0-30

    // 1 scale - range 55-200
    case 1:
      scale = clamp(value, 55L, 200L);;
      break;

    // 2 - Palletes 10

    // 3 - Cooling - range 55-200
    case 3: {
      _cooling = clamp(value, 55L, 200L);
      break;
    }

    // 4 - Deviation - range 5-50
    case 4: {
      _deviation = clamp(value, 5L, 50L);;
      break;
    }


    default:
      EffectCalc::setControl(idx, value);
  }
}

bool EffectFire2012::run() {
  if (curPalette == nullptr || dryrun()){
    return false;
  }

  return fire2012Routine();
}

bool EffectFire2012::fire2012Routine() {
  _sparking = qadd8(8, scale);
  int fire_base = (fb->h()/6)>6 ? 6 : fb->h()/6 + 1;

  // Loop for each column individually
  for (size_t x = 0; x != noise.w(); ++x)
  {

    uint8_t col_cooling = random8(_cooling - _deviation, _cooling + _deviation);
    uint8_t col_sparkling = random8(_sparking - _deviation, _sparking + _deviation);

    // Step 1.  Cool down every cell a little
    for (size_t y = 0; y != noise.h(); ++y)
      noise.at(x,y) = qsub8(noise.at(x,y), random8(0, col_cooling * 10 / noise.h() + 2));

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (size_t k = noise.h()-1; k != 3; k--)
      noise.at(x,k) = (noise.at(x,k - 1) + noise.at(x,k - 2) + noise.at(x,k - 3)) / 3;

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < col_sparkling)
    {
      int j = random8(fire_base);
      noise.at(x,j) = qadd8(noise.at(x,j), random8() | spark_min_T); // 196, 255
    }

    // Step 4.  Map from heat cells to LED colors (invert Y)
    for (size_t y = 0; y != fb->h(); ++y)
      fb->at(x, fb->maxHeightIndex() - y) = ColorFromPalette(*curPalette, scale8(noise.at(x,y), 240));
//      nblend(fb->at(x, fb->maxHeightIndex() - y), ColorFromPalette(*curPalette, noise.at(x,y)*3/4 + noise.at( (x + 1)%fb->w(), y)/3 ), fireSmoothing);
  }

  return true;
}

#if !defined (OBSOLETE_CODE)
// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// Classic (c) SottNick
// New (c) Vortigont
#define PAUSE_MAX 7

void EffectCube2d::swapBuff() {
  for (uint8_t y = 0; y < fb->h(); y++) { // переписываем виртуальный экран в настоящий
    for(uint8_t x = 0; x < fb->w(); x++) {
      fb->at(x,y) = ledbuff.at(x,y);
    }
  }
}

bool EffectCube2d::run(){
  if (dryrun(classic ? 4. : 3., classic ? 3 : EFFECTS_RUN_TIMER))
    return false;

  // aquire mutex
  std::unique_lock<std::mutex> lock(_mtx, std::defer_lock);
  if (!lock.try_lock())
    return false;

  if (classic)
    return cube2dClassicRoutine();
  else
    return cube2dRoutine();
}

// !++
void EffectCube2d::setControl(size_t idx, int32_t value)
{
  if(_val->getId()==3) { EffectCalc::setDynCtrl(_val).toInt(); cubesize(); }
  else if(_val->getId()==4) { sizeX = EffectCalc::setDynCtrl(_val).toInt(); cubesize(); }
  else if(_val->getId()==5) { sizeY = EffectCalc::setDynCtrl(_val).toInt(); cubesize(); }
  else if(_val->getId()==6) { classic = EffectCalc::setDynCtrl(_val).toInt(); cubesize(); }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)

  return String();
}

void EffectCube2d::load(){
  palettesload();    // подгружаем дефолтные палитры
  //cubesize();
}

// задаем размерность кубов
void EffectCube2d::cubesize() {
  if (curPalette == nullptr) {
    return;
  }

  fb->clear();

  // aquire mutex
  std::unique_lock<std::mutex> lock(_mtx);

  cntY = fb->h() / (sizeY+1) + !!(fb->h() / (sizeY+1));
	fieldY = (sizeY + 1U) * cntY;

  cntX = fb->w() / (sizeX+1) + !!(fb->w() / (sizeX+1));
	fieldX = (sizeX + 1U) * cntX;

  ledbuff.resize(fieldX, fieldY);   // создаем виртуальную матрицу, размером кратную размеру кубика+1

  //LOG(printf_P, PSTR("CUBE2D Size: lfb_size:%d sizeX,Y:%d,%d cntX,Y:%d,%d fieldX,Y:%d,%d\n"), ledbuff.size(), sizeX,sizeY, cntX,cntY, fieldX,fieldY );
  uint8_t x=0, y = 0;
  CRGB color;

  for (uint8_t j = 0U; j < cntY; j++) //cntY
  {
    y = j * (sizeY + 1U);
    for (uint8_t i = 0U; i < cntX; i++) //cntX
    {
      x = i * (sizeX + 1U);
      if (scale == FASTLED_PALETTS_COUNT + 1U)
        color = CHSV(46U, 0U, 32U + random8(256U-32U));
      else {
        int cnt = 10;
        while (--cnt){
          color = scale > 0 ? ColorFromPalette(*curPalette, random(1024)>>1, random8(128, 255)) : CRGB(random8(), random8(), random8());
          if (color >= CRGB(10,10,10)) break;  // Не хотелось бы получать слишком тёмные кубики
        }
      }

      //LOG(printf_P, PSTR("CUBE2D Rect: x,y:%d,%d sX,sY:%d,%d\n"), x,y, sizeX, sizeY);
      for (uint8_t k = 0U; k < sizeY; ++k){
        for (uint8_t m = 0U; m < sizeX; ++m){
          ledbuff.at(x+m, y+k) = color;
        }
      }
    }
  }

  if (classic) {
    storage.assign(fb->w(), std::vector<uint8_t>(fb->h()) );    // todo: get rid of this
    currentStep = 4U; // текущий шаг сдвига первоначально с перебором (от 0 до shiftSteps-1)
    shiftSteps = 4U; // всего шагов сдвига (от 3 до 4)
    pauseSteps = 0U; // осталось шагов паузы
    seamlessX = (fieldX == fb->w());
    globalShiftX = 0;
    globalShiftY = 0;
    gX = 0;
    gY = 0;
  } else {
    pauseSteps = CUBE2D_PAUSE_FRAMES; // осталось шагов паузы
    shiftSteps = 0;
  }
}

bool EffectCube2d::cube2dRoutine()
{
  if (curPalette == nullptr) {
    return false;
  }

  if (!pauseSteps){
    pauseSteps--;
    return false; // пропускаем кадры после прокрутки кубика (делаем паузу)
  }

  if (!shiftSteps) {  // если цикл вращения завершён
    // ====== определяем направление прокруток на будущий цикл
    pauseSteps = CUBE2D_PAUSE_FRAMES;
    direction = random8()%2;  // сдвиг 0 - строки, 1 - столбцы
    moveItems.assign(direction ? cntX : cntY, 0);

    for ( auto &item : moveItems ){
      item = random8()%3; // 1 - fwd, 0 - bkw, 2 - none
    }

    shiftSteps = ((direction ? sizeY : sizeX)+1) * random8(direction ? cntY : cntX);  // такой рандом добавляет случайную задержку в паузу, попадая на "0"
    return false;
  }


  //двигаем, что получилось...
  shiftSteps--;

  for (uint8_t i=0; i<(direction ? cntX : cntY); i++){
    if (moveItems.at(i)==2) // skip some items
      continue;

    direction ? cube2dmoveCols(i, moveItems.at(i)) : cube2dmoveRows(i, moveItems.at(i));
  }
  swapBuff();
  return true;
}

// идём по горизонтали, крутим по вертикали (столбцы двигаются)
void EffectCube2d::cube2dmoveCols(uint8_t moveItem, bool movedirection){
  uint16_t x, anim0;
  CRGB color, color2;

  x = moveItem * (sizeX + 1U);
  anim0 = 0;

      // если крутим столбец вниз
      if (!movedirection){
          // берём цвет от нижней строчки
          color = ledbuff.at(x, anim0);
          for (uint8_t k = anim0; k < anim0+fieldY-1; k++)
          {
            // берём цвет от строчки над нашей
            color2 = ledbuff.at(x, k+1);
            for (uint8_t m = x; m < x + sizeX; m++)
              // копируем его на всю нашу строку
              ledbuff.at(m, k) = color2;
          }
          for   (uint8_t m = x; m < x + sizeX; m++)
            // цвет нижней строчки копируем на всю верхнюю
            ledbuff.at(m, anim0+fieldY-1) = color;
          return;
        }

      // крутим столбец вверх
      // берём цвет от верхней строчки
      color = ledbuff.at(x,anim0+fieldY-1);
      for (uint8_t k = anim0+fieldY-1; k > anim0 ; k--)
      {
        color2 = ledbuff.at(x, k-1);
        for (uint8_t m = x; m < x + sizeX; m++)
          // копируем его на всю нашу строку
          ledbuff.at(m, k ) = color2;
      }
      for   (uint8_t m = x; m < x + sizeX; m++)
        // цвет верхней строчки копируем на всю нижнюю
        ledbuff.at(m, anim0) = color;
}

// идём по вертикали, крутим по горизонтали (строки двигаются)
void EffectCube2d::cube2dmoveRows(uint8_t moveItem, bool movedirection){
  uint16_t y, anim0;
  CRGB color, color2;

  y = moveItem * (sizeY + 1U);
  anim0 = 0;

  // крутим строку влево
  if (!movedirection){
    color = ledbuff.at(anim0, y);                            // берём цвет от левой колонки (левого пикселя)
    for (uint8_t k = anim0; k < anim0+fieldX-1; k++)
    {
      color2 = ledbuff.at(k+1, y);                           // берём цвет от колонки (пикселя) правее
      for (uint8_t m = y; m < y + sizeY; m++)
        // копируем его на всю нашу колонку
        ledbuff.at(k,m) = color2;
    }
    for   (uint8_t m = y; m < y + sizeY; m++)
      // цвет левой колонки копируем на всю правую
      ledbuff.at(anim0+fieldX-1, m) = color;
   return;
  }

  //  крутим строку вправо
  // берём цвет от правой колонки
  color = ledbuff.at(anim0+fieldX-1, y);
  for (uint8_t k = anim0+fieldX-1; k > anim0 ; k--)
  {
    // берём цвет от колонки левее
    color2 = ledbuff.at(k-1, y);
    for (uint8_t m = y; m < y + sizeY; m++)
      // копируем его на всю нашу колонку
      ledbuff.at(k, m) = color2;
  }
  for   (uint8_t m = y; m < y + sizeY; m++)
    // цвет правой колонки копируем на всю левую
    ledbuff.at(anim0, m) = color;
}

bool EffectCube2d::cube2dClassicRoutine()
{
  CRGB color, color2;
  int8_t shift, shiftAll;
  uint8_t anim0, x, y;

  //двигаем, что получилось...
  if (pauseSteps == 0 && currentStep < shiftSteps) // если пауза закончилась, а цикл вращения ещё не завершён
  {
    currentStep++;
    if (direction)
    {
      for (uint8_t i = 0U; i < cntX; i++)
      {
        x = (gX + i * (sizeX + 1U)) % fb->w();
        if (storage[i][0] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          storage[i][0]--;
          shift = storage[i][1] - 1; // в первой ячейке храним направление прокрутки

          if (globalShiftY == 0)
            anim0 = (gY == 0U) ? 0U : gY - 1U;
          else if (globalShiftY > 0)
            anim0 = gY;
          else
            anim0 = gY - 1U;

          if (shift < 0) // если крутим столбец вниз
          {
            color = ledbuff.at(x, anim0);
            for (uint8_t k = anim0; k < anim0 + fieldY - 1; k++)
            {
              color2 = ledbuff.at(x, k + 1); // берём цвет от строчки над нашей
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff.at(m % fieldX, k) = color2;    // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              ledbuff.at(m % fieldX, anim0 + fieldY - 1) = color;  // цвет нижней строчки копируем на всю верхнюю
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color =  ledbuff.at(x, anim0 + fieldY - 1); // берём цвет от верхней строчки
            for (uint8_t k = anim0 + fieldY - 1; k > anim0; k--)
            {
              color2 = ledbuff.at(x, k - 1); // берём цвет от строчки под нашей
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff.at(m % fb->w(), k) = color2; // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              ledbuff.at(m % fb->w(), anim0) = color; // цвет верхней строчки копируем на всю нижнюю
          }
        }
      }
    }
    else
    {
      for (uint8_t j = 0U; j < cntY; j++)
      {
        y = (gY + j * (sizeY + 1U))  % fb->h();
        if (storage[0][j] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          storage[0][j]--;
          shift = storage[1][j] - 1; // в первой ячейке храним направление прокрутки

          if (seamlessX)
            anim0 = 0U;
          else if (globalShiftX == 0)
            anim0 = (gX == 0U) ? 0U : gX - 1U;
          else if (globalShiftX > 0)
            anim0 = gX;
          else
            anim0 = gX - 1U;

          if (shift < 0) // если крутим строку влево
          {
            color = ledbuff.at(anim0, y); // берём цвет от левой колонки (левого пикселя)
            for (uint8_t k = anim0; k < anim0 + fieldX - 1; k++)
            {
              color2 = ledbuff.at(k + 1, y); // берём цвет от колонки (пикселя) правее
              for (uint8_t m = y; m < y + sizeY; m++)
                ledbuff.at(k, m) = color2; // копируем его на всю нашу колонку
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              ledbuff.at(anim0 + fieldX - 1, m) = color;   // цвет левой колонки копируем на всю правую
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = ledbuff.at(anim0 + fieldX - 1, y); // берём цвет от правой колонки
            for (uint8_t k = anim0 + fieldX - 1; k > anim0; k--)
            {
              color2 = ledbuff.at(k - 1, y); // берём цвет от колонки левее
              for (uint8_t m = y; m < y + sizeY; m++)
                ledbuff.at(k, m) = color2; // копируем его на всю нашу колонку
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              ledbuff.at(anim0, m) = color; // цвет правой колонки копируем на всю левую
          }
        }
      }
    }
  }
  else if (pauseSteps != 0U) // пропускаем кадры после прокрутки кубика (делаем паузу)
    pauseSteps--;

  if (currentStep >= shiftSteps) // если цикл вращения завершён, меняем местами соотвествующие ячейки (цвет в них) и точку первой ячейки
  {
    currentStep = 0U;
    pauseSteps = PAUSE_MAX;
    //если часть ячеек двигалась на 1 пиксель, пододвигаем глобальные координаты начала
    gY = gY + globalShiftY; //+= globalShiftY;
    globalShiftY = 0;
    //gX += globalShiftX; для бесшовной не годится
    gX = (fb->w() + gX + globalShiftX) % fb->w();
    globalShiftX = 0;

    //пришла пора выбрать следующие параметры вращения
    shiftAll = 0;
    direction = random8(2U);
    if (direction) // идём по горизонтали, крутим по вертикали (столбцы двигаются)
    {
      for (uint8_t i = 0U; i < cntX; i++)
      {
        storage[i][1] = random8(3);
        shift = storage[i][1] - 1; // в первой ячейке храним направление прокрутки
        if (shiftAll == 0)
          shiftAll = shift;
        else if (shift != 0 && shiftAll != shift)
          shiftAll = 50;
      }
      shiftSteps = sizeY + ((gY - shiftAll >= 0 && gY - shiftAll + fieldY < (int)fb->h()) ? random8(2U) : 1U);

      if (shiftSteps == sizeY) // значит полюбому shiftAll было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
      {
        globalShiftY = 1 - shiftAll; //временно на единичку больше, чем надо
        for (uint8_t i = 0U; i < cntX; i++)
          if (storage[i][1] == 1U) // если ячейка никуда не планировала двигаться
          {
            storage[i][1] = globalShiftY;
            storage[i][0] = 1U; // в нулевой ячейке храним количество ходов сдвига
          }
          else
            storage[i][0] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
        globalShiftY--;
      }
      else
        for (uint8_t i = 0U; i < cntX; i++)
          if (storage[i][1] != 1U)
            storage[i][0] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
    }
    else // идём по вертикали, крутим по горизонтали (строки двигаются)
    {
      for (uint8_t j = 0U; j < cntY; j++)
      {
        storage[1][j] = random8(3);
        shift = storage[1][j] - 1; // в первой ячейке храним направление прокрутки
        if (shiftAll == 0)
          shiftAll = shift;
        else if (shift != 0 && shiftAll != shift)
          shiftAll = 50;
      }
      if (seamlessX)
        shiftSteps = sizeX + ((shiftAll < 50) ? random8(2U) : 1U);
      else
        shiftSteps = sizeX + ((gX - shiftAll >= 0 && gX - shiftAll + fieldX < (int)fb->w()) ? random8(2U) : 1U);

      if (shiftSteps == sizeX) // значит полюбому shiftAll было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
      {
        globalShiftX = 1 - shiftAll; //временно на единичку больше, чем надо
        for (uint8_t j = 0U; j < cntY; j++)
          if (storage[1][j] == 1U) // если ячейка никуда не планировала двигаться
          {
            storage[1][j] = globalShiftX;
            storage[0][j] = 1U; // в нулевой ячейке храним количество ходов сдвига
          }
          else
            storage[0][j] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
        globalShiftX--;
      }
      else
        for (uint8_t j = 0U; j < cntY; j++)
          if (storage[1][j] != 1U)
            storage[0][j] = shiftSteps; // в нулевой ячейке храним количество ходов сдвига
    }
  }
  swapBuff();
  return true;
}
#endif  // OBSOLETE_CODE

// ----------- Эффекты "Пикассо" (c) obliterator
void EffectPicassoMetaBalls::_make_palettes(){
  palettes.clear();

  //
  //palettes.add(MBVioletColors_gp, 0, 16); //затычка для плитры под генератор

  palettes.add(MBVioletColors_gp, 0, 16);

  palettes.add(ib_jul01_gp, 60, 16, 200);

  palettes.add(es_pinksplash_08_gp, 125, 16);

  // 4 palettes.add(departure_gp, 0);
  palettes.add(departure_gp, 140, 16, 220);

  palettes.add(es_landscape_64_gp, 25, 16, 250);
  // 7 palettes.add(es_landscape_64_gp, 125);
  palettes.add(es_landscape_64_gp, 175, 50, 220);
  // 9 palettes.add(es_landscape_33_gp, 0);
  // 10 palettes.add(es_landscape_33_gp, 50);
  palettes.add(es_landscape_33_gp, 50, 50);
  // 12 palettes.add(es_ocean_breeze_036_gp, 0);

  // 13 palettes.add(GMT_drywet_gp, 0);
  // 14 palettes.add(GMT_drywet_gp, 75);
  palettes.add(GMT_drywet_gp, 150, 0, 200);

  //16 palettes.add(fire_gp, 175);

  // 17 palettes.add(Pink_Purple_gp, 25);
  palettes.add(Pink_Purple_gp, 175, 0, 220);

  // 19 palettes.add(Sunset_Real_gp, 25, 0, 200);
  // 20 palettes.add(Sunset_Real_gp, 50, 0, 220);

  palettes.add(BlacK_Magenta_Red_gp, 25);

  generate(true);
}

void EffectPicassoBase::generate(bool reset){
  if (scale != particles.size()){
    particles.assign(scale, Particle());
    reset = true;
    Serial.printf("New num of parts:%u\n", scale);
  }

  float minSpeed = 0.2, maxSpeed = 0.8;
  for (auto &particle : particles){
    if (reset) {
      particle.position_x = random(0, fb->w());
      particle.position_y = random(0, fb->h());

      particle.speed_x = (-maxSpeed / 3) + (maxSpeed * (float)random(1, 100) / 100);
      particle.speed_x += particle.speed_x > 0 ? minSpeed : -minSpeed;

      particle.speed_y = (-maxSpeed / 2) + (maxSpeed * (float)random(1, 100) / 100);
      particle.speed_y += particle.speed_y > 0 ? minSpeed : -minSpeed;

      particle.color = CHSV(random8(1U, 255U), 255U, 255U);
      //particle.hue_next = particle.color.h;

      particle.hue_next = random8(1U, 255U);
      particle.hue_step = (particle.hue_next - particle.color.h) / 25;
    };

    if (particle.hue_next != particle.color.h && particle.hue_step)
      particle.color.h += particle.hue_step;
  }
}

void EffectPicassoBase::position(){
  for (auto &particle : particles){
    if (particle.position_x + particle.speed_x > fb->w() || particle.position_x + particle.speed_x < 0) {
      particle.speed_x *= -1;
    }

    if (particle.position_y + particle.speed_y > fb->h() || particle.position_y + particle.speed_y < 0) {
      particle.speed_y *= -1;
    }

    particle.position_x += particle.speed_x*speedFactor;
    particle.position_y += particle.speed_y*speedFactor;
  };
}

bool EffectPicassoShapes::run(){
  generate();
  position();

  if (_dimming) fb->fade(_dimming);

  unsigned iter = (particles.size() - particles.size()%2) / 2;
  for (unsigned i = 0; i != iter; ++i) {
    Particle &p1 = particles.at(i);
    Particle &p2 = particles.at(particles.size()-1-i);
    switch (_figure){
    case 1:
      EffectMath::drawSquareF(fabs(p1.position_x - p2.position_x), fabs(p1.position_y - p2.position_y), fabs(p1.position_x - p1.position_y), p1.color, fb);
      break;
    case 2:
      EffectMath::drawCircleF(fabs(p1.position_x - p2.position_x), fabs(p1.position_y - p2.position_y), fabs(p1.position_x - p1.position_y), p1.color, fb);
      break;
  	default:
      EffectMath::drawLineF(p1.position_x, p1.position_y, p2.position_x, p2.position_y, p1.color, fb);
      //EffectMath::drawLine(static_cast<int>(p1.position_x), static_cast<int>(p1.position_y), static_cast<int>(p2.position_x), static_cast<int>(p2.position_y), p1.color, fb);
    }
  }

  EVERY_N_MILLIS(20000){
    generate(true);
  }

  EffectMath::blur2d(fb, _blur);
  return true;
}

void EffectPicassoShapes::setControl(size_t idx, int32_t value) {
  switch (idx){
    // 0 - speed - range 0-30
    case 0:
      speedFactor = value/10.0;
      break;

    // 1 scale - as-is value clamped to PICASSO_MIN_PARTICLES PICASSO_MAX_PARTICLES
    case 1:
      scale = clamp(value, PICASSO_MIN_PARTICLES, PICASSO_MAX_PARTICLES);
      break;

    // 2 - Dimming - range 0-255
    case 2: {
      _dimming = value;
      break;
    }

    // 3 - Blur - range 0-255
    case 3: {
      _blur = value;
      break;
    }

    // 4 - figures - range 0-3
    case 4: {
      _figure = value;
      break;
    }


    default:
      EffectCalc::setControl(idx, value);
  }
}

bool EffectPicassoMetaBalls::run(){
  generate();
  position();
  fb->clear();

 // сила возмущения
  unsigned mx = EffectMath::fmap(scale, 0U, 255U, 200U, 80U);
  // радиус возмущения
  unsigned sc = EffectMath::fmap(scale, 0U, 255U, 12, 7);
  // отсечка расчетов (оптимизация скорости)
  unsigned tr = sc * 2 / 3;

  for (unsigned x = 0; x < fb->w(); x++) {
    for (unsigned y = 0; y < fb->h(); y++) {
      float sum = 0;
      for (auto &p1 : particles){
        if ((unsigned)abs(x - p1.position_x) > tr || (unsigned)abs(y - p1.position_y) > tr) continue;
        float d = EffectMath::distance(x, y, p1.position_x, p1.position_y);
        if (d < 2) {
          // дополнительно подсвечиваем сердцевину
          sum += EffectMath::mapcurve(d, 0, 2, 255, mx, EffectMath::InQuad);
        } else if (d < sc) {
          // ореол резко демпфируем, во избежание размазывания и пересвета
          sum += EffectMath::mapcurve(d, 2, sc, mx, 0, EffectMath::OutQuart);
        }

        if (sum > 255) { sum = 255; break; }
      }
      if ( sum > 5) // do not fill the background with palette color
        fb->at(x, y) = palettes[_palette_idx].GetColor((uint8_t)sum, 255);
    }
  }

  return true;
}

void EffectPicassoMetaBalls::setControl(size_t idx, int32_t value) {
  switch (idx){
    // 0 - speed - range 1-10
    case 0:
      speedFactor = value/10.0;
      break;

    // 1 scale - as-is value clamped to PICASSO_MIN_PARTICLES PICASSO_MAX_PARTICLES
    case 1:
      scale = clamp(value, PICASSO_MIN_PARTICLES, PICASSO_MAX_PARTICLES);
      break;

    // 2 palletes - range 0-_num_of_palettes
    case 2: {
      _palette_idx = clamp(value, 0L, _num_of_palettes-1);
      break;
    }
/*
    // 3 - custom palette hue - range 0-255
    case 3: {
      _dyn_palette_generator(value);
      break;
    }
*/
    default:
      EffectCalc::setControl(idx, value);
  }
}
/*
void EffectPicassoMetaBalls::_dyn_palette_generator(uint8_t hue){
    TDynamicRGBGradientPalette_byte dynpal[20] = {
        0,  0,  0,  0,
        1,  0,  0,  0,
       80,  0,  0,  0,
      150,  0,  0,  0,
      255,  0,  0,  0
    };

    CRGB *color = (CRGB *)dynpal + 1; *color = CHSV(hue + 255, 255U, 255U);
    color = (CRGB *)(dynpal + 5);   *color = CHSV(hue + 135, 255U, 200U);
    color = (CRGB *)(dynpal + 9);   *color = CHSV(hue + 160, 255U, 120U);
    color = (CRGB *)(dynpal + 13);  *color = CHSV(hue + 150, 255U, 255U);
    color = (CRGB *)(dynpal + 17);  *color = CHSV(hue + 255, 255U, 255U);
    CRGBPalette32 pal;
    pal.loadDynamicGradientPalette(dynpal);
    palettes.add(0, pal, 0, 16);
}
*/


// ----------- Эффекты "Лавовая лампа" (c) obliterator
EffectLiquidLamp::EffectLiquidLamp(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer) {
  scale = LIQLAMP_MIN_PARTICLES;
#define LIQLAMP_PALLETE_MAX_IDX 16
  // эта палитра создана под эффект
  palettes.add(MBVioletColors_gp, 0, 16);
  // палитры частично подогнаные под эффект
  palettes.add(ib_jul01_gp, 60, 16, 200);
  palettes.add(Sunset_Real_gp, 25, 0, 200);
  palettes.add(es_pinksplash_08_gp, 125, 16);
  palettes.add(es_landscape_33_gp, 50, 50);
  palettes.add(es_landscape_64_gp, 175, 50, 220);
  palettes.add(es_landscape_64_gp, 25, 16, 250);
  palettes.add(es_landscape_33_gp, 0);
  palettes.add(es_ocean_breeze_036_gp, 0);
  palettes.add(GMT_drywet_gp, 0);
  palettes.add(GMT_drywet_gp, 75);
  palettes.add(GMT_drywet_gp, 150, 0, 200);
  palettes.add(fire_gp, 175);
  palettes.add(Pink_Purple_gp, 25);
  palettes.add(Pink_Purple_gp, 175, 0, 220);
  palettes.add(Sunset_Real_gp, 50, 0, 220);
  palettes.add(BlacK_Magenta_Red_gp, 25);
}

void EffectLiquidLamp::generate(bool reset){
  if (scale != particles.size())
    particles.assign(scale, Particle());
  else if (!reset) return;

  for (auto &curr : particles){
    curr.position_x = random(0, fb->w());
    curr.position_y = 0;
    curr.mass = random(LIQLAMP_MASS_MIN, LIQLAMP_MASS_MAX);
    curr.spf = EffectMath::fmap(curr.mass, LIQLAMP_MASS_MIN, LIQLAMP_MASS_MAX, 0.0015, 0.0005);
    curr.rad = EffectMath::fmap(curr.mass, LIQLAMP_MASS_MIN, LIQLAMP_MASS_MAX, 2, 3);
    curr.mx = map(curr.mass, LIQLAMP_MASS_MIN, LIQLAMP_MASS_MAX, 60, 80); // сила возмущения
    curr.sc = map(curr.mass, LIQLAMP_MASS_MIN, LIQLAMP_MASS_MAX, 6, 10); // радиус возмущения
    curr.tr = curr.sc  * 2 / 3; // отсечка расчетов (оптимизация скорости)
  };
}

void EffectLiquidLamp::position(){
  for (auto &curr : particles){
    curr.hot += EffectMath::mapcurve(curr.position_y, 0, fb->h(), 5, -5, EffectMath::InOutQuad) * speedFactor;

    float heat = (curr.hot / curr.mass) - 1;
    if (heat > 0 && curr.position_y < fb->h()) {
      curr.speed_y += heat * curr.spf;
    }
    if (curr.position_y > 0) {
      curr.speed_y -= 0.07;
    }

    if (curr.speed_y) curr.speed_y *= 0.85;
    curr.position_y += curr.speed_y * speedFactor;

    if (_physics) {
      curr.speed_x *= 0.7;
      curr.position_x += curr.speed_x * speedFactor;
    }

    if (curr.position_x > fb->w()) curr.position_x -= fb->w();
    if (curr.position_x < 0) curr.position_x += fb->w();
    if (curr.position_y > fb->h()) curr.position_y = fb->h();
    if (curr.position_y < 0) curr.position_y = 0;
  };
}

void EffectLiquidLamp::physic(){
  for (auto p1 = particles.begin(); p1 != particles.end(); ++p1){
    // отключаем физику на границах, чтобы не слипались шары
    if (p1->position_y < 3 || p1->position_y > fb->maxHeightIndex()) continue;

    for (auto p2 = p1 + 1; p2 != particles.end(); ++p2) {
      if (p2->position_y < 3 || p2->position_y > fb->maxHeightIndex()) continue;
      float radius = 3;//(p1->rad + p2->rad);
      if (p1->position_x + radius > p2->position_x
					&& p1->position_x < radius + p2->position_x
					&& p1->position_y + radius > p2->position_y
					&& p1->position_y < radius + p2->position_y
      ){
          float dist = EffectMath::distance(p1->position_x, p1->position_y, p2->position_x, p2->position_y);
          if (dist <= radius) {
            float nx = (p2->position_x - p1->position_x) / dist;
            float ny = (p2->position_y - p1->position_y) / dist;
            float p = 2 * (p1->speed_x * nx + p1->speed_y * ny - p2->speed_x * nx - p2->speed_y * ny) / (p1->mass + p2->mass);
            float pnx = p * nx, pny = p * ny;
            p1->speed_x = p1->speed_x - pnx * p1->mass;
            p1->speed_y = p1->speed_y - pny * p1->mass;
            p2->speed_x = p2->speed_x + pnx * p2->mass;
            p2->speed_y = p2->speed_y + pny * p2->mass;
          }
			}
    }
  }
}

void EffectLiquidLamp::setControl(size_t idx, int32_t value) {
  switch (idx){
    // 0 - speed - range 0-255
    case 0:
      speedFactor = value / 128.0;  //EffectMath::fmap(value, 1, 10, 0.01, 0.4);
      break;
    // 1 scale - num of particles, as-is value clamped to 1-width/4
    case 1:
      scale = clamp(value, static_cast<int32_t>(LIQLAMP_MIN_PARTICLES), static_cast<int32_t>(LIQLAMP_MAX_PARTICLES));
      break;

    // custom palletes
    case 2:
      _pallete_id = clamp(value, static_cast<int32_t>(0), static_cast<int32_t>(LIQLAMP_PALLETE_MAX_IDX));
      break;

    // 3 - hue for custom pallete - range 1-255
    case 3: {
      _dynamic_pallete(value);
      _pallete_id = 0;
      break;
    }

    // 4 - shar's filter - raw, range 0-4
    case 4: {
      filter = clamp(value, static_cast<int32_t>(0), static_cast<int32_t>(4));
      break;
    }

    // 5 - physics
    case 5: {
      _physics = value;
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectLiquidLamp::_dynamic_pallete(uint8_t hue){
    TDynamicRGBGradientPalette_byte dynpal[20] = {
        0,  0,  0,  0,
        1,  0,  0,  0,
       80,  0,  0,  0,
      150,  0,  0,  0,
      255,  0,  0,  0
    };
    CRGB *color = (CRGB *)dynpal + 1;
    *color = CHSV(hue + 255, 255U, 255U); color = (CRGB *)(dynpal + 5);
    *color = CHSV(hue + 135, 255U, 200U); color = (CRGB *)(dynpal + 9);
    *color = CHSV(hue + 160, 255U, 120U); color = (CRGB *)(dynpal + 13);
    *color = CHSV(hue + 150, 255U, 255U); color = (CRGB *)(dynpal + 17);
    *color = CHSV(hue + 255, 255U, 255U);
    CRGBPalette32 pal;    pal.loadDynamicGradientPalette(dynpal);
    palettes.add(0, pal, 0, 16);
}

bool EffectLiquidLamp::routine(){
  generate();
  position();
  fb->clear();
  if (_physics) physic();

  uint8_t f = filter; // local scope copy to provide thread-safety

  if (f < 2 && (buff)) {
    buff.reset();
  } else {
    if (!buff) buff = std::make_unique< Vector2D<uint8_t> >(fb->w(), fb->h());
  }

  for (unsigned x = 0; x != fb->maxWidthIndex(); x++) {
    for (unsigned y = 0; y != fb->maxHeightIndex(); y++) {
      float sum = 0;
      for (auto &p1 : particles){
        if ((unsigned)abs(x - p1.position_x) > p1.tr || (unsigned)abs(y - p1.position_y) > p1.tr) continue;
        float d = EffectMath::distance(x, y, p1.position_x, p1.position_y);
        if (d < p1.rad) {
          sum += EffectMath::mapcurve(d, 0, p1.rad, 255, p1.mx, EffectMath::InQuad);
        } else if (d < p1.sc){
          sum += EffectMath::mapcurve(d, p1.rad, p1.sc, p1.mx, 0, EffectMath::OutQuart);
        }
        if (sum > 255) { sum = 255; break; }
      }

      if (f < 2) {
        if ( sum > 5) // do not fill the background with palette color
          fb->at(x, fb->h() - y) = palettes[_pallete_id].GetColor(sum, f ? sum : 255);
      } else {
        buff->at(x,y) = sum;
      }
    }
  }

  if (f < 2) return true;

  // apply Scharr's filter
    static constexpr std::array<int, 9> dh_scharr = {  3,  10,  3,
                                                       0,   0,  0,
                                                      -3, -10, -3  };
    static constexpr std::array<int, 9> dv_scharr = {  3,   0,  -3,
                                                      10,   0, -10,
                                                       3,   0,  -3 };
    float min = 0, max = 0;
    for (int16_t x = 1; x < fb->maxWidthIndex() -1; x++) {
      for (int16_t y = 1; y < fb->maxHeightIndex() -1; y++) {
        if (buff->at(x,y) < 5) continue;   // skip out of range pixels, it gives huge performance boost

        int gh = 0, gv = 0, idx = 0;

        for (int v = -1; v != 2; ++v) {
          for (int h = -1; h != 2; ++h) {
            gh += dh_scharr[idx] * buff->at(x+h,y+v);
            gv += dv_scharr[idx] * buff->at(x+h,y+v);
            ++idx;
          }
        }

        float v = EffectMath::sqrt((gh * gh) + (gv * gv));
        if (v < min) min = v;
        if (v > max) max = v;
        v = 1 - (v - min) / (max - min);
        size_t cnt = f;
        while (--cnt) v *= v; // pow(v,f) на esp32 оч медленный
        fb->at(x, fb->h() - y) = palettes[_pallete_id].GetColor(buff->at(x,y), v * 255);
      }
    }

  return true;
}

// ------- Эффект "Вихри"
// Based on Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternFlowField.h
// Copyright(c) 2014 Jason Coon
//адаптация SottNick
void EffectWhirl::load(){
  palettesload();    // подгружаем дефолтные палитры
  _boids_init();
}

void EffectWhirl::_boids_init(){
  _x = random16();
  _y = random16();
  _z = random16();

  _boids.resize(_boids_num);
  for (auto &boid : _boids)
    boid = Boid(EffectMath::randomf(0, fb->w()), 0);
}

void EffectWhirl::setControl(size_t idx, int32_t value){
  switch (idx){
    // 0 - speed - range ~5-150
    case 0:
      speedFactor = value / 10;
      break;
    // 1 scale - some scale parameter (default)

    // number of boids
    case 2:
      _boids_num = clamp(value, static_cast<int32_t>(1), static_cast<int32_t>(250));
      break;

    // 3 - fade - range 1-255
    case 3: {
      _fade = value;
      break;
    }

    // 4 - blur - range 1-255
    case 4: {
      _blur = value;
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }
}

bool EffectWhirl::_whirlRoutine() {
  if (_boids_num != _boids.size())
    _boids_init();

  fb->fade(_fade);

  for (auto &boid : _boids){
    float ioffset = scale * boid.location.x;
    float joffset = scale * boid.location.y;

    auto angle = inoise8(_x + ioffset, _y + joffset, _z);

    boid.velocity.x = (sin8(angle) * 0.0078125 - speedFactor);
    boid.velocity.y = -(cos8(angle) * 0.0078125 - speedFactor);
    boid.update();

    EffectMath::drawPixelXYF(boid.location.x, boid.location.y, ColorFromPalette(*curPalette, angle + hue), fb); // + hue постепенно сдвигает палитру по кругу

    if (boid.location.x < 0 || boid.location.x >= fb->w() || boid.location.y < 0 || boid.location.y >= fb->h()) {
      boid.location.x = EffectMath::randomf(0, fb->w());
      boid.location.y = 0;
    }
  }
  EffectMath::blur2d(fb, _blur);

  hue += speedFactor;
  _x += speedFactor;
  _y += speedFactor;
  _z += speedFactor;
  return true;
}

#if !defined (OBSOLETE_CODE)
// ------- Эффект "Звезды"
// !++
void EffectStar::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) {
    speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt()/380.0+0.05) * getBaseSpeedFactor();
    _speed = getCtrlVal(1).toInt();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectStar::load(){
  palettesload();    // подгружаем дефолтные палитры

  // стартуем с центра
  driftx = fb->w()/2.0;
  drifty = fb->h()/2.0;

  cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation gives a float value between -1 and 1
  sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation in the y direction gives a float value between -1 and 1

  int num = 0;
  for (auto &s : stars){
    s.points = random8(3, 9); // количество углов в звезде
    s.cntdelay = _speed / 5 + (num++ << 2) + 1U; // задержка следующего пуска звезды
    s.color = random8();
  }
}

void EffectStar::drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler)// random multipoint star
{
  radius2 = 255.0 / points;
  for (int i = 0; i < points; i++)
  {
/*
    LOG(printf_P, "Line1: %f\t%f\t%f\t%f\n", xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128));
    LOG(printf_P, "Line2: %f\t%f\t%f\t%f\n\n", xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128));
*/
// TODO: have no idea why all calculations were done using floats, but drawing is done with ints, looks like Kostyamat's implementation
    EffectMath::drawLine( static_cast<int16_t>(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128)),
                          ColorFromPalette(*curPalette, koler),
                          fb);
    EffectMath::drawLine( static_cast<int16_t>(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128)),
                          static_cast<int16_t>(ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128)),
                          ColorFromPalette(*curPalette, koler),
                          fb);
  }
}

bool EffectStar::run() {
  fb->fade(255U -  90); // работает быстрее чем dimAll

  speedFactor = ((float)speed/380.0+0.05);

  counter += speedFactor; // определяет то, с какой скоростью будет приближаться звезда

  if (driftx > (fb->w() - spirocenterX / 2U))//change directin of drift if you get near the right 1/4 of the screen
    cangle = 0 - fabs(cangle);
  if (driftx < spirocenterX / 2U)//change directin of drift if you get near the right 1/4 of the screen
    cangle = fabs(cangle);
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    driftx = driftx + (cangle * speedFactor);//move the x center every so often

  if (drifty > ( fb->h() - spirocenterY / 2U))// if y gets too big, reverse
    sangle = 0 - fabs(sangle);
  if (drifty < spirocenterY / 2U) // if y gets too small reverse
    sangle = fabs(sangle);
  //if ((counter + CENTER_DRIFT_SPEED / 2U) % CENTER_DRIFT_SPEED == 0)
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    drifty =  drifty + (sangle * speedFactor);//move the y center every so often

  for (auto &s : stars) {
    if (counter >= s.cntdelay)//(counter >= ringdelay)
    {
      if (counter - s.cntdelay <= fb->w() + 5) {
        EffectStar::drawStar(static_cast<int16_t>(driftx), static_cast<int16_t>(drifty), 2 * (counter - s.cntdelay), (counter - s.cntdelay), s.points, STAR_BLENDER + s.color, s.color);
        s.color += speedFactor; // в зависимости от знака - направление вращения
      } else
        s.cntdelay = counter + (stars.size() << 1) + 1U; // задержка следующего пуска звезды
    }
  }

  EffectMath::blur2d(fb, 30U); //fadeToBlackBy() сам блурит, уменьшил блур под микрофон

  return true;
}




// ------ Эффект "Притяжение"
// Базовый Attract проект Аврора
void EffectAttract::load() {
  palettesload();
  //speedFactor = EffectMath::fmap((float)speed, 1., 255., 0.02*getBaseSpeedFactor(), 1.*getBaseSpeedFactor());
  setup();
}

// !++
void EffectAttract::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap((float)EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.02, 1.) * getBaseSpeedFactor();
  else if(_val->getId()==2) _energy = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) _mass = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  setup();
  return String();
}

void EffectAttract::setup(){
  int cnt = 0;
  for (auto &i : boids){
    i = Boid(15, 16 - cnt);
    i.mass = (float)random(1, map(_mass, 1, 255, 128, 1024)) / 100.0f * speedFactor; //(1.0/speed);
    i.velocity.x = (float)random(5, map(_energy, 1, 255, 16, 768)) / 500.0f; // * speedFactor; //(1.0/speed);
    i.velocity.x *= 1-2*random(0, 2); // -1 или 1
    i.velocity.y = 0;
    i.colorIndex = cnt++ * 32;
  }
}

bool EffectAttract::run() {
  fb->fade(255U - beatsin8(3, 170, 250));

  for (auto &boid : boids){
    //boid.acceleration *= speedFactor/10;
    PVector force(attract(boid));
    boid.applyForce(force);

    boid.update();
    EffectMath::drawPixelXYF(boid.location.x, boid.location.y, ColorFromPalette(*curPalette, boid.colorIndex), fb);
  }
  return true;
}

PVector EffectAttract::attract(Boid &m) {
    PVector force = location - m.location;   // Calculate direction of force
    float d = force.mag();                              // Distance between objects
    d = constrain(d, 5.0f, 32.0f);                        // Limiting the distance to eliminate "extreme" results for very close or very far objects
    force.normalize();                                  // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (G * mass * m.mass) / (d * d);      // Calculate gravitional force magnitude
    force *= strength;                                  // Get force vector --> magnitude * direction
    return force;
}

//------------ Эффект "Змейки"
// вариант субпикселя и поведения от kDn
void EffectSnake::load() {
  palettesload();
  reset();
}

void EffectSnake::reset(){
  for (auto &i : snakes){
    i.reset();
    i.pixels[0].x = fb->w() / 2; // пусть расползаются из центра
    i.pixels[0].y = fb->h() / 2; // так будет интереснее
    i.direction = (dir_t)(random8(4));
    i.internal_speedf = (random(2) ? 0.5 : 0.33)+1.0/random(snakes.size()) + 0.5;
  }
}

// !++
void EffectSnake::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt()/ 512.0 + 0.025) * getBaseSpeedFactor();
  else if(_val->getId()==4) {
    snakes.assign(EffectCalc::setDynCtrl(_val).toInt(), Snake(snake_len));
    reset();
  }
  else if(_val->getId()==5) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) onecolor = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSnake::run() {
  fb->fade(speed<25 ? 5 : speed/2 ); // длина хвоста будет зависеть от скорости

  hue += speedFactor/snakes.size();

  hue = hue>255? hue-255 : hue;

  int i = 0;
  for (auto &snake : snakes){

    if(onecolor){
      fill_palette(colors.data(), snake_len, hue, 1, *curPalette, 255-(i*8), LINEARBLEND);
    } else {
      fill_palette(colors.data(), snake_len, (
        (speed<25 || speed>230) ? (i%2 ? hue : 255-hue) : (i%2 ? hue*(i+1) : (255-hue)*(i+1))
      ), 1, *curPalette, 255-(i*8), LINEARBLEND); // вообще в цикле заполнять палитры может быть немножко тяжело... но зато разнообразнее по цветам
    }
    snake.shuffleDown(speedFactor, subPix);

    if (random((speed<25)?speed*50:speed*10) < speed) {// как часто будут повороты :), логика загадочная, но на малой скорости лучше змейкам круги не наматывать :)
      snake.newDirection();
    }

    snake.move(speedFactor, fb->w(), fb->h());
    snake.draw(colors, i, subPix, fb, false /*isDebug()*/);
    ++i;
  }
  return true;
}

void EffectSnake::Snake::draw(std::vector<CRGB> &colors, int snakenb, bool subpix, LedFB<CRGB> *fb, bool isDebug)
{
  int len= isDebug ? 1 : pixels.size();
  for (int i = 0; i < len; i++) // (int)snake_len
  {/*
    if(isDebug){ // тест сабпикселя
      fb->clear(); 
    }
  */
    if (subpix){
      EffectMath::drawPixelXYF(pixels[i].x, pixels[i].y, colors[i], fb);
    }
    else {
      if(i!=0)
        fb->at(pixels[i].x, pixels[i].y) = colors[i];
      else if(direction< dir_t::LEFT)
        EffectMath::drawPixelXYF_Y(pixels[i].x, pixels[i].y, colors[i], fb);
      else
        EffectMath::drawPixelXYF_X(pixels[i].x, pixels[i].y, colors[i], fb);
    }
  }
}

void EffectSnake::Snake::move(float speedy, uint16_t w,  uint16_t h){
  float inc = speedy*internal_speedf;

  switch (direction){
  case dir_t::UP:
    pixels[0].y = pixels[0].y >= h ? inc : (pixels[0].y + inc);
    break;
  case dir_t::LEFT:
    pixels[0].x = pixels[0].x >= w ? inc : (pixels[0].x + inc);
    break;
  case dir_t::DOWN:
    pixels[0].y = pixels[0].y <= 0 ? h - inc : pixels[0].y - inc;
    break;
  case dir_t::RIGHT:
    pixels[0].x = pixels[0].x <= 0 ? w - inc : pixels[0].x - inc;
    break;
  }
}

void EffectSnake::Snake::newDirection(){
    switch (direction)
    {
    case dir_t::UP:
    case dir_t::DOWN:
    direction = random(0, 2) == 1 ? dir_t::RIGHT : dir_t::LEFT;
    break;

    case dir_t::LEFT:
    case dir_t::RIGHT:
    direction = random(0, 2) == 1 ? dir_t::DOWN : dir_t::UP;

    default:
    break;
    }
};

void EffectSnake::Snake::shuffleDown(float speedy, bool subpix)
{
    internal_counter+=speedy*internal_speedf;

    if(internal_counter>1.0){
        for (size_t i = pixels.size() - 1; i > 0; i--)
        {
            if(subpix)
                pixels[i] = pixels[i - 1];
            else {
                pixels[i].x = (uint8_t)pixels[i - 1].x;
                pixels[i].y = (uint8_t)pixels[i - 1].y;
            }
        }
        float f;
        internal_counter=modf(internal_counter, &f);
    }
}

void EffectSnake::Snake::reset()
{
    direction = dir_t::UP;
    for (auto &i : pixels){
      i.x = i.y = 0;
    }
}
#endif  // DISABLED_CODE


//------------ Эффект "Nexus"
// (с) kostyamat 4.12.2020
void EffectNexus::reconfig() {
  for (auto &nx : nxdots) {
    nx.direct = random(0, 4);                     // задаем направление
    nx.posX = random(0, fb->w());                   // Разбрасываем частицы по ширине
    nx.posY = random(0, fb->h());                  // и по высоте
    nx.color = ColorFromPalette(*curPalette, random8(0, 9) * 31, 255); // цвет капли
    nx.accel = (float)random(5, 20) / 100;        // делаем частицам немного разное ускорение 
  }
}

// !++
void EffectNexus::setControl(size_t idx, int32_t value) {
  switch (idx){
    // move speed
    case 0:
      speedFactor = EffectMath::fmap(value, 1, 10, 0.05, 1.33);
      //LOGV(T_Effect, printf, "Nexus speed=%d, speedfactor=%2.2f\n", value, speedFactor);
      break;
    // scale
    case 1: {
      scale = map(value, 1, 10, NEXUS_MIN, NEXUS_MAX);
      LOGV(T_Effect, printf, "Nexus scale=%d\n", scale);
      nxdots.assign(scale, Nexus());
      reconfig();
      break;
    }
    // fade speed
    case 3: {
      speed = value;
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectNexus::load() {
  palettesload();
  randomSeed(millis());
  reconfig();
}

bool EffectNexus::run() {
  // use integral speed as fade speed
  fb->fade(map(speed, 1, 10, 1, 50));

  for (auto &nx : nxdots){
    switch (nx.direct){
      case 0:   // вверх
        nx.posY += (speedFactor + nx.accel);
        break;
      case 1:   //  вниз 
        nx.posY -= (speedFactor + nx.accel);
        break;
      case 2:   // вправо
        nx.posX += (speedFactor + nx.accel);
        break;
      case 3:   // влево
        nx.posX -= (speedFactor + nx.accel);
        break;
      default:
        break;
    }

    // Обеспечиваем бесшовность по Y. И переносим каплю в начало трека
    if (nx.posY < 0) {
      nx.posY = (float)fb->maxHeightIndex();    
      resetDot(nx);
    }

    if (nx.posY > (fb->maxHeightIndex())) {
      nx.posY = 0;
      resetDot(nx);
    }

    // Обеспечиваем бесшовность по X.
    if (nx.posX < 0) {
      nx.posX = fb->maxWidthIndex();
      resetDot(nx);
    }
    if (nx.posX > fb->maxWidthIndex()) {
      nx.posX = 0;
      resetDot(nx);
    }

    switch (nx.direct){
      case 0:   // вверх
      case 1:   //  вниз 
        EffectMath::drawPixelXYF_Y(nx.posX, nx.posY, nx.color, fb, 0);
        break;
      default:
      //case 2:   // вправо
      //case 3:   // влево
        EffectMath::drawPixelXYF_X(nx.posX, nx.posY, nx.color, fb, 0);
    }
  }
  return true;
}

void EffectNexus::resetDot(Nexus &nx) {
  randomSeed(micros());
  nx.direct = random8(0, 4);                     // задаем направление
  nx.color = ColorFromPalette(*curPalette, random(0, 9) * 31, 255);              // цвет 
  nx.accel = (float)random(5, 10) / 70;     // делаем частицам немного разное ускорение 
  switch (nx.direct){
    case 0:   // вверх
      nx.posX = random8(0, fb->w()); // Разбрасываем капли по ширине
      nx.posY = 0;  // и по высоте
      break;
    case 1:   //  вниз 
      nx.posX = random8(0, fb->w()); // Разбрасываем капли по ширине
      nx.posY = fb->maxHeightIndex();  // и по высоте
      break;
    case 2:   // вправо
      nx.posX = 0; // Разбрасываем капли по ширине
      nx.posY = random8(0, fb->h());  // и по высоте
      break;
    //case 3:   // влево
    default:
      nx.posX = fb->maxWidthIndex(); // Разбрасываем капли по ширине
      nx.posY = random8(0, fb->h());  // и по высоте
  } 
}


//-------- Эффект "Детские сны"
// (c) Stepko https://editor.soulmatelights.com/gallery/505
// !++
void EffectSmokeballs::setControl(size_t idx, int32_t value){
  switch (idx){
    // 0 - speed - range 1-10
    case 0:
      speedFactor = EffectMath::fmap(value, 1, 10, 0.01, 0.4);
      break;
    // 1 scale - as-is value clamped to 1-width/4
    case 1:
      scale = clamp(value, 1L, static_cast<int32_t>( fb->w()));
      break;

    // 2 palletes - default

    // 3 - fade - intensity range 1-50
    case 3: {
      dimming = value;
      break;
    }

    // 4 - blur - raw, range 10-80
    case 4: {
      blur = value;
      break;
    }

    // 5 - invert Y - bool
    case 5: {
      _invertY = value;
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectSmokeballs::load(){
  palettesload();
  regen();
}

void EffectSmokeballs::regen() {
  waves.assign(scale, Wave());
  for (auto &w : waves){
    //w.pos = w.reg =  random((fb->w() * 10) - ((fb->w() / 3) * 20)); // сумма maxMin + reg не должна выскакивать за макс.Х
    w.pos = w.reg = 10 * random(fb->w()-fb->w()/8); // сумма maxMin + reg не должна выскакивать за макс.Х
    w.sSpeed = EffectMath::randomf(5, 5 * fb->w());
    w.maxMin = random((fb->w() / 2) * 10, (fb->w() / 3) * 20);
    w.waveColors = random(0, 9) * 28;
    //LOG(printf, "Wave pos:%u, mm:%u\n", w.pos, w.maxMin);
  }
}

bool EffectSmokeballs::run(){
  // resize must be done inside this routine to provide thread-safety for controls change
  if (waves.size() != scale) regen();

  //uint8_t _amount = map(scale, 1, 32, 2, waves.size()-1);
  // shift Up
  if (_invertY){
    for (int16_t y = 0;  y != fb->maxHeightIndex(); ++y)
      for (int16_t x = 0; x != fb->w(); ++x ){
        int16_t yy = y + 1;
        fb->at(x, y) = fb->at(x, yy);
      }
  } else {
    for (int16_t y = fb->maxHeightIndex(); y != 0; --y)
      for (int16_t x = 0; x != fb->w(); ++x ){
        int16_t yy = y - 1;
        fb->at(x, y) = fb->at(x, yy);
      }
  }

  fb->fade(dimming);
  EffectMath::blur2d(fb, blur);
  for (auto &w : waves){
    w.pos = beatsin16((uint8_t)(w.sSpeed * speedFactor), w.reg, w.maxMin + w.reg, w.waveColors*256, w.waveColors*8);
    EffectMath::drawPixelXYF((float)w.pos / 10., _invertY ? fb->maxHeightIndex() : 0, ColorFromPalette(*curPalette, w.waveColors), fb);
  }

  EVERY_N_SECONDS(20){
    for (auto &w : waves ){
      w.reg += random(-20,20);
      w.waveColors += 28;
    }
  }

  if (random8(255) > 252 ) regen();
  return true;
}


#if !defined (OBSOLETE_CODE)
// ----------- Эффект "Ёлки-Палки"
// "Cell" (C) Elliott Kember из примеров программы Soulmate
// Spaider и Spruce (c) stepko
void EffectCell::cell(){
  speedFactor = EffectMath::fmap((float)speed, 1., 255., .33*getBaseSpeedFactor(), 3.*getBaseSpeedFactor());
  offsetX = beatsin16(6. * speedFactor, -180, 180);
  offsetY = beatsin16(6. * speedFactor, -180, 180, 12000);
  for (uint8_t x = 0; x < fb->w(); x++) {
    for (uint8_t y = 0; y < fb->h(); y++) {
      int16_t hue = x * beatsin16(10. * speedFactor, 1, 10) + offsetY;
      fb->at(x, y) = CHSV(hue, 200, sin8(x * 30 + offsetX));
      hue = y * 3 + offsetX;
      fb->at(x, y) += CHSV(hue, 200, sin8(y * 30 + offsetY));
    }
  }
  EffectMath::nightMode(fb); // пригасим немного, чтобы видить структуру, и убрать пересветы
} 

// !++ Тут лучше все оставить как есть, пускай в теле эффекта скорость пересчитывает
void EffectCell::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speed = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) {
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    effId = _scale ? _scale : random(_val->getMin().toInt(), _val->getMax().toInt()+1)+1;
  } else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectCell::run(){
  if (_scale == 0) {
    EVERY_N_SECONDS(60) {
      effId ++;
      if (effId == 7)
        effId = 1;
    }
  } else effId = constrain(_scale, 1, 7);

  switch (effId)
  {
  case 1:
    cell();
    break;
  case 2:
  case 3:
  case 4:
    spruce();
    break;
  case 5:
    spider();
    break;
  case 6:
    vals();
    break;
  default:
    break;
  }

  return true;
}

void EffectCell::spruce() {
  hue++;
  fb->fade( map(speed, 1, 255, 1, 10));
  uint8_t z;
  if (effId == 3) z = triwave8(hue);
  else z = beatsin8(1, 1, 255);
  for (uint8_t i = 0; i < fb->minDim(); i++) {
    x = beatsin16(i * (map(speed, 1, 255, 3, 20)), 
                     i * 2, 
                     (fb->minDim() * 4 - 2) - (i * 2 + 2));
    if (effId == 2) 
      EffectMath::drawPixelXYF_X(x/4 + height_adj(), i, random8(10) == 0 ? CHSV(random8(), random8(32, 255), 255) : CHSV(100, 255, map(speed, 1, 255, 128, 100)), fb);
    else
      EffectMath::drawPixelXYF_X(x/4 + height_adj(), i, CHSV(hue + i * z, 255, 255), fb);
  }
  if (!(fb->w()& 0x01))
    fb->at(fb->w()/2 - ((millis()>>9) & 0x01 ? 1:0), fb->minDim() - 1 - ((millis()>>8) & 0x01 ? 1:0)) = CHSV(0, 255, 255);
  else
    fb->at(fb->w()/2, fb->minDim() - 1) = CHSV(0, (millis()>>9) & 0x01 ? 0 : 255, 255);

  if (glitch) EffectMath::confetti(fb, density);
}

void EffectCell::spider() {
  speedFactor = EffectMath::fmap(speed, 1, 255, 20., 2.) * getBaseSpeedFactor();
  fb->fade( 50);
  for (uint8_t c = 0; c < Lines; c++) {
    float xx = 2. + sin8((float)(millis() & 0x7FFFFF) / speedFactor + 1000 * c * Scale) / 12.;
    float yy = 2. + cos8((float)(millis() & 0x7FFFFF) / speedFactor + 1500 * c * Scale) / 12.;
    EffectMath::drawLineF(xx, yy, (float)fb->w() - xx - 1, (float)fb->h() - yy - 1, CHSV(c * (256 / Lines), 200, 255), fb);
    
  }
}

void EffectCell::vals() {
  speedFactor = map(speed, 1, 255, 100, 512) * getBaseSpeedFactor();
  fb->fade(128);
  a += 1;
  for (byte i = 0; i < 12; i++) {
    EffectMath::drawLineF((float)beatsin88((10 + i) * speedFactor, 0, fb->maxWidthIndex() * 2, i * i) / 2, (float)beatsin88((12 - i) * speedFactor, 0, fb->maxHeightIndex() * 2, i * 5) / 2, (float)beatsin88((8 + i) * speedFactor, 0, fb->maxWidthIndex() * 2, i * 20) / 2, (float)beatsin88((14 - i) * speedFactor, 0, fb->maxHeightIndex() * 2, i * 5) / 2, CHSV(21 * i + (byte)a * i, 255, 255), fb);
  }
}

// ----------- Эффект "Осцилятор"
// (c) Сотнег (SottNick)
bool EffectOscillator::run() {
  if (millis() - timer < (unsigned)map(speed, 1U, 255U, 70, 15)) return false;
  else timer = millis(); // не могу сообразить, как по другому скоростью управлять
  CRGB currColors[3];
  for (uint8_t c = 0; c < 3; c++)
    currColors[c] = ColorFromPalette(*curPalette, c * 85U + hue);

  // расчёт химической реакции и отрисовка мира
  uint16_t colorCount[3] = {0U, 0U, 0U};
  hue++;

  fb->clear();
  for (uint8_t y = 0; y < oscillatingWorld.h(); y++) {
      for (uint8_t x = 0; x < oscillatingWorld.w(); x++) {
          if (oscillatingWorld.at(x,y).red){
             colorCount[0]++;
             if (greenNeighbours(x, y) > 2)
                oscillatingWorld.at(x,y).color = 1U;
          }
          else if (oscillatingWorld.at(x,y).green){
             colorCount[1]++;
             if (blueNeighbours(x, y) > 2)
                oscillatingWorld.at(x,y).color = 2U;
          }
          else {//if (oscillatingWorld.at(x,y).blue){
             colorCount[2]++;
             if (redNeighbours(x, y) > 2)
                oscillatingWorld.at(x,y).color = 0U;
          }
          drawPixelXYFseamless((float)x + 0.5, (float)y + 0.5, currColors[oscillatingWorld.at(x,y).color]);
      }
  }


  // проверка зацикливания
  if (colorCount[0] == deltaHue && colorCount[1] == deltaHue2 && colorCount[2] == deltaValue) {
    step++;
    if (step > 10U){
      if (colorCount[0] < colorCount[1])
        step = 0;
      else
        step = 1;
      if (colorCount[2] < colorCount[step])
        step = 2;
      colorCount[step] = 0U;
      step = 0U;
    }
  }
  else
    step = 0U;
  
  // вброс хаоса
  if (hue == hue2){// чтобы не каждый ход
    hue2 += random8(220U) + 36U;
    uint8_t tx = random8(oscillatingWorld.w());
    deltaHue = oscillatingWorld.at(tx, 0U).color + 1U;
    if (deltaHue > 2U) deltaHue = 0U;
    oscillatingWorld.at(tx, 0U).color = deltaHue;
    oscillatingWorld.at((tx + 1U) % oscillatingWorld.w(), 0U).color = deltaHue;
    oscillatingWorld.at((tx + 2U) % oscillatingWorld.w(), 0U).color = deltaHue;
  }

  deltaHue = colorCount[0];
  deltaHue2 = colorCount[1];
  deltaValue = colorCount[2];

  // вброс исчезнувшего цвета
  for (uint8_t c = 0; c < 3; c++)
  {
    if (colorCount[c] < 6U){
      uint8_t tx = random8(oscillatingWorld.w());
      uint8_t ty = random8(oscillatingWorld.h());
      if (random8(2U)){
        oscillatingWorld.at(tx, ty).color = c;
        oscillatingWorld.at((tx + 1U) % oscillatingWorld.w(), ty).color = c;
        oscillatingWorld.at((tx + 2U) % oscillatingWorld.w(), ty).color = c;
      }
      else {
        oscillatingWorld.at(tx, ty).color = c;
        oscillatingWorld.at(tx, (ty + 1U) % oscillatingWorld.h()).color = c;
        oscillatingWorld.at(tx, (ty + 2U) % oscillatingWorld.h()).color = c;
      }
    }
  }

  // перенос на следующий цикл
  for (uint8_t x = 0; x < oscillatingWorld.w(); x++) {
      for (uint8_t y = 0; y < oscillatingWorld.h(); y++) {
          setCellColors(x, y);
      }
  }

  //fpsmeter();
  return true;
}

void EffectOscillator::load() {
  palettesload();
  step = 0U;
 //случайное заполнение
  for (uint8_t i = 0; i < oscillatingWorld.w(); i++) {
    for (uint8_t j = 0; j < oscillatingWorld.h(); j++) {
      oscillatingWorld.at(i, j).color = random8(3);
      setCellColors(i, j);
    }
  }
  timer = millis();
}

void EffectOscillator::drawPixelXYFseamless(float x, float y, CRGB color)
{
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {EffectMath::wu_weight(ix, iy), EffectMath::wu_weight(xx, iy),
                   EffectMath::wu_weight(ix, yy), EffectMath::wu_weight(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t xn = (uint8_t)(x + (i & 1)) % oscillatingWorld.w();
    uint8_t yn = (uint8_t)(y + ((i >> 1) & 1)) % oscillatingWorld.h();
    CRGB clr = fb->at(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    fb->at(xn, yn) = clr;
  }
}

int EffectOscillator::redNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), y).red) +
         (oscillatingWorld.at(x, (y + 1) % oscillatingWorld.h()).red) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), y).red) +
         (oscillatingWorld.at(x, (y + fb->maxHeightIndex()) % oscillatingWorld.h()).red) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).red) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).red) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).red) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).red);
    }

int EffectOscillator::blueNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), y).blue) +
         (oscillatingWorld.at(x, (y + 1) % oscillatingWorld.h()).blue) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), y).blue) +
         (oscillatingWorld.at(x, (y + fb->maxHeightIndex()) % oscillatingWorld.h()).blue) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).blue) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).blue) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).blue) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).blue);
}
  
int EffectOscillator::greenNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), y).green) +
         (oscillatingWorld.at(x, (y + 1) % oscillatingWorld.h()).green) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), y).green) +
         (oscillatingWorld.at(x, (y + fb->maxHeightIndex()) % oscillatingWorld.h()).green) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).green) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + 1) % oscillatingWorld.h()).green) +
         (oscillatingWorld.at((x + fb->maxWidthIndex()) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).green) +
         (oscillatingWorld.at((x + 1) % oscillatingWorld.w(), (y + fb->maxHeightIndex()) % oscillatingWorld.h()).green);
}

void EffectOscillator::setCellColors(uint8_t x, uint8_t y) {
  oscillatingWorld.at(x,y).red = (oscillatingWorld.at(x,y).color == 0U);
  oscillatingWorld.at(x,y).green = (oscillatingWorld.at(x,y).color == 1U);
  oscillatingWorld.at(x,y).blue = (oscillatingWorld.at(x,y).color == 2U);
}


//-------- по мотивам Эффектов Particle System -------------------------
// https://github.com/fuse314/arduino-particle-sys
// https://github.com/giladaya/arduino-particle-sys
// https://www.youtube.com/watch?v=S6novCRlHV8&t=51s

void EffectFairy::particlesUpdate(TObject &i) {
  i.state -= 1 * speedFactor;

  //apply velocity
  i.posX += i.speedX * speedFactor;
  i.posY += i.speedY * speedFactor;
  if(i.state == 0 || i.posX <= -1 || i.posX >= fb->w() || i.posY <= -1 || i.posY >= fb->h()) 
    i.isShift = false;
}

// ============= ЭФФЕКТ ИСТОЧНИК ===============
// (c) SottNick
// выглядит как https://github.com/fuse314/arduino-particle-sys/blob/master/examples/StarfieldFastLED/StarfieldFastLED.ino

void EffectFairy::fountEmit(TObject &i) {
  if (hue++ & 0x01)
    hue2++;

  i.posX = fb->w() * 0.5;
  i.posY = fb->h() * 0.5;


  i.speedX = (((float)random8()-127.)/512.); 
  i.speedY = EffectMath::sqrt(0.0626-i.speedX * i.speedX); 
  
  if(random8(2U)) i.speedY=-i.speedY;

  i.state = EffectMath::randomf(50, 250); 
  i.hue = hue2;

  i.isShift = true; 
}

void EffectFairy::fount(){
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле

  fb->dim(EffectMath::fmap(speed, 1, 255, 180, 127)); //ахах-ха. очередной эффект, к которому нужно будет "подобрать коэффициенты"

  //go over particles and update matrix cells on the way
  for (auto &i : units){
    if (!i.isShift && step) {
      fountEmit(i);
      step--;
    }
    if (i.isShift) { 
      particlesUpdate(i);

      //generate RGB values for particle
      CRGB baseRGB;
        baseRGB = CHSV(i.hue, 255, _video); 

      baseRGB.nscale8(i.state);
      EffectMath::drawPixelXYF(i.posX, i.posY, baseRGB, fb, 0);
    }
  }
  if (blur) EffectMath::blur2d(fb, blur * 10); // Размытие 
}

// ============= ЭФФЕКТ ФЕЯ ===============
// (c) SottNick
#define FAIRY_BEHAVIOR //типа сложное поведение

void EffectFairy::fairyEmit(TObject &i) {
    if (deltaHue++ & 0x01)
      if (hue++ & 0x01)
        hue2++;//counter++;
    i.posX = boids[0].location.x;
    i.posY = boids[0].location.y;

    //хотите навставлять speedFactor? - тут не забудьте
    //i.speedX = ((float)random8()-127.)/512./0.25*speedFactor; 
    i.speedX = ((float)random8()-127.)/512.; 
    i.speedY = EffectMath::sqrt(0.0626- i.speedX*i.speedX);
    if(random8(2U)) { i.speedY *= -1; }

    i.state = random8(20, 80); 
    i.hue = hue2;
    i.isShift = true;
}

bool EffectFairy::fairy() {
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле
  
#ifdef FAIRY_BEHAVIOR
  if (!deltaHue && deltaHue2 && fabs(boids[0].velocity.x) + fabs(boids[0].velocity.y) < 0.15){ 
    deltaHue2 = 0U;
    
    boids[1].velocity.x = (((float)random8()+255.) / 4080.);
    boids[1].velocity.y = (((float)random8()+255.) / 2040.);
    if (boids[0].location.x > fb->w() * 0.5) boids[1].velocity.x = -boids[1].velocity.x;
    if (boids[0].location.y > fb->h() * 0.5) boids[1].velocity.y = -boids[1].velocity.y;
  }
  if (!deltaHue2){
    step = 1U;
    
    boids[0].location.x += boids[1].velocity.x * speedFactor;
    boids[0].location.y += boids[1].velocity.y * speedFactor;
    deltaHue2 = (boids[0].location.x <= 0 || boids[0].location.x >= fb->maxWidthIndex() || boids[0].location.y <= 0 || boids[0].location.y >= fb->maxHeightIndex());
  }
  else
#endif // FAIRY_BEHAVIOR
  {  
    PVector attractLocation = PVector(fb->w() * 0.5, fb->h() * 0.5);
    //float attractMass = 10;
    //float attractG = .5;
    // перемножаем и получаем 5.
    Boid boid = boids[0];
    PVector force = attractLocation - boid.location;      // Calculate direction of force
    float d = force.mag();                                // Distance between objects
    d = constrain(d, 5.0f, fb->h());//видео снято на 5.0f  // Limiting the distance to eliminate "extreme" results for very close or very far objects
    //d = constrain(d, modes[currentMode].Scale / 10.0, fb->h());

    force.normalize();                                    // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (5. * boid.mass) / (d * d);          // Calculate gravitional force magnitude 5.=attractG*attractMass
    //float attractMass = (modes[currentMode].Scale) / 10.0 * .5;
    force *= strength * speedFactor;                                    // Get force vector --> magnitude * direction
    boid.applyForce(force);
    boid.update();
    
    if (boid.location.x <= -1) boid.location.x = -boid.location.x;
    else if (boid.location.x >= fb->w()) boid.location.x = -boid.location.x+fb->w()+fb->w();
    if (boid.location.y <= -1) boid.location.y = -boid.location.y;
    else if (boid.location.y >= fb->h()) boid.location.y = -boid.location.y+fb->h()+fb->h();
    boids[0] = boid;

    if (!deltaHue) {
      if (random8(3U)){
        d = ((random8(2U)) ? boids[0].velocity.x : boids[0].velocity.y) * ((random8(2U)) ? .2 : -.2);
        boids[0].velocity.x += d * speedFactor;
        boids[0].velocity.y -= d * speedFactor;
      }
      else {
        if (fabs(boids[0].velocity.x) < 0.02)
          boids[0].velocity.x = -boids[0].velocity.x;
        else if (fabs(boids[0].velocity.y) < 0.02)
          boids[0].velocity.y = -boids[0].velocity.y;
      }
    }
  }

  fb->dim(EffectMath::fmap(speed, 1, 255, 180, 127));

  //go over particles and update matrix cells on the way
  for(auto &i : units){
    if (!i.isShift && step) {
      fairyEmit(i);
      step--;
    }
    if (i.isShift){
      // вернуться и поглядеть, что это
      if (type && i.speedY > -1) i.speedY -= 0.05; //apply acceleration
      particlesUpdate(i);

      //generate RGB values for particle
      CRGB baseRGB = CHSV(i.hue, 255,255); 

      baseRGB.nscale8(i.state);//эквивалент
      EffectMath::drawPixelXYF(i.posX, i.posY, baseRGB, fb, 0);
    }
  }

  EffectMath::drawPixelXYF(boids[0].location.x, boids[0].location.y, CHSV(hue, 160U, 255U) /*temp*/, fb, 0);  
  return true;
}

bool EffectFairy::run() {
  _video = 255;

  switch (effect)
  {
  case effect_t::fairy:
    return fairy();
    break;
  case effect_t::fountain:
    fount();
    break; 
  default:
    break;
  }
  //fpsmeter();
  return true;
}


void EffectFairy::load(){
  //---- Общее для двух эффектов
  if(effect == effect_t::fairy)
    deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
  else
    deltaValue = units.size() / (EffectMath::sqrt(CENTER_X_MAJOR * CENTER_X_MAJOR + CENTER_Y_MAJOR * CENTER_Y_MAJOR) * 4U) + 1U; // 4 - это потому что за 1 цикл частица пролетает ровно четверть расстояния между 2мя соседними пикселями

  units.assign( map(scale, 1, 255, FAIRY_MIN_COUNT, _max_units()), TObject() );

  for (auto &i : units)
    i.isShift = false;

  //---- Только для эффекта Фея
  // лень было придумывать алгоритм для таектории феи, поэтому это будет нулевой "бойд" из эффекта Притяжение
  boids[0] = Boid(random8(fb->w()), random8(fb->h()));
  boids[0].mass = 0.5;//((float)random8(33U, 134U)) / 100.; // random(0.1, 2); // сюда можно поставить регулятор разлёта. чем меньше число, тем дальше от центра будет вылет
  boids[0].velocity.x = ((float) random8(46U, 100U)) / 500.0;
  if (random8(2U)) boids[0].velocity.x = -boids[0].velocity.x;
  boids[0].velocity.y = 0;
  hue = random8();
  #ifdef FAIRY_BEHAVIOR
    deltaHue2 = 1U;
  #endif
}

// !++
void EffectFairy::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) {
    if (effect == effect_t::fairy) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.05, .25) * getBaseSpeedFactor();
    else speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.2, 1.) * getBaseSpeedFactor();
  } else if(_val->getId()==2) {
    EffectCalc::setDynCtrl(_val);   // маразм
    units.assign( map(scale, 1, 255, FAIRY_MIN_COUNT, _max_units()), TObject() );
    units.shrink_to_fit();
  } else if(_val->getId()==3) type = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) blur = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) gain = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// ---------- Эффект "Бульбулятор"
// "Circles" (C) Elliott Kember https://editor.soulmatelights.com/gallery/11
// адаптация и переделка - kostyamat
//!++
void EffectCircles::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 5) * getBaseSpeedFactor();
  else if(_val->getId()==2) {
    circles.assign( map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, CIRCLES_MIN, fb->w()), Circle());
  }
  else if(_val->getId()==4) gain = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectCircles::load() {
  palettesload();
  for (auto &i : circles) reset(i);
}

void EffectCircles::drawCircle(LedFB<CRGB> *fb, Circle &circle) {
  int16_t centerX = circle.centerX;
  int16_t centerY = circle.centerY;
  uint8_t hue = circle.hue;
  float radius = circle.radius();

  int16_t startX = centerX - ceil(radius);
  int16_t endX = centerX + ceil(radius);
  int16_t startY = centerY - ceil(radius);
  int16_t endY = centerY + ceil(radius);

  for (int16_t x = startX; x < endX; x++) {
    for (int16_t y = startY; y < endY; y++) {
      float distance = EffectMath::sqrt(sq(x - centerX) + sq(y - centerY));
      if (distance > radius)
        continue;

      uint16_t brightness;
      if (radius < 1) { // last pixel
        brightness = 255.0 * radius;
      } else {
        float percentage = distance / radius;
        float fraction = 1.0 - percentage;
        brightness = (float)_video * fraction;
      }
      fb->at(x, y) += ColorFromPalette(*curPalette, hue, brightness);
    }
  }
}

bool EffectCircles::run() {
  _video = 255;

  randomSeed(millis());
  fb->clear();
  for (auto &i : circles){
    i.bpm += speedFactor;
    if (i.radius() < 0.001) {
      i.hue = random(0, fb->w()) * 255 / circles.size();

      move(i);
    }
    drawCircle(fb, i);
  }
  return true;
}

// ----------- Эффект "Шары"
// (c) stepko and kostyamat https://wokwi.com/arduino/projects/289839434049782281
// 07.02.2021
void EffectBalls::load() {
  palettesload();
  speedFactor = EffectMath::fmap(speed, 1, 255, 0.15, 0.5) * getBaseSpeedFactor();
  reset();
}

void EffectBalls::reset(){
  randomSeed(millis());
  for (auto &i : balls){
    i.radius = EffectMath::randomf(0.5, radiusMax);
    i.spdy = EffectMath::randomf(0.5, 1.1) * speedFactor;
    i.spdx = EffectMath::randomf(0.5, 1.1) * speedFactor;
    i.y = random(0, fb->w());
    i.x = random(0, fb->h());
    i.color = random(0, 255);
  }
}

// !++
void EffectBalls::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.15, 0.5) * getBaseSpeedFactor();
  else if(_val->getId()==2) {   // Scale
    balls.assign( map(scale, 1, 255, BALLS_MIN, fb->maxDim()), Ball() );
    balls.shrink_to_fit();
    reset();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectBalls::run() {
  fb->fade(map(speed, 1, 255, 5, 20));

  for (auto &i : balls){
    if (i.rrad) {  // тут у нас шарики надуваются\сдуваются по ходу движения
      i.radius += (fabs(i.spdy) > fabs(i.spdx)? fabs(i.spdy) : fabs(i.spdx)) * 0.1 * speedFactor;
      if (i.radius >= radiusMax) {
        i.rrad = false;
      }
    } else {
      i.radius -= (fabs(i.spdy) > fabs(i.spdx)? fabs(i.spdy) : fabs(i.spdx)) * 0.1 * speedFactor;
      if (i.radius < 1.) {
        i.rrad = true;
        i.color = random(0, 255);
      }
    }


    //EffectMath::drawCircleF(i.x, i.y, i.radius, ColorFromPalette(*curPalette, i.color), 0.5);
    if (i.radius > 1) 
      EffectMath::fill_circleF(i.x, i.y, i.radius, ColorFromPalette(*curPalette, i.color), fb);
    else 
      EffectMath::drawPixelXYF(i.x, i.y, ColorFromPalette(*curPalette, i.color), fb);


    if (i.y + i.radius >= fb->maxHeightIndex())
      i.y += (i.spdy * ((fb->maxHeightIndex() - i.y) / i.radius + 0.005));
    else if (i.y - i.radius <= 0)
      i.y += (i.spdy * (i.y / i.radius + 0.005));
    else
      i.y += i.spdy;
    //-----------------------
    if (i.x + i.radius >= fb->maxWidthIndex())
      i.x += (i.spdx * ((fb->maxWidthIndex() - i.x) / i.radius + 0.005));
    else if (i.x - i.radius <= 0)
      i.x += (i.spdx * (i.x / i.radius + 0.005));
    else
      i.x += i.spdx;
    //------------------------
    if (i.y < 0.01) {
      i.spdy = EffectMath::randomf(0.5, 1.1) * speedFactor;
      i.y = 0.01;
    }
    else if (i.y > (float)fb->maxHeightIndex()) {
      i.spdy = EffectMath::randomf(0.5, 1.1) * speedFactor;
      i.spdy = -i.spdy;
      i.y = (float)fb->maxHeightIndex();
    }
    //----------------------
    if (i.x < 0.01) {
      i.spdx = EffectMath::randomf(0.5, 1.1) * speedFactor;
      i.x = 0.01;
    }
    else if (i.x > fb->maxWidthIndex()) {
      i.spdx = EffectMath::randomf(0.5, 1.1) * speedFactor;
      i.spdx = -i.spdx;
      i.x = fb->maxWidthIndex();
    }
  }
  EffectMath::blur2d(fb, 48);
  return true;
}

// ---------- Эффект-игра "Лабиринт"
EffectMaze::EffectMaze(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer),
  _mwidth(fb->w()%2 ? fb->w() : fb->w()-1),
  _mheight(fb->h()%2 ? fb->h() : fb->h()-1),
  maxSolves(_mwidth*_mheight*5),
  maze(Vector2D<uint8_t>(_mwidth, _mheight))
  { fb->clear(); };

void EffectMaze::newGameMaze() {
  playerPos[0] = !MAZE_SHIFT;
  playerPos[1] = !MAZE_SHIFT;

  gameOverFlag = false;
  buttons = 4;

  GenerateMaze();    // генерировать лабиринт обычным способом
  SolveMaze();       // найти путь

  unsigned h{MAZE_FOV}, w{MAZE_FOV};
  if (!(MAZE_GAMEMODE || mazeMode)){
    h = _mheight;
    w = _mwidth;
  }

  for (unsigned y = 0; y < h; y++)
    for (unsigned x = 0; x < w; x++)
      fb->at(x, y) = maze.at(x + MAZE_SHIFT, y + MAZE_SHIFT) == 1 ? color : CRGB::Black;

  // Отрисовка - с видимыми границами по периметру (настройки MAZE_SHIFT выше)
  // Слева от начальной позиции делаем дыру - это вход
  if (playerPos[0]>0) {
    playerPos[0] = playerPos[0] - 1;
    fb->at(playerPos[0], playerPos[1]) = 0x000000;
  }
  
  fb->at(playerPos[0], playerPos[1]) = playerColor;

  mazeStarted = false;  
}

bool EffectMaze::run() {
  if (loadingFlag || gameOverFlag) {  
    if (loadingFlag) fb->clear();
    gameTimer = map(speed, 1, 255, 500, 50);   // установить начальную скорость
    loadingFlag = false;
    newGameMaze();
    // modeCode = MC_MAZE;
  }

  if (gameDemo && !gamePaused) demoMaze();
  buttonsTickMaze();
  return true;
}

void EffectMaze::buttonsTickMaze() {

  if (gameDemo && (millis() - timer < gameTimer)) { // тут крутим скорость в демо-режиме
    return;
  }
  timer = millis();

  if (checkButtons()) {
    bool btnPressed = false;
    if (buttons == 3) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[0] - 1;
      if (newPos >= 0 && newPos <= fb->maxWidthIndex())
        if (!fb->at(newPos, playerPos[1])) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 1) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[0] + 1;
      if (newPos >= 0 && newPos <= fb->maxWidthIndex())
        if (!fb->at(newPos, playerPos[1])) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 0) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] + 1;
      if (newPos >= 0 && newPos <= fb->maxHeightIndex())
        if (!fb->at(playerPos[0], newPos)) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
    }
    if (buttons == 2) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] - 1;
      if (newPos >= 0 && newPos <= fb->maxHeightIndex())
        if (!fb->at(playerPos[0], newPos)) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
    }
    if (btnPressed && !mazeStarted) {
      mazeStarted = true;
      labTimer = millis();
    }
    buttons = 4;
  }
}

void EffectMaze::movePlayer(int8_t nowX, int8_t nowY, int8_t prevX, int8_t prevY) {
  if (!track) fb->at(prevX, prevY) = 0x000000;
  fb->at(nowX, nowY) = playerColor;

  if ((nowX == (_mwidth - 2) - MAZE_SHIFT) && (nowY == (_mheight - 1) - MAZE_SHIFT)) {
    gameOverFlag = true;
    return;
  }

  if (MAZE_GAMEMODE || mazeMode) {
    for (int8_t y = nowY - MAZE_FOV; y < nowY + MAZE_FOV; y++)
      for (int8_t x = nowX - MAZE_FOV; x < nowX + MAZE_FOV; x++) {
        if (x < 0 || x > fb->maxWidthIndex() || y < 0 || y > fb->maxHeightIndex())  continue;
        if (maze.at(x + MAZE_SHIFT, y + MAZE_SHIFT) == 1)   fb->at(x, y) = CRGB::Aqua;
      }
  }
}

void EffectMaze::demoMaze() {
  if (checkPath(0, 1)) buttons = 0;
  if (checkPath(1, 0)) buttons = 1;
  if (checkPath(0, -1)) buttons = 2;
  if (checkPath(-1, 0)) buttons = 3;
}

bool EffectMaze::checkPath(int8_t x, int8_t y) {
  // если проверяемая клетка является путью к выходу
  if ( maze.at(playerPos[0] + x + MAZE_SHIFT, playerPos[1] + y + MAZE_SHIFT) == 2) {
    maze.at(playerPos[0] + MAZE_SHIFT, playerPos[1] + MAZE_SHIFT) = 4;  // убираем текущую клетку из пути (2 - метка пути, ставим любое число, например 4)
    return true;
  }

  return false;
}

// копаем лабиринт
void EffectMaze::CarveMaze(int x, int y) {
  unsigned x1, y1;
  unsigned x2, y2;
  int dx, dy;
  int dir, count;

  dir = random(10) % 4;
  count = 0;
  while (count < 4) {
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    x1 = x + dx;
    y1 = y + dy;
    x2 = x1 + dx;
    y2 = y1 + dy;
    if (   x2 > 0 && x2 < maze.w() && y2 > 0 && y2 < maze.h()
           && maze.at(x1, y1) == 1 && maze.at(x2,y2) == 1) {
      maze.at(x1, y1) = 0;
      maze.at(x2, y2) = 0;
      x = x2; y = y2;
      dir = random(10) % 4;
      count = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
    }
  }
}

// генератор лабиринта
void EffectMaze::GenerateMaze() {
  unsigned x, y;
  uint8_t init = 1;
  maze.reset((size_t)maze.w(), (size_t)maze.h(), init);

  maze.at(0,1) = 0;
  for (y = 1; y < maze.h(); y += 2)
    for (x = 1; x < maze.w(); x += 2)
      CarveMaze(x, y);


  // вход и выход
  maze.at(1,0) = 0;
  maze.at(maze.w()-2, maze.h()-1) = 0;

  track = random8(0,2);
  color = CHSV(hue += 8, random8(192, 255), 192);
  
  playerColor = CHSV(hue + random(63, 127), random8(127, 200), 255);
}

// решатель (ищет путь)
void EffectMaze::SolveMaze() {
  int dir{0}, count{0};
  unsigned x{1}, y{1};
  int dx, dy;
  int forward = 1;
  // Remove the entry and exit. 
  maze.at(1,0) = 1;
  maze.at(maze.w()-2, maze.h()-1) = 1;

  unsigned int attempts = 0;
  while (x != maze.w() - 2 || y != maze.h() - 2) {
    if (attempts++ > maxSolves) {   // если решатель не может найти решение (maxSolves в 5 раз больше числа клеток лабиринта)
      gameOverFlag = true;          // перегенерировать лабиринт
      break;                        // прервать решение
    }
    dx = 0; dy = 0;
    switch (dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
    }
    if (   (forward  && maze.at(x+dx, y+dy) == 0)
           || (!forward && maze.at(x+dx, y+dy) == 2 )) {
      maze.at(x,y) = forward ? 2 : 3; 
      x += dx;
      y += dy;
      forward = 1;
      count = dir = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
      if (count > 3)
        forward = count = 0;
    }
  }
  
  // Replace the entry and exit.
  maze.at(maze.w()-2, maze.h()-2 ) = 2;
  maze.at(maze.w()-2, maze.h()-1) =2;
}

// !++
void EffectMaze::setControl(size_t idx, int32_t value){
  if(_val->getId()==1)
    gameTimer = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 500, 50);   // установить скорость
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}


// --------- Эффект "Вьющийся Цвет"
// (c) Stepko https://wokwi.com/arduino/projects/283705656027906572
// доработка - kostyamat
bool EffectFrizzles::run() {
  _speed = EffectMath::fmap(speed, 1, 255, 0.25, 3);
  if (scale <= 127) _scale = EffectMath::fmap(scale, 1, 255, 1, 8);
  else _scale = EffectMath::fmap(scale, 1, 255, 8, 1);

  for(float i= (float)8 * _scale; i> 0; i--)
    fb->at(beatsin8(12. * _speed + i * _speed, 0, fb->maxWidthIndex()), beatsin8(15. * _speed + i * _speed, 0, fb->maxHeightIndex())) = CHSV(beatsin8(12. * _speed, 0, 255), scale > 127 ? 255 - i*8 : 255, scale > 127 ? 127 + i*8 : 255);
  EffectMath::blur2d(fb, 16);
  return true;
}

#endif //#if !defined (OBSOLETE_CODE)

// ----------------- Эффект "Магма"
// (c) Сотнег (SottNick) 2021
// адаптация и доводка до ума - kostyamat
void EffectMagma::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.clear();
  palettes.reserve(12);
  palettes.push_back(&MagmaColor_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&NormalFire2_p);
  palettes.push_back(&NormalFire3_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&WaterfallColors_p);
  LOGV(T_Effect, printf, "Loaded %u palettes\n", palettes.size());
  curPalette = &MagmaColor_p;
}

void EffectMagma::load() {
  palettesload();
  regen();
}

// !++
void EffectMagma::setControl(size_t idx, int32_t value){
  switch (idx){
    // speed
    case 0:
      speedFactor = EffectMath::fmap(value, 1, 10, 0.05, 1.2);
      LOGV(T_Effect, printf, "Magma speed=%d, speedfactor=%2.2f\n", value, speedFactor);
      break;
    // scale
    case 1: {
      scale = map(value, 1, 10, MAGMA_MIN_OBJ, MAGMA_MAX_OBJ);
      LOGV(T_Effect, printf, "Magma scale=%d\n", scale);
      particles.assign(scale, Magma());
      regen();
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }

}

void EffectMagma::regen() {
  randomSeed(millis());
  for (size_t j = 0; j != shiftHue.size(); ++j){
    shiftHue[j] = map(j, 0, fb->h()+fb->h()/4, 255, 0); // init colorfade table
  }

  for (auto &i : particles){
    leapersRestart_leaper(i);
    i.hue = 50U;
  }
}

bool EffectMagma::run() {
  fb->fade(50);
  

  for (auto &i : particles){
    leapersMove_leaper(i);
    i.hue = 50U;
    EffectMath::drawPixelXYF(i.posX, i.posY, ColorFromPalette(*curPalette, i.hue), fb, 0);
  }

  for (size_t i = 0; i != fb->w(); ++i) {
    for (size_t j = 0; j != fb->h(); ++j) {
     fb->at(i, j) += ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j + ff_y) * deltaHue, ff_z), shiftHue.at(j)), 127U);
    }
  }

  ff_y += speedFactor * 2;
  ff_z += speedFactor;
  //EffectMath::blur2d(fb->data(), fb->w, fb->h, 32);
  return true;
}

void EffectMagma::leapersMove_leaper(Magma &l) {

  l.shift -= gravity * speedFactor;
  l.posX  += l.speedX * speedFactor;
  l.posY  -= l.shift * speedFactor;

/*
  // bounce off the ceiling (floor inverted) with some probability
  if (l.posY < fb->h()/5 && random8()<32) {
    l.shift *= -1;
  }
*/
  // settled on the floor (ceiling inverted)?
  if (l.posY > fb->h() - fb->h()/8) {
    leapersRestart_leaper(l);
  }

  // bounce off the sides of the screen?
  if ((l.posX < 0 || l.posX > fb->w()) && random8()<32) {
    leapersRestart_leaper(l);
  }
}

void EffectMagma::leapersRestart_leaper(Magma &l) {
  // leap up and to the side with some random component
  l.speedX = EffectMath::randomf(0.2, 1 + speedFactor);
  if (random8() % 2) l.speedX *= -1;
  l.shift = EffectMath::randomf(0.85, 3 + speedFactor);
  l.posX = std::rand() % fb->w();
  l.posY = std::rand() % fb->h() + (fb->h() - fb->h()/4);
}

#if !defined (OBSOLETE_CODE)
// --------------------- Эффект "Звездный Десант"
// Starship Troopers https://editor.soulmatelights.com/gallery/839-starship-troopers
// Based on (c) stepko`s codes https://editor.soulmatelights.com/gallery/639-space-ships
// reworked (c) kostyamat (subpixel, shift speed control, etc)  08.04.2021

// !++
void EffectStarShips::setControl(size_t idx, int32_t value){
  if (_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    _fade = map(speed, 1, 255, 35, 25);
    speedFactor = EffectMath::fmap(speed, 1, 255, 0.20, 1);
    chance = 4096. / speedFactor;
  }
  else if (_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if (_val->getId()==4) {
    _dir = EffectCalc::setDynCtrl(_val).toInt();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectStarShips::load() {
  palettesload();
  fb->clear();
}

bool EffectStarShips::run() {
  fb->fade(_fade);
  switch (dir) {
    case 0: // Up
      for (byte x = 0; x < fb->w(); x++) {
		    if (!_dir and x > fb->w()/2 and random8(chance) == DIR_CHARGE) {count++; break;}
        for (float y = 0; y < fb->h(); y+=speedFactor) {
          fb->at(x, y) = (((int)y == fb->maxHeightIndex()) ? CRGB::Black : fb->at(x, y + 1));
        }
      }
      break;
    case 1: // Up - Right 
      for (float x = 0; x < fb->w(); x+=speedFactor) {
        if (!_dir and (uint8_t)x > fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (byte y = 0; y < fb->h(); y++) {
          fb->at(x, y) = ((y == fb->maxHeightIndex() or (int)x == fb->maxWidthIndex()) ? CRGB::Black : fb->at(x + 1, y + 1));
        }
      }
      break;
    case 2: // Right
      for (float x = 0; x < fb->w(); x+=speedFactor) {
        if (!_dir and (uint8_t)x > fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb->maxHeightIndex(); y > 0; y--) {
          fb->at(x, y) = (((int)x == fb->maxWidthIndex()) ? CRGB::Black : fb->at(x + 1, y));
        }
      }
      break;
    case 3: // Down - Right 
      for (float x = 0; x < fb->w(); x+=speedFactor) {
        if (!_dir and (uint8_t)x > fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb->maxHeightIndex(); y > 0; y--) {
          fb->at(x, y) = (((int)x == fb->maxWidthIndex() or y == 0) ? CRGB::Black : fb->at(x + 1, y - 1));
        }
      }
      break;
    case 4: // Down
      for (byte x = 0; x < fb->w(); x++) {
		    if (!_dir and x < fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (float y = fb->maxHeightIndex(); y > 0; y-=speedFactor) {
          fb->at(x, y) = (((int)y == 0) ? CRGB::Black : fb->at(x, y - 1));
        }
      }
      break;
    case 5: // Down - Left
      for (float x = fb->maxWidthIndex(); x > 0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb->maxHeightIndex(); y > 0; y--) {
          fb->at(x, y) = ((y == 0 or (int)x == 0) ? CRGB::Black : fb->at(x - 1, y - 1));
        }
      }
      break;
    case 6: // Left
      for (float x = fb->maxWidthIndex(); x > 0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb->maxHeightIndex(); y > 0; y--) {
          fb->at(x, y) = ((int)x == 0 ? CRGB::Black : fb->at(x - 1, y));
        }
      }
      break;
    case 7: // Up - Left 
      for (float x = fb->w() -1; x >0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < fb->w()/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb->maxHeightIndex(); y > 0; y--) {
          fb->at(x, y) = ((y == fb->maxHeightIndex() or (int)x == 0) ? CRGB::Black : fb->at(x - 1, y + 1));
        }
      }
      break;
  }

  for (byte i = 0; i < _scale; i++) {
    float x = (float)beatsin88(3840*speedFactor + i*256, 0, fb->maxWidthIndex() *4, 0, _scale*i*256) /4;
    float y = (float)beatsin88(3072*speedFactor + i*256, 0, fb->maxWidthIndex() *4, 0, 0) /4;
    if ((x >= 0 and x <= fb->maxWidthIndex()) and (y >= 0 and y <= fb->maxHeightIndex())) draw(x, y, ColorFromPalette(*curPalette, beatsin88(256*12.*speedFactor + i*256, 0, 255), 255));
  }

  if (_dir) 
    dir = _dir - 1;
  else dir = count%8;
  if (dir == 0) randomSeed(millis());
  EffectMath::blur2d(fb, 16);
  return true;
}

void EffectStarShips::draw(float x, float y, CRGB color) {
  EffectMath::drawPixelXYF(x, y, color, fb);
  if (fb->w() > 24 || fb->h() > 24) {
    if (x < fb->w() - 2) EffectMath::drawPixelXYF(x + 1, y, color, fb);
    if (x > 1) EffectMath::drawPixelXYF(x - 1, y, color, fb);
    if (y < fb->h() - 2) EffectMath::drawPixelXYF(x, y + 1, color, fb);
    if (y > 1) EffectMath::drawPixelXYF(x, y - 1, color, fb);
  }
}

// ------------- Эффект "Флаги"
// (c) Stepko + kostyamat
// 17.03.21
// https://editor.soulmatelights.com/gallery/739-flags
EffectFlags::EffectFlags(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){
  switcher = new Task();
  switcher->set(CHANGE_FLAG_TIME * TASK_SECOND, TASK_FOREVER, [this](){ flag = random(total_flags); });
  ts.addTask(*switcher);
}

void EffectFlags::setControl(size_t idx, int32_t value){
  if (_val->getId()==1)
    _speed = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 1, 16);
  else if (_val->getId()==3) flag = EffectCalc::setDynCtrl(_val).toInt();
  else if (_val->getId()==4) EffectCalc::setDynCtrl(_val).toInt() == 1 ? switcher->enableDelayed() : switcher->disable();     // random flag switch
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

EffectFlags::~EffectFlags(){
  // I can't destory Tasks here due to Scheduler is non-thread-safe, let's delegate it to the scheduler
  // it's not perfiect, but do not have other options for now
  switcher->setCallback(nullptr);
  switcher->setSelfDestruct(true);
  switcher->setIterations(0);
  switcher->enable();           // need this if task was disabled
  switcher = nullptr;
}

bool EffectFlags::run() {
  fb->fade(32);
  for (uint8_t i = 0; i < fb->w(); i++) {
    thisVal = inoise8((float) i * DEVIATOR, counter, (int)count/*(float)i * SPEED_ADJ/2*/);
    thisMax = map(thisVal, 0, 255, 0, fb->maxHeightIndex());
    switch (flag)
    {
    case 0:
      russia(i);
      break;
    case 1:
      germany(i);
      break;
    case 2:
      usa(i);
      break;
    case 3:
      ukraine(i);
      break;
    case 4:
      belarus(i);
      break;
    case 5:
      italy(i);
      break;
    case 6:
      spain(i);
      break;
    case 7:
      uk(i);
      break;
    case 8:
      france(i);
      break;
    case 9:
      poland(i);
      break;
    
    default:
      russia(i);
    }

  }
  EffectMath::blur2d(fb, 32);
  counter += _speed;
  return true;
}



void EffectFlags::germany(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax - fb->h() / 4) ? CHSV(68, 255, thisVal) : (j < thisMax + fb->h() / 4) ? CHSV(0, 255, thisVal)
    : CHSV(0, 0, thisVal / 2.5);
  }
}

//Ukraine
void EffectFlags::ukraine(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax) ? CHSV(50, 255, thisVal) : CHSV(150, 255, thisVal);
  }
}

//Belarus
void EffectFlags::belarus(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax - fb->h() / 4) ? CHSV(0, 224, thisVal) : (j < thisMax + fb->h() / 4) ? CHSV(0, 0, thisVal)
    : CHSV(0, 224, thisVal);
  }
}
//Russia
void EffectFlags::russia(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax - fb->h() / 4) ? CHSV(0, 255, thisVal) : (j < thisMax + fb->h() / 4) ? CHSV(150, 255, thisVal)
    : CHSV(0, 0, thisVal);
  }
}
//Poland
void EffectFlags::poland(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax + 1) ? CHSV(248, 214, (float)thisVal * 0.83) : CHSV(25, 3, (float)thisVal * 0.91);
  }
}

void EffectFlags::usa(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) +=
    ((i <= fb->w() / 2) && (j + thisMax > fb->h() - 1 + fb->h() / 16)) ? 
    ((i % 2 && ((int)j - fb->h() / 16 + thisMax) % 2) ? 
    CHSV(160, 0, thisVal) : CHSV(160, 255, thisVal)) 
    : ((j + 1 + thisMax) % 6 < 3 ? CHSV(0, 0, thisVal) : CHSV(0, 255, thisVal));
  }
}

//Italy
void EffectFlags::italy(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (i < fb->w() / 3) ? CHSV(90, 255, thisVal) : (i < fb->w() - 1 - fb->w() / 3) ? CHSV(0, 0, thisVal)
    : CHSV(0, 255, thisVal);
  }
}

//France
void EffectFlags::france(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (i < fb->w() / 3) ? CHSV(160, 255, thisVal) : (i < fb->w() - 1 - fb->w() / 3) ? CHSV(0, 0, thisVal)
    : CHSV(0, 255, thisVal);
  }
}

//UK
void EffectFlags::uk(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
      fb->at(i, j) += 
      (
          (
              (i > fb->w() / 2 + 1 || i < fb->w() / 2 - 2) && ((i - (int)(j + thisMax - (fb->h() * 2 - fb->w()) / 2) > -2) && (i - (j + thisMax - (fb->h() * 2 - fb->w()) / 2) < 2))
          )
              ||
          (
              (i > fb->w() / 2 + 1 || i < fb->w() / 2 - 2) && ( (((int)fb->w() - 1 - i - ((int)j + thisMax - (int)(fb->h() * 2 - fb->w()) / 2) > -2) && (fb->w() - 1 - i - (int)(j + thisMax - (fb->h() * 2 - fb->w()) / 2) < 2)) )
          )
      || 
      (fb->w() / 2 - i == 0) || (fb->w() / 2 - 1 - i == 0) 
      || 
      ((fb->h() - (j + thisMax)) == 0) || ((fb->h() - 1 - (j + thisMax)) == 0)) ? 
      CHSV(0, 255, thisVal) 
      : 
      (((i - (int)(j + thisMax - (fb->h() * 2 - fb->w()) / 2) > -4) 
      && (i - (j + thisMax - (fb->h() * 2 - fb->w()) / 2) < 4)) 
      || 
      (((int)fb->w() - 1 - i - (int)(j + thisMax - (fb->h() * 2 - fb->w()) / 2) > -4) 
      && (fb->w() - 1 - i - (int)(j + thisMax - (fb->h() * 2 - fb->w()) / 2) < 4)) 
      || (fb->w() / 2 + 1 - i == 0) || (fb->w() / 2 - 2 - i == 0) 
      || (fb->h() + 1 - (j + thisMax) == 0) || (fb->h() - 2 - (int)(j + thisMax) == 0)) ? 
      CHSV(0, 0, thisVal)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            : CHSV(150, 255, thisVal);
  }
}

//Spain
void EffectFlags::spain(uint8_t i){
  for (uint8_t j = 0; j < fb->h(); j++){
    fb->at(i, j) += 
    (j < thisMax - fb->h() / 3) ? 
    CHSV(250, 224, (float)thisVal * 0.68) : (j < thisMax + fb->h() / 3) ? CHSV(64, 255, (float)thisVal * 0.98)
    : CHSV(250, 224, (float)thisVal * 0.68);
  }
}


// ============= Эффект Цветные драже ===============
// (c) SottNick
//по мотивам визуала эффекта by Yaroslaw Turbin 14.12.2020
//https://vk.com/ldirko программный код которого он запретил брать
// !++
void EffectPile::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speed = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==2) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectPile::load() {
  palettesload();    // подгружаем палитры
}

bool EffectPile::run() {
  if (dryrun(2.5))
    return false;
    // если насыпалось уже достаточно, бахаем рандомные песчинки
  uint8_t temp = map8(random(256), _scale, 255U);
  if (pcnt >= map8(temp, 2U, fb->h() - 3U)) {
    //temp = 255U - temp + 2;
    //if (temp < 2) temp = 255;
    temp = fb->h() + 1U - pcnt;
    if (!random(4U)) {// иногда песка осыпается до половины разом
      if (random(2U)) {
        temp = 2U;
      } else {
        temp = 3U;
      }
    }
    for (uint8_t y = 0; y < pcnt; y++)
      for (uint8_t x = 0; x < fb->w(); x++)
        if (!random(temp))
          fb->at(x,y) = 0;
  }

  pcnt = 0U;
  // осыпаем всё, что есть на экране
  for (uint8_t y = 1; y < fb->h(); y++)
    for (uint8_t x = 0; x < fb->w(); x++)
      if (fb->at(x, y))
      { // проверяем для каждой песчинки
        if (!fb->at(x, y - 1))
        { // если под нами пусто, просто падаем
          fb->at(x, y - 1) = fb->at(x, y);
          fb->at(x, y) = 0;
        }
        else if (x > 0U && !fb->at(x - 1, y - 1) && x < fb->w() - 1 && !fb->at(x + 1, y - 1))
        { // если под нами пик
          if (random8(2U))
            fb->at(x - 1, y - 1) = fb->at(x, y);
          else
            fb->at(x - 1, y - 1) = fb->at(x, y);
          fb->at(x, y) = 0;
          pcnt = y - 1;
        }
        else if (x > 0U && !fb->at(x - 1, y - 1))
        { // если под нами склон налево
          fb->at(x - 1, y - 1) = fb->at(x, y);
          fb->at(x, y) = 0;
          pcnt = y - 1;
        }
        else if (x < fb->w() - 1 && !fb->at(x + 1, y - 1))
        { // если под нами склон направо
          fb->at(x + 1, y - 1) = fb->at(x, y);
          fb->at(x, y) = 0;
          pcnt = y - 1;
        }
        else // если под нами плато
          pcnt = y;
      }
  // эмиттер новых песчинок
  if (!fb->at(fb->w()/2, fb->h() - 2) && !fb->at(fb->w()/2 + !!(fb->w()%2), fb->h() - 2) && !random(3))
  {
    temp = random(2) ? fb->w()/2 : fb->w()/2 + !!(fb->w()%2);
    fb->at(temp, fb->h() - 1) = ColorFromPalette(*curPalette, random8());
  }
  return true;
}

// ============= Эффект ДНК ===============
// (c) Stepko
// https://editor.soulmatelights.com/gallery/1520-dna
//по мотивам визуала эффекта by Yaroslaw Turbin
//https://vk.com/ldirko программный код которого он запретил брать
// !++
void EffectDNA::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor  = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 5) * getBaseSpeedFactor();
  else if(_val->getId()==3)  type = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4)  _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) bals = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectDNA::run() {
  fb->fade(32);
  t += speedFactor; // (float)millis()/10;
  if (type == 0) {
    EVERY_N_SECONDS(30) {
      _type ++;
      if (_type > 3)
        _type = 1;
    }
  } else _type = type;

  if (_type == 1 or _type == 4) a = (256.0 / (float)fb->w());
  else a = (256.0 / (float)fb->h());

  for (byte i = 0; i < ((_type == 1) ? fb->h() : fb->w()); i++) {
    uint16_t shift = (i * _scale);
    float sin1 = (1.0 + sin(radians(t + shift))) * 128.0;
    byte brightFront =  constrain(112 * (1 + sin(radians(t + shift + 90))) + 30, 96, 255); 
    byte brightBack =  constrain(112 * (1 + sin(radians(t + shift + 270))) + 30, 96, 255);

    float x = 0, y = 0, x1 = 0, y1 = 0;
    uint8_t width_height;

    switch (_type)
    {
    case 1: // Вертикальная ДНК
      width_height = fb->w();
      x = sin1 /a;
      y = i;
      x1 = (float)(width_height - 1) - (sin1 / a);
      y1 = i;
      break;
    case 2: // Вертикально-горизонтальная
      width_height = fb->h();
      x = sin1 /a;
      y = i;
      y1 = (float)(width_height - 1) - (sin1 / a);
      x1 = i;
      break;
    case 3: // Горизонтальная ДНК
      width_height = fb->h();
      y = sin1 /a;
      x = i;
      y1 = (float)(width_height - 1) - (sin1 / a);
      x1 = i;
      break;
    
    default:
      break;
    }

    if (flag or !bals) {
      EffectMath::drawPixelXYF(x, y, CHSV(sin1, 255, brightFront), fb);
    }
    if (!flag or !bals)
      EffectMath::drawPixelXYF(x1, y1, CHSV(~(byte)sin1, 255, brightBack), fb);
    flag = !flag; 
  }
  EffectMath::blur2d(fb, 64);

  return true;
}

// ----------- Эффект "Дым"
// based on cod by @Stepko (c) 23/12/2021

// !++
void EffectSmoker::setControl(size_t idx, int32_t value) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 3, 20) * getBaseSpeedFactor();
  else if(_val->getId()==2) color = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) saturation = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSmoker::run() {
  t += speedFactor;
  for (byte x = 0; x < fb->w(); x++) {
    for (byte y = 0; y < fb->h(); y++) { 
      uint8_t bri= inoise8(x * beatsin8(glitch, 20, 40), (y * _scale) - t);
      nblend(fb->at(x, y), CHSV(color, saturation, bri), speedFactor);}
  }
  
  EVERY_N_SECONDS(random8(10, 31)) {
    glitch = random(1, 3);
  }
  return true;
}

// ----------- Эффект "Мираж"
// based on cod by @Stepko (c) 23/12/2021


// -------------------- Эффект "Акварель"
// (c) kostyamat 26.12.2021
// https://editor.soulmatelights.com/gallery/1587-oil
// !++
void EffectWcolor::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) {
    speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.1, 0.5);
    blur = 64.f * speedFactor;
    speedFactor *= getBaseSpeedFactor();
  }  else if(_val->getId()==3) {
    blots.assign(  map(EffectCalc::setDynCtrl(_val).toInt(), 1, 8, fb->h()/4, fb->h()), Blot(fb->w()/2));
    load();
  }
  else if(_val->getId()==4) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectWcolor::load() {
  for (auto &i : blots) {
    i.reset(fb->w(), fb->h());
  }
}

bool EffectWcolor::run() {
  fb->fade(blur);
  int cnt{0};
  random16_set_seed(millis());
  for (auto &i : blots){
    i.drawing(fb);
    i.appendXY( mode ? ((float)inoise8(t+= speedFactor, 0, cnt * 100) / 256) - 0.5f : 0, -speedFactor);
    if(i.getY() < -0.1) {
      i.reset(fb->w(), fb->h());
    }
    ++cnt;
  }
  //EffectMath::blur2d(fb->data(), fb->w, fb->h, 32); 
  return true;
}

void EffectWcolor::Blot::reset(int w, int h) {
    x0 = random(-5, w - 5);
    float y0 = EffectMath::randomf(-1, h+1);
    uint8_t dy;
    int cnt{0};
    for (int i{0}; i!=x.size(); ++i) {
        bool f = random(0,2);
        dy = random(0, 2); 
        x[i] = x0 + cnt;
        if (f)
          y[i] = float((cnt ? y[cnt-1] : y0) + dy);
        else 
          y[i] = float((cnt ? y[cnt-1] : y0) - dy);
        ++cnt;
    }
    hue = random(0, 256);
    sat = random(160, 256);
    bri = random(128, 256);
    
}

void EffectWcolor::Blot::drawing(LedFB<CRGB> *fb) {
    for (uint8_t i = 0; i < y.size(); i++) {
        byte bright = constrain(bri / fb->h() * (y[i] + fb->h() - y0), 32, 255);
        if (y[i] > -0.1)
            EffectMath::drawPixelXYF(x[i], y[i], CHSV(hue, sat, bright), fb, 0);
    }
}

float EffectWcolor::Blot::getY() {
    float result = y[0];
    for (uint8_t i = 1; i < y.size(); i++) {
        if (y[i] > result) result = y[i];
    }
    return result;
}
#endif //#if !defined (OBSOLETE_CODE)

// ----------- Эффект "Неопалимая купина"
void EffectRadialFire::setControl(size_t idx, int32_t value){
  switch (idx){
    // 0 speed - raw, expected range 2-20
    // 1 scale - raw, value clamped to 1-width/4
    case 1:
      scale = value;  //clamp(value, 1L, static_cast<int32_t>( fb->w()));
      break;

    // 2 palletes - default

    // 3 - Mode switch boolean
    case 3: {
      _invert = value;
      break;
    }

    // 4 - Radius offset, raw,  0-50
    case 4: {
      _radius = value / 10;
      break;
    }

    default:
      EffectCalc::setControl(idx, value);
  }
}

void EffectRadialFire::load() {
  constexpr float theta = 180 / 2 / PI;
  int offset_x{-fb->w()/2}, offset_y{-fb->h()/2};
  // precompute vectors (this is a very compute-heavy operation)
  for (int y = 0; y < fb->h(); ++y) {
    for (int x = 0; x < fb->w(); ++x) {
      xy_angle.at(x, y) = atan2(y+offset_y, x+offset_x) * theta * fb->maxDim();
      xy_radius.at(x, y) = hypot(x+offset_x, y+offset_y);
    }
  }
  palettesload();
}

bool EffectRadialFire::run() {
  t += speed;
  for (uint16_t y = 0; y < fb->h(); ++y) {
    for (uint16_t x = 0; x < fb->w(); ++x) {
      int32_t radius = _invert ? fb->maxDim() - /* 3 */ _radius - xy_radius.at(x,y) : xy_radius.at(x,y) + _radius;
      auto bri = inoise8(xy_angle.at(x,y), radius * scale - t, x * scale) - radius * (256 /fb->maxDim());
      auto col = bri;
      if (bri)
        bri = 256 - bri / 5;

      nblend(fb->at(x, y), ColorFromPalette(*curPalette, col, bri), speed);
    }
  }
  return true;
}

#if !defined (OBSOLETE_CODE)
void EffectSplashBals::setControl(size_t idx, int32_t value){
  if(_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    speedFactor = EffectMath::fmap(speed, 1, 255, 1, 3) * getBaseSpeedFactor();
  } else if(_val->getId()==3) {count = EffectCalc::setDynCtrl(_val).toInt();
  } /* else if(_val->getId()==5) mode = EffectCalc::setDynCtrl(_val).toInt();*/
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectSplashBals::load() {
  for (auto &b : balls){
    b.iniX1 = random(0, 8);
    b.iniY1 = random(1, 9);
    b.iniX2 = random(0, 8);
    b.iniY2 = random(1, 9);
  }
  palettesload();
}

bool EffectSplashBals::run() {
  fb->fade(100);
  hue++;

  for (auto &b : balls){
    b.x1 = (float)beatsin88(((10UL + b.iniX1) * 256) * speedFactor, 0, (fb->w() - 1) * dev) / dev;
    b.y1 = (float)beatsin88(((10UL + b.iniY1) * 256) * speedFactor, 0, (fb->h() - 1) * dev) / dev;
    b.x2 = (float)beatsin88(((10UL + b.iniX2) * 256) * speedFactor, 0, (fb->w() - 1) * dev) / dev;
    b.y2 = (float)beatsin88(((10UL + b.iniY2) * 256) * speedFactor, 0, (fb->h() - 1) * dev) / dev;
      float a = dist(b.x1, b.y1, b.x2, b.y2);
      if (a <= float(min(fb->w(), fb->h()) / 2)) {
        EffectMath::drawLineF(b.x1, b.y1, b.x2, b.y2, CHSV(0, 0, EffectMath::fmap(a, std::min(fb->w(), fb->h()), 0, 48, 255)), fb);
      }
    EffectMath::fill_circleF(b.x1, b.y1, EffectMath::fmap(fabs(float(fb->w() / 2) - b.x1), 0, fb->w() / 2, R, 0.2), ColorFromPalette(*curPalette, 256 - 256/fb->h() * fabs(float(fb->h()/2) - b.y1)), fb);
  }
  EffectMath::blur2d(fb, 48);
  return true;
}

float EffectSplashBals::dist(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  int a = y2 - y1;
  int b = x2 - x1;
  a *= a;
  b *= b;
  a += b;
  return EffectMath::sqrt(a);
};

// Effect Mira
void EffectMira::load(){
    fb->clear();
    // 0,0
    fb->at(0,0) = fb->at(1,0) = fb->at(2,0) = fb->at(3,0) = fb->at(0,1) = CRGB::Red;

    // w,0
    fb->at(fb->w()-3,0) = fb->at(fb->w()-2,0) = fb->at(fb->w()-1,0) = fb->at(fb->w()-1,1) = CRGB::Green;

    // 0,h
    fb->at(1,fb->h()-3) = fb->at(1,fb->h()-2) = fb->at(1,fb->h()-1) = CRGB::Red;

    // w,h
    fb->at(fb->w()-2,fb->h()-3) = fb->at(fb->w()-2,fb->h()-2) = fb->at(fb->w()-2,fb->h()-1) = CRGB::Green;
}

bool EffectMira::run(){
  if (!cnt++) return true;
  if (cnt < 10*50) return false;  // первые 10 сек показываем настроечные уголки

  if (cnt %10 == 0){
    //LOG(printf, "%d %d %d %d\n", x, y, fb->w(), fb->h());
    fb->at(x,y) = CRGB::Red;
    if (++x == fb->w()){
      x = 0;
      ++y;
      if (y== fb->h()){
        y = 0;
        fb->clear();
      }
    }
    return true;
  }
  return false;
}

#endif //#if !defined (OBSOLETE_CODE)
