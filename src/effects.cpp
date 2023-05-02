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

#include "lamp.h"
#include "patterns.h"
#include "effects.h"
#include "log.h"   // LOG macro

// непустой дефолтный деструктор (если понадобится)
// EffectCalc::~EffectCalc(){LOG(println, "Effect object destroyed");}

// ------------- Эффект "Конфетти" --------------
bool EffectSparcles::run(){
  if (dryrun(3.0))
    return false;
  return sparklesRoutine();
}

// !--
String EffectSparcles::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) eff = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSparcles::sparklesRoutine()
{

#ifdef MIC_EFFECTS
  uint8_t mic = getMicMapMaxPeak();
  uint8_t mic_f = map(getMicMapFreq(), LOW_FREQ_MAP_VAL, HI_FREQ_MAP_VAL, 0, 255);
  if (isMicOn() && eff > 5)
    fb.fade(255 - mic);

  fb.fade(isMicOn() ? map(scale, 1, 255, 100, 1) : map(scale, 1, 255, 50, 1));
#else
  fb.fade(map(scale, 1, 255, 1, 50));
#endif

  CHSV currentHSV;

  for (uint8_t i = 0; i < (uint8_t)round(2.5 * (speed / 255.0) + 1); i++) {
    uint8_t x = random8(0U, WIDTH);
    uint8_t y = random8(0U, HEIGHT);
#ifdef MIC_EFFECTS
    switch (eff)
  {
  case 1 :
  case 6 :
    mic_f = getMicMapFreq();
    break;
  case 2 :
  case 7 :
    mic_f = map(getMicMapFreq(), LOW_FREQ_MAP_VAL, HI_FREQ_MAP_VAL, 0, 255);
    break;
  case 3 :
  case 8 :
    mic_f = sin8(getMicMapFreq());
    break;
  case 4 :
  case 9 :
    mic_f = cos8(getMicMapFreq());
    break;
  case 5 :
  case 10 :
    mic_f = random8(255U);
    break;
  }

#endif
    if (fb.pixel(x, y)) {
#ifdef MIC_EFFECTS
      if (isMicOn()) {
        currentHSV = CHSV(mic_f, 255U - getMicMapMaxPeak()/3, constrain(mic * 1.25f, 48, 255));
      }
      else
        currentHSV = CHSV(random8(1U, 255U), random8(192U, 255U), random8(192U, 255U));
#else
        currentHSV = CHSV(random8(1U, 255U), random8(192U, 255U), random8(192U, 255U));
#endif
      fb.pixel(x, y) = currentHSV;
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
String EffectWhiteColorStripe::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) shift = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWhiteColorStripe::whiteColorStripeRoutine()
{
  FastLED.clear();

#ifdef MIC_EFFECTS
  byte _scale = isMicOn() ? (256.0/getMicMapMaxPeak()+0.3)*scale : scale;
  byte _speed = isMicOn() ? (256.0/getMicMapFreq()+0.3)*speed : speed;
#else
  byte _scale = scale;
  byte _speed = speed;
#endif
    if(_scale < 126){
        uint8_t centerY = fb.cfg.maxHeightIndex() / 2U;
        for (int16_t y = centerY; y >= 0; y--)
        {
          int br = (MAX_BRIGHTNESS)-constrain(map(_scale,126,1,1,15)*(centerY-y)*((centerY-y)/(HEIGHT*(0.0005*brightness))),1,MAX_BRIGHTNESS); if(br<0) br=0;
          int _shift = isMicOn() ? 0 : map(shift,1,255,-centerY,centerY);

            CRGB color = CHSV(
              45U,                                                        // определяем тон
              map(_speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
              (y == centerY ? (MAX_BRIGHTNESS) : br));                        // определяем яркость для центральной вертикальной полосы (или двух) яркость всегда равна MAX_BRIGHTNESS
                                                                          // для остальных вертикальных полос яркость равна либо MAX_BRIGHTNESS, либо вычисляется по br

          for (int16_t x = 0U; x < (int16_t)WIDTH; x++)
          {
            fb.pixel(x, y + _shift) = color;                // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
            fb.pixel(x, (HEIGHT - y + _shift) - 1) = color; // при нечётной - одна, но дважды
          }
        }
    } else if(_scale > 128){
        uint8_t centerX = fb.cfg.maxWidthIndex() / 2U;
        for (int16_t y = 0U; y < (int16_t)HEIGHT; y++){
          for (int16_t x = centerX; x >= 0; x--){
            int br = (MAX_BRIGHTNESS)-constrain(map(_scale,128,255,1,15)*(centerX-x)*((centerX-x)/(WIDTH*(0.0005*brightness))),1,MAX_BRIGHTNESS); if(br<0) br=0;
            int _shift = isMicOn() ? 0 : map(shift,1,255,-centerX,centerX);

            CRGB color = CHSV(
              45U,                                                        // определяем тон
              map(_speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
              (x == centerX ? (MAX_BRIGHTNESS) : br));                        // определяем яркость для центральной вертикальной полосы (или двух) яркость всегда равна MAX_BRIGHTNESS
                                                                          // для остальных вертикальных полос яркость равна либо MAX_BRIGHTNESS, либо вычисляется по br

            fb.pixel(x + _shift, y) = color;                // при чётной ширине матрицы максимально яркими отрисуются 2 центральных вертикальных полосы
            fb.pixel((WIDTH - x + _shift) - 1, y) = color;  // при нечётной - одна, но дважды
          }
        }
    }
    else {
        for (int16_t y = 0; y < (int16_t)HEIGHT; y++){
          for (int16_t x = 0; x < (int16_t)WIDTH; x++){
            CRGB color = CHSV(
              45U,                                                       // определяем тон
              (brightness>=0 ? map(_speed, 0U, 255U, 0U, 170U) : 0),     // определяем насыщенность
              (MAX_BRIGHTNESS));
            fb.pixel(x, y) = color;                        // 127 - заливка полная
          }
        }
    }
  return true;
}

// ========== Эффект "Эффектопад"
// совместное творчество юзеров форума https://community.alexgyver.ru/
void EffectEverythingFall::load(){
    palettesload();    // подгружаем дефолтные палитры
}

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKINGNEW 80U // 50 // 30 // 120 // 90 // 60
bool EffectEverythingFall::run(){
  if (dryrun(4.0))
    return false;

  uint8_t coolingnew = map (scale, 1, 255, 93, 10);

  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Cool down every cell a little
    for (uint8_t i = 0; i < HEIGHT; i++) {
      heat[x][i] = qsub8(heat[x][i], random(0, coolingnew));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (unsigned int k = fb.cfg.maxHeightIndex(); k >= 2; k--) {
      heat[x][k] = (heat[x][k - 1] + heat[x][k - 2] + heat[x][k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKINGNEW) {
      int y = random(2);
      heat[x][y] = qadd8(heat[x][y], random(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t j = 0; j < HEIGHT; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8(heat[x][(uint8_t)j], 240);
      nblend(fb.pixel(x, fb.cfg.maxHeightIndex() - j), ColorFromPalette(*curPalette, colorindex /*, heat[x][j]*/), 50);
    }
  }
  return true;
}

// --------------------------- эффект пульс ----------------------
// Stefan Petrick's PULSE Effect mod by PalPalych for GyverLamp

// !++
String EffectPulse::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.05, 1.0) * EffectCalc::speedfactor;
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
#ifdef MIC_EFFECTS
  #define FADE 255U - (isMicOn() ? getMicMapMaxPeak()*2 : 248U) // (isMicOn() ? 300U - getMicMapMaxPeak() : 5U)
  #define BLUR (isMicOn() ? getMicMapMaxPeak()/3 : 10U) //(isMicOn() ? map(getMicMapMaxPeak(), 1, 255, 1, 30) : 10U)
#else
  #define FADE 1U
  #define BLUR 10U
#endif

  //fb.fade(FADE);
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
    fb.fade(FADE);
    centerX = random8(WIDTH - 5U) + 3U;
    centerY = random8(HEIGHT - 5U) + 3U;
    _pulse_hueall += _pulse_delta;
    pulse_hue = random8(0U, 255U);
    currentRadius = random8(3U, 9U);
    pulse_step = 0;
  }
  pulse_step+=speedFactor;
  EffectMath::blur2d(fb, BLUR);
  return true;
}

// радуги 2D
// ------------- радуга вертикальная/горизонтальная ----------------
bool EffectRainbow::run(){
  // коэф. влияния замаплен на скорость, 4 ползунок нафиг не нужен
  hue += (6.0 * (speed / 255.0) + 0.05 ); // скорость смещения цвета зависит от кривизны наклна линии, коэф. 6.0 и 0.05
#ifdef MIC_EFFECTS
    micCoef = (getMicMapMaxPeak() > map(speed, 1, 255, 100, 10) and isMicOn() ? getMicMapMaxPeak() : 100.0)/100.0;
    twirlFactor = EffectMath::fmap((float)scale, 85, 170, 8.3, 24);      // на сколько оборотов будет закручена матрица, [0..3]
    twirlFactor *= getMicMapMaxPeak() > map(speed, 1, 255, 80, 10) and isMicOn() ? 1.5f * ((float)getMicMapFreq() / 255.0f) : 1.0f;
#else
    twirlFactor = EffectMath::fmap((float)scale, 85, 170, 8.3, 24);      // на сколько оборотов будет закручена матрица, [0..3]
    micCoef = 1.0;
#endif
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
  for (uint8_t i = 0U; i < (isVertical?WIDTH:HEIGHT); i++)
  {
    for (uint8_t j = 0U; j < (isVertical?HEIGHT:WIDTH); j++)
    {
      CHSV thisColor = CHSV(((hue + i * scale) * micCoef), 255, 255);
      fb.pixel((isVertical?i:j), (isVertical?j:i)) = thisColor;
    }
  }
  return true;
}

// ------------- радуга диагональная -------------
bool EffectRainbow::rainbowDiagonalRoutine()
{
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      CRGB thisColor = CHSV((uint8_t)(hue + ((float)WIDTH / (float)HEIGHT * i + j * twirlFactor) * ((float)255 / (float)fb.cfg.maxDim())), 255, 255);
      fb.pixel(i, j) = thisColor;
    }
  }
  return true;
}

// ------------- цвета -----------------
void EffectColors::load(){
    fb.fill(CHSV(scale, 255U, 55U));
}

bool EffectColors::run(){
  return colorsRoutine();
}

// void EffectColors::setscl(const byte _scl){
//   EffectCalc::setscl(_scl);
//   modeColor = scale;
// }

// !--
String EffectColors::setDynCtrl(UIControl*_val){
  if(_val->getId()==2) modeColor = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectColors::colorsRoutine()
{
  static unsigned int step = 0; // доп. задержка
  unsigned int delay = (speed==1)?4294967294:255-speed+1; // на скорости 1 будет очень долгое ожидание)))

  ihue = (speed==1)?scale:ihue;
  step=(step+1)%(delay+1);

  if(step!=delay) {

#ifdef MIC_EFFECTS
  uint16_t mmf = getMicMapFreq();
  uint16_t mmp = getMicMapMaxPeak();

#if defined(LAMP_DEBUG) && defined(MIC_EFFECTS)
EVERY_N_SECONDS(1){
  LOG(printf_P,PSTR("MF: %5.2f MMF: %d MMP: %d scale %d speed: %d\n"), getMicFreq(), mmf, mmp, scale, speed);
}
#endif
  if(isMicOn()){
    // включен микрофон
    if(scale>=127){
      uint8_t pos = (round(3.0*(mmf+(25.0*speed/255.0))/255.0))*HEIGHT/8; // двигаем частоты по диапазону в зависимости от скорости и делим на 4 части 0...3
      for(uint8_t y=pos;y<pos+HEIGHT/8;y++){
        for(uint8_t x=0; x<WIDTH; x++){
          fb.pixel(x, y) = CHSV(mmf / 1.5, 255U, constrain(mmp * (2.0 * (scale >> 1) / 127.0 + 0.33), 1, 255));
          fb.pixel(x, fb.cfg.maxHeightIndex() - y) = CHSV(mmf / 1.5, 255U, constrain(mmp * (2.0 * (scale >> 1) / 127.0 + 0.33), 1, 255));
        }
      }
      fb.dim(254); // плавно гасим
    } else {
      if(mmp>scale) // если амплитуда превышает масштаб
        fb.fill(CHSV(constrain(mmf*(2.0*speed/255.0),1,255), 255U, constrain(mmp*(2.0*scale/127.0+1.5),1,255))); // превышает минимаьный уровень громкости, значит выводим текущую частоту
      else
        fb.dim(252); // плавно гасим
    }
  } else {
    // выключен микрофон
    switch(mode){
      case 1:
        if(!step){
          if(!modeColor){
            modeColor = scale;
            fb.fill(CHSV(modeColor, 255U, 255U));
          }
          else {
            modeColor = 0;
            FastLED.clear();
          }          
        }
        break;
      case 2:
        fb.fill(CHSV(ihue, 255U, 255U));
        break;
      case 3:
        if(!step){
          if(scale<=127)
            modeColor = ~modeColor;
          else
            modeColor = modeColor ? 0 : map(scale,128,255,1,255);
          fb.fill(CHSV(modeColor, 255U, 255U));
        }
        break;
      default:
        fb.fill(CHSV(ihue, 255U, 255U));      
    }
  }
#else
  fb.fill(CHSV(ihue, 255U, 255U));
#endif
  } else {
    ihue += scale; // смещаемся на следущий
  }
  return true;
}

// ------------- Эффект "New Матрица" ---------------
// !--
String EffectMatrix::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==1) _speed = EffectMath::fmap((float)EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.06, 0.4)*EffectCalc::speedfactor;
  else if(_val->getId()==3){
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    lighters.assign( map(_scale, 1, 32, 1, fb.cfg.w()), Lighter() );
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
    i.posX = random(0, fb.cfg.w());
    i.posY = EffectMath::randomf(fb.cfg.h() - fb.cfg.h() /2, fb.cfg.h());
    i.spdX = 1;
    i.spdY = (float)random(10, 20) / 10.0f;
    i.color = hue;
    i.light = random(196,255);
  }
}

bool EffectMatrix::matrixRoutine()
{
  
  fb.dim(map(speed, 1, 255, 252, 240));
  
  CHSV color;

  for (auto &i : lighters){
    i.posY -= i.spdY * _speed;

    if (white) {
      color = rgb2hsv_approximate(CRGB::Gray);
      color.val = i.light;
    } else if (randColor) {
      EVERY_N_MILLIS(600 * speedfactor / _speed) {
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
      i.posX = random(0, fb.cfg.maxWidthIndex());
      i.posY = EffectMath::randomf(fb.cfg.h() - fb.cfg.h() /2, fb.cfg.h());
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
    i.posX = random(-fb.cfg.w(), fb.cfg.w());
    i.posY = random(fb.cfg.maxHeightIndex(), fb.cfg.h() + 4);
    i.spdX = EffectMath::randomf(-1, 1);  // X
    i.spdY = EffectMath::randomf(1, 2);   // Y
    i.color = random(0U, 255U);
    i.light = 255;
  }
}

// !++
String EffectStarFall::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) _speed = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.25, .5)*EffectCalc::speedfactor;
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
  fb.dim(255 - (effId == 2 ? 70 : 60) * _speed);
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
        i.posX = random(-fb.cfg.w(), fb.cfg.w());
        i.posY = effId > 1 ? random(fb.cfg.h() / 2, fb.cfg.h() + 4) : random(fb.cfg.maxHeightIndex(), fb.cfg.h() + 4);
        i.spdX = EffectMath::randomf(-1, 1);  // X
        i.spdY = EffectMath::randomf(1, 2);   // Y
      } else {
        i.posX = (float)random(-(fb.cfg.w() * 10 - 2), (fb.cfg.w() * 10 - 2)) / 10.0f;
        i.posY = random(fb.cfg.h(), fb.cfg.h() + 4);
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
    i.posX = static_cast<float>(random(0, fb.cfg.w()));
    i.posY = static_cast<float>(random(0, fb.cfg.h()));

    i.spdX = random(-fb.cfg.w()/2, fb.cfg.w()/2);
    i.spdY = random(-fb.cfg.h()/2, fb.cfg.h()/2);

    i.color = random(0U, 255U);
    i.light = random(1U, 3U)*127U;
  }
}

// !++
String EffectLighters::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 4096.0f + 0.005f)*EffectCalc::speedfactor;
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
  fb.fade(50);

  EVERY_N_MILLIS(333)   // randomize lighters motion
  {
    for (auto &i : lighters){
      i.spdX += random(-fb.cfg.w()/4, fb.cfg.w()/4) + random(0, 10)/10.0;
      i.spdY += random(-fb.cfg.h()/4, fb.cfg.h()/4) + random(0, 10)/10.0;

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

    if (i.posX < 0) i.posX = (float)fb.cfg.maxWidthIndex();
    if (i.posX > fb.cfg.maxWidthIndex()) i.posX = 0.0f;

    if (i.posY < 0.0f){
      i.posY = fb.cfg.maxHeightIndex();
      i.spdY = -i.spdY/10.0;
      i.spdX = -i.spdX/10.0;
    }
    if (i.posY > fb.cfg.maxHeightIndex()){
      i.posY = 0;
      i.spdY = -i.spdY/10.0;
      i.spdX = -i.spdX/10.0;
    }
    //LOG(printf, "Draw P0:%3.2f P1:%3.2f S0:%3.2f S1:%3.2f\n", lightersPos[0U][i], lightersPos[1U][i], lightersSpeed[0U][i], lightersSpeed[1U][i]);

    if (subPix)
      EffectMath::drawPixelXYF(i.posX, i.posY, CHSV(i.color, 255U-(++c*2), i.light), fb, 0);
    else
      fb.pixel(static_cast<int>(i.posX), static_cast<int>(i.posY)) = CHSV(i.color, 255U-(++c*2), i.light);
  }
  return true;
}

// ------------- светлячки со шлейфом -------------
// !++
String EffectLighterTracers::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.01, .1)*EffectCalc::speedfactor;
  else if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectLighterTracers::load(){
  for (uint8_t j = 0U; j < _AMOUNT; j++)
  {
    int8_t sign;
    // забиваем случайными данными
    coord[j][0U] = (float)WIDTH / 2.0f;
    random(0, 2) ? sign = 1 : sign = -1;
    vector[j][0U] = ((float)random(40, 150) / 10.0f) * sign;
    coord[j][1U] = (float)HEIGHT / 2;
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

  fb.fade(map(speed, 1, 255, 6, 55)); // размер шлейфа должен сохранять размер, не зависимо от скорости

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

    if ((uint16_t)coord[j][0U] > fb.cfg.maxWidthIndex())
    {
      coord[j][0U] = fb.cfg.maxWidthIndex();
      vector[j][0U] = -vector[j][0U];
    }
    if ((uint16_t)coord[j][1U] > fb.cfg.maxHeightIndex())
    {
      coord[j][1U] = fb.cfg.maxHeightIndex();
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
String EffectLightBalls::setDynCtrl(UIControl*_val){
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

  fb.pixel( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 29, 200U, 255U);
  fb.pixel( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 41, 200U, 255U);
  fb.pixel( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 37, 200U, 255U);
  fb.pixel( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS) += CHSV( ms / 53, 200U, 255U);

  return true;
}

// ------------- эффект "блуждающий кубик" -------------
// !++
String EffectBall::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) {
    speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.02, 0.15) * EffectCalc::speedfactor;
  }
  else if(_val->getId()==2) {
    EffectCalc::setDynCtrl(_val).toInt();
    if (scale <= 85)
      ballSize = map(scale, 1, 85, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
    else if (scale > 85 and scale <= 170)
      ballSize = map(scale, 170, 86, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
    else
      ballSize = map(scale, 171, 255, 1U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 1));
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectBall::load() {
  palettesload();
  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] = i? float(WIDTH - ballSize) / 2 : float(HEIGHT - ballSize) / 2;
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
  if ((int8_t)coordB[0U] > (int8_t)(WIDTH - ballSize))
  {
    coordB[0U] = (WIDTH - ballSize);
    vectorB[0U] = -vectorB[0U];
    ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255));
  }
  if ((int8_t)coordB[1U] > (int8_t)(HEIGHT - ballSize))
  {
    coordB[1U] = (HEIGHT - ballSize);
    vectorB[1U] = -vectorB[1U];
    ballColor = ColorFromPalette(*curPalette, random(1, 250), random(200, 255));
  }

  if (scale <= 85)  // при масштабе до 85 выводим кубик без шлейфа
    fb.clear();
  else if (scale > 85 and scale <= 170)
    fb.fade(255 - map(speed, 1, 255, 245, 200)); // выводим кубик со шлейфом, длинна которого зависит от скорости.
  else
    fb.fade(255 - map(speed, 1, 255, 253, 248)); // выводим кубик с длинным шлейфом, длинна которого зависит от скорости.

  for (uint8_t i = 0; i < ballSize; i++)
  {
    for (uint8_t j = 0; j < ballSize; j++)
    {
      EffectMath::drawPixelXYF(coordB[0U] + (float)i, coordB[1U] + (float)j, ballColor, fb, 0);
    }
  }
  return true;
}

// ----------- Эффекты "Лава, Зебра, etc"
void Effect3DNoise::fillNoiseLED()
{
  uint8_t dataSmoothing = 0;
  if (speed < 50)
  {
    dataSmoothing = 200 - (speed * 4);
  }
  for (uint8_t i = 0; i < fb.cfg.minDim()*2; i++)
  {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < fb.cfg.maxDim(); j++)
    {
      int32_t joffset = _scale * j;

      uint8_t data = inoise8(x + ioffset, y + joffset, z);

      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (dataSmoothing)
      {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }

      noise[i][j] = data;
    }
  }
  z += _speed;

  // apply slow drift to X and Y, just for visual variation.
  x += _speed * 0.125; // 1/8
  y -= _speed * 0.0625; // 1/16

  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      uint8_t index = noise[j%(fb.cfg.minDim()*2)][i];
      uint8_t bri =   noise[i%(fb.cfg.minDim()*2)][j];
      // if this palette is a 'loop', add a slowly-changing base value
      if ( colorLoop)
      {
        index += ihue;
      }
      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( bri > 127 && blurIm)
      {
        bri = 255;
      }
      else
      {
        bri = dim8_raw( bri * 2);
      }
      CRGB color = ColorFromPalette( *curPalette, index, bri);

      fb.pixel(i, j) = color;
    }
  }
  ihue += 1;
}

void Effect3DNoise::fillnoise8()
{
  for (uint8_t i = 0; i < fb.cfg.minDim()*2; i++)
  {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < fb.cfg.maxDim(); j++)
    {
      int32_t joffset = _scale * j;
      noise[i][j] = inoise8(x + ioffset, y + joffset, z);
    }
  }
  z += _speed;
}

void Effect3DNoise::load(){
  palettesload();
  fillnoise8();
}

// !++
String Effect3DNoise::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3 && _val->getVal().toInt()==0 && !isRandDemo())
    curPalette = &ZeebraColors_p;
  else if(_val->getId()==4) blurIm = EffectCalc::setDynCtrl(_val).toInt();
   else if(_val->getId()==5) colorLoop = EffectCalc::setDynCtrl(_val).toInt();
   else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  fillnoise8();
  return String();
}

bool Effect3DNoise::run(){
  #ifdef MIC_EFFECTS
    uint8_t mmf = isMicOn() ? getMicMapFreq() : 0;
    uint8_t mmp = isMicOn() ? getMicMapMaxPeak() : 0;
    _scale = (NOISE_SCALE_AMP*(float)scale/255.0+NOISE_SCALE_ADD)*(mmf>0?(1.5*mmf/255.0):1);
    _speed = NOISE_SCALE_AMP*(float)speed/512.0*(mmf<LOW_FREQ_MAP_VAL && mmp>MIN_PEAK_LEVEL?10:2.5*mmp/255.0+1);
  #else
    _scale = NOISE_SCALE_AMP*scale/255.0+NOISE_SCALE_ADD;
    _speed = NOISE_SCALE_AMP*speed/512.0;
  #endif

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
  FastLED.clear();
  if (_scale <= 16) {
    bballsNUM_BALLS =  map(_scale, 1, 16, 1, bballsMaxNUM_BALLS);
  } else {
    bballsNUM_BALLS =  map(_scale, 32, 17, 1, bballsMaxNUM_BALLS);
  }
  balls.assign(bballsNUM_BALLS, Ball());

  randomSeed(millis());
  for (size_t i = 0; i != balls.size(); ++i){
    balls[i].color = random(0, 255);
    balls[i].x = (i+1) * WIDTH / bballsNUM_BALLS;
    balls[i].vimpact = bballsVImpact0 + EffectMath::randomf( - 2., 2.);                   // And "pop" up at vImpact0
    balls[i].cor = 0.9 - float(i) / pow(bballsNUM_BALLS, 2);
    if (halo){
      balls[i].brightness = 200;
    } else if ( i && balls[i].x == balls[i-1].x){      // skip 1st interation
      balls[i].brightness = balls[i-1].brightness + 32;
    }
  }
}

// !++ (разобраться отдельно)
String EffectBBalls::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) _speed = (1550 - EffectCalc::setDynCtrl(_val).toInt() * 3);
  else if(_val->getId()==3) { _scale = EffectCalc::setDynCtrl(_val).toInt(); load(); }
  else if(_val->getId()==4) halo = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectBBalls::bBallsRoutine()
{
  fb.fade(_scale <= 16 ? 255 : 50);
  hue += (float)speed/ 1024;
  for (auto &bball : balls){
    bballsTCycle =  millis() - bball.tlast;     // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    bballsHi = 0.55 * bballsGRAVITY * pow( (float)bballsTCycle / _speed , 2) + bball.vimpact * (float)bballsTCycle / _speed;

    if ( bballsHi < 0 ) {
      bball.tlast = millis();
      bballsHi = 0.0f;                            // If the ball crossed the threshold of the "ground," put it back on the ground
      bball.vimpact = bball.cor * bball.vimpact ;   // and recalculate its new upward velocity as it's old velocity * COR

      //if ( bball.vimpact < 0.01 ) bball.vimpact = bballsVImpact0;  // If the ball is barely moving, "pop" it back up at vImpact0
      if ( bball.vimpact < 0.1 ) // сделал, чтобы мячики меняли свою прыгучесть и положение каждый цикл
      {
        bball.cor = 0.90 - (EffectMath::randomf(0., 9.)) / pow(EffectMath::randomf(4., 9.), 2.); // сделал, чтобы мячики меняли свою прыгучесть каждый цикл
        bball.shift = bball.cor >= 0.85;                             // если мячик максимальной прыгучести, то разрешаем ему сдвинуться
        bball.vimpact = bballsVImpact0;
      }
    }

    bball.pos = bballsHi * (float)fb.cfg.maxHeightIndex() / bballsH0;       // Map "h" to a "pos" integer index position on the LED strip

    if (bball.shift > 0.0f && bball.pos >= (float)fb.cfg.maxHeightIndex() - .5) {                  // если мячик получил право, то пускай сдвинется на максимальной высоте 1 раз
      bball.shift = 0.0f;
      if (bball.color % 2 == 0) {                                       // чётные налево, нечётные направо
        if (bball.x < 0) bball.x = (fb.cfg.maxWidthIndex());
        else bball.x -= 1;
      } else {
        if (bball.x > fb.cfg.maxWidthIndex()) bball.x = 0;
        else bball.x += 1;
      }
    }
  }

  // Adjust balls brightness
  for (unsigned i = 0; i != balls.size(); ++i){
    if (halo){ // если ореол включен
      EffectMath::drawCircleF(balls[i].x, balls[i].pos + 2.75, 3., CHSV(balls[i].color + (byte)hue, 225, balls[i].brightness), fb);
    } else {
      if (!i){
        balls[i].brightness = 156;
        continue;    // skip first iteration
      } 
      // попытка создать объем с помощью яркости. Идея в том, что шарик на переднем фоне должен быть ярче, чем другой,
      // который движится в том же Х. И каждый следующий ярче предыдущего.
      balls[i].brightness = balls[i].x == balls[i-1].x ? balls[i].brightness + 32 : 156;
      EffectMath::drawPixelXYF_Y(balls[i].x, balls[i].pos, CHSV(balls[i].color + (byte)hue, 255, balls[i].brightness), fb, 5);
    }
  }

  return true;
}

// ***** SINUSOID3 / СИНУСОИД3 *****
/*
  Sinusoid3 by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  read more about the concept: https://www.youtube.com/watch?v=mubH-w_gwdA
*/
// !++
String EffectSinusoid3::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) e_s3_speed = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.033, 1) * EffectCalc::speedfactor;
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
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
          CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 134.3447 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 170.3884 * time_shift))) / 32767.0;
          v = 127 * (1 + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.b = ~v;
          fb.pixel(x, y) = color;
        }
      }
      break;
    case 1: //it's not sinusoid II
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
		  CRGB color;
          float cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 98.301 * time_shift))) / 32767.0; // the 8 centers the middle on a 16x16
          float cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 72.0874 * time_shift))) / 32767.0;
          int8_t v = 127 * (float(0.001 * time_shift * e_s3_speed) + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.r = ~v;
          
          cx = (y - semiHeightMajor) + float(e_s3_size * (sin16(e_s3_speed * 68.8107 * time_shift))) / 32767.0;
          cy = (x - semiWidthMajor) + float(e_s3_size * (cos16(e_s3_speed * 65.534 * time_shift))) / 32767.0;
          v = 127 * (((float)(0.001 * time_shift * e_s3_speed)) + sin16(127 * _scale * EffectMath::sqrt((((float) cx * cx) + ((float) cy * cy)))) / 32767.0);
          color.g = ~v;
		  fb.pixel(x, y) = color;
        }
      }
      break;
    case 2: //Sinusoid III
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
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
          fb.pixel(x, y) = color;
        }
      }
      break;
    case 3: //Sinusoid IV
      for (uint8_t y = 0; y < HEIGHT; y++) {
        for (uint8_t x = 0; x < WIDTH; x++) {
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
          fb.pixel(x, y) = color;
        }
      }
      break;
  }
  return true;
}

/*
 ***** METABALLS / МЕТАСФЕРЫ *****
Metaballs proof of concept by Stefan Petrick 
https://gist.github.com/StefanPetrick/170fbf141390fafb9c0c76b8a0d34e54
*/

// !++
String EffectMetaBalls::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 2);
  else if(_val->getId()==2) scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectMetaBalls::load(){
palettesload();}

bool EffectMetaBalls::run()
{
  // get some 3 random moving points
  unsigned long t = millis() * speedFactor;
  // get some 3 random moving points
  uint8_t x1 = beatsin88(23 * 256 * speedFactor, 0, fb.cfg.maxWidthIndex());//V1
  uint8_t y1 = beatsin88(28 * 256 * speedFactor, 0, fb.cfg.maxHeightIndex());

  //uint8_t x1 = inoise8(t, 12355, 85) / hormap;// V2
  //uint8_t y1 = inoise8(t, 5, 685) / vermap;

  uint8_t x2 = inoise8(t, 25355, 685) / hormap;
  uint8_t y2 = inoise8(t, 355, 11685) / vermap;

  uint8_t x3 = inoise8(t, 55355, 6685) / hormap;
  uint8_t y3 = inoise8(t, 25355, 22685) / vermap;

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {

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
        fb.pixel(x, y) = ColorFromPalette(*curPalette, color * 9);
      } else {
        fb.pixel(x, y) = ColorFromPalette(*curPalette, 0);
      }
      // show the 3 points, too
      fb.pixel(x1, y1) = CRGB(255, 255, 255);
      fb.pixel(x2, y2) = CRGB(255, 255, 255);
      fb.pixel(x3, y3) = CRGB(255, 255, 255);
    }
  }
  return true;
}

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
String EffectSpiro::setDynCtrl(UIControl*_val) {
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

  //fb.dim(254U - palettescale);
  //fb.dim(250-speed_factor*7);
  uint8_t dim = beatsin8(16. / speedFactor, 5, 10);
  fb.dim(254 - dim);

  for (int i = 0; i < spirocount; i++) {
    uint8_t  x = EffectMath::mapsincos8(MAP_SIN, spirotheta1 + i * spirooffset, spirominx, spiromaxx);
    uint8_t  y = EffectMath::mapsincos8(MAP_COS, spirotheta1 + i * spirooffset, spirominy, spiromaxy);
    uint8_t x2 = EffectMath::mapsincos8(MAP_SIN, spirotheta2 + i * spirooffset, x - spiroradiusx, x + spiroradiusx);
    uint8_t y2 = EffectMath::mapsincos8(MAP_COS, spirotheta2 + i * spirooffset, y - spiroradiusy, y + spiroradiusy);
    CRGB color = ColorFromPalette(*curPalette, (spirohueoffset + i * spirooffset), 128U);
    fb.pixel(x2, y2) += color;

    if(x2 == spirocenterX && y2 == spirocenterY) change = true; // в центре могут находится некоторое время
  }

  spirotheta2 += speedFactor * 2;
  spirotheta1 += speedFactor;
  spirohueoffset += speedFactor;

  if (change && !spirohandledChange) { // меняем кол-во спиралей
    spirohandledChange = true;

    if (spirocount >= WIDTH || spirocount == 1)
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

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
// ***** Парящий огонь, Кровавые Небеса, Радужный Змей и т.п.
// базис (c) Stefan Petrick
void EffectComet::drawFillRect2_fast(int8_t x1, int8_t y1, int8_t x2, int8_t y2, CRGB color)
{ // Fine if: 0<x1<x2<WIDTH && 0<y1<y2<HEIGHT
  for (int8_t xP = x1; xP <= x2; xP++)
  {
    for (int8_t yP = y1; yP <= y2; yP++)
    {
      fb.pixel(xP, yP) += color;
    }
  }
}

void EffectComet::moveFractionalNoise(bool direction, int8_t amplitude, float shift) {
  int16_t zD, zF;
  uint16_t _side_a = direction ? fb.cfg.h() : fb.cfg.w();
  uint16_t _side_b = direction ? fb.cfg.w() : fb.cfg.h();

  for (auto &i : noise3d.map)
    for (uint16_t a = 0; a < _side_a; a++) {
      uint8_t _pixel = direction ? i.at(noise3d.xy(a,0)) : i.at(noise3d.xy(0,a));
      int16_t amount = ((int16_t)(_pixel - 128) * 2 * amplitude + shift * 256);
      int8_t delta = ((uint16_t)fabs(amount) >> 8) ;
      int8_t fraction = ((uint16_t)fabs(amount) & 255);
      for (uint8_t b = 0 ; b < _side_b; b++) {
        if (amount < 0) {
          zD = b - delta; zF = zD - 1;
        } else {
          zD = b + delta; zF = zD + 1;
        }
        //Serial.printf("zD: %d, zF:%d\n", zD, zF);
        CRGB pixelA(CRGB::Black);
        if ((zD >= 0) && (zD < _side_b))
          pixelA = direction ? fb.pixel(zD%fb.cfg.w(), a%fb.cfg.h()) : fb.pixel(a%fb.cfg.w(), zD%fb.cfg.h());

        CRGB pixelB(CRGB::Black);
        if ((zF >= 0) && (zF < _side_b))
          pixelB = direction ? fb.pixel(zF%fb.cfg.w(), a%fb.cfg.h()) : fb.pixel(a%fb.cfg.w(), zF%fb.cfg.h());
        uint16_t x = direction ? b : a;
        uint16_t y = direction ? a : b;
        fb.pixel(x, y) = pixelA.nscale8(ease8InOutApprox(255 - fraction)) + pixelB.nscale8(ease8InOutApprox(fraction));   // lerp8by8(PixelA, PixelB, fraction );
        //Serial.printf("x:%d, y:%d, r:%u g:%u b:%u\n", x, y, result.pixel(x, y).r, result.pixel(x, y).g, result.pixel(x, y).b);
      }
    }
}

void EffectComet::load() {
  for (auto &i : noise3d.opt){
    i.e_x = random16();
    i.e_y = random16();
    i.e_z = random16();
    i.e_scaleX = 6000;
    i.e_scaleY = 6000;
  }
}

//!++
String EffectComet::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) _speed = EffectMath::fmap((float)EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.1, 1.0)*EffectCalc::speedfactor;
  else if(_val->getId()==3) {
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    if(_scale==6)
      _speed = EffectMath::fmap(getCtrlVal(1).toInt(), 1., 255., 0.1, .5)*EffectCalc::speedfactor;
    else
      _speed = EffectMath::fmap(getCtrlVal(1).toInt(), 1., 255., 0.1, 1.0)*EffectCalc::speedfactor;  
  }
  else if(_val->getId()==4) colorId = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) smooth = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) blur = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectComet::run(){
  speedy = map(speed, 1, 255, 20, 255);
  
  effId = _scale;
  switch (effId)
  {
  case 1 :
    return rainbowCometRoutine();
    break;
  case 2 :
    return rainbowComet3Routine();
    break;
  case 3 :
    return firelineRoutine();
    break;
  case 4 :
    return fractfireRoutine();
    break;
  case 5 :
    return flsnakeRoutine();
    break;
  case 6 :
    return smokeRoutine();
    break;
  default:
    return false;
  }
}

bool EffectComet::smokeRoutine() {
  // if(isDebug()){
  //   FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  // }
  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  CRGB color;//, color2;
  hsv2rgb_spectrum(CHSV(hue, (colorId == 255) ? 0 : beatsin8(speed, 220, 255, 0, 180), beatsin8(speed / 2, 64, 255)), color);


  spiral += 3. * _speed;
  if (random8(fb.cfg.w()) != 0U) // встречная спираль движется не всегда синхронно основной
    spiral2 += 3. * _speed;

  for (float i = 0; i < fb.cfg.h(); i+= 0.5) {
    float n = (float)quadwave8(i * 4. + spiral) / (256. / (float)fb.cfg.h() + 1.0);
    float n2 = (float)quadwave8(i * 5. + beatsin8((smooth*3) * _speed)) / (256. / (float)fb.cfg.w() + 1.0);

    EffectMath::drawPixelXYF(n, fb.cfg.maxHeightIndex() - i, color, fb, 0);
    EffectMath::drawPixelXYF(fb.cfg.maxWidthIndex() - n2, fb.cfg.maxHeightIndex() - i, color, fb, 0);
  }
  
  // скорость движения по массиву noise
  // if(!isDebug()){
    noise3d.opt[0].e_x += 1000 * _speed; //1000;
    noise3d.opt[0].e_y += 1000 * _speed; //1000;
    noise3d.opt[0].e_z += 1000 * _speed; //1000;
    noise3d.opt[0].e_scaleX = 2000 * (blur/5);//12000;
    noise3d.opt[0].e_scaleY = 1333 * smooth;

    // fill noise map
    noise3d.fillNoise(eNs_noisesmooth);

    moveFractionalNoise(MOVE_X, fb.cfg.w() / (getCtrlVal(3).toInt() + 2));//4
    moveFractionalNoise(MOVE_Y, fb.cfg.h() / 8, 0.33);//4

    EffectMath::blur2d(fb,128); // без размытия как-то пиксельно, наверное...  
  // }
  return true;
}

bool EffectComet::firelineRoutine() {
  // if(!isDebug()) 
    fb.fade(map(blur, 1, 64, 20, 5)); 
  // else FastLED.clear();

  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  for (uint8_t i = 1; i < fb.cfg.w(); i += 2) {
    fb.pixel( i, e_centerY) += CHSV(hue + i * 2 , colorId == 255 ? 64 : 255, 255);
  }
  // Noise
  float beat2 = (10.0 -  (float)beatsin88(3 * speedy, 10, 20)) / 10.;
  noise3d.opt[0].e_x += 12 * speedy; // 3000;
  noise3d.opt[0].e_y += 12 * speedy; // 3000;
  noise3d.opt[0].e_z += 12 * speedy; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * smooth; // 8000
  noise3d.opt[0].e_scaleY = 1333 * smooth; // 8000;
  count ++;

  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_Y, 3);
  moveFractionalNoise(MOVE_X, 3, beat2);
  return true;
}

bool EffectComet::fractfireRoutine() {
  // if(!isDebug()) 
    fb.fade(map(blur, 1, 64, 20, 5)); 
  // else FastLED.clear();

  float beat = (float)beatsin88(5 * speedy, 50, 100) / 100 ;
  count ++;
  if (colorId == 1) {
    if (count%2 == 0) hue ++;
  }
  else hue = colorId;

  for (uint8_t i = 1; i < fb.cfg.w(); i += 2) {
    fb.pixel(i, fb.cfg.maxHeightIndex()) += CHSV(hue + i * 2, colorId == 255 ? 64 : 255, 255);
  }
  // Noise
  noise3d.opt[0].e_y += 12 * speedy; // 3000;
  noise3d.opt[0].e_z += 12 * speedy; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * smooth; // 8000;
  noise3d.opt[0].e_scaleY = 1333 * smooth; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_Y, 2, beat);
  moveFractionalNoise(MOVE_X, 3);
  //EffectMath::blur2d(fb, 32); // нужно ли размытие?
  return true;
}

bool EffectComet::flsnakeRoutine() {
  // if(!isDebug()) 
    fb.dim(blur); 
  // else FastLED.clear();
  
  ++count;
  if (colorId == 1 or colorId == 255) {
    if (count%2 == 0) ++hue;
  }
  else hue = colorId;

  for (uint8_t y = 2; y < fb.cfg.maxHeightIndex(); y += 5) {
    for (uint8_t x = 2; x < fb.cfg.maxWidthIndex(); x += 5) {
      fb.pixel(x, y) += CHSV(x * y + hue, colorId == 255 ? 64 : 255, 255);
      fb.pixel(x + 1, y) += CHSV((x + 4) * y + hue, colorId == 255 ? 64 : 255, 255);
      fb.pixel(x, y + 1) += CHSV(x * (y + 4) + hue, colorId == 255 ? 64 : 255, 255);
      fb.pixel(x + 1, y + 1) += CHSV((x + 4) * (y + 4) + hue, colorId == 255 ? 64 : 255, 255);
    }
  }
  // Noise
  noise3d.opt[0].e_x += 12 * speedy; // 3000;
  noise3d.opt[0].e_y += 12 * speedy; // 3000;
  noise3d.opt[0].e_z += 12 * speedy; // 3000;
  noise3d.opt[0].e_scaleX = 1333 * smooth; // 8000
  noise3d.opt[0].e_scaleY = 1333 * smooth; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);

  moveFractionalNoise(MOVE_X, 5);
  moveFractionalNoise(MOVE_Y, 5);
  return true;
}

bool EffectComet::rainbowCometRoutine()
{ // Rainbow Comet by PalPalych
/*
  Follow the Rainbow Comet Efect by PalPalych
  Speed = tail dispersing
  Scale = 0 - Random 3d color
          1...127 - time depending color
          128...254 - selected color
          255 - white
*/

  EffectMath::blur2d(fb,e_com_BLUR);    // < -- размытие хвоста
  if (blur < 64) fb.fade(map(blur, 1, 64, 32, 0));

  // if(isDebug()){
  //   FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  // }
  CRGB _eNs_color;
  if (colorId == 255) {
    _eNs_color= CRGB::White;
  } else if (colorId == 1) {
    _eNs_color = CHSV(noise3d.map_lxy(0,0,0) * e_com_3DCOLORSPEED , 255, 255);
  } else if (colorId >1 && colorId < 128) {
    _eNs_color = CHSV(millis() / ((uint16_t)colorId + 1U) * 4 + 10, 255, 255);
  } else {
    _eNs_color = CHSV((colorId - 128) * 2, 255, 255);
  }

  drawFillRect2_fast(e_centerX, e_centerY, e_centerX + 1, e_centerY + 1, _eNs_color);

  // Noise
  noise3d.opt[0].e_x += 12 * speedy; // 3000;
  noise3d.opt[0].e_y += 12 * speedy; // 3000;
  noise3d.opt[0].e_z += 12 * speedy; // 3000;
  noise3d.opt[0].e_scaleX = 667 * smooth; // 8000
  noise3d.opt[0].e_scaleY = 667 * smooth; // 8000;
  noise3d.fillNoise(eNs_noisesmooth);
  //noise3d.printmap();
  moveFractionalNoise(MOVE_X, fb.cfg.w() / 3U);
  moveFractionalNoise(MOVE_Y, fb.cfg.h() / 3U, 0.5);
  return true;
}

bool EffectComet::rainbowComet3Routine()
{ 
  count++;
  EffectMath::blur2d(fb,e_com_BLUR);    // < -- размытие хвоста
  if (blur < 64) fb.fade(map(blur, 1, 64, 32, 0));

  if (count%2 == 0) hue++;

  // if(isDebug()){
  //   FastLED.clear(); // для отладки чистим матрицу, чтобы показать перемещение точек
  // }

  CHSV color = rgb2hsv_approximate(CRGB::Green);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  EffectMath::drawPixelXYF(fb.cfg.w() / 2, fb.cfg.h() / 2, color, fb, 0); // зеленый стоит по центру

  color = rgb2hsv_approximate(CRGB::Red);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  float xx = 2. + (float)sin8( millis() / (10. / _speed)) / 22.;
  float yy = 2. + (float)cos8( millis() / (9. / _speed)) / 22.;
  EffectMath::drawPixelXYF(xx, yy, color, fb, 0);

  xx = 4. + (float)sin8( millis() / (10. / _speed)) / 32.;
  yy = 4. + (float)sin8( millis() / (7. / _speed)) / 32.;
  color = rgb2hsv_approximate(CRGB::Blue);
  if (colorId == 1) color.hue += hue;
  else if (colorId == 255) color.sat = 64;
  else color.hue += colorId;
  EffectMath::drawPixelXYF(xx, yy, color, fb, 0);

  noise3d.opt[0].e_x += 3000 * _speed;
  noise3d.opt[0].e_y += 3000 * _speed;
  noise3d.opt[0].e_z += 3000 * _speed;
  noise3d.opt[0].e_scaleX = 667 * smooth; // 4000;
  noise3d.opt[0].e_scaleY = 667 * smooth; // 4000;
  noise3d.fillNoise(eNs_noisesmooth);
  moveFractionalNoise(MOVE_X, fb.cfg.w() / 6);
  moveFractionalNoise(MOVE_Y, fb.cfg.h() / 6, 0.33);

  return true;
}

// ============= ЭФФЕКТ ПРИЗМАТА ===============
// Prismata Loading Animation
void EffectPrismata::load(){
  palettesload();
}

bool EffectPrismata::run() {
  EVERY_N_MILLIS(100) {
    spirohueoffset += 1;
  }

  fb.fade(map(fadelvl, 1, 255, 130, 2)); // делаем шлейф

  for (byte x = 0; x < WIDTH; x++) {
      float y = (float)beatsin16((uint8_t)x + speedFactor, 0, fb.cfg.maxHeightIndex()* 10) / 10.0f;
      EffectMath::drawPixelXYF_Y(x, y, ColorFromPalette(*curPalette, ((uint16_t)x + spirohueoffset) * 4), fb);
    }
  return true;
}

// !++
String EffectPrismata::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 2.0);
  else if(_val->getId()==3) fadelvl = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// ============= ЭФФЕКТ СТАЯ ===============
// Адаптация от (c) SottNick
void EffectFlock::load(){
  palettesload();    // подгружаем дефолтные палитры
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++)
  {
    boids[i] = Boid(random8(0,WIDTH), random(0, HEIGHT));
    boids[i].maxspeed = 0.380 * speedFactor + 0.380 / 2;
    boids[i].maxforce = 0.015 * speedFactor + 0.015 / 2;
  }
  predator = Boid(random8(0,WIDTH), random(0, HEIGHT) );
  predator.maxspeed = 0.385 * speedFactor + 0.385 / 2;
  predator.maxforce = 0.020 * speedFactor + 0.020 / 2;
  predator.neighbordist = 8.0;
  predator.desiredseparation = 0.0;
}

// !++
String EffectFlock::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) {
    speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 196.0)*EffectCalc::speedfactor;
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

  fb.fade(map(speed, 1, 255, 220, 10));

  bool applyWind = random(0, 255) > 240;
  if (applyWind) {
    wind.x = Boid::randomf() * .015 * speedFactor + .015 / 2;
    wind.y = Boid::randomf() * .015 * speedFactor + .015 / 2;
  }

  CRGB color = ColorFromPalette(*curPalette, hueoffset, 170);
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {

    Boid * boid = &boids[i];
    if (predatorPresent) {
          // flee from predator
          boid->repelForce(predator.location, 8);
        }
    boid->run(boids, AVAILABLE_BOID_COUNT);
    boid->wrapAroundBorders();
    PVector location = boid->location;
    EffectMath::drawPixelXYF(location.x, location.y, color, fb);
    if (applyWind) {
          boid->applyForce(wind);
          applyWind = false;
        }
  }
  if (predatorPresent) {
    predator.run(boids, AVAILABLE_BOID_COUNT);
    predator.wrapAroundBorders();
    color = ColorFromPalette(*curPalette, hueoffset + 128, 255);
    PVector location = predator.location;
    EffectMath::drawPixelXYF(location.x, location.y, color, fb);
  }

  return true;
}

// ============= SWIRL /  ВОДОВОРОТ ===============
// https://gist.github.com/kriegsman/5adca44e14ad025e6d3b
// Copyright (c) 2014 Mark Kriegsman
void EffectSwirl::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectSwirl::run(){
  return swirlRoutine();
}

#define e_swi_BORDER (1U)  // размытие экрана за активный кадр
bool EffectSwirl::swirlRoutine()
{
  if (curPalette == nullptr) {
    return false;
  }

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
#if (WIDTH < 25)
  byte blurAmount = beatsin8(2, 10, 180);
  EffectMath::blur2d(fb,blurAmount);
#else
  // Never mind, on my 64x96 array, the dots are just too small
   EffectMath::blur2d(fb,172);
#endif

  // Use two out-of-sync sine waves
  uint8_t xi = beatsin8(27 * ((float)speed / 100.0) + 5, e_swi_BORDER, WIDTH - e_swi_BORDER); // borderWidth
  uint8_t xj = beatsin8(41 * ((float)speed / 100.0) + 5, e_swi_BORDER, WIDTH - e_swi_BORDER);
  uint8_t yi = beatsin8(27 * ((float)speed / 100.0) + 5, e_swi_BORDER, HEIGHT - e_swi_BORDER); // borderWidth
  uint8_t yj = beatsin8(41 * ((float)speed / 100.0) + 5, e_swi_BORDER, HEIGHT - e_swi_BORDER);
  // Also calculate some reflections
  uint8_t nxi = fb.cfg.maxWidthIndex() - xi;
  uint8_t nyi = fb.cfg.maxHeightIndex() -yi;
  uint8_t nxj = fb.cfg.maxWidthIndex() - xj;
  uint8_t nyj = fb.cfg.maxHeightIndex() - yj;

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  fb.pixel(xi, yj) = CRGB(fb.pixel(xi, yj)) + ColorFromPalette(*curPalette, ms / 11);
  fb.pixel(xj, yi) = CRGB(fb.pixel(xj, yi)) + ColorFromPalette(*curPalette, ms / 13);
  fb.pixel(nxi, nyj) = CRGB(fb.pixel(nxi, nyj)) + ColorFromPalette(*curPalette, ms / 17);
  fb.pixel(nxj, nyi) = CRGB(fb.pixel(nxj, nyi)) + ColorFromPalette(*curPalette, ms / 29);
  fb.pixel(xi, nyj) = CRGB(fb.pixel(xi, nyj)) + ColorFromPalette(*curPalette, ms / 37);
  fb.pixel(nxi, yj) = CRGB(fb.pixel(nxi, yj)) + ColorFromPalette(*curPalette, ms / 41);

  return true;
}

// ============= DRIFT / ДРИФТ ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift.h

void EffectDrift::load(){
  palettesload();    // подгружаем дефолтные палитры
}

// !++
String EffectDrift::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) _dri_speed = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 2., 20.);
  else if(_val->getId()==4) driftType = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectDrift::run(){
  if (driftType == 1 or driftType == 2)
    FastLED.clear();
  else
    fb.fade(beatsin88(350. * EffectMath::fmap((float)speed, 1., 255., 1., 5.), 512, 4096) / 256);

  
  _dri_delta = beatsin8(1U);

  dri_phase++;    // это примерно каждый кадр и есть

  switch (driftType)
  {
  case 1:
  case 3:
    return incrementalDriftRoutine();
    break;
  case 2:
  case 4:
    return incrementalDriftRoutine2();
    break;
  default:
    return false;
  }
}

bool EffectDrift::incrementalDriftRoutine()
{
  if (curPalette == nullptr) {
    return false;
  }

  for (uint8_t i = 1; i < maxDim / 2U; i++) { // возможно, стоит здесь использовать const MINLENGTH
    int8_t x = beatsin8((float)(maxDim/2 - i) * _dri_speed, maxDim / 2U - 1 - i, maxDim / 2U - 1 + 1U + i, 0, 64U + dri_phase); // используем константы центра матрицы из эффекта Кометы
    int8_t y = beatsin8((float)(maxDim/2 - i) * _dri_speed, maxDim / 2U - 1 - i, maxDim / 2U - 1 + 1U + i, 0, dri_phase);       // используем константы центра матрицы из эффекта Кометы
    EffectMath::wu_pixel((x-width_adj) * 256, (y-height_adj) * 256, ColorFromPalette(RainbowColors_p, (i - 1U) * maxDim_steps + _dri_delta), fb);
  }
  EffectMath::blur2d(fb, beatsin8(3U, 5, 100));
  return true;
}

// ============= DRIFT 2 / ДРИФТ 2 ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift2.h
bool EffectDrift::incrementalDriftRoutine2()
{
  if (curPalette == nullptr) {
    return false;
  }

  for (uint8_t i = 0; i < maxDim; i++){
    int8_t x = 0;
    int8_t y = 0;
    CRGB color;
    if (i < maxDim / 2U)
    {
      x = beatsin8((i + 1) * _dri_speed, i + 1U, maxDim- 1 - i, 0, 64U + dri_phase);
      y = beatsin8((i + 1) * _dri_speed, i + 1U, maxDim - 1 - i, 0, dri_phase);
      color = ColorFromPalette(RainbowColors_p, i * maxDim_steps * 2U + _dri_delta);
    }
    else
    {
      x = beatsin8((maxDim - i) * _dri_speed, maxDim - 1 - i, i + 1U, 0, dri_phase);
      y = beatsin8((maxDim - i) * _dri_speed, maxDim - 1 - i, i + 1U, 0, 64U + dri_phase);
      color = ColorFromPalette(RainbowColors_p, ~(i * maxDim_steps + _dri_delta)); 
    }
    EffectMath::wu_pixel((x-width_adj) * 256, (y-height_adj) * 256, color, fb);
  }
  EffectMath::blur2d(fb, beatsin8(3U, 5, 100));
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
  for (uint32_t idx = 0; idx < num_leds; idx++) {
    if (random(0,255) < tnum) {                                // чем ниже tnum, тем чаще будут заполняться элементы лампы
      ledsbuff[idx].r = random8();                           // оттенок пикселя
      ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS * 2 + 1); // скорость и направление (нарастает 1-4 или угасает 5-8)
      ledsbuff[idx].b = random8();                           // яркость
    }
    else
      ledsbuff[idx] = 0; // всё выкл
  }
}

// !++
String EffectTwinkles::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 1, 8);//((float)TWINKLES_MULTIPLIER * (float)EffectCalc::setDynCtrl(_val).toInt() / 380.0);
  else if(_val->getId()==2) { tnum = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 5, 132); setup(); } 
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}


bool EffectTwinkles::run(){
  return twinklesRoutine();
}

bool EffectTwinkles::twinklesRoutine()
{
  if (curPalette == nullptr) {
    return false;
  }

  for (uint16_t idx = 0; idx < num_leds; idx++)
  {
    if (ledsbuff[idx].b == 0)
    {
      if (random(0,255) < tnum && thue > 0)
      {                                                         // если пиксель ещё не горит, зажигаем каждый ХЗй
        ledsbuff[idx].r = random8();                            // оттенок пикселя
        ledsbuff[idx].g = random8(1, TWINKLES_SPEEDS * 2 + 1);  // скорость и направление (нарастает 1-4, но не угасает 5-8)
        ledsbuff[idx].b = ledsbuff[idx].g;                      // яркость
        thue--;                                                 // уменьшаем количество погасших пикселей
      }
    }
    else if (ledsbuff[idx].g <= TWINKLES_SPEEDS)
    { // если нарастание яркости
      if (ledsbuff[idx].b > 255U - ledsbuff[idx].g - speedFactor)
      { // если досигнут максимум
        ledsbuff[idx].b = 255U;
        ledsbuff[idx].g = ledsbuff[idx].g + TWINKLES_SPEEDS;
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b + ledsbuff[idx].g + speedFactor;
    }
    else
    { // если угасание яркости
      if (ledsbuff[idx].b <= ledsbuff[idx].g - TWINKLES_SPEEDS + speedFactor)
      {                      // если досигнут минимум
        ledsbuff[idx].b = 0; // всё выкл
        thue++;              // считаем количество погасших пикселей
      }
      else
        ledsbuff[idx].b = ledsbuff[idx].b - ledsbuff[idx].g + TWINKLES_SPEEDS - speedFactor;
    }
    if (ledsbuff[idx].b == 0)
      fb[idx] = CRGB::Black;
    else
      fb[idx] = ColorFromPalette(*curPalette, ledsbuff[idx].r, ledsbuff[idx].b);
    }
  EffectMath::blur2d(fb, 32); // так они не только разгороються, но и раздуваються. Красивше :)
  return true;
}

// ============= RADAR / РАДАР ===============
void EffectRadar::load(){
  palettesload();    // подгружаем дефолтные палитры
}

bool EffectRadar::run(){
  return radarRoutine();
}

// !++
String EffectRadar::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectRadar::radarRoutine()
{
  if (curPalette == nullptr)
    return false;

  if (subPix)
  {
    fb.fade(5 + 20 * (float)speed / 255);
    for (float offset = 0.0f; offset < (float)maxDim /2; offset +=0.25)
    {
      float x = (float)EffectMath::mapsincos8(false, eff_theta, offset * 4, maxDim * 4 - offset * 4) / 4.  - width_adj_f;
      float y = (float)EffectMath::mapsincos8(true, eff_theta, offset * 4, maxDim * 4 - offset * 4) / 4.  - height_adj_f;
      CRGB color = ColorFromPalette(*curPalette, hue, 255 / random8(1, 12));
      EffectMath::drawPixelXYF(x, y, color, fb);
    }
  }
  else
  {
    uint8_t _scale = palettescale; // диапазоны внутри палитры, влияют на степень размытия хвоста
    EffectMath::blur2d(fb, beatsin8(5U, 3U, 10U));
    fb.dim(255U - (0 + _scale * 1.5));

    for (uint8_t offset = 0; offset < maxDim /2; offset++)
    {
      fb.pixel(EffectMath::mapsincos8(false, eff_theta, offset, maxDim - offset) - width_adj,
               EffectMath::mapsincos8(true, eff_theta, offset, maxDim - offset) - height_adj) = ColorFromPalette(*curPalette, 255U - (offset * 16U + eff_offset));
    }
  }
  //EVERY_N_MILLIS(EFFECTS_RUN_TIMER) {
    eff_theta += 5.5 * (speed / 255.0) + 1;
    eff_offset += 3.5 * ((255 - speed) / 255.0) + 1;
    if (subPix) {
    //  hue = random8();
    hue = millis() / 16;// eff_offset;
    }
  //}
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
String EffectWaves::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.25, 1)*EffectCalc::speedfactor;
  else if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWaves::wavesRoutine() {
  if (curPalette == nullptr) {
    return false;
  }
  
  fb.dim(255 - 10 * speedFactor); // димирование зависит от скорости, чем быстрее - тем больше димировать
  EffectMath::blur2d(fb, 20); // @Palpalych советует делать размытие. вот в этом эффекте его явно не хватает... (есть сабпиксель, он сам размывает)
  
  float n = 0;
  for (float i = 0.0; i < (_scale <=4 ? WIDTH : HEIGHT); i+= 0.5) {
    n = (float)quadwave8(i * 4 + waveTheta) / (256.0 / ((float)(_scale <=4 ? HEIGHT : WIDTH) -1));
    switch (_scale) {
      case 1: // одна волна горизонтально, справа на лево 
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 2: // две волны горизонтально, справа на лево
        EffectMath::drawPixelXYF(i, n, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF(i, (float)fb.cfg.maxHeightIndex() - n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 3: // одна волна горизонтально, слева на право 
        EffectMath::drawPixelXYF((float)fb.cfg.maxWidthIndex() - i, n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 4: // две волны горизонтально, слева на право
        EffectMath::drawPixelXYF((float)fb.cfg.maxWidthIndex() - i, n, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb.cfg.maxWidthIndex() - i, (float)fb.cfg.maxHeightIndex() - n, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 5: // одна волна вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 6: // две волны вертликально, сверху вниз
        EffectMath::drawPixelXYF(n, i, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb.cfg.maxWidthIndex() - n, i, ColorFromPalette(*curPalette, whue + i), fb);
      break;
      case 7: // одна волна верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)fb.cfg.maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
        break;
      case 8: // две волны верликально, снизу вверх
        EffectMath::drawPixelXYF(n, (float)fb.cfg.maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
        EffectMath::drawPixelXYF((float)fb.cfg.maxWidthIndex() - n, (float)fb.cfg.maxHeightIndex() - i, ColorFromPalette(*curPalette, whue + i), fb);
      break;
    }
  }
  waveTheta += 5.0 * speedFactor;
  whue += 2.5 * speedFactor;

  return true;
}

// ============= FIRE 2012 /  ОГОНЬ 2012 ===============
// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
/*
 * Эффект "Огонь 2012"
 */
void EffectFire2012::load(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&WhiteBlackColors_p);

  usepalettes = true; // активируем "переключатель" палитр
  scale2pallete();    // выбираем палитру согласно "шкале"

  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy(millis());
}

String EffectFire2012::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2012::run() {
  if (curPalette == nullptr) {
    return false;
  }
  if (dryrun(4.0))
    return false;
#ifdef MIC_EFFECTS
  cooling = isMicOn() ? 255 - getMicMapMaxPeak() : 130;
#endif
  return fire2012Routine();
}

bool EffectFire2012::fire2012Routine() {
  sparking = 64 + _scale;
  int fire_base = (fb.cfg.h()/6)>6 ? 6 : fb.cfg.h()/6 + 1;

  // Loop for each column individually
  for (uint8_t x = 0; x < fb.cfg.w(); x++)
  {
    // Step 1.  Cool down every cell a little
    for (uint8_t y = 0; y < fb.cfg.h(); y++)
      noise3d.map_lxy(0,x,y) = qsub8(noise3d.map_lxy(0,x,y), random(0, ((cooling * 10) / fb.cfg.h()) + 2));

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint8_t k = fb.cfg.maxHeightIndex(); k > 2; k--)
      noise3d.map_lxy(0,x,k) = (noise3d.map_lxy(0,x,k - 1) + noise3d.map_lxy(0,x,k - 2) + noise3d.map_lxy(0,x,k - 3)) / 3;

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random(255) < sparking)
    {
      int j = random(fire_base);
      noise3d.map_lxy(0,x,j) = qadd8(noise3d.map_lxy(0,x,j), random(96, 255)); // 196, 255
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint8_t y = 0; y < fb.cfg.h(); y++)
      nblend(fb.pixel(x, y), ColorFromPalette(*curPalette, ((noise3d.map_lxy(0,x,y) * 0.7) + noise3d.map_lxy(0, wrapX(x + 1), y) * 0.3)), fireSmoothing);
  }
  return true;
}

// ============= FIRE 2018 /  ОГОНЬ 2018 ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
// https://gist.github.com/StefanPetrick/819e873492f344ebebac5bcd2fdd8aa8
// https://gist.github.com/StefanPetrick/1ba4584e534ba99ca259c1103754e4c5
// !++
String EffectFire2018::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) isLinSpeed = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2018::run()
{
  // some changing values
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  uint16_t ctrl = ((ctrl1 + ctrl2) / 2);

  // parameters for the heatmap
#ifndef MIC_EFFECTS
  uint16_t _speed = isLinSpeed ? speed : beatsin88(map(speed, 1, 255, 80, 200), 5, map(speed, 1, 255, 10, 255));     // speed пересекается с переменной в родительском классе
#else
  byte mic_p = getMicMapMaxPeak();
  uint16_t _speed = isMicOn() ? (mic_p > map(speed, 1, 255, 225, 20) ? mic_p : 20) : (isLinSpeed ? map(speed, 1, 255, 20, 100) : beatsin88(map(speed, 1, 255, 80, 200), 5, map(speed, 1, 255, 10, 255)));     // speed пересекается с переменной в родительском классе
#endif

  // EVERY_N_SECONDS(3){
  //   LOG(printf_P, PSTR("%d %d\n"), _speed, ctrl);
  // }
  for (auto &i : noise.opt){
    i.e_x = 3 * ctrl * _speed;
    i.e_y = 5 * millis() * _speed;
    i.e_z = 5 * millis() * _speed;
    i.e_scaleX = ctrl1 / 2;
    i.e_scaleY = ctrl2 / 2;
    _speed -= _speed/4;
  }

  //calculate noise data
  noise.fillNoise();

  // shift the buffer one line up, last line goes to first, but we will overwrite it later
  for (uint8_t y = fb.cfg.maxHeightIndex(); y; --y)
    std::swap(fire18heat[y], fire18heat[y-1]);

  // draw lowest line - seed the fire somewhere from the middle of the noise map
  std::memcpy(fire18heat[0].data(), noise.map[0].data() + noise.xy(0, noise.e_centerY), noise.w);

  //dim
  for (uint8_t y = 0; y != noise.h-1; y++)
    for (uint8_t x = 0; x != noise.w-1; x++)
    {
      uint8_t dim = 255 - noise.map_lxy(0, x, y) / 1.7 * constrain(0.05*brightness+0.01,0.01,1.0);  // todo: wtf??? this constrain has a range of ~0-20 ints, why floats for this???
      fire18heat[y][x] = scale8(fire18heat[y][x], dim);
      //uint8_t dim = noise3dx[0][x][y];
      // high value = high flames
      //dim = dim / 1.7 * constrain(0.05*myLamp.effects.getBrightness()+0.01,0.01,1.0); //точно нужен прямой доступ?
      //dim = 255 - dim;
    }

  for (uint8_t y = 0; y != noise.h-1; y++)
  {
    for (uint8_t x = 0; x != noise.w-1; x++)
    {
      // map the colors based on heatmap
      CRGB color(fire18heat[y][x], (float)fire18heat[y][x] * (scale/5.0) * 0.01, 0);  // todo: wtf??? more nifty floats
      color*=2.5;

      // dim the result based on 2nd noise layer
      //color = fb.pixel(x, fb.cfg.maxHeightIndex() - y);
      color.nscale8(noise.map_lxy(1,x,y));
      fb.pixel(x, y) = color;
    }
  }
  return true;
}

// ------------------------------ ЭФФЕКТ КОЛЬЦА / КОДОВЫЙ ЗАМОК ----------------------
// (c) SottNick
// из-за повторного использоваия переменных от других эффектов теперь в этом коде невозможно что-то понять.
// поэтому для понимания придётся сперва заменить названия переменных на человеческие. но всё равно это песец, конечно.

//uint8_t ringColor[HEIGHT]; // начальный оттенок каждого кольца (оттенка из палитры) 0-255
//uint8_t huePos[HEIGHT]; // местоположение начального оттенка кольца 0-fb.cfg.maxWidthIndex()
//uint8_t shiftHueDir[HEIGHT]; // 4 бита на ringHueShift, 4 на ringHueShift2
////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число
//uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
//uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от WIDTH/5 до WIDTH-3
bool EffectRingsLock::run(){
  if (dryrun(3.0))
    return false;
  return ringsRoutine();
}

void EffectRingsLock::load(){
  palettesload();
  ringsSet();
}

// !++
String EffectRingsLock::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { ringWidth = EffectCalc::setDynCtrl(_val).toInt(); ringsSet(); }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

// Установка параметров колец
void EffectRingsLock::ringsSet(){
  if (curPalette == nullptr) {
    return;
  }
  //FastLED.clear();

  ringNb = (float)HEIGHT / ringWidth + ((HEIGHT % ringWidth == 0U) ? 0U : 1U)%HEIGHT; // количество колец
  upRingHue = ringWidth - (ringWidth * ringNb - HEIGHT) / 2U; // толщина верхнего кольца. может быть меньше нижнего
  downRingHue = HEIGHT - upRingHue - (ringNb - 2U) * ringWidth; // толщина нижнего кольца = всё оставшееся

  for (uint8_t i = 0; i < ringNb; i++)
  {
    if (!i) ringColor[i] = 0; //random(255U - WIDTH / 4U);
    ringColor[i] = ringColor[i - 1] + 64; // начальный оттенок кольца из палитры 0-255 за минусом длины кольца, делённой пополам
    shiftHueDir[i] = random8();
    huePos[i] = random8(); 
    stepCount = 0U;
    currentRing = random(ringNb);
  }
}

bool EffectRingsLock::ringsRoutine()
{
  uint8_t h, x, y;
  FastLED.clear();

  for (uint8_t i = 0; i < ringNb; i++)
  {
    if (i != currentRing) // если это не активное кольцо
    {
       h = shiftHueDir[i] & 0x0F; // сдвигаем оттенок внутри кольца
       if (h > 8U)
         //ringColor[i] += (uint8_t)(7U - h); // с такой скоростью сдвиг оттенка от вращения кольца не отличается
         ringColor[i]--;
       else
         //ringColor[i] += h;
         ringColor[i]++;
    } else {
      if (stepCount == 0) { // если сдвиг активного кольца завершён, выбираем следующее
        currentRing = random(ringNb);
        do {
          stepCount = WIDTH - 3U - random8((WIDTH - 3U) * 2U); // проворот кольца от хз до хз
        } while (stepCount < WIDTH / 5U || stepCount > 255U - WIDTH / 5U);
      } else {
        if (stepCount > 127U)
          {
            stepCount++;
            huePos[i] = (huePos[i] + 1U) % WIDTH;
          }
        else
          {
            stepCount--;
            huePos[i] = (huePos[i] - 1U + WIDTH) % WIDTH;
          }
      }
    }

    // отрисовываем кольца
    h = (shiftHueDir[i] >> 4) & 0x0F; // берём шаг для градиента вутри кольца
    if (h > 8U)
      h = 7U - h;
    for (uint8_t j = 0U; j < ((i == 0U) ? downRingHue : ((i == ringNb - 1U) ? upRingHue : ringWidth)); j++) // от 0 до (толщина кольца - 1)
    {
      y = i * ringWidth + j - ((i == 0U) ? 0U : ringWidth - downRingHue);
      for (uint8_t k = 0; k < WIDTH / 2U - 1; k++) // полукольцо
        {
          x = (huePos[i] + k) % WIDTH; // первая половина кольца
          fb.pixel(x, y) = ColorFromPalette(*curPalette, ringColor[i]/* + k * h*/);
          x = (fb.cfg.maxWidthIndex() + huePos[i] - k) % WIDTH; // вторая половина кольца (зеркальная первой)
          fb.pixel(x, y) = ColorFromPalette(*curPalette, ringColor[i] + k * h);
        }
      if (WIDTH & 0x01) //(WIDTH % 2U > 0U) // если число пикселей по ширине матрицы нечётное, тогда не забываем и про среднее значение
      {
        x = (huePos[i] + WIDTH / 2U) % WIDTH;
        fb.pixel(x, y) = ColorFromPalette(*curPalette, ringColor[i] + WIDTH / 2U * h);
      }
    }
  }
  return true;
}

// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// Classic (c) SottNick
// New (c) Vortigont
#define PAUSE_MAX 7

void EffectCube2d::swapBuff() {
  for (uint8_t y = 0; y < fb.cfg.h(); y++) { // переписываем виртуальный экран в настоящий
    for(uint8_t x = 0; x < fb.cfg.w(); x++) {
      fb.pixel(x,y) = ledbuff.pixel(x,y);
    }
  }
}

bool EffectCube2d::run(){
  if (dryrun(classic ? 4. : 3., classic ? 3 : EFFECTS_RUN_TIMER))
    return false;
  if (classic)
    return cube2dClassicRoutine();
  else
    return cube2dRoutine();
}

// !++
String EffectCube2d::setDynCtrl(UIControl*_val)
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
  cubesize();
}

// задаем размерность кубов
void EffectCube2d::cubesize() {
  if (curPalette == nullptr) {
    return;
  }

  fb.clear();

  cntY = fb.cfg.h() / (sizeY+1) + !!(fb.cfg.h() / (sizeY+1));
	fieldY = (sizeY + 1U) * cntY;

  cntX = fb.cfg.w() / (sizeX+1) + !!(fb.cfg.w() / (sizeX+1));
	fieldX = (sizeX + 1U) * cntX;

  ledbuff = LedFB(fieldX, fieldY);   // создаем виртуальную матрицу, размером кратную размеру кубика+1

  //LOG(printf_P, PSTR("CUBE2D Size: scX=%d, scY=%d, scaleY=%d, cntX=%d, cntY=%d\n"), cubeScaleX, cubeScaleY, scaleY, cntX, cntY);
  uint8_t x=0, y = 0;
  CRGB color;

  for (uint8_t j = 0U; j < cntY; j++)
  {
    y = j * (sizeY + 1U);
    for (uint8_t i = 0U; i < cntX; i++)
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

      for (uint8_t k = 0U; k < sizeY; k++){
        for (uint8_t m = 0U; m < sizeX; m++){
          ledbuff.pixel(x+m, y+k) = color;
        }
      }
    }
  }
  if (classic) {
    storage.assign(fb.cfg.w(), std::vector<uint8_t>(fb.cfg.h()) );    // todo: get rid of this
    currentStep = 4U; // текущий шаг сдвига первоначально с перебором (от 0 до shiftSteps-1)
    shiftSteps = 4U; // всего шагов сдвига (от 3 до 4)
    pauseSteps = 0U; // осталось шагов паузы
    seamlessX = (fieldX == fb.cfg.w());
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
          color = ledbuff.pixel(x, anim0);
          for (uint8_t k = anim0; k < anim0+fieldY-1; k++)
          {
            // берём цвет от строчки над нашей
            color2 = ledbuff.pixel(x, k+1);
            for (uint8_t m = x; m < x + sizeX; m++)
              // копируем его на всю нашу строку
              ledbuff.pixel(m, k) = color2;
          }
          for   (uint8_t m = x; m < x + sizeX; m++)
            // цвет нижней строчки копируем на всю верхнюю
            ledbuff.pixel(m, anim0+fieldY-1) = color;
          return;
        }

      // крутим столбец вверх
      // берём цвет от верхней строчки
      color = ledbuff.pixel(x,anim0+fieldY-1);
      for (uint8_t k = anim0+fieldY-1; k > anim0 ; k--)
      {
        color2 = ledbuff.pixel(x, k-1);
        for (uint8_t m = x; m < x + sizeX; m++)
          // копируем его на всю нашу строку
          ledbuff.pixel(m, k ) = color2;
      }
      for   (uint8_t m = x; m < x + sizeX; m++)
        // цвет верхней строчки копируем на всю нижнюю
        ledbuff.pixel(m, anim0) = color;
}

// идём по вертикали, крутим по горизонтали (строки двигаются)
void EffectCube2d::cube2dmoveRows(uint8_t moveItem, bool movedirection){
  uint16_t y, anim0;
  CRGB color, color2;

  y = moveItem * (sizeY + 1U);
  anim0 = 0;

  // крутим строку влево
  if (!movedirection){
    color = ledbuff.pixel(anim0, y);                            // берём цвет от левой колонки (левого пикселя)
    for (uint8_t k = anim0; k < anim0+fieldX-1; k++)
    {
      color2 = ledbuff.pixel(k+1, y);                           // берём цвет от колонки (пикселя) правее
      for (uint8_t m = y; m < y + sizeY; m++)
        // копируем его на всю нашу колонку
        ledbuff.pixel(k,m) = color2;
    }
    for   (uint8_t m = y; m < y + sizeY; m++)
      // цвет левой колонки копируем на всю правую
      ledbuff.pixel(anim0+fieldX-1, m) = color;
   return;
  }

  //  крутим строку вправо
  // берём цвет от правой колонки
  color = ledbuff.pixel(anim0+fieldX-1, y);
  for (uint8_t k = anim0+fieldX-1; k > anim0 ; k--)
  {
    // берём цвет от колонки левее
    color2 = ledbuff.pixel(k-1, y);
    for (uint8_t m = y; m < y + sizeY; m++)
      // копируем его на всю нашу колонку
      ledbuff.pixel(k, m) = color2;
  }
  for   (uint8_t m = y; m < y + sizeY; m++)
    // цвет правой колонки копируем на всю левую
    ledbuff.pixel(anim0, m) = color;
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
        x = (gX + i * (sizeX + 1U)) % fb.cfg.w();
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
            color = ledbuff.pixel(x, anim0);
            for (uint8_t k = anim0; k < anim0 + fieldY - 1; k++)
            {
              color2 = ledbuff.pixel(x, k + 1); // берём цвет от строчки над нашей
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff.pixel(m % fieldX, k) = color2;    // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              ledbuff.pixel(m % fieldX, anim0 + fieldY - 1) = color;  // цвет нижней строчки копируем на всю верхнюю
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color =  ledbuff.pixel(x, anim0 + fieldY - 1); // берём цвет от верхней строчки
            for (uint8_t k = anim0 + fieldY - 1; k > anim0; k--)
            {
              color2 = ledbuff.pixel(x, k - 1); // берём цвет от строчки под нашей
              for (uint8_t m = x; m < x + sizeX; m++)
                ledbuff.pixel(m % fb.cfg.w(), k) = color2; // копируем его на всю нашу строку
            }
            for (uint8_t m = x; m < x + sizeX; m++)
              ledbuff.pixel(m % fb.cfg.w(), anim0) = color; // цвет верхней строчки копируем на всю нижнюю
          }
        }
      }
    }
    else
    {
      for (uint8_t j = 0U; j < cntY; j++)
      {
        y = (gY + j * (sizeY + 1U))  % fb.cfg.h();
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
            color = ledbuff.pixel(anim0, y); // берём цвет от левой колонки (левого пикселя)
            for (uint8_t k = anim0; k < anim0 + fieldX - 1; k++)
            {
              color2 = ledbuff.pixel(k + 1, y); // берём цвет от колонки (пикселя) правее
              for (uint8_t m = y; m < y + sizeY; m++)
                ledbuff.pixel(k, m) = color2; // копируем его на всю нашу колонку
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              ledbuff.pixel(anim0 + fieldX - 1, m) = color;   // цвет левой колонки копируем на всю правую
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = ledbuff.pixel(anim0 + fieldX - 1, y); // берём цвет от правой колонки
            for (uint8_t k = anim0 + fieldX - 1; k > anim0; k--)
            {
              color2 = ledbuff.pixel(k - 1, y); // берём цвет от колонки левее
              for (uint8_t m = y; m < y + sizeY; m++)
                ledbuff.pixel(k, m) = color2; // копируем его на всю нашу колонку
            }
            for (uint8_t m = y; m < y + sizeY; m++)
              ledbuff.pixel(anim0, m) = color; // цвет правой колонки копируем на всю левую
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
    gX = (fb.cfg.w() + gX + globalShiftX) % fb.cfg.w();
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
      shiftSteps = sizeY + ((gY - shiftAll >= 0 && gY - shiftAll + fieldY < (int)fb.cfg.h()) ? random8(2U) : 1U);

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
        shiftSteps = sizeX + ((gX - shiftAll >= 0 && gX - shiftAll + fieldX < (int)fb.cfg.w()) ? random8(2U) : 1U);

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

//-------------- Эффект "Часы"
bool EffectTime::run(){
  // if(isDebug())
  //   return palleteTest(fb, opt);
  // else {
    if((millis() - lastrun - EFFECTS_RUN_TIMER) < (unsigned)((255-speed)) && (speed==1 || speed==255)){
        fb.dim(254);
      return true;
    } else {
      lastrun = millis();
      if (myLamp.isPrintingNow()) // если выводится бегущая строка, то эффект приостанавливаем! Специально обученный костыль, т.к. вывод статического и динамического текста одноверенно не совместимы
        return true;
    }
    return timePrintRoutine();
  // }
}

void EffectTime::load(){
  FastLED.clear();
  palettesload();    // подгружаем дефолтные палитры

  if(((curTimePos<=(signed)LET_WIDTH*2-(LET_WIDTH/2)) || (curTimePos>=(signed)WIDTH+(LET_WIDTH/2))) )
  {
    curTimePos = random(LET_WIDTH*2,WIDTH);
    hColor[0] = ColorFromPalette(*curPalette, random8());
    mColor[0] = ColorFromPalette(*curPalette, random8());
  }
}

bool EffectTime::palleteTest()
{
  FastLED.clear();
  float sf = 0.996078431372549+speed/255.; // смещение, для скорости 1 смещения не будет, т.к. суммарный коэф. == 1
  for(uint8_t y=0; y<HEIGHT; y++)
    for(uint8_t x=0; x<WIDTH; x++)
      fb.pixel(fb.cfg.maxWidthIndex() - x,fb.cfg.maxHeightIndex()-y) = ColorFromPalette(*curPalette, (y*x*sf), 127);
  return true;
}

bool EffectTime::timePrintRoutine()
{
  if (speed==254 || speed==1 || speed==255){
    EVERY_N_SECONDS(5){
      FastLED.clear();

      String tmp = embui.timeProcessor.getFormattedShortTime();
      if(HEIGHT>=16){
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2,WIDTH) : (WIDTH+LET_WIDTH*2)/2;
        myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 64), false, false, HEIGHT-LET_HEIGHT, xPos);
        myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 127), false, false, (int8_t)HEIGHT-(int8_t)(LET_HEIGHT*2), xPos);
      } else if(WIDTH>=21){ // требуется минимум 5*4+1 символов
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2+1,WIDTH) : (WIDTH+LET_WIDTH*2+1)/2;
        myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 64), false, false, HEIGHT-LET_HEIGHT, xPos);
        myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : 127), false, false, HEIGHT-LET_HEIGHT, xPos-(LET_WIDTH*2+1));
      } else {
        //int16_t xPos = random(LET_WIDTH*2,WIDTH); // вывод часов/минут попеременно...
        int16_t xPos = speed!=254 ? random(LET_WIDTH*2,WIDTH) : (WIDTH+LET_WIDTH*2)/2;
        isMinute=!isMinute;
        myLamp.sendStringToLamp(isMinute?tmp.substring(3,5).c_str():tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, speed!=254 ? random8() : isMinute ? 64 : 127), false, false, HEIGHT-LET_HEIGHT, xPos);
      }
    }
  } else {
    //FastLED.clear();
    EVERY_N_SECONDS(5){
      isMinute=!isMinute;
    }
    fb.dim(250-speed/3); // небольшой шлейф, чисто как визуальный эффект :)
    int16_t xPos = curTimePos;
    if((xPos<=(signed)LET_WIDTH*2-((signed)LET_WIDTH/2)) || (xPos>=(signed)WIDTH+((signed)LET_WIDTH/2))){
      if(xPos<=(signed)LET_WIDTH*2){
        timeShiftDir = false;
        xPos=LET_WIDTH*2-(LET_WIDTH/2); // будет на полсимвола выходить за пределы, так задумано :)
      } else {
        timeShiftDir = true;
        xPos=WIDTH+(LET_WIDTH/2); // будет на полсимвола выходить за пределы, так задумано :)
      }
      hColor[0] = ColorFromPalette(*curPalette, random8());
      mColor[0] = ColorFromPalette(*curPalette, random8());
    }
    String tmp = embui.timeProcessor.getFormattedShortTime();
    uint8_t shift = beatsin8(speed/5, -1, 1);
    if(HEIGHT>=16){
      myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), hColor[0], false, false, HEIGHT-LET_HEIGHT+shift, xPos);
      myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), mColor[0], false, false, HEIGHT-(LET_HEIGHT*2)+shift, xPos);
    } else if(WIDTH>=21){
      myLamp.sendStringToLamp(tmp.substring(0,2).c_str(), hColor[0], false, false, HEIGHT-LET_HEIGHT+shift, xPos+(LET_WIDTH*2+1));
      myLamp.sendStringToLamp(tmp.substring(3,5).c_str(), mColor[0], false, false, HEIGHT-LET_HEIGHT+shift, xPos);
    } else if(WIDTH>=10){
      myLamp.sendStringToLamp(isMinute ? tmp.substring(3,5).c_str() : tmp.substring(0,2).c_str(), hColor[0], false, false, HEIGHT-LET_HEIGHT+shift, xPos);
    } else {
        xPos = random(LET_WIDTH*2,WIDTH); // вывод часов/минут попеременно...
        myLamp.sendStringToLamp(isMinute ? tmp.substring(3,5).c_str() : tmp.substring(0,2).c_str(), ColorFromPalette(*curPalette, (int)color_idx%16), false, false, HEIGHT-LET_HEIGHT, xPos);
        color_idx=color_idx+(speed/256.0);
    }
    curTimePos=curTimePos+(0.23*(speed/255.0))*(timeShiftDir?-1:1); // смещаем
  }
  return true;

}

// ----------- Эффекты "Пикассо" (c) obliterator
EffectPicasso::EffectPicasso(LedFB &framebuffer) : EffectCalc(framebuffer){
  palettes.add(MBVioletColors_gp, 0, 16); // будет заменен генератором
  palettes.add(MBVioletColors_gp, 0, 16);

  palettes.add(ib_jul01_gp, 60, 16, 200);

  palettes.add(es_pinksplash_08_gp, 125, 16);

  palettes.add(departure_gp, 0);
  palettes.add(departure_gp, 140, 16, 220);

  palettes.add(es_landscape_64_gp, 25, 16, 250);
  palettes.add(es_landscape_64_gp, 125);
  palettes.add(es_landscape_64_gp, 175, 50, 220);

  palettes.add(es_ocean_breeze_036_gp, 0);

  palettes.add(es_landscape_33_gp, 0);
  palettes.add(es_landscape_33_gp, 50);
  palettes.add(es_landscape_33_gp, 50, 50);

  palettes.add(GMT_drywet_gp, 0);
  palettes.add(GMT_drywet_gp, 75);
  palettes.add(GMT_drywet_gp, 150, 0, 200);

  palettes.add(fire_gp, 175);

  palettes.add(Pink_Purple_gp, 25);
  palettes.add(Pink_Purple_gp, 175, 0, 220);

  palettes.add(Sunset_Real_gp, 25, 0, 200);
  palettes.add(Sunset_Real_gp, 50, 0, 220);

  palettes.add(BlacK_Magenta_Red_gp, 25);

  generate(true);
}

void EffectPicasso::generate(bool reset){
  unsigned numParticles = map(scale, 0U, 255U, PICASSO_MIN_PARTICLES, PICASSO_MAX_PARTICLES);

  if (numParticles != particles.size()){
    particles.assign(numParticles, Particle());
    reset = true;
  }

  double minSpeed = 0.2, maxSpeed = 0.8;
  for (auto &particle : particles){
    if (reset) {
      particle.position_x = random8(0, WIDTH);
      particle.position_y = random8(0, HEIGHT);

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

void EffectPicasso::position(){
  for (auto &particle : particles){
    if (particle.position_x + particle.speed_x > WIDTH || particle.position_x + particle.speed_x < 0) {
      particle.speed_x *= -1;
    }

    if (particle.position_y + particle.speed_y > HEIGHT || particle.position_y + particle.speed_y < 0) {
      particle.speed_y *= -1;
    }

    particle.position_x += particle.speed_x*speedFactor;
    particle.position_y += particle.speed_y*speedFactor;
  };
}

bool EffectPicasso::picassoRoutine(){
  generate();
  position();
  if (effId > 1) fb.dim(180);

  unsigned iter = (particles.size() - particles.size()%2) / 2;
  for (unsigned i = 0; i != iter; ++i) {
    Particle &p1 = particles[i];
    Particle &p2 = particles[particles.size()-1-i];
    switch (effId){
    case 1:
      EffectMath::drawLine(static_cast<int>(p1.position_x), static_cast<int>(p1.position_y), static_cast<int>(p2.position_x), static_cast<int>(p2.position_y), p1.color, fb);
      break;
    case 2:
      EffectMath::drawLineF(p1.position_x, p1.position_y, p2.position_x, p2.position_y, p1.color, fb);
      break;
    case 3:
      EffectMath::drawCircleF(fabs(p1.position_x - p2.position_x), fabs(p1.position_y - p2.position_y), fabs(p1.position_x - p1.position_y), p1.color, fb);
      break;
  	default:
      EffectMath::drawSquareF(fabs(p1.position_x - p2.position_x), fabs(p1.position_y - p2.position_y), fabs(p1.position_x - p1.position_y), p1.color, fb);
    }
  }

  EVERY_N_MILLIS(20000){
    generate(true);
  }

  EffectMath::blur2d(fb, 80);
  return true;
}

// !++
String EffectPicasso::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() /255.0+0.1)*EffectCalc::speedfactor;
  else if(_val->getId()==3) pidx = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) {
    byte hue = EffectCalc::setDynCtrl(_val).toInt();
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
    CRGBPalette32 pal;
    pal.loadDynamicGradientPalette(dynpal);
    palettes.add(0, pal, 0, 16);
  }
  else if(_val->getId()==5) effId = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPicasso::metaBallsRoutine(){
  generate();
  position();

 // сила возмущения
  unsigned mx = EffectMath::fmap(scale, 0U, 255U, 200U, 80U);
  // радиус возмущения
  unsigned sc = EffectMath::fmap(scale, 0U, 255U, 12, 7);
  // отсечка расчетов (оптимизация скорости)
  unsigned tr = sc * 2 / 3;

  for (unsigned x = 0; x < WIDTH; x++) {
    for (unsigned y = 0; y < HEIGHT; y++) {
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

        if (sum >= 255) { sum = 255; break; }
      }
      CRGB color = palettes[pidx].GetColor((uint8_t)sum, 255);
      fb.pixel(x, y) = color;
      }
  }

  return true;
}

bool EffectPicasso::run(){
  switch (effect)
  {
  case EFF_PICASSO:
    picassoRoutine();
    break;
  case EFF_PICASSO4:
    metaBallsRoutine();
    break;
  default:
    break;
  }
  return true;
}

// -------- Эффект "Прыгуны" (c) obliterator
void EffectLeapers::load() {
  generate();
}

void EffectLeapers::restart_leaper(Leaper &l) {
  // leap up and to the side with some random component
#ifdef MIC_EFFECTS
  uint8_t mic = getMicMaxPeak();
  uint8_t rand = random(5, 50 + _rv * 4);
  l.xd = static_cast<float>(isMicOn() ? 25 + mic : rand) / 100.0;
  l.yd = static_cast<float>(isMicOn() ? 25 + mic : rand) / 50.0;
#else
  l.xd = static_cast<float>(random8(5, 50 + _rv * 4)) / 100;
  l.yd = static_cast<float>(random8(5, 100 + _rv * 3)) / 50;
#endif

  // for variety, sometimes go 20% faster
  if (random8() < 12) {
    l.xd += l.xd * 0.2;
    l.yd += l.yd * 0.2;
  }

  // leap towards the centre of the screen
  if (l.x > (WIDTH / 2)) {
    l.xd *= -1;
  }
  l.color += 8;
}

void EffectLeapers::move_leaper(Leaper &l) {
#define GRAVITY            0.06
#define SETTLED_THRESHOLD  0.15
#define WALL_FRICTION      0.95
#define WIND               0.98    // wind resistance

  l.x += l.xd * speedFactor;
  l.y += l.yd * speedFactor;

  // bounce off the floor and ceiling?
  if (l.y < 0 || l.y > (fb.cfg.maxHeightIndex() - SETTLED_THRESHOLD)) {
    l.yd = (-l.yd * WALL_FRICTION);
    l.xd = (l.xd * WALL_FRICTION);
    if (l.y > (fb.cfg.maxHeightIndex() - SETTLED_THRESHOLD)) l.y += l.yd;
    if (l.y < 0) l.y = 0;
    // settled on the floor?
    if (l.y <= SETTLED_THRESHOLD && fabs(l.yd) <= SETTLED_THRESHOLD) {
      restart_leaper(l);
    }
  }

  // bounce off the sides of the screen?
  if (l.x <= 0 || l.x >= fb.cfg.maxWidthIndex()) {
    l.xd = (-l.xd * WALL_FRICTION);
    l.yd = (l.yd * WALL_FRICTION);
    if (l.x < 0) l.x = 0;
    if (l.x > fb.cfg.maxWidthIndex()) l.x = fb.cfg.maxWidthIndex();
  }

  l.yd -= GRAVITY*speedFactor;
  l.xd *= WIND;
  l.yd *= WIND;
}

void EffectLeapers::generate(){
  for (auto &curr : leapers){
    curr.x = EffectMath::randomf(0, fb.cfg.maxWidthIndex());
    curr.y = EffectMath::randomf(0, fb.cfg.maxHeightIndex());
    curr.xd = ((float)random(5, 50 + _rv * 4) / 100);
    curr.yd = ((float)random(5, 100 + _rv * 3) / 50);

    curr.color = random8();
  };
}

// !++
String EffectLeapers::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt()/256.0 + 0.33)*EffectCalc::speedfactor;
  else if(_val->getId()==2) {
    long num = map(EffectCalc::setDynCtrl(_val).toInt(), 0U, 255U, LEAPERS_MIN, LEAPERS_MAX);
    leapers.assign(num, Leaper());
    generate();
  }
  else if(_val->getId()==3) _rv = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)

  return String();
}

bool EffectLeapers::run(){
  EVERY_N_SECONDS(30) {
    randomSeed(millis());
  }

  //fb.dim(0);
  FastLED.clear();

  //for (unsigned i = 0; i < numParticles; i++) {
  for (auto &l : leapers){
    move_leaper(l);
    EffectMath::drawPixelXYF(l.x, l.y, CHSV(l.color, 255, 255), fb);
  };

  EffectMath::blur2d(fb, 20);
  return true;
}



// ----------- Эффекты "Лавовая лампа" (c) obliterator
EffectLiquidLamp::EffectLiquidLamp(LedFB &framebuffer) : EffectCalc(framebuffer) {
  // эта палитра создана под эффект
  palettes.add(MBVioletColors_gp, 0, 16);
  // палитры частично подогнаные под эффект
  palettes.add(ib_jul01_gp, 60, 16, 200);
  palettes.add(Sunset_Real_gp, 25, 0, 200);
  palettes.add(es_landscape_33_gp, 50, 50);
  palettes.add(es_pinksplash_08_gp, 125, 16);
  palettes.add(es_landscape_64_gp, 175, 50, 220);
  palettes.add(es_landscape_64_gp, 25, 16, 250);
  palettes.add(es_ocean_breeze_036_gp, 0);
  palettes.add(es_landscape_33_gp, 0);
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
  unsigned num = map(scale, 0U, 255, LIQLAMP_MIN_PARTICLES, LIQLAMP_MAX_PARTICLES);

  if (num != particles.size())
    particles.assign(num, Particle());
  else if (!reset) return;

  for (auto &curr : particles){
    curr.position_x = random(0, WIDTH);
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
    curr.hot += EffectMath::mapcurve(curr.position_y, 0, HEIGHT, 5, -5, EffectMath::InOutQuad) * speedFactor;

    float heat = (curr.hot / curr.mass) - 1;
    if (heat > 0 && curr.position_y < HEIGHT) {
      curr.speed_y += heat * curr.spf;
    }
    if (curr.position_y > 0) {
      curr.speed_y -= 0.07;
    }

    if (curr.speed_y) curr.speed_y *= 0.85;
    curr.position_y += curr.speed_y * speedFactor;

    if (physic_on) {
      curr.speed_x *= 0.7;
      curr.position_x += curr.speed_x * speedFactor;
    }

    if (curr.position_x > WIDTH) curr.position_x -= WIDTH;
    if (curr.position_x < 0) curr.position_x += WIDTH;
    if (curr.position_y > HEIGHT) curr.position_y = HEIGHT;
    if (curr.position_y < 0) curr.position_y = 0;
  };
}

void EffectLiquidLamp::physic(){
  for (auto p1 = particles.begin(); p1 != particles.end(); ++p1){
    // отключаем физику на границах, чтобы не слипались шары
    if (p1->position_y < 3 || p1->position_y > fb.cfg.maxHeightIndex()) continue;

    for (auto p2 = p1 + 1; p2 != particles.end(); ++p2) {
      if (p2->position_y < 3 || p2->position_y > fb.cfg.maxHeightIndex()) continue;
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

// !++
String EffectLiquidLamp::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 127.0 + 0.1)*EffectCalc::speedfactor;
  else if(_val->getId()==3) pidx = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) {
    byte hue = EffectCalc::setDynCtrl(_val).toInt();
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
  else if(_val->getId()==5) filter = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) physic_on = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectLiquidLamp::routine(){
  generate();
  position();
  if (physic_on) physic();

  for (unsigned x = 0; x < WIDTH; x++) {
    for (unsigned y = 0; y < HEIGHT; y++) {
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

      if (filter < 2) {
        fb.pixel(x, y) = palettes[pidx].GetColor(sum, filter? sum : 255);
      } else {
        buff[x][y] = sum;
      }
    }
  }

  if (filter < 2) return true;

  // use Scharr's filter
    static constexpr std::array<int, 9> dh_scharr = {3, 10, 3,  0, 0,   0, -3, -10, -3};
    static constexpr std::array<int, 9> dv_scharr = {3, 0, -3, 10, 0, -10,  3,   0, -3};
    float min =0, max = 0;
    for (int16_t x = 1; x < fb.cfg.maxWidthIndex() -1; x++) {
      for (int16_t y = 1; y < fb.cfg.maxHeightIndex() -1; y++) {
        int gh = 0, gv = 0, idx = 0;

        for (int v = -1; v != 2; ++v) {
          for (int h = -1; h != 2; ++h) {
            gh += dh_scharr[idx] * buff[x+h][y+v];
            gv += dv_scharr[idx] * buff[x+h][y+v];
            ++idx;
          }
        }
        buff2[x][y] = EffectMath::sqrt((gh * gh) + (gv * gv));
        if (buff2[x][y] < min) min = buff2[x][y];
        if (buff2[x][y] > max) max = buff2[x][y];
      }
    }

    for (unsigned x = 0; x < (unsigned)fb.cfg.maxWidthIndex(); x++) {
      for (unsigned y = 0; y < (unsigned)fb.cfg.maxHeightIndex(); y++) {
        float val = buff2[x][y];
        val = 1 - (val - min) / (max - min);
        unsigned step = filter - 1;
        while (step) { val *= val; --step; } // почему-то это быстрее чем pow
        fb.pixel(x, y) = palettes[pidx].GetColor(buff[x][y], val * 255);
      }
    }

  return true;
}

// ------- Эффект "Вихри"
// Based on Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternFlowField.h
// Copyright(c) 2014 Jason Coon
//адаптация SottNick
bool EffectWhirl::run(){

  return whirlRoutine();
}

void EffectWhirl::load(){
  palettesload();    // подгружаем дефолтные палитры
  ff_x = random16();
  ff_y = random16();
  ff_z = random16();
  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
      boids[i] = Boid(EffectMath::randomf(0, WIDTH), 0);
  }

}

// !++
String EffectWhirl::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 1.1) * EffectCalc::speedfactor;
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectWhirl::whirlRoutine() {
#ifdef MIC_EFFECTS
  micPick = isMicOn() ? getMicMaxPeak() : 0;
#endif
  fb.fade(15. * speedFactor);

  for (uint8_t i = 0; i < AVAILABLE_BOID_COUNT; i++) {
    Boid * boid = &boids[i];

    float ioffset = (float)ff_scale * boid->location.x;
    float joffset = (float)ff_scale * boid->location.y;

    byte angle = inoise8(ff_x + ioffset, ff_y + joffset, ff_z);

    boid->velocity.x = ((float)sin8(angle) * 0.0078125 - speedFactor);
    boid->velocity.y = -((float)cos8(angle) * 0.0078125 - speedFactor);
    boid->update();
#ifdef MIC_EFFECTS
    if (!isMicOn())
      EffectMath::drawPixelXYF(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + (uint8_t)hue), fb); // + hue постепенно сдвигает палитру по кругу
    else
      EffectMath::drawPixelXYF(boid->location.x, boid->location.y, CHSV(getMicMapFreq(), 255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255)), fb); // + hue постепенно сдвигает палитру по кругу

#else
    EffectMath::drawPixelXYF(boid->location.x, boid->location.y, ColorFromPalette(*curPalette, angle + (uint8_t)hue), fb); // + hue постепенно сдвигает палитру по кругу
#endif
    if (boid->location.x < 0 || boid->location.x >= WIDTH || boid->location.y < 0 || boid->location.y >= HEIGHT) {
      boid->location.x = EffectMath::randomf(0, WIDTH);
      boid->location.y = 0;
    }
  }
  EffectMath::blur2d(fb, 30U);

  hue += speedFactor;
  ff_x += speedFactor;
  ff_y += speedFactor;
  ff_z += speedFactor;
  return true;
}

// ------------- Эффект "Блики на воде Цвета"
// Идея SottNick
// переписал на программные блики + паттерны - (c) kostyamat
// Генератор бликов (c) stepko

void EffectAquarium::load(){
  currentPalette = PartyColors_p;
  for (auto &i : drops) {
    i.posX = random(fb.cfg.w());
    i.posY = random(fb.cfg.h());
    i.radius = EffectMath::randomf(-1, maxRadius());
  }
}

String EffectAquarium::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.1, 1.);
  else if(_val->getId()==2) scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) satur = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) glare = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectAquarium::nDrops(uint8_t bri) {

  fill_solid(currentPalette, 16, CHSV(hue, satur, bri));
  currentPalette[10] = CHSV(hue, satur - 60, 255);
  currentPalette[9] = CHSV(hue, 255 - satur, 210);
  currentPalette[8] = CHSV(hue, 255 - satur, 210);
  currentPalette[7] = CHSV(hue, satur - 60, 255);
  fb.fill(ColorFromPalette(currentPalette, 1));

  for (auto &i : drops) {
    EffectMath::drawCircle(i.posX, i.posY, i.radius, ColorFromPalette(currentPalette, (256/16)*8.5-i.radius), fb);
    EffectMath::drawCircle(i.posX, i.posY, i.radius - 1., ColorFromPalette(currentPalette,(256/16)*7.5-i.radius, 256/i.radius), fb);
    if (i.radius >= maxRadius()) {
      i.radius = -1;
      i.posX = random(fb.cfg.w());
      i.posY = random(fb.cfg.h());
    } else
      i.radius += 0.25;
  }

  EffectMath::blur2d(fb, 128);
}

void EffectAquarium::nGlare(uint8_t bri) {

  fill_solid(currentPalette, 16, CHSV(hue, satur, bri));
  currentPalette[10] = CHSV(hue, satur - 60, 225);
  currentPalette[9] = CHSV(hue, 255 - satur, 180);
  currentPalette[8] = CHSV(hue, 255 - satur, 180);
  currentPalette[7] = CHSV(hue, satur - 60, 225);

  fillNoiseLED();
  
  EffectMath::blur2d(fb, 100);
}

void EffectAquarium::fillNoiseLED() {
  uint8_t  dataSmoothing = 200 - (_speed * 4);
  for (uint8_t i = 0; i < fb.cfg.h(); i++) {
    int32_t ioffset = _scale * i;
    for (uint8_t j = 0; j < fb.cfg.w(); j++) {
      int32_t joffset = _scale * j;
      
      uint8_t data = inoise8(x + ioffset, y + joffset, z);
      
      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));
      noise.map_lxy(0, j, i) = scale8(noise.map_lxy(0, j, i), dataSmoothing) + scale8(data, 256 - dataSmoothing);
      fb.pixel(j, i) = ColorFromPalette(currentPalette, noise.map_lxy(0, j, i));
    }
  }
  z += _speed;
  x += _speed / 16 * sin8(millis() / 10000);
  y += _speed / 16 * cos8(millis() / 10000);
}

bool EffectAquarium::run() {
#ifdef MIC_EFFECTS
  byte _video = isMicOn() ? constrain(getMicMaxPeak() * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48U, 255U) : 255;
#else
  byte _video = 255;
#endif
  switch (glare) { //
  case 2:
    nGlare(_video);
    break;
  default:
    nDrops(_video);
  }

/*  абсолютно непонятная одноцветная заливка
  if (!glare) {// если блики выключены
    for (byte x = 0; x < WIDTH; x++)
    for (byte y = 0U; y < HEIGHT; y++)
    {
#ifdef MIC_EFFECTS
      if (isMicOn()) {
        hue = getMicMapFreq();
        EffectMath::drawPixelXY(x, y, CHSV((uint8_t)hue, satur, _video));
      }
      else
        EffectMath::drawPixelXY(x, y, CHSV((uint8_t)hue, satur, 255U));
#else
      EffectMath::drawPixelXY(x, y, CHSV((uint8_t)hue, satur, 255U));
#endif
    }
  }
*/
  if (speed == 1) {
    hue = scale;
  }
  else {
    hue += speedFactor;
  }

  return true;
}


// ------- Эффект "Звезды"
// !++
String EffectStar::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    _speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt()/380.0+0.05) * speedfactor;
    _speed = getCtrlVal(1).toInt();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectStar::load(){
  palettesload();    // подгружаем дефолтные палитры
  counter = 0.0;

  // стартуем с центра
  driftx = WIDTH/2.0;
  drifty = HEIGHT/2.0;

  cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation gives a float value between -1 and 1
  sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation in the y direction gives a float value between -1 and 1

  stars_count = WIDTH / 2U;
  if (stars_count > STARS_NUM) stars_count = STARS_NUM;
  for (uint8_t num = 0; num < stars_count; num++) {
    points[num] = random8(3, 9); // количество углов в звезде
    cntdelay[num] = _speed / 5 + (num << 2) + 1U; // задержка следующего пуска звезды
    color[num] = random8();
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
#ifdef MIC_EFFECTS
    EffectMath::drawLine( xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128),
                          isMicOn() ? CHSV(koler+getMicMapFreq(),255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255)) : ColorFromPalette(*curPalette, koler),
                          fb);
    EffectMath::drawLine( xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128),
                          isMicOn() ? CHSV(koler+getMicMapFreq(), 255-micPick, constrain(micPick * EffectMath::fmap(scale, 1.0f, 255.0f, 1.25f, 5.0f), 48, 255)) : ColorFromPalette(*curPalette, koler),
                          fb);

#else
    EffectMath::drawLine( xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128),
                          ColorFromPalette(*curPalette, koler),
                          fb);
    EffectMath::drawLine( xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128),
                          xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128),
                          ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128),
                          ColorFromPalette(*curPalette, koler),
                          fb);
#endif
  }
}

bool EffectStar::run() {

#ifdef MIC_EFFECTS
  micPick = getMicMaxPeak();
  fb.fade(255U - (isMicOn() ? micPick*2 : 90)); // работает быстрее чем dimAll
#else
  fb.fade(165);
#endif

  _speedFactor = ((float)speed/380.0+0.05);

  counter+=_speedFactor; // определяет то, с какой скоростью будет приближаться звезда

  if (driftx > (WIDTH - spirocenterX / 2U))//change directin of drift if you get near the right 1/4 of the screen
    cangle = 0 - fabs(cangle);
  if (driftx < spirocenterX / 2U)//change directin of drift if you get near the right 1/4 of the screen
    cangle = fabs(cangle);
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    driftx = driftx + (cangle * _speedFactor);//move the x center every so often

  if (drifty > ( HEIGHT - spirocenterY / 2U))// if y gets too big, reverse
    sangle = 0 - fabs(sangle);
  if (drifty < spirocenterY / 2U) // if y gets too small reverse
    sangle = fabs(sangle);
  //if ((counter + CENTER_DRIFT_SPEED / 2U) % CENTER_DRIFT_SPEED == 0)
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0)
    drifty =  drifty + (sangle * _speedFactor);//move the y center every so often

  for (uint8_t num = 0; num < stars_count; num++) {
    if (counter >= cntdelay[num])//(counter >= ringdelay)
    {
      if (counter - cntdelay[num] <= WIDTH + 5) {
        EffectStar::drawStar(driftx, drifty, 2 * (counter - cntdelay[num]), (counter - cntdelay[num]), points[num], STAR_BLENDER + color[num], color[num]);
        color[num]+=_speedFactor; // в зависимости от знака - направление вращения
      }
      else
        cntdelay[num] = counter + (stars_count << 1) + 1U; // задержка следующего пуска звезды
    }
  }
#ifdef MIC_EFFECTS
  EffectMath::blur2d(fb, isMicOn() ? micPick/2 : 30U); //fadeToBlackBy() сам блурит, уменьшил блур под микрофон
#else
  EffectMath::blur2d(fb, 30U);
#endif
  return true;
}

//---------- Эффект "Фейерверк"
//адаптация и переписал - kostyamat
//https://gist.github.com/jasoncoon/0cccc5ba7ab108c0a373
// !++
String EffectFireworks::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) {
    gDot.assign(EffectCalc::setDynCtrl(_val).toInt(), Dot());
    gDot.shrink_to_fit();
    sparkGen();
  }
  else if(_val->getId()==4) flashing = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}
/*
CRGB &Dot::piXY(LedFB &leds, byte x, byte y) {
  x -= PIXEL_X_OFFSET;
  y -= PIXEL_Y_OFFSET;
  return leds.pixel(x,y);
}
*/
void Dot::Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor, uint8_t dim)
  {
    yv = 0 + random16(1500) - 500;
    xv = basedv + random16(2000) - 1000;
    y = basey;
    x = basex;
    color = basecolor;
    //EffectMath::makeBrighter(color, 50);
    color *= dim; //50;
    theType = 2;
    show = 1;
  }

void Dot::GroundLaunch(DotsStore &store){
    yv = 600 + random16(400 + (25 * HEIGHT));
    if(yv > 1200) yv = 1200;
    xv = random16(600) - 300;
    y = 0;
    x = 0x8000;
    color = CHSV(0, 0, 130); // цвет запускаемого снаряда
    show = 1;
}

void Dot::Move(DotsStore &store, bool flashing){
    if( !show) return;
    yv -= gGravity;
    xv = scale15by8_local( xv, gDrag);
    yv = scale15by8_local( yv, gDrag);

    if( theType == 2) {
      xv = scale15by8_local( xv, gDrag);
      yv = scale15by8_local( yv, gDrag);
      color.nscale8( 255);
      if( !color) {
        show = 0;
      }
    }
    // if we'd hit the ground, bounce
    if( yv < 0 && (y < (-yv)) ) {
      if( theType == 2 ) {
        show = 0;
      } else {
        yv = -yv;
        yv = scale15by8_local( yv, gBounce);
        if( yv < 500 ) {
          show = 0;
        }
      }
    }
    if( (yv < -300) /* && (!(oyv < 0))*/ ) {
      // pinnacle
      if( theType == 1 ) {

        if( (y > (uint16_t)(0x8000)) && (random8() < 32) && flashing) {
          // boom
          LEDS.showColor( CRGB::Gray);
          LEDS.showColor( CRGB::Black);
        }

        show = 0;

        store.gSkyburst = true;
        store.gBurstx = x;
        store.gBursty = y;
        store.gBurstxv = xv;
        store.gBurstyv = yv;
        store.gBurstcolor = CRGB(random8(), random8(), random8());
      }
    }
    if( theType == 2) {
      if( ((xv >  0) && (x > xv)) ||
          ((xv < 0 ) && (x < (0xFFFF + xv))) )  {
        x += xv;
      } else {
        show = 0;
      }
    } else {
      x += xv;
    }
    y += yv;
}

void EffectFireworks::draw(Dot &d){
    if( !d.show) return;
    byte ix, xe, xc;
    byte iy, ye, yc;
    _screenscale( d.x, _model_w(), ix, xe);
    _screenscale( d.y, _model_h(), iy, ye);
    yc = 255 - ye;
    xc = 255 - xe;

    CRGB c00 = CRGB( dim8_video( scale8( scale8( d.color.r, yc), xc)),
                     dim8_video( scale8( scale8( d.color.g, yc), xc)),
                     dim8_video( scale8( scale8( d.color.b, yc), xc))
                     );
    CRGB c01 = CRGB( dim8_video( scale8( scale8( d.color.r, ye), xc)),
                     dim8_video( scale8( scale8( d.color.g, ye), xc)),
                     dim8_video( scale8( scale8( d.color.b, ye), xc))
                     );

    CRGB c10 = CRGB( dim8_video( scale8( scale8( d.color.r, yc), xe)),
                     dim8_video( scale8( scale8( d.color.g, yc), xe)),
                     dim8_video( scale8( scale8( d.color.b, yc), xe))
                     );
    CRGB c11 = CRGB( dim8_video( scale8( scale8( d.color.r, ye), xe)),
                     dim8_video( scale8( scale8( d.color.g, ye), xe)),
                     dim8_video( scale8( scale8( d.color.b, ye), xe))
                     );

    fb.pixel(ix - _x_offset(), iy - _y_offset()) += c00;          //piXY(leds, ix, iy) 
    fb.pixel(ix - _x_offset(), iy + 1 - _y_offset()) += c01;      //piXY(leds, ix, iy + 1) += c01;
    fb.pixel(ix + 1 - _x_offset(), iy - _y_offset()) += c10;      //piXY(leds, ix + 1, iy) += c10;
    fb.pixel(ix + 1 - _x_offset(), iy + 1 - _y_offset()) += c11;  //piXY(leds, ix + 1, iy + 1) += c11;
}

bool EffectFireworks::run()
{
  random16_add_entropy(millis());
  dim = beatsin8(100, 20, 100);
  EVERY_N_MILLISECONDS(EFFECTS_RUN_TIMER * 10) {
    valDim = random8(25, 50);
  }
  //EVERY_N_MILLISECONDS(10) {    // WHY?
    return fireworksRoutine();
  //}
  //return false;
}

void EffectFireworks::sparkGen() {
  for (auto &c : gDot){
    if( c.show == 0 ) {
      if( c.cntdown == 0) {
        c.GroundLaunch(store);
        c.theType = 1;
        c.cntdown = random16(1200 - speed*4) + 1;
      } else {
        --c.cntdown;
      }
    }
  }

  if( store.gSkyburst) {
    byte nsparks = random8( gSparks.size() / 2, gSparks.size() - 1);
    for( byte b = 0; b < nsparks; b++) {
      store.gBurstcolor = CHSV(random8(), 200, 100);
      gSparks[b].Skyburst( store.gBurstx, store.gBursty, store.gBurstyv, store.gBurstcolor, dim);
      store.gSkyburst = false;
    }
  }
  //EffectMath::blur2d(fb, 20);
}

bool EffectFireworks::fireworksRoutine()
{

  fb.fade(valDim);
  sparkGen();

  for (auto &a : gDot){
    a.Move(store, flashing);
    draw(a);
  }

  for( auto &b : gSparks) {
    b.Move(store, flashing);
    draw(b);
  }
    return true;
}

void EffectFireworks::_screenscale(accum88 a, byte N, byte &screen, byte &screenerr){
  byte ia = a >> 8;
  screen = scale8(ia, N);
  byte m = screen * (256 / N);
  screenerr = (ia - m) * scale8(255, N);
}

// ------------ Эффект "Тихий Океан"
//  "Pacifica" перенос кода kostyamat
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino

// Add one layer of waves into the led array
void EffectPacific::pacifica_one_layer(const TProgmemRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < num_leds; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    fb[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void EffectPacific::pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for( uint16_t i = 0; i < num_leds; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = fb[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      fb[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void EffectPacific::pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < num_leds; i++) {
    fb[i].blue = scale8( fb[i].blue,  145);
    fb[i].green= scale8( fb[i].green, 200);
    fb[i] |= CRGB( 2, 5, 7);
  }
}

// !++
String EffectPacific::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1)  speedFactor = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 620, 60)*EffectCalc::speedfactor;
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPacific::run()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = millis();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedFactor1 = beatsin16(3, 179, 269);
  uint16_t speedFactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedFactor1) / speedFactor;
  uint32_t deltams2 = (deltams * speedFactor2) / speedFactor;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( fb.data(), fb.size(), CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer(pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer(pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer(pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer(pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();

  EffectMath::blur2d(fb, 20);
  return true;
}

#ifdef MIC_EFFECTS
//----- Эффект "Осциллограф" (c) kostyamat
// !++
String EffectOsc::setDynCtrl(UIControl*_val) {
#ifdef ESP32
  pointer = 4096/(getMicScale()*2);
#else
  pointer = 1024/(getMicScale()*2);
#endif
  if(_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    if (speed <= 127) {
      div = EffectMath::fmap(speed, 1, 127, 0.5, 4);
      oscHV = HEIGHT;
      oscilLimit = WIDTH;
    } else{
      div = EffectMath::fmap(speed, 128, 255, 0.5, 4);
      oscHV = WIDTH;
      oscilLimit = HEIGHT;
    }
  }
  //else if(_val->getId()==2) scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) gain = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectOsc::run() {
  if((millis() - lastrun ) <= (isMicOn() ? 15U : map(speed, speed <= 127 ? 1 : 128, speed <= 12 ? 128 : 255, 15, 60))) 
    return false;
  else {
    lastrun = millis();
  }
  //fb.fade(200);
  FastLED.clear();

  if (scale == 1) {
    byte micPick = (isMicOn()? getMicMaxPeak() : random8(200));
    color = CHSV((isMicOn()? getMicFreq() : random(240)), 255, scale == 1 ? 100 : constrain(micPick * EffectMath::fmap(gain, 1., 255., 1., 5.), 51, 255));
  }
  else if (scale == 255)
    color = CHSV(0, 0, 255);
  else 
    color = CHSV(scale, 255, 255);

  for (float x = 0.; x < oscHV; x += div) {
    if (speed < 128)
      EffectMath::drawLineF(y[0], x, y[1], (x + div), color, fb);
    else
      EffectMath::drawLineF(x, y[0], (x + div), y[1], color, fb);

    y[0] = y[1];
    y[1] = EffectMath::fmap(
                          (isMicOn() ? analogRead(MIC_PIN) :  EffectMath::randomf(pointer - gain, pointer + gain)),
                          gain,
                          pointer * 2. - gain,
                          0., 
                          oscilLimit - 1);
    delayMicroseconds((uint16_t)(1024.0 * div));

  }

return true;
}
#endif

// ------ Эффект "Вышиванка" (с) проект Aurora "Munch"
void EffectMunch::load() {
  palettesload();
}

// !++
String EffectMunch::setDynCtrl(UIControl*_val){
  if(_val->getId()==4) rand = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  flag = rand ? false : true;
  return String();
}

bool EffectMunch::run() {
   if (dryrun(2.0))
    return false;
  return munchRoutine();
}

bool EffectMunch::munchRoutine() {
  //fb.fade(200); EffectMath::setPixel(
  if (flag) rand = beat8(5)/32; // Хрень, конечно, но хоть какое-то разнообразие.
  CRGB color;
  for (uint8_t x = 0; x < minDimLocal; x++) {
    for (uint8_t y = 0; y < minDimLocal; y++) {
      color = (x ^ y ^ flip) < count ? ColorFromPalette(*curPalette, ((x ^ y) << rand) + generation) : CRGB::Black;
      if (x < WIDTH and y < HEIGHT) fb.pixel(x, y) = color;
      if (x + minDimLocal < WIDTH and y < HEIGHT) fb.pixel(x + minDimLocal, y) = color;
      if (y + minDimLocal < HEIGHT and x < WIDTH) fb.pixel(x, y + minDimLocal) = color;
      if (x + minDimLocal < WIDTH and y + minDimLocal < HEIGHT) fb.pixel(x + minDimLocal, y + minDimLocal) = color;
      
    }
  }

  count += dir;

  if (count <= 0 || count >= mic[0]) {
    dir = -dir;
    if (count <= 0) {
      mic[0] = mic[1];
      if (flip == 0)
        flip = mic[1] - 1;
      else
        flip = 0;
    }
  }

  generation++;
#ifdef MIC_EFFECTS
  mic[1] = isMicOn() ? map(getMicMapMaxPeak(), 0, 255, 0, minDimLocal) : minDimLocal;
#else
  mic[1] = minDimLocal;
#endif
  return true;
}

// ------ Эффект "Цветной шум" (с) https://gist.github.com/StefanPetrick/c856b6d681ec3122e5551403aabfcc68
DEFINE_GRADIENT_PALETTE( pit ) {
  0,     3,   3,   3,
  64,   13,   13, 255,  //blue
  128,   3,   3,   3,
  192, 255, 130,   3 ,  //orange
  255,   3,   3,   3
};

// !++
String EffectNoise::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 2, 16 );
  else if(_val->getId()==4) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectNoise::run() {
  fb.dim(200U);
    uint8_t layer = 0;

  CRGBPalette16 Pal( pit );

  //modulate the position so that it increases/decreases x
  //(here based on the top left pixel - it could be any position else)
  //the factor "2" defines the max speed of the x movement
  //the "-255" defines the median moving direction
  x[layer] = x[layer] + (noise[layer][0][0] * speedFactor) - 255U;
  //modulate the position so that it increases/decreases y
  //(here based on the top right pixel - it could be any position else)
  y[layer] = y[layer] + (noise[layer][fb.cfg.maxWidthIndex()][0] * speedFactor) - 255U;
  //z just in one direction but with the additional "1" to make sure to never get stuck
  //in case the movement is stopped by a crazy parameter (noise data) combination
  //(here based on the down left pixel - it could be any position else)
  z[layer] += 1 + ((noise[layer][0][fb.cfg.maxHeightIndex()]) / 4);
  //set the scaling based on left and right pixel of the middle line
  //here you can set the range of the zoom in both dimensions
  scale_x[layer] = 8000 + (noise[layer][0][CentreY] * 16);
  scale_y[layer] = 8000 + (noise[layer][fb.cfg.maxWidthIndex()][CentreY] * 16);

  //calculate the noise data
  for (uint8_t i = 0; i < WIDTH; i++) {
    uint32_t ioffset = scale_x[layer] * (i - CentreX);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      uint32_t joffset = scale_y[layer] * (j - CentreY);
      uint16_t data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]);
      // limit the 16 bit results to the interesting range
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      // normalize
      data = data - 11000;
      // scale down that the result fits into a byte
      data = data / 161;
      // store the result in the array
      noise[layer][i][j] = data;
    }
  }

  //map the colors
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      //I will add this overlay CRGB later for more colors
      //it´s basically a rainbow mapping with an inverted brightness mask
      CRGB overlay;
      if (palettepos == 14) overlay = CHSV(160,255 - noise[layer][x][y], noise[layer][fb.cfg.maxWidthIndex()][fb.cfg.maxHeightIndex()] + noise[layer][x][y]);
      else overlay = CHSV(noise[layer][y][x], 255, noise[layer][x][y]);
      //here the actual colormapping happens - note the additional colorshift caused by the down right pixel noise[layer][15][15]
      if (palettepos == 4) EffectMath::drawPixelXYF(x, fb.cfg.maxHeightIndex() - y, CHSV(160, 0 , noise[layer][x][y]), fb, 35);
      else fb.pixel(x, y) = ColorFromPalette(palettepos > 0 ? *curPalette : Pal, noise[layer][fb.cfg.maxWidthIndex()][fb.cfg.maxHeightIndex()] + noise[layer][x][y]) + overlay;
    }
  }

  //make it looking nice
  if (palettepos != 4) {
    if (type) EffectMath::nightMode(fb);
    //else EffectMath::gammaCorrection();   // why need this additional gamma correction, if brigtness already adjusted with dim8*?
    EffectMath::blur2d(fb, 32);
  } else EffectMath::blur2d(fb, 48);
  //and show it!
 return true;
}

void EffectNoise::load() {
  palettesload();
}

// ---- Эффект "Мотыльки"
// (с) Сотнег, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
void EffectButterfly::load()
{
  for (uint8_t i = 0U; i < BUTTERFLY_MAX_COUNT; i++)
  {
    butterflysPosX[i] = random8(WIDTH);
    butterflysPosY[i] = random8(HEIGHT);
    butterflysSpeedX[i] = 0;
    butterflysSpeedY[i] = 0;
    butterflysTurn[i] = 0;
    butterflysColor[i] = (isColored) ? random8() : 0U;
    butterflysBrightness[i] = 255U;
  }
}

// !++
String EffectButterfly::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 2048.0 + 0.01) *EffectCalc::speedfactor;
  else if(_val->getId()==3) cnt = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) wings = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) {
    isColored = EffectCalc::setDynCtrl(_val).toInt();
    isColored = !isColored;

    for (uint8_t i = 0U; i < BUTTERFLY_MAX_COUNT; i++)
    {
      butterflysColor[i] = (isColored) ? random8() : 0U;
    }
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectButterfly::run()
{
  byte _scale = cnt;

  if (isColored) // для режима смены цвета фона фиксируем количество мотыльков
    deltaValue = (_scale > BUTTERFLY_MAX_COUNT) ? BUTTERFLY_MAX_COUNT : _scale;
  else
    deltaValue = BUTTERFLY_FIX_COUNT;

  if (_scale != 1U and !isColored)
    hue = map(_scale, 2, BUTTERFLY_MAX_COUNT + 1U, 0, 255);
  hue2 = (_scale == 1U) ? 100U : 190U; // вычисление базового оттенка

  if (wings && isColored)
    fb.fade(200);
  else
    FastLED.clear();

  float maxspeed;
  uint8_t tmp;
  if (++step >= deltaValue)
    step = 0U;
  for (uint8_t i = 0U; i < deltaValue; i++)
  {
    butterflysPosX[i] += butterflysSpeedX[i]*speedFactor;
    butterflysPosY[i] += butterflysSpeedY[i]*speedFactor;

    if (butterflysPosX[i] < 0)
      butterflysPosX[i] = (float)fb.cfg.maxWidthIndex() + butterflysPosX[i];
    if (butterflysPosX[i] > fb.cfg.maxWidthIndex())
      butterflysPosX[i] = butterflysPosX[i] + 1 - WIDTH;

    if (butterflysPosY[i] < 0)
    {
      butterflysPosY[i] = -butterflysPosY[i];
      butterflysSpeedY[i] = -butterflysSpeedY[i];
      //butterflysSpeedX[i] = -butterflysSpeedX[i];
    }
    if (butterflysPosY[i] > fb.cfg.maxHeightIndex())
    {
      butterflysPosY[i] = (HEIGHT << 1U) - 2U - butterflysPosY[i];
      butterflysSpeedY[i] = -butterflysSpeedY[i];
      //butterflysSpeedX[i] = -butterflysSpeedX[i];
    }

    //проворот траектории
    maxspeed = fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]); // максимальная суммарная скорость
    if (maxspeed == fabs(butterflysSpeedX[i] + butterflysSpeedY[i]))
      {
          if (butterflysSpeedX[i] > 0) // правый верхний сектор вектора
          {
            butterflysSpeedX[i] += butterflysTurn[i];
            if (butterflysSpeedX[i] > maxspeed) // если вектор переехал вниз
              {
                butterflysSpeedX[i] = maxspeed + maxspeed - butterflysSpeedX[i];
                butterflysSpeedY[i] = butterflysSpeedX[i] - maxspeed;
              }
            else
              butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
          }
          else                           // левый нижний сектор
          {
            butterflysSpeedX[i] -= butterflysTurn[i];
            if (butterflysSpeedX[i] + maxspeed < 0) // если вектор переехал вверх
              {
                butterflysSpeedX[i] = 0 - butterflysSpeedX[i] - maxspeed - maxspeed;
                butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
              }
            else
              butterflysSpeedY[i] = fabs(butterflysSpeedX[i]) - maxspeed;
          }
      }
    else //левый верхний и правый нижний секторы вектора
      {
          if (butterflysSpeedX[i] > 0) // правый нижний сектор
          {
            butterflysSpeedX[i] -= butterflysTurn[i];
            if (butterflysSpeedX[i] > maxspeed) // если вектор переехал наверх
              {
                butterflysSpeedX[i] = maxspeed + maxspeed - butterflysSpeedX[i];
                butterflysSpeedY[i] = maxspeed - butterflysSpeedX[i];
              }
            else
              butterflysSpeedY[i] = fabs(butterflysSpeedX[i]) - maxspeed;
          }
          else                           // левый верхний сектор
          {
            butterflysSpeedX[i] += butterflysTurn[i];
            if (butterflysSpeedX[i] + maxspeed < 0) // если вектор переехал вниз
              {
                butterflysSpeedX[i] = 0 - butterflysSpeedX[i] - maxspeed - maxspeed;
                butterflysSpeedY[i] = 0 - butterflysSpeedX[i] - maxspeed;
              }
            else
              butterflysSpeedY[i] = maxspeed - fabs(butterflysSpeedX[i]);
          }
      }

    if (butterflysBrightness[i] == 255U)
    {
      if (step == i && random8(2U) == 0U)//(step == 0U && ((pcnt + i) & 0x01))
      {
        butterflysBrightness[i] = random8(220U,244U);
        butterflysSpeedX[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) butterflysSpeedX[i] = -butterflysSpeedX[i];
        butterflysSpeedY[i] = (float)random8(101U) / 20.0f + 1.0f;
        if (random8(2U) == 0U) butterflysSpeedY[i] = -butterflysSpeedY[i];
        // проворот траектории
        //butterflysTurn[i] = (float)random8((fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]))*2.0+2.0) / 40.0f;
        butterflysTurn[i] = (float)random8((fabs(butterflysSpeedX[i])+fabs(butterflysSpeedY[i]))*20.0f+2.0f) / 200.0f;
        if (random8(2U) == 0U) butterflysTurn[i] = -butterflysTurn[i];
      }
    }
    else
    {
      if (step == i)
        butterflysBrightness[i]++;
      tmp = 255U - butterflysBrightness[i];
      if (tmp == 0U || ((uint16_t)(butterflysPosX[i] * tmp) % tmp == 0U && (uint16_t)(butterflysPosY[i] * tmp) % tmp == 0U))
      {
        butterflysPosX[i] = round(butterflysPosX[i]);
        butterflysPosY[i] = round(butterflysPosY[i]);
        butterflysSpeedX[i] = 0;
        butterflysSpeedY[i] = 0;
        butterflysTurn[i] = 0;
        butterflysBrightness[i] = 255U;
      }
    }

    if (wings)
      EffectMath::drawPixelXYF(butterflysPosX[i], butterflysPosY[i], CHSV(butterflysColor[i], 255U, (butterflysBrightness[i] == 255U) ? 255U : 128U + random8(2U) * 111U), fb);
    else
      EffectMath::drawPixelXYF(butterflysPosX[i], butterflysPosY[i], CHSV(butterflysColor[i], 255U, butterflysBrightness[i]), fb);
  }

  // постобработка кадра
  if (isColored){
    for (uint8_t i = 0U; i < deltaValue; i++) // ещё раз рисуем всех Мотыльков, которые "сидят на стекле"
      if (butterflysBrightness[i] == 255U)
        fb.pixel(butterflysPosX[i], butterflysPosY[i]) = CHSV(butterflysColor[i], 255U, butterflysBrightness[i]);
  }
  else {
    //теперь инверсия всей матрицы
    if (_scale == 1U)
      if (++deltaHue == 0U) hue++;
    for (uint16_t i = 0U; i < num_leds; i++)
      fb[i] = CHSV(hue, hue2, 255U - fb[i].r);
  }
  return true;
}

// ---- Эффект "Тени"
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/effects.ino
bool EffectShadows::run() {

  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 225), (40 * 256));
#ifdef MIC_EFFECTS
  uint8_t msmultiplier = isMicOn() ? getMicMapMaxPeak() : getCtrlVal(3) == "false" ? beatsin88(map(speed, 1, 255, 100, 255), 32, map(speed, 1, 255, 60, 255)) : speed; // beatsin88(147, 32, 60);
  byte effectBrightness = isMicOn() ? getMicMapMaxPeak() * 1.5f : scale;
#else
  uint8_t msmultiplier = getCtrlVal(3) == "false" ? beatsin88(map(speed, 1, 255, 100, 255), 32, map(speed, 1, 255, 60, 255)) : speed; // beatsin88(147, 32, 60);
  byte effectBrightness = scale;
#endif
  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;

  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for( uint16_t i = 0 ; i < num_leds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768U;

    uint32_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536U;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536U;
    bri8 += (255 - brightdepth);

    nblend(fb[fb.size()-1-i], CHSV( hue8, sat8, map8(bri8, map(effectBrightness, 1, 255, 32, 125), map(effectBrightness, 1, 255, 125, 250))), 64);
  }
  return true;
}

// ---- Эффект "Узоры"
// (c) kostyamat (Kostyantyn Matviyevskyy) 2020
// переделано kDn
// идея https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/patterns.ino
String EffectPatterns::setDynCtrl(UIControl*_val) {
  if(_val->getId()==3) _speed = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) _sc = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) _subpixel = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==7) _sinMove = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPatterns::run() {
  return patternsRoutine();
}

void EffectPatterns::drawPicture_XY() {
  float vx, vy, f;
  vx = modff(xsin, &f);
  vy = modff(ysin, &f);

  fb.dim(127);

  for (int16_t y = -1; y < fb.cfg.h(); y++){
    for (int16_t x = -1; x < fb.cfg.h(); x++){

      auto &in = buff[abs((int)(ysin + y)) % PATTERNS_BUFFSIZE] [abs((int)(xsin + x)) % PATTERNS_BUFFSIZE];
      CHSV color2 = colorMR[in]; // CHSV(HUE_BLUE, 255, 255);

      if(_subpixel){
        if(!_speed)
          EffectMath::drawPixelXYF_X(((float)x-vx), (float)((float)y-vy), color2, fb, 0);
        else if(!_scale)
          EffectMath::drawPixelXYF_Y(((float)x-vx), (float)((float)y-vy), color2, fb, 0);
        else{
            EffectMath::drawPixelXYF(((float)x-vx), (float)((float)y-vy), color2, fb, 0);
        }
      } else {
        fb.pixel(x, y) = color2;
      }
    }
  }
}

void EffectPatterns::load() {
  if (_sc == 0)
    patternIdx = random(0, MAX_PATTERN);
   // Цвета с индексом 6 и 7 - случайные, определяются в момент настройки эффекта
  colorMR[6] = CHSV(random8(), 255U, 255U);
  colorMR[7].hue = colorMR[6].hue + 96; //(beatsin8(1, 0, 255, 0, 127), 255U, 255U);

  // this is ugly, could use 4 times less buffer for sprite
  for (byte y = 0; y < PATTERNS_BUFFSIZE; y++){
    for (byte x = 0; x < PATTERNS_BUFFSIZE; x++){
      buff[y][x] = pgm_read_byte(&patterns[patternIdx][y % 10U][x % 10U]);
    }
  }
}

bool EffectPatterns::patternsRoutine()
{
  _speedX = EffectMath::fmap(_scale, -32, 32, 0.75, -0.75);
  _speedY = EffectMath::fmap(_speed, -32, 32, 0.75, -0.75);

  if(!_sinMove){
    xsin += _speedX;
    ysin += _speedY;
  } else {
    xsin = float(beatsin16(5, 0, abs(_scale)*30)) /10; // for X and Y texture move
    ysin = float(beatsin16(6, 0, abs(_speed)*30))/10; // for X and Y texture move
  }
  int8_t chkIdx = patternIdx;
  if (_sc == 0) {
    EVERY_N_SECONDS(10) {
      patternIdx ++;
      if (patternIdx >= MAX_PATTERN) patternIdx = 0;
    }
  } else patternIdx = _sc%(sizeof(patterns)/sizeof(Pattern));

  if(chkIdx != patternIdx){
    for (byte y = 0; y < PATTERNS_BUFFSIZE; y++)
      for (byte x = 0; x < PATTERNS_BUFFSIZE; x++)
        buff[y][x] = pgm_read_byte(&patterns[patternIdx][y % 10U][x % 10U]);
  }

  double corr = fabs(_speedX) + fabs(_speedY);

  colorMR[6] = CHSV(beatsin88(EffectMath::fmap(corr, 0.1, 1.5, 350., 1200.), 0, 255), 255, 255);
  colorMR[7].hue = colorMR[6].hue + 96; 
  colorMR[7].sat = beatsin88(EffectMath::fmap(corr, 0.1, 1.5, 150, 900), 0, 255);
  colorMR[7].val = beatsin88(EffectMath::fmap(corr, 0.1, 1.5, 450, 1300), 0, 255);
  drawPicture_XY();

  return true;
}

// ***************************** "Стрелки" *****************************
void EffectArrows::load(){
    FastLED.clear();
    arrow_complete = false;
    arrow_mode_orig = _scale-1;
    arrow_mode = arrow_mode_orig == 0 ? random8(1,5) : arrow_mode_orig;
    arrow_play_mode_count_orig[0] = 0;
    arrow_play_mode_count_orig[1] = 4;  // 4 фазы - все стрелки показаны по кругу один раз - переходить к следующему ->
    arrow_play_mode_count_orig[2] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[3] = 4;  // 1 фаза - все к центру (1) повторить по 4 раза -> 4
    arrow_play_mode_count_orig[4] = 4;  // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[5] = 4;  // 1 фаза - все сразу (1) повторить по 4 раза -> 4
    for (byte i=0; i<6; i++) {
      arrow_play_mode_count[i] = arrow_play_mode_count_orig[i];
    }
    arrowSetupForMode(arrow_mode, true);
}

// !++
String EffectArrows::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) { speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt() / 768.0 + 0.15)*EffectCalc::speedfactor; }
  else if(_val->getId()==3) { _scale = EffectCalc::setDynCtrl(_val).toInt(); load();}
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectArrows::run() {
  if (_scale == 1) {
    EVERY_N_SECONDS((3000U / speed))
    {
      load();
    }
  }

  FastLED.clear();

  CHSV color;
  // движение стрелки - cлева направо
  if ((arrow_direction & 0x01) > 0) {
    color = CHSV(arrow_hue[0], 255, 255);
    for (float x = 0; x <= 7; x+=0.33) {
      for (byte y = 0; y <= x; y++) {
        if (arrow_x[0] - x >= 0 && arrow_x[0] - x <= stop_x[0]) {
          CHSV clr = (x < 4 || (x >= 4 && y < 2)) ? color : CHSV(0,0,0);
          EffectMath::sDrawPixelXYF_X(arrow_x[0] - x, arrow_y[0] - y, clr);
          EffectMath::sDrawPixelXYF_X(arrow_x[0] - x, arrow_y[0] + y, clr);
        }
      }
    }
    arrow_x[0]+= speedFactor;
  }

  // движение стрелки - cнизу вверх
  if ((arrow_direction & 0x02) > 0) {
    color = CHSV(arrow_hue[1], 255, 255);
    for (float y = 0; y <= 7; y+=0.33) {
      for (byte x = 0; x <= y; x++) {
        if (arrow_y[1] - y >= 0 && arrow_y[1] - y <= stop_y[1]) {
          CHSV clr = (y < 4 || (y >= 4 && x < 2)) ? color : CHSV(0,0,0);
          EffectMath::sDrawPixelXYF_Y(arrow_x[1] - x, arrow_y[1] - y, clr);
          EffectMath::sDrawPixelXYF_Y(arrow_x[1] + x, arrow_y[1] - y, clr);
        }
      }
    }
    arrow_y[1]+= speedFactor;
  }

  // движение стрелки - cправа налево
  if ((arrow_direction & 0x04) > 0) {
    color = CHSV(arrow_hue[2], 255, 255);
    for (float x = 0; x <= 7; x+=0.33) {
      for (byte y = 0; y <= x; y++) {
        if (arrow_x[2] + x >= stop_x[2] && arrow_x[2] + x < WIDTH) {
          CHSV clr = (x < 4 || (x >= 4 && y < 2)) ? color : CHSV(0,0,0);
          EffectMath::sDrawPixelXYF_X(arrow_x[2] + x, arrow_y[2] - y, clr);
          EffectMath::sDrawPixelXYF_X(arrow_x[2] + x, arrow_y[2] + y, clr);
        }
      }
    }
    arrow_x[2]-= speedFactor;
  }

  // движение стрелки - cверху вниз
  if ((arrow_direction & 0x08) > 0) {
    color = CHSV(arrow_hue[3], 255, 255);
    for (float y = 0; y <= 7; y+=0.33) {
      for (byte x = 0; x <= y; x++) {
        if (arrow_y[3] + y >= stop_y[3] && arrow_y[3] + y < HEIGHT) {
          CHSV clr = (y < 4 || (y >= 4 && x < 2)) ? color : CHSV(0,0,0);
          EffectMath::sDrawPixelXYF_Y(arrow_x[3] - x, arrow_y[3] + y, clr);
          EffectMath::sDrawPixelXYF_Y(arrow_x[3] + x, arrow_y[3] + y, clr);
        }
      }
    }
    arrow_y[3]-= speedFactor;
  }

  // Проверка завершения движения стрелки, переход к следующей фазе или режиму

  switch (arrow_mode) {

    case 1:
      // Последовательно - слева-направо -> снизу вверх -> справа налево -> сверху вниз и далее по циклу
      // В каждый сомент времени сктивна только одна стрелка, если она дошла до края - переключиться на следующую и задать ее начальные координаты
      arrow_complete = false;
      switch (arrow_direction) {
        case 1: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 2: arrow_complete = arrow_y[1] > stop_y[1]; break;
        case 4: arrow_complete = arrow_x[2] < stop_x[2]; break;
        case 8: arrow_complete = arrow_y[3] < stop_y[3]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = (arrow_direction << 1) & 0x0F;
        if (arrow_direction == 0) arrow_direction = 1;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[1]--;
          if (arrow_play_mode_count[1] == 0) {
            arrow_play_mode_count[1] = arrow_play_mode_count_orig[1];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 2:
      // Одновременно горизонтальные навстречу до половины экрана
      // Затем одновременно вертикальные до половины экрана. Далее - повторять
      arrow_complete = false;
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа встречаются в центре одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху встречаются в центре одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[2]--;
          if (arrow_play_mode_count[2] == 0) {
            arrow_play_mode_count[2] = arrow_play_mode_count_orig[2];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 3:
      // Одновременно со всех сторон к центру
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[3]--;
          if (arrow_play_mode_count[3] == 0) {
            arrow_play_mode_count[3] = arrow_play_mode_count_orig[3];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 4:
      // Одновременно слева/справа от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // одновременно снизу/сверху от края до края со смещением вертикальной оси на 1/3 ширины
      // Завершение кадра режима - когда все стрелки собрались в центре.
      // Проверять стрелки по самой длинной стороне
      switch (arrow_direction) {
        case  5: arrow_complete = arrow_x[0] > stop_x[0]; break;   // Стрелка слева и справа движутся и достигают края одновременно - проверять только стрелку слева
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;   // Стрелка снизу и сверху движутся и достигают края одновременно - проверять только стрелку снизу
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[4]--;
          if (arrow_play_mode_count[4] == 0) {
            arrow_play_mode_count[4] = arrow_play_mode_count_orig[4];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 5:
      // Одновременно со всех сторон от края до края со смещением горизонтальной оси на 1/3 высоты, далее
      // Проверять стрелки по самой длинной стороне
      if (WIDTH >= HEIGHT)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[5]--;
          if (arrow_play_mode_count[5] == 0) {
            arrow_play_mode_count[5] = arrow_play_mode_count_orig[5];
            arrow_mode = random8(1,5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;
  }
  return true;
}

void EffectArrows::arrowSetupForMode(byte mode, bool change) {
    switch (mode) {
      case 1:
        if (change) arrow_direction = 1;
        arrowSetup_mode1();    // От края матрицы к краю, по центру гориз и верт
        break;
      case 2:
        if (change) arrow_direction = 5;
        arrowSetup_mode2();    // По центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 3:
        if (change) arrow_direction = 15;
        arrowSetup_mode2();    // как и в режиме 2 - по центру матрицы (гориз / верт) - ограничение - центр матрицы
        break;
      case 4:
        if (change) arrow_direction = 5;
        arrowSetup_mode4();    // От края матрицы к краю, верт / гориз
        break;
      case 5:
        if (change) arrow_direction = 15;
        arrowSetup_mode4();    // как и в режиме 4 от края матрицы к краю, на 1/3
        break;
    }
}
void EffectArrows::arrowSetup_mode1() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = fb.cfg.maxWidthIndex();
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = fb.cfg.maxHeightIndex();
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо
  }
}

void EffectArrows::arrowSetup_mode2() {
  // Слева направо до половины экрана
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = HEIGHT / 2;
    stop_x [0] = WIDTH / 2 - 1;  // до центра экрана
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх до половины экрана
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = WIDTH / 2;
    stop_y [1] = HEIGHT / 2 - 1; // до центра экрана
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево до половины экрана
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = fb.cfg.maxWidthIndex();
    arrow_y[2] = HEIGHT / 2;
    stop_x [2] = WIDTH / 2;      // до центра экрана
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз до половины экрана
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = fb.cfg.maxHeightIndex();
    arrow_x[3] = WIDTH / 2;
    stop_y [3] = HEIGHT / 2;     // до центра экрана
    stop_x [3] = 0;              // неприменимо
  }
}

void EffectArrows::arrowSetup_mode4() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = (HEIGHT / 3) * 2;
    stop_x [0] = WIDTH + 7;      // скрывается за экраном на 7 пикселей
    stop_y [0] = 0;              // неприменимо
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = (WIDTH / 3) * 2;
    stop_y [1] = HEIGHT + 7;     // скрывается за экраном на 7 пикселей
    stop_x [1] = 0;              // неприменимо
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = fb.cfg.maxWidthIndex();
    arrow_y[2] = HEIGHT / 3;
    stop_x [2] = -7;             // скрывается за экраном на 7 пикселей
    stop_y [2] = 0;              // неприменимо
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = fb.cfg.maxHeightIndex();
    arrow_x[3] = WIDTH / 3;
    stop_y [3] = -7;             // скрывается за экраном на 7 пикселей
    stop_x [3] = 0;              // неприменимо
  }
}

// ------ Эффект "Дикие шарики"
// (с) https://gist.github.com/bonjurroughs/9c107fa5f428fb01d484#file-noise-balls
// !++
String EffectNBals::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) beat1 = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 8, 128);
  else if(_val->getId()==3) beat2 = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectNBals::run() {
  balls_timer();
  blur();
  return true;
}

void EffectNBals::blur() {
  EffectMath::blur2d(fb, beatsin8(2, 0, 60));
  // Use two out-of-sync sine waves
  uint8_t  i = beatsin8( beat1, 0, fb.cfg.maxWidthIndex());
  uint8_t  j = beatsin8(fabs(beat1 - beat2), 0, fb.cfg.maxHeightIndex());
  // Also calculate some reflections
  uint8_t ni = fb.cfg.maxWidthIndex() - i;
  uint8_t nj = fb.cfg.maxHeightIndex() - j;
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();

  switch(balls){
  case 1:
    fb.pixel(ni, nj) += CHSV( ms / 17, 200, 255);
    break;
  case 3:
    fb.pixel(ni, nj) += CHSV( ms / 17, 200, 255);
    fb.pixel(ni, j) += CHSV( ms / 41, 200, 255);
    break;
  case 4:
    fb.pixel(ni, nj) += CHSV( ms / 17, 200, 255);
    fb.pixel(ni, j) += CHSV( ms / 41, 200, 255);
    fb.pixel( i,nj) += CHSV( ms / 37, 200, 255);
    fb.pixel( i, j) += CHSV( ms / 11, 200, 255);
    break;
  case 2:
    fb.pixel(ni, j) += CHSV( ms / 41, 200, 255);
    fb.pixel( i, j) += CHSV( ms / 13, 200, 255);
    break;
  }
}

void EffectNBals::balls_timer() {
  uint8_t secondHand = ((millis() / 1000)) % 60;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;

    if(( secondHand == 30)||( secondHand == 0))  {
      balls += 1;
      if(balls > 4) {
        balls = 1;
      }
    }
  }
}

// ------ Эффект "Притяжение"
// Базовый Attract проект Аврора
void EffectAttract::load() {
  palettesload();
  //speedFactor = EffectMath::fmap((float)speed, 1., 255., 0.02*EffectCalc::speedfactor, 1.*EffectCalc::speedfactor);
  for (int i = 0; i < count; i++)
  {
    int direction = 1-2*random(0, 2); // -1 или 1
    Boid boid = Boid(15, 16 - i);
    boid.mass = (float)random(1, map(_mass, 1, 255, 128, 1024)) / 100.0f * speedFactor; //(1.0/speed);
    boid.velocity.x = (float)random(5, map(_energy, 1, 255, 16, 768)) / 500.0f; 
    boid.velocity.x *= direction;
    boid.velocity.y = 0;
    boid.colorIndex = i * 32;
    boid.location.x = EffectMath::randomf(0, fb.cfg.maxWidthIndex()); 
    boid.location.y = EffectMath::randomf(0, fb.cfg.maxHeightIndex()); 
    boids[i] = boid;
  }

}

// !++
String EffectAttract::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap((float)EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.02, 1.) * EffectCalc::speedfactor;
  else if(_val->getId()==2) _energy = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) _mass = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  setup();
  return String();
}

void EffectAttract::setup(){
  for (int i = 0; i < count; i++)
  {
    int direction = 1-2*random(0, 2); // -1 или 1
    Boid boid = Boid(15, 16 - i);
    boid.mass = (float)random(1, map(_mass, 1, 255, 128, 1024)) / 100.0f * speedFactor; //(1.0/speed);
    boid.velocity.x = (float)random(5, map(_energy, 1, 255, 16, 768)) / 500.0f; // * speedFactor; //(1.0/speed);
    boid.velocity.x *= direction;
    boid.velocity.y = 0;
    boid.colorIndex = i * 32;
    boids[i] = boid;
  }
}

bool EffectAttract::run() {
  uint8_t dim = beatsin8(3, 170, 250);
  fb.fade(255U - dim);

  for (int i = 0; i < count; i++) // count
  {

    Boid boid = boids[i];
    //boid.acceleration *= speedFactor/10;
    PVector force = attract(boid);
    boid.applyForce(force);

    boid.update();
    EffectMath::drawPixelXYF(boid.location.x, boid.location.y, ColorFromPalette(*curPalette, boid.colorIndex), fb);

    boids[i] = boid;
  }
  return true;
}

//------------ Эффект "Змейки"
// вариант субпикселя и поведения от kDn
void EffectSnake::load() {
  palettesload();

  for(uint8_t i=0;i<MAX_SNAKES;i++){
    snakes[i].reset();
    snakes[i].pixels[0].x = WIDTH / 2; // пусть расползаются из центра
    snakes[i].pixels[0].y = HEIGHT / 2; // так будет интереснее
    snakes[i].direction = (EffectSnake::Direction)random(4);
    snakes[i].internal_speedf = ((random(2) ? 0.5 : 0.33)+1.0/(random(i+1)+1))+0.5;
  }
}

// !++
String EffectSnake::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = ((float)EffectCalc::setDynCtrl(_val).toInt()/ 512.0 + 0.025) * EffectCalc::speedfactor;
  else if(_val->getId()==4) snakeCount = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) subPix = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) onecolor = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSnake::run() {
  fb.fade(speed<25 ? 5 : speed/2 ); // длина хвоста будет зависеть от скорости
#ifdef MIC_EFFECTS
  hue+=(speedFactor/snakeCount+(isMicOn() ? getMicMapFreq()/127.0 : 0));
#else
  hue+=speedFactor/snakeCount;
#endif
  hue = hue>255? hue-255 : hue;

  for (int i = snakeCount - 1; i >= 0; i--)
  {
    EffectSnake::Snake &snake = snakes[i];

    if(onecolor){
      fill_palette(colors, SNAKE_LENGTH, hue, 1, *curPalette, 255-(i*8), LINEARBLEND);
    } else {
      fill_palette(colors, SNAKE_LENGTH, (
        (speed<25 || speed>230) ? (i%2 ? hue : 255-hue) : (i%2 ? hue*(i+1) : (255-hue)*(i+1))
      ), 1, *curPalette, 255-(i*8), LINEARBLEND); // вообще в цикле заполнять палитры может быть немножко тяжело... но зато разнообразнее по цветам
    }
    snake.shuffleDown(speedFactor, subPix);

#ifdef MIC_EFFECTS
    if(getMicMapMaxPeak()>speed/3.0+75.0 && isMicOn()) {
      snake.newDirection();
    } else if (random((speed<25)?speed*50:speed*10) < speed && !isMicOn()) {// как часто будут повороты :), логика загадочная, но на малой скорости лучше змейкам круги не наматывать :)
      snake.newDirection();
    }
#else
    if (random((speed<25)?speed*50:speed*10) < speed){ // как часто будут повороты :), логика загадочная, но на малой скорости лучше змейкам круги не наматывать :)
      snake.newDirection();
    }
#endif

    snake.move(speedFactor);
    snake.draw(colors, i, subPix, fb, false /*isDebug()*/);
  }
  return true;
}

void EffectSnake::Snake::draw(CRGB colors[SNAKE_LENGTH], int snakenb, bool subpix, LedFB &fb, bool isDebug)
{
  int len= isDebug ? 1 : (int)SNAKE_LENGTH;
  for (int i = 0; i < len; i++) // (int)SNAKE_LENGTH
  {
    if(isDebug){ // тест сабпикселя
      FastLED.clear(); 
    }

    if (subpix){
      EffectMath::drawPixelXYF(pixels[i].x, pixels[i].y, colors[i], fb);
    }
    else {
      if(i!=0)
        fb.pixel(pixels[i].x, pixels[i].y) = colors[i];
      else if(direction<LEFT)
        EffectMath::drawPixelXYF_Y(pixels[i].x, pixels[i].y, colors[i], fb);
      else
        EffectMath::drawPixelXYF_X(pixels[i].x, pixels[i].y, colors[i], fb);
    }
  }
}

//------------ Эффект "Nexus"
// (с) kostyamat 4.12.2020
void EffectNexus::reload() {
  for (auto &nx : nxdots) {
    nx.direct = random(0, 4);                     // задаем направление
    nx.posX = random(0, WIDTH);                   // Разбрасываем частицы по ширине
    nx.posY = random(0, HEIGHT);                  // и по высоте
    nx.color = ColorFromPalette(*curPalette, random8(0, 9) * 31, 255); // цвет капли
    nx.accel = (float)random(5, 11) / 70;        // делаем частицам немного разное ускорение 
  }
}

// !++
String EffectNexus::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.1, .33) * EffectCalc::speedfactor;
  else if(_val->getId()==3){
    _scale = EffectCalc::setDynCtrl(_val).toInt();
    nxdots.assign(_scale, Nexus());
    reload();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectNexus::load() {
  palettesload();
  randomSeed(millis());
  reload();
}

bool EffectNexus::run() {
  fb.fade(map(speed, 1, 255, 11, 33));

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
      nx.posY = (float)fb.cfg.maxHeightIndex();    
      resetDot(nx);
    }

    if (nx.posY > (fb.cfg.maxHeightIndex())) {
      nx.posY = 0;
      resetDot(nx);
    }

    // Обеспечиваем бесшовность по X.
    if (nx.posX < 0) {
      nx.posX = fb.cfg.maxWidthIndex();
      resetDot(nx);
    }
    if (nx.posX > fb.cfg.maxWidthIndex()) {
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
      nx.posX = random8(0, WIDTH); // Разбрасываем капли по ширине
      nx.posY = 0;  // и по высоте
      break;
    case 1:   //  вниз 
      nx.posX = random8(0, WIDTH); // Разбрасываем капли по ширине
      nx.posY = fb.cfg.maxHeightIndex();  // и по высоте
      break;
    case 2:   // вправо
      nx.posX = 0; // Разбрасываем капли по ширине
      nx.posY = random8(0, HEIGHT);  // и по высоте
      break;
    //case 3:   // влево
    default:
      nx.posX = fb.cfg.maxWidthIndex(); // Разбрасываем капли по ширине
      nx.posY = random8(0, HEIGHT);  // и по высоте
  } 
}

// ----------- Эфеект "Змеиный Остров"
// (c) Сотнег
// База https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-53132
// адаптация и доработки kostyamat
// !++
String EffectTest::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.06, 0.5) * EffectCalc::speedfactor;
  else if(_val->getId()==3) SnakeNum = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  regen();
  return String();
}

void EffectTest::regen() {

  for (uint8_t i = 0; i < map(SnakeNum, 1, 10, 2, MAX_SNAKES); i++)
  {
    snakeLast[i] = 0;
    snakePosX[i] = random8(WIDTH / 2 - WIDTH / 4, WIDTH/2 + WIDTH / 4);
    snakePosY[i] = random8(HEIGHT / 2 - HEIGHT / 4, HEIGHT / 2 + HEIGHT / 4);
    snakeSpeedX[i] = EffectMath::randomf(0.2, 1.5);//(255. + random8()) / 255.;
    snakeSpeedY[i] = EffectMath::randomf(0.2, 1.5);
    //snakeTurn[i] = 0;
    snakeColor[i] = random8(map(SnakeNum, 1, 10, 2, MAX_SNAKES) * 255/map(SnakeNum, 1, 10, 2, MAX_SNAKES));
    snakeDirect[i] = random8(4); //     B00           направление головы змейки
                                 // B10     B11
                                 //     B01
  }
}

bool EffectTest::run() {
  FastLED.clear(); 
  int8_t dx = 0, dy = 0;
  for (uint8_t i = 0; i < map(SnakeNum, 1, 10, 2, MAX_SNAKES); i++)
  {
    snakeSpeedY[i] += snakeSpeedX[i] * speedFactor;
    if (snakeSpeedY[i] >= 1)
    {
      snakeSpeedY[i] = snakeSpeedY[i] - (int)snakeSpeedY[i];
      if (random8(8) <= 1U)
        if (random8(2U))
        {                                           // <- поворот налево
          snakeLast[i] = (snakeLast[i] << 2) | B01; // младший бит = поворот
          switch (snakeDirect[i])
          {
          case B10:
            snakeDirect[i] = B01;
            if (snakePosY[i] == 0U)
              snakePosY[i] = fb.cfg.maxHeightIndex();
            else
              snakePosY[i]--;
            break;
          case B11:
            snakeDirect[i] = B00;
            if (snakePosY[i] >= fb.cfg.maxHeightIndex())
              snakePosY[i] = 0U;
            else
              snakePosY[i]++;
            break;
          case B00:
            snakeDirect[i] = B10;
            if (snakePosX[i] == 0U)
              snakePosX[i] = fb.cfg.maxWidthIndex();
            else
              snakePosX[i]--;
            break;
          case B01:
            snakeDirect[i] = B11;
            if (snakePosX[i] >= fb.cfg.maxWidthIndex())
              snakePosX[i] = 0U;
            else
              snakePosX[i]++;
            break;
          }
        }
        else
        {                                           // -> поворот направо
          snakeLast[i] = (snakeLast[i] << 2) | B11; // младший бит = поворот, старший = направо
          switch (snakeDirect[i])
          {
          case B11:
            snakeDirect[i] = B01;
            if (snakePosY[i] == 0U)
              snakePosY[i] = fb.cfg.maxHeightIndex();
            else
              snakePosY[i]--;
            break;
          case B10:
            snakeDirect[i] = B00;
            if (snakePosY[i] >= fb.cfg.maxHeightIndex())
              snakePosY[i] = 0U;
            else
              snakePosY[i]++;
            break;
          case B01:
            snakeDirect[i] = B10;
            if (snakePosX[i] == 0U)
              snakePosX[i] = fb.cfg.maxWidthIndex();
            else
              snakePosX[i]--;
            break;
          case B00:
            snakeDirect[i] = B11;
            if (snakePosX[i] >= fb.cfg.maxWidthIndex())
              snakePosX[i] = 0U;
            else
              snakePosX[i]++;
            break;
          }
        }
      else
      { // двигаем без поворота
        snakeLast[i] = (snakeLast[i] << 2);
        switch (snakeDirect[i])
        {
        case B01:
          if (snakePosY[i] == 0U)
            snakePosY[i] = fb.cfg.maxHeightIndex();
          else
            snakePosY[i]--;
          break;
        case B00:
          if (snakePosY[i] >= fb.cfg.maxHeightIndex())
            snakePosY[i] = 0U;
          else
            snakePosY[i]++;
          break;
        case B10:
          if (snakePosX[i] == 0U)
            snakePosX[i] = fb.cfg.maxWidthIndex();
          else
            snakePosX[i]--;
          break;
        case B11:
          if (snakePosX[i] >= fb.cfg.maxWidthIndex())
            snakePosX[i] = 0U;
          else
            snakePosX[i]++;
          break;
        }
      }
    }
    switch (snakeDirect[i])
    {
    case B01:
      dy = 1;
      dx = 0;
      break;
    case B00:
      dy = -1;
      dx = 0;
      break;
    case B10:
      dy = 0;
      dx = 1;
      break;
    case B11:
      dy = 0;
      dx = -1;
      break;
    }

    long temp = snakeLast[i];
    uint8_t x = snakePosX[i];
    uint8_t y = snakePosY[i];
    EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i], snakeSpeedY[i] * 255), fb);
    for (uint8_t m = 0; m < SNAKE_LENGTH; m++)
    { // 16 бит распаковываем, 14 ещё остаётся без дела в запасе, 2 на хвостик
      x = (WIDTH + x + dx) % WIDTH;
      y = (HEIGHT + y + dy) % HEIGHT;  
      EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i] + m * 4U, 255U), fb);

      if (temp & B01)
      { // младший бит = поворот, старший = направо
        temp = temp >> 1;
        if (temp & B01)
        { // старший бит = направо
          if (dx == 0)
          {
            dx = 0 - dy;
            dy = 0;
          }
          else
          {
            dy = dx;
            dx = 0;
          }
        }
        else
        { // иначе налево
          if (dx == 0)
          {
            dx = dy;
            dy = 0;
          }
          else
          {
            dy = 0 - dx;
            dx = 0;
          }
        }
        temp = temp >> 1;
      }
      else
      { // если без поворота
        temp = temp >> 2;
      }
    }
    x = (WIDTH + x + dx) % WIDTH;
    y = (HEIGHT + y + dy) % HEIGHT;
    EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, snakeColor[i] + SNAKE_LENGTH * 4U, (1 - snakeSpeedY[i]) * 255), fb); // хвостик
  }

  return true;
}

void EffectTest::load() {
  palettesload();
  regen();
}

// ----------- Эфеект "Попкорн"
// (C) Aaron Gotwalt (Soulmate)
// адаптация и доработки kostyamat
void EffectPopcorn::restart_rocket(uint8_t r) {
  rockets[r].xd = (float)(random(-(WIDTH * HEIGHT + (WIDTH*2)), WIDTH * HEIGHT + (WIDTH*2))) / 256.0; 
  if ((rockets[r].x < 0 && rockets[r].xd < 0) || (rockets[r].x > fb.cfg.maxWidthIndex() && rockets[r].xd > 0)) { // меняем направление только после выхода за пределы экрана
    // leap towards the centre of the screen
    rockets[r].xd = -rockets[r].xd;
  }
  // controls the leap height
  rockets[r].yd = EffectMath::randomf(0.9, EffectMath::fmap(speed, 1., 255., (float)HEIGHT / 1.50, (float)HEIGHT / 3.));// (float)(random8() * 8 + HEIGHT * 10.5) / 256.0;
  rockets[r].hue = random8();
  rockets[r].x = EffectMath::randomf(center - (float)WIDTH / 6, center + (float)WIDTH / 6);
}

void EffectPopcorn::reload(){
  //numRockets = map(scale, 1, 32, 6, WIDTH * 3);
  //rockets.resize(numRockets);

  for (uint8_t r = 0; r < numRockets; r++) {
    rockets[r].x = random8(WIDTH);
    rockets[r].y = random8(HEIGHT);
    rockets[r].xd = 0;
    rockets[r].yd = -1;
    rockets[r].hue = random8();
  }
}

bool EffectPopcorn::run() {
  randomSeed(micros());
  if (blurred) fb.fade( 30. * speedFactor);
  else FastLED.clear();
  float popcornGravity = 0.1 * speedFactor;

  for (uint8_t r = 0; r < numRockets; r++) {
    // add the X & Y velocities to the positions
    rockets[r].x += rockets[r].xd ;
    if (rockets[r].x > fb.cfg.maxWidthIndex())
      rockets[r].x = rockets[r].x - fb.cfg.maxWidthIndex();
    if (rockets[r].x < 0)
      rockets[r].x = fb.cfg.maxWidthIndex() + rockets[r].x;
    rockets[r].y += rockets[r].yd * speedFactor;
    
    if (rockets[r].y >= (float)HEIGHT){
      //rockets[r].y = HEIGHT+HEIGHT - 2 - rockets[r].y;
      rockets[r].yd = -0.001; //rockets[r].yd;
    } 
    

    // bounce off the floor?
    if (rockets[r].y < 0 && rockets[r].yd < -0.7) { // 0.7 вычислено в экселе. скорость свободного падения ниже этой не падает. если ниже, значит ещё есть ускорение
      rockets[r].yd = (-rockets[r].yd) * 0.9375;//* 240) >> 8;
      rockets[r].y = rockets[r].yd; //чё это значило вообще?!
      rockets[r].y = -rockets[r].y;
    }

    // settled on the floor?
    if (rockets[r].y <= -1)
      restart_rocket(r);


    // bounce off the sides of the screen?
    /*if (rockets[r].x < 0 || rockets[r].x > (int)WIDTH * 256) {
      rockets[r].xd = (-rockets[r].xd * 248) >> 8;
      // force back onto the screen, otherwise they eventually sneak away
      if (rockets[r].x < 0) {
        rockets[r].x = rockets[r].xd;
        rockets[r].yd += rockets[r].xd;
      } else {
        rockets[r].x = (WIDTH * 256) - rockets[r].xd;
      }
    }*/

    // popcornGravity
    rockets[r].yd -= popcornGravity;

    // viscosity
    rockets[r].xd *= 0.875;
    rockets[r].yd *= 0.875;


    // make the acme gray, because why not
    if (-0.004 > rockets[r].yd and rockets[r].yd < 0.004)
      EffectMath::drawPixelXYF(rockets[r].x,
                rockets[r].y,
                revCol ? ColorFromPalette(*curPalette, rockets[r].hue) : CRGB::Pink,
                fb,
                blurred ? 35 : 0);
    else
      EffectMath::drawPixelXYF(rockets[r].x, rockets[r].y,
                revCol ? CRGB::Gray : ColorFromPalette(*curPalette, rockets[r].hue),
                fb,
                blurred ? 35 : 0);
  }
  return true;
}

// !++
String EffectPopcorn::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) { speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., 0.25, 0.75)*EffectCalc::speedfactor; }
  else if(_val->getId()==3) { uint8_t density = EffectCalc::setDynCtrl(_val).toInt(); numRockets = 5 + density; rockets.resize(numRockets); reload(); }
  else if(_val->getId()==5) blurred = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) revCol = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectPopcorn::load() {
  palettesload();
  reload();
}

//-------- Эффект "Детские сны"
// (c) Stepko https://editor.soulmatelights.com/gallery/505
// !++
String EffectSmokeballs::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1., 255., .02, .1)*EffectCalc::speedfactor; // попробовал разные способы управления скоростью. Этот максимально приемлемый, хотя и сильно тупой.
  else if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  regen();
  return String();
}

void EffectSmokeballs::load(){
  palettesload();
  regen();
}

void EffectSmokeballs::regen() {
  randomSeed(millis());
  for (auto &w : waves){
    w.pos = w.reg =  random((WIDTH * 10) - ((WIDTH / 3) * 20)); // сумма maxMin + reg не должна выскакивать за макс.Х
    w.sSpeed = EffectMath::randomf(5., (float)(16 * WIDTH)); //random(50, 16 * WIDTH) / random(1, 10);
    w.maxMin = random((WIDTH / 2) * 10, (WIDTH / 3) * 20);
    w.waveColors = random(0, 9) * 28;
  }
}

bool EffectSmokeballs::run(){
  uint8_t _amount = map(_scale, 1, 16, 2, WAVES_AMOUNT);
  shiftUp();
  fb.dim(240);
  EffectMath::blur2d(fb, 20);
  for (byte j = 0; j < _amount; j++) {
    waves[j].pos = beatsin16((uint8_t)(waves[j].sSpeed * (speedFactor * 5.)), waves[j].reg, waves[j].maxMin + waves[j].reg, waves[j].waveColors*256, waves[j].waveColors*8);
    EffectMath::drawPixelXYF((float)waves[j].pos / 10., 0.05, ColorFromPalette(*curPalette, waves[j].waveColors), fb);
  }
  EVERY_N_SECONDS(20){
    for (byte j = 0; j < _amount; j++) {
      waves[j].reg += random(-20,20);
      waves[j].waveColors += 28;
    }
  }

  if (random8(255) > 253 ) regen();
  return true;
}

void EffectSmokeballs::shiftUp(){       
  for (byte x = 0; x < WIDTH; x++) {
    for (int16_t y = HEIGHT; y > 0; --y) {
      fb.pixel(x, y) = fb.pixel(x, y - 1);
    }
  }
}

// ----------- Эффект "Ёлки-Палки"
// "Cell" (C) Elliott Kember из примеров программы Soulmate
// Spaider и Spruce (c) stepko
void EffectCell::cell(){
  speedFactor = EffectMath::fmap((float)speed, 1., 255., .33*EffectCalc::speedfactor, 3.*EffectCalc::speedfactor);
  offsetX = beatsin16(6. * speedFactor, -180, 180);
  offsetY = beatsin16(6. * speedFactor, -180, 180, 12000);
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      int16_t hue = x * beatsin16(10. * speedFactor, 1, 10) + offsetY;
      fb.pixel(x, y) = CHSV(hue, 200, sin8(x * 30 + offsetX));
      hue = y * 3 + offsetX;
      fb.pixel(x, y) += CHSV(hue, 200, sin8(y * 30 + offsetY));
    }
  }
  EffectMath::nightMode(fb); // пригасим немного, чтобы видить структуру, и убрать пересветы
} 

// !++ Тут лучше все оставить как есть, пускай в теле эффекта скорость пересчитывает
String EffectCell::setDynCtrl(UIControl*_val) {
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
  fb.fade( map(speed, 1, 255, 1, 10));
  uint8_t z;
  if (effId == 3) z = triwave8(hue);
  else z = beatsin8(1, 1, 255);
  for (uint8_t i = 0; i < minDim; i++) {
    x = beatsin16(i * (map(speed, 1, 255, 3, 20)), 
                     i * 2, 
                     (minDim * 4 - 2) - (i * 2 + 2));
    if (effId == 2) 
      EffectMath::drawPixelXYF_X(x/4 + height_adj, i, random8(10) == 0 ? CHSV(random8(), random8(32, 255), 255) : CHSV(100, 255, map(speed, 1, 255, 128, 100)), fb);
    else
      EffectMath::drawPixelXYF_X(x/4 + height_adj, i, CHSV(hue + i * z, 255, 255), fb);
  }
  if (!(WIDTH& 0x01))
    fb.pixel(WIDTH/2 - ((millis()>>9) & 0x01 ? 1:0), minDim - 1 - ((millis()>>8) & 0x01 ? 1:0)) = CHSV(0, 255, 255);
  else
    fb.pixel(WIDTH/2, minDim - 1) = CHSV(0, (millis()>>9) & 0x01 ? 0 : 255, 255);

  if (glitch) EffectMath::confetti(fb, density);
}

void EffectCell::spider() {
  speedFactor = EffectMath::fmap(speed, 1, 255, 20., 2.) * EffectCalc::speedfactor;
  fb.fade( 50);
  for (uint8_t c = 0; c < Lines; c++) {
    float xx = 2. + sin8((float)(millis() & 0x7FFFFF) / speedFactor + 1000 * c * Scale) / 12.;
    float yy = 2. + cos8((float)(millis() & 0x7FFFFF) / speedFactor + 1500 * c * Scale) / 12.;
    EffectMath::drawLineF(xx, yy, (float)WIDTH - xx - 1, (float)HEIGHT - yy - 1, CHSV(c * (256 / Lines), 200, 255), fb);
    
  }
}

void EffectCell::vals() {
  speedFactor = map(speed, 1, 255, 100, 512) * speedfactor;
  fb.fade(128);
  a += 1;
  for (byte i = 0; i < 12; i++) {
    EffectMath::drawLineF((float)beatsin88((10 + i) * speedFactor, 0, fb.cfg.maxWidthIndex() * 2, i * i) / 2, (float)beatsin88((12 - i) * speedFactor, 0, fb.cfg.maxHeightIndex() * 2, i * 5) / 2, (float)beatsin88((8 + i) * speedFactor, 0, fb.cfg.maxWidthIndex() * 2, i * 20) / 2, (float)beatsin88((14 - i) * speedFactor, 0, fb.cfg.maxHeightIndex() * 2, i * 5) / 2, CHSV(21 * i + (byte)a * i, 255, 255), fb);
  }
}

// ---------- Эффект "Тикси Ленд"
// (c)  Martin Kleppe @aemkei, https://github.com/owenmcateer/tixy.land-display
// !++
String EffectTLand::setDynCtrl(UIControl*_val){
  if(_val->getId()==3) { animation = EffectCalc::setDynCtrl(_val).toInt(); isSeq = animation ? false : true; if(!animation) animation = random(_val->getMin().toInt()+1, _val->getMax().toInt()+1);}
  else if(_val->getId()==4) { hue = EffectCalc::setDynCtrl(_val).toInt(); ishue = hue; }
  else if(_val->getId()==5) { hue2 = EffectCalc::setDynCtrl(_val).toInt(); ishue2 = hue2; }
  else if(_val->getId()==6) fine = _val->getVal().toInt(); // качество не рандомим, а берем как есть
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectTLand::run() {
  t = (double)(millis()&0xFFFFF) / map(speed, 1, 255, 1200, 128); ; // на больших значениях будет странное поведение, поэтому уменьшаем точность, хоть и будет иногда срыв картинки, но в 18 минут, так что - хрен с ним
  shift = (shift+1)%fine; // 0...3
  if(!ishue) hue++;
  if(!ishue2) hue2++;

  for( byte x = 0; x < WIDTH; x++) {
    for( byte y = 0; y < HEIGHT; y++) {
      //if(myLamp.getPixelNumber(x,y)%fine==shift)
      if((x*WIDTH+y)%fine==shift)
        processFrame(fb, t, x, y);
    }
  }
  
  if (isSeq) {
    EVERY_N_SECONDS(30) {
      animation++;
    }
  }
  return true;
}

void EffectTLand::processFrame(LedFB &fb, double t, double x, double y) {
  double i = (y * WIDTH) + x;
  int16_t frame = constrain(code(i, x, y), -1, 1) * 255;

  if (frame > 0) {
    fb.pixel(x, y) = CHSV(hue, frame, frame);
  }
  else if (frame < 0) {
    fb.pixel(x, y) = CHSV(hue2, frame * -1, frame * -1);
  } else fb.pixel(x, y) = CRGB::Black;
}

float EffectTLand::code(double i, double x, double y) {

  switch (animation) {
    /**
     * Motus Art
     * @motus_art
     */
    case 1:
      // Plasma
      //return sin(x + t) / 2 + sin(y + t) / 2 + sin(x + y + t) / 3;
      return (sin16((x + t) * 8192.0) * 0.5 + sin16((y + t) * 8192.0) * 0.5 + sin16((x + y + t) * 8192.0) * 0.3333333333333333) / 32767.0;
      break;

    case 2:
      // Up&Down
      //return sin(cos(x) * y / 8 + t);
      return sin16((cos16(x*8192.0) / 32767.0 * y / (HEIGHT/2.0) + t)*8192.0)/32767.0;
      break;

    case 3:
      //return sin(atan(y / x) + t);
      return sin16((EffectMath::atan_fast(y / x) + t)*8192.0)/32767.0;
      break;

    /**
     * tixy.land website
     */
    case 4:
      // Emitting rings
      //return sin(t - EffectMath::sqrt(((x - 7.5)*(x - 7.5)) + (y - 6)*(y - 6)));
      return sin16((t - EffectMath::sqrt((x - (WIDTH/2))*(x - (WIDTH/2)) + (y - (HEIGHT/2))*(y - (HEIGHT/2))))*8192.0)/32767.0;
      break;

    case 5:
      // Rotation
      //return sin(PI * 2 * atan((y - 8) / (x - 8)) + 5 * t);
      return sin16((PI * 2.5 * EffectMath::atan_fast((y - (HEIGHT/2)) / (x - (WIDTH/2))) + 5 * t) * 8192.0)/32767.0;
      break;

    case 6:
      // Vertical fade
      //return sin(y / 8 + t);
      return sin16((y / 8 + t)*8192.0)/32767.0;
      break;

    case 7:
      // Smooth noise
      //return cos(t + i + x * y);
      return cos16((t + i + x * y)*8192.0)/32767.0;
      break;

    case 8:
      // Waves
      //return sin(x / 2) - sin(x - t) - y + 6;
      return (sin16(x * 4096.0) - sin16((x - t) * 8192.0)) / 32767.0 - y + (HEIGHT/2);
      break;

    case 9:
      // Drop
      //return fmod(8 * t, 13) - hypot(x - 7.5, y - 7.5);
      return fmod(8 * t, 13) - EffectMath::sqrt((x - (WIDTH/2))*(x - (WIDTH/2))+(y - (HEIGHT/2))*(y - (HEIGHT/2))); //hypot(x - (WIDTH/2), y - (HEIGHT/2));
      break;

    case 10:
      // Ripples @thespite
      //return sin(t - EffectMath::sqrt(x * x + y * y));
      return sin16((t - EffectMath::sqrt(x * x + y * y))*8192.0)/32767.0;
      break;

    case 11:
      // Bloop bloop bloop @v21
      //return (x - 8) * (y - 8) - sin(t / 2.) * 64;
      return (x - (WIDTH/2)) * (y - (HEIGHT/2)) - sin16(t*4096.0)/512.0;
      break;


    /**
     * Reddit
     */
     case 12:
      // lurkerurke https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgcwsn/
      //return sin((x - 7.5) * (y - 7.5) / 5 * t + t);
      return sin16(((x - (WIDTH/2)) * (y - (HEIGHT/2)) / 5 * t + t)*8192.0)/32767.0;
      break;

    case 13:
      // SN0WFAKER https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk7c0/
      //return sin(atan((y - 7.5) / (x - 7.5)) + t);
      return sin16((EffectMath::atan_fast((y - (HEIGHT/2)) / (x - (WIDTH/2))) + t) * 8192.0)/32767.0;
      break;

    case 14:
      //return  cos(((int)x ^ (int)y) * t); //sin(((int)(x / sin(t) / 50) ^ (int)(y / sin(t) / 50)) + t); //pow(cos(((int)y ^ (int)x) + t), cos((x > y) + t));
      return  cos16((((int)x ^ (int)y) * t)* 8192.0)/32767.0;
      break;

    case 15:
      // detunized https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgk30l/
      //return sin(y / 8 + t * 0.5) + x / 16 - 0.5;
      return sin16((y / (HEIGHT/2) + t * 0.5)*8192.0)/32767.0 + x / 16 - 0.5;
      break;

    case 16:
      // Andres_A https://www.reddit.com/r/programming/comments/jpqbux/minimal_16x16_dots_coding_environment/gbgzdnj/
      //return 1. - hypot(sin(t) * 9 - x, cos(t) * 9 - y) / 9;
      //return 1. - hypot(sin(1.5*t) * 16 + x, cos(t*2) * 16 + y) / 4;
      //return 1. - hypot(8 * sin(1.5*t) + x - 8, (8 * cos(t*2) + y - 8))*(sin(0.5*t+1.0)+1.0); // https://tixy.land/?code=1.+-+hypot%288+*+sin%281.5*t%29+%2B+x+-+8%2C+%288+*+cos%28t*2%29+%2B+y+-+8%29%29*%28sin%280.5*t%2B1.0%29%2B1.0%29
      {
        float _x=sin16(12288.0*t)/32767.0 * WIDTH + x - (WIDTH/2);
        float _y=cos16(16384.0*t)/32767.0 * HEIGHT + y - (HEIGHT/2);
        //float _size=sin16((0.5*t+1.0)*8192.0)/32767.0+1.0;
        float _size=1.0/4.0; // 4.0 - постоянный размер шарика
        return 1. - EffectMath::sqrt(_x*_x+_y*_y)*_size; 
      }
      break;

    /**
     * @akella
     * https://twitter.com/akella/status/1323549082552619008
     */
    case 17:
      //return sin(6 * atan2(y - 8, x) + t);
      return sin16((6 * EffectMath::atan2_fast(y - (HEIGHT/2), x) + t)*8192.0)/32767.0;
      break;

    case 18:
      //return sin(i / 5 + t);
      return sin16((i / 5 + t)*16384.0)/32767.0;
      break;

    /**
     * Paul Malin
     * https://twitter.com/P_Malin/
     */

    case 19:
      // Matrix Rain https://twitter.com/P_Malin/status/1323583013880553472
      //return 1. - fmod((x * x - y + t * (fmod(1 + x * x, 5)) * 6), 16) / 16;
      return 1. - fmod((x * x - (fb.cfg.maxHeightIndex() - y) + t * (1 + fmod(x * x, 5)) * 3), WIDTH) / HEIGHT;
      break;

    case 20:
      // Burst https://twitter.com/P_Malin/status/1323605999274594304
      //return -10. / ((x - 8) * (x - 8) + (y - 8) * (y - 8) - fmod(t*0.3, 0.7) * 200);
      return -10. / ((x - (WIDTH/2)) * (x - (WIDTH/2)) + (y - (HEIGHT/2)) * (y - (HEIGHT/2)) - fmod(t*0.3, 0.7) * 200);
      break;

    case 21:
      // Rays
      //return sin(atan2(x, y) * 5 + t * 2);
      return sin16((EffectMath::atan2_fast(x, y) * 5 + t * 2)*8192.0)/32767.0;
      break;

    case 22:
      // Starfield https://twitter.com/P_Malin/status/1323702220320313346 
      //return !((int)(x + (t/2) * 50 / (fmod(y * y, 5.9) + 1)) & 15) / (fmod(y * y, 5.9) + 1);
      {
        uint16_t _y = (fb.cfg.maxHeightIndex() - y);
        float d = (fmod(_y * _y + 4, 4.1) + 0.85) * 0.5; // коэффициенты тут отвечают за яркость (размер), скорость, смещение, подбираются экспериментально :)
        return !((int)(x + t * 7.0 / d) & 15) / d; // 7.0 - множитель скорости
      }
      break;

    case 23:
      //return sin(3 * atan2(y - 7.5 + sin(t) * 5, x - 7.5 + sin(t) * 5) + t * 5);
      //return sin(3 * atan2(y - 7.5 + sin(t) * 4, x - 7.5 + sin(t) * 4) + t * 1.5 + 5);
      return sin16((3.5*EffectMath::atan2_fast(y - (HEIGHT/2) + sin16(t*8192.0) * 0.00006, x - (WIDTH/2) + sin16(t*8192.0) * 0.00006) + t * 1.5 + 5)*8192.0)/32767.0;
      break;

    case 24:
      //return (y - 8) / 3 - tan(x / 6 + 1.87) * sin(t * 2);
      //return (y - 8) / 3 - tan(x / 6 + 1.87) * sin16(t * 16834.0)/32767.0;
      return (y - 8) / 3 - EffectMath::tan2pi_fast((x / 6 + 1.87)/PI*2) * sin16(t * 16834.0)/32767.0;
      break;

    case 25:
      //return (y - 8) / 3 - (sin(x / 4 + t * 2));
      return (y - 8) / 3 - (sin16((x / 4 + t * 2)*8192.0)/32767.0);
      break;

    case 26:
      //return fmod(i, 4) - fmod(y, 4) + sin(t);
      return fmod(i, 4) - fmod(y, 4) + sin16(t*8192.0)/32767.0;
      break;

    case 27:
      //return cos(sin((x * t / 10)) * PI) + cos(sin(y * t / 10 + (EffectMath::sqrt(abs(cos(x * t))))) * PI);
      return cos(sin16(x * t * 819.2) / 32767.0 * PI) + cos16((sin16((y * t / 10 + (EffectMath::sqrt(abs(cos16(x * t * 8192.0)/32767.0))))*8192.0)/32767.0 * PI)*8192.0)/32767.0;
      break;

    case 28:
      //return -.4 / (hypot(x - fmod(t, 10), y - fmod(t, 8)) - fmod(t, 2) * 9);
      {
        float _x = x - fmod(t, WIDTH);
        float _y = y - fmod(t, HEIGHT);
        return -.4 / (EffectMath::sqrt(_x*_x+_y*_y) - fmod(t, 2) * 9);
      }
      break;

    case 29:
      //return sin(x / 3 * sin(t / 3) * 2) + cos(y / 4 * sin(t / 2) * 2);
      return sin16(x / 3 * sin16(t * 2730.666666666667) / 2.0) / 32767.0 + cos16(y / 4 * sin16(t * 4096.0) / 2.0) / 32767.0;
      break;

    case 30:
      //return sin(x * x * 3 * i / 1e4 - y / 2 + t * 2);
      return sin16((x * x * 3 * i / 1e4 - y / 2 + t * 2)*8192.0)/32767.0;
      break;

    case 31:
      //return 1. - fabs((x - 6) * cos(t) + (y - 6) * sin(t));
      return 1. - fabs((x - (WIDTH/2)) * cos16(t*8192.0)/32767.0 + (y - (HEIGHT/2)) * sin16(t*8192.0)/32767.0);
      break;

    case 32:
      //return 1. / 32 * tan(t / 64 * x * tan(i - x));
      //return (((x-8)/y+t)&1^1/y*8&1)*y/5;
      return ((((uint32_t)((x-8)/(HEIGHT-y)+t) & 1 ) ^ (uint32_t)((1./(HEIGHT-y)) * 8)) & 1) * (HEIGHT-y) / 8;
      break;

    case 33:
      return EffectMath::atan_fast((x - (WIDTH/2)) * (y - (HEIGHT/2))) - 2.5 * sin16(t*8192.0)/32767.0;
      break;

    case 34:
      //return sin(cos(y) * t) * cos(sin(x) * t);
      return sin16(cos16(y*8192.0)* 0.25 * t)/32767.0 * cos16(sin16(x*8192.0)* 0.25 * t)/32767.0;
      break;

    case 35:
      //return sin(y * (t/4)) * cos(x * (t/4));
      return sin16(y * t * 2048.0) / 32767.0 * cos16(x * t * 2048.0) / 32767.0;
      break;

    default:
      animation = 1;
      return sin16(t*8192.0)/32767.0;
      break;
  }
}

// ----------- Эффект "Осцилятор"
// (c) Сотнег (SottNick)
bool EffectOscilator::run() {
  if (millis() - timer < (unsigned)map(speed, 1U, 255U, 70, 15)) return true;
  else timer = millis(); // не могу сообразить, как по другому скоростью управлять
  CRGB currColors[3];
  for (uint8_t c = 0; c < 3; c++)
    currColors[c] = ColorFromPalette(*curPalette, c * 85U + hue);

  // расчёт химической реакции и отрисовка мира
  uint16_t colorCount[3] = {0U, 0U, 0U};
  hue++;
  FastLED.clear();
  for (uint8_t x = 0; x < WIDTH; x++) {
      for (uint8_t y = 0; y < HEIGHT; y++) {
          if (oscillatingWorld[x][y].red){
             colorCount[0]++;
             if (greenNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 1U;
          }
          else if (oscillatingWorld[x][y].green){
             colorCount[1]++;
             if (blueNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 2U;
          }
          else {//if (oscillatingWorld[x][y].blue){
             colorCount[2]++;
             if (redNeighbours(x, y) > 2)
                oscillatingWorld[x][y].color = 0U;
          }
          drawPixelXYFseamless((float)x + 0.5, (float)y + 0.5, currColors[oscillatingWorld[x][y].color]);
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
    uint8_t tx = random8(WIDTH);
    deltaHue = oscillatingWorld[tx][0U].color + 1U;
    if (deltaHue > 2U) deltaHue = 0U;
    oscillatingWorld[tx][0U].color = deltaHue;
    oscillatingWorld[(tx + 1U) % WIDTH][0U].color = deltaHue;
    oscillatingWorld[(tx + 2U) % WIDTH][0U].color = deltaHue;
  }

  deltaHue = colorCount[0];
  deltaHue2 = colorCount[1];
  deltaValue = colorCount[2];

  // вброс исчезнувшего цвета
  for (uint8_t c = 0; c < 3; c++)
  {
    if (colorCount[c] < 6U){
      uint8_t tx = random8(WIDTH);
      uint8_t ty = random8(HEIGHT);
      if (random8(2U)){
        oscillatingWorld[tx][ty].color = c;
        oscillatingWorld[(tx + 1U) % WIDTH][ty].color = c;
        oscillatingWorld[(tx + 2U) % WIDTH][ty].color = c;
      }
      else {
        oscillatingWorld[tx][ty].color = c;
        oscillatingWorld[tx][(ty + 1U) % HEIGHT].color = c;
        oscillatingWorld[tx][(ty + 2U) % HEIGHT].color = c;
      }
    }
  }

  // перенос на следующий цикл
  for (uint8_t x = 0; x < WIDTH; x++) {
      for (uint8_t y = 0; y < HEIGHT; y++) {
          setCellColors(x, y);
      }
  }

  //fpsmeter();
  return true;
}

void EffectOscilator::load() {
  palettesload();
  step = 0U;
 //случайное заполнение
  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
      oscillatingWorld[i][j].color = random8(3);
      setCellColors(i, j);
    }
  }
  timer = millis();
}

void EffectOscilator::drawPixelXYFseamless(float x, float y, CRGB color)
{
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  uint8_t wu[4] = {EffectMath::wu_weight(ix, iy), EffectMath::wu_weight(xx, iy),
                   EffectMath::wu_weight(ix, yy), EffectMath::wu_weight(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t xn = (uint8_t)(x + (i & 1)) % fb.cfg.w();
    uint8_t yn = (uint8_t)(y + ((i >> 1) & 1)) % fb.cfg.h();
    CRGB clr = fb.pixel(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    fb.pixel(xn, yn) = clr;
  }
}

int EffectOscilator::redNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].red) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][y].red) +
         (oscillatingWorld[x][(y + fb.cfg.maxHeightIndex()) % HEIGHT].red) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + 1) % HEIGHT].red) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].red) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].red);
    }

int EffectOscilator::blueNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].blue) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][y].blue) +
         (oscillatingWorld[x][(y + fb.cfg.maxHeightIndex()) % HEIGHT].blue) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + 1) % HEIGHT].blue) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].blue) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].blue);
}
  
int EffectOscilator::greenNeighbours(uint8_t x, uint8_t y) {
  return (oscillatingWorld[(x + 1) % WIDTH][y].green) +
         (oscillatingWorld[x][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][y].green) +
         (oscillatingWorld[x][(y + fb.cfg.maxHeightIndex()) % HEIGHT].green) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + 1) % HEIGHT].green) +
         (oscillatingWorld[(x + fb.cfg.maxWidthIndex()) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].green) +
         (oscillatingWorld[(x + 1) % WIDTH][(y + fb.cfg.maxHeightIndex()) % HEIGHT].green);
}

void EffectOscilator::setCellColors(uint8_t x, uint8_t y) {
  oscillatingWorld[x][y].red = (oscillatingWorld[x][y].color == 0U);
  oscillatingWorld[x][y].green = (oscillatingWorld[x][y].color == 1U);
  oscillatingWorld[x][y].blue = (oscillatingWorld[x][y].color == 2U);
}

//------------ Эффект "Шторм" 
// (с) kostyamat 1.12.2020
// !++
String EffectWrain::setDynCtrl(UIControl*_val)
{
  if(_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    speedFactor = EffectMath::fmap(speed, 1, 255, 0.125, .75) * EffectCalc::speedfactor;
  }
  else if(_val->getId()==3) _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4) { uint8_t val = EffectCalc::setDynCtrl(_val).toInt(); white = (val == FASTLED_PALETTS_COUNT); randColor = (val == 0); }
  else if(_val->getId()==5) clouds = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==6) storm = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==7) type = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectWrain::reload() {
  randomSeed(millis());
  for (byte i = 0; i < DROP_CNT; i++) {
    drops[i].posX = EffectMath::randomf(0, WIDTH); // Разбрасываем капли по ширине
    drops[i].posY = EffectMath::randomf(0, HEIGHT);  // и по высоте
    drops[i].color = random(0, 9) * 31;              // цвет капли
    drops[i].accell = (float)random(5, 10) / 100;     // делаем частицам немного разное ускорение 
    drops[i].bri = random(170, 255);
  }
}

void EffectWrain::load() {
  palettesload();
  reload();
}

bool EffectWrain::run() {

  if (_flash and (millis() - timer < 500)) 
    _flash = true;
  else _flash = false;

  if (_flash == false) {
    switch (type)
    {
    case 1:
    case 5:
      FastLED.clear();
      break;
    case 2:
    case 6:
      fb.fade(200. * speedFactor);
      break;
    case 3:
    case 7:
      fb.fade(100. * speedFactor);
      break;
    case 4:
    case 8:
      fb.fade(50. * speedFactor);
      break;
    default:
      break;
    } 
  }

  //
  for (byte i = 0; i < map(_scale, 1, 45, 2, DROP_CNT); i++) {
    drops[i].color++;
    drops[i].posX += (speedFactor * dotChaos + drops[i].accell) * dotDirect; // смещение по горизонтали
    drops[i].posY -= (speedFactor + drops[i].accell);

    // Обеспечиваем бесшовность по Y.
    if (drops[i].posY < 0)
    {                                                             // достигли низа, обновляем каплю
      drops[i].posY = ((float)HEIGHT - (clouds ? cloudHeight : 1.)); // переносим каплю в начало трека
      drops[i].posX += EffectMath::randomf(-1, 1);                   // сдвигаем каплю туда-сюда по горизонтали
      drops[i].bri = random(170, 200);                               // задаем капле новое значение яркости
    }
    if (drops[i].posY > (fb.cfg.maxHeightIndex()))
      drops[i].posY = 0;

    // Обеспечиваем бесшовность по X.
    if (drops[i].posX < 0)
      drops[i].posX = fb.cfg.maxWidthIndex();
    if (drops[i].posX > fb.cfg.maxWidthIndex())
      drops[i].posX = 0;

    if (randColor) {
      if (dotDirect) EffectMath::drawPixelXYF(drops[i].posX, drops[i].posY, CHSV(drops[i].color, 256U - beatsin88(2 * speed, 1, 196), beatsin88(1 * speed, 64, 255)), fb);
      else EffectMath::drawPixelXYF_Y(drops[i].posX, drops[i].posY, CHSV(drops[i].color, 256U - beatsin88(2 * speed, 1, 196), beatsin88(1 * speed, 64, 255)), fb);
    } else if (white) {
      CHSV color = rgb2hsv_approximate(CRGB::Gray);
      color.value = drops[i].bri - 48;
      if (dotDirect) EffectMath::drawPixelXYF(drops[i].posX, drops[i].posY, color, fb);
      else EffectMath::drawPixelXYF_Y(drops[i].posX, drops[i].posY, color, fb);
    }
    else {
      CHSV color = rgb2hsv_approximate(ColorFromPalette(*curPalette, drops[i].color, drops[i].bri));
      color.sat = 128;
      if (dotDirect) EffectMath::drawPixelXYF(drops[i].posX, drops[i].posY, color, fb);
      else EffectMath::drawPixelXYF_Y(drops[i].posX, drops[i].posY, color, fb);
    }
  }

  // Раздуваем\угасаем ветер
  if (type <= 4) {
    uint8_t val = triwave8(windProgress += speedFactor);
    dotChaos = (float)val / 254;
    if (val == 0) {
      dotDirect = random(-1, 2); //выбираем направление ветра лево-право, рандом 2 не возвращает (как не странно).
    }
  } else dotDirect = 0;

    // Рисуем тучку и молнию
  if (clouds) {
    if (randColor) curPalette = palettes.at(0);  // устанавливаем палитру RainbowColors_p
    if (white) curPalette = palettes.at(FASTLED_PALETTS_COUNT-1);     // WaterfallColors_p
    if (storm) _flash = Lightning(200);
    Clouds((storm ? _flash : false));
  } else if (storm) {
    _flash = Lightning(200);
  }
  return true;
}

bool EffectWrain::Lightning(uint16_t chanse)
{
  if (random16() > chanse) return false;  // no lightning this time

  CRGB lightningColor = CHSV(30,90,255);
  timer = millis();
  std::vector<uint8_t>lightning(WIDTH*HEIGHT, 0);

    lightning[scale8(random8(), fb.cfg.maxWidthIndex()) + fb.cfg.maxHeightIndex() * WIDTH] = 255; // Random starting location
    for (uint8_t ly = fb.cfg.maxHeightIndex(); ly > 1; ly--)
    {
      for (uint8_t lx = 1; lx < fb.cfg.maxWidthIndex(); lx++)
      {
        if (lightning[lx + ly * WIDTH] == 255)
        {
          lightning[lx + ly * WIDTH] = 0;
          uint8_t dir = random8(4);
          switch (dir)
          {
          case 0:
            fb.pixel(lx + 1, ly - 1) = lightningColor;
            lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // move down and right
            break;
          case 1:
            fb.pixel(lx, ly - 1) = lightningColor;
            lightning[lx + (ly - 1) * WIDTH] = 255;                                 // move down
            break;
          case 2:
            fb.pixel(lx - 1, ly - 1) = lightningColor;
            lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // move down and left
            break;
          case 3:
            fb.pixel(lx - 1, ly - 1) = lightningColor;
            lightning[(lx - 1) + (ly - 1) * WIDTH] = 255; // fork down and left
            fb.pixel(lx - 1, ly - 1) = lightningColor;
            lightning[(lx + 1) + (ly - 1) * WIDTH] = 255; // fork down and right
            break;
          }
        }
      }
    }

    return true;
}

// Функция рисует тучу в верхней части матрицы 
void EffectWrain::Clouds(bool flash)
{
  uint8_t dataSmoothing = 50; //196
  uint16_t noiseX = beatsin16(1, 10, 4000, 0, 150);
  uint16_t noiseY = beatsin16(1, 1000, 10000, 0, 50);
  uint16_t noiseZ = beatsin16(1, 10, 4000, 0, 100);
  uint16_t noiseScale = 50; // A value of 1 will be so zoomed in, you'll mostly see solid colors. A value of 4011 will be very zoomed out and shimmery

  // This is the array that we keep our computed noise values in
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    int xoffset = noiseScale * x;

    for (uint8_t z = 0; z < cloudHeight; z++) {
      int yoffset = noiseScale * z ;
      uint8_t noiseData = qsub8(inoise8(noiseX + xoffset, noiseY + yoffset, noiseZ), 16);
      noiseData = qadd8(noiseData, scale8(noiseData, 39));
      _noise[x * cloudHeight + z] = scale8(_noise[x * cloudHeight + z], dataSmoothing) + scale8(noiseData, 256 - dataSmoothing);
      if (flash)
        fb.pixel(x, HEIGHT - z - 1) = CHSV(random8(20,30), 250, random8(64, 100));
      else 
        nblend(fb.pixel(x, fb.cfg.maxHeightIndex() - z), ColorFromPalette(*curPalette, _noise[x * cloudHeight + z], _noise[x * cloudHeight + z]), (500 / cloudHeight));
    }
    noiseZ++;
  }
  if (millis() - timer < 300) {
    for (uint8_t i = 0; i < WIDTH; i++)
    {
        EffectMath::drawPixelXYF(i, EffectMath::randomf((float)HEIGHT - 4.5, (float)HEIGHT - 2.5), CHSV(0, 250, random8(120, 200)), fb, 0);
    }
  }
}

//-------- по мотивам Эффектов Particle System -------------------------
// https://github.com/fuse314/arduino-particle-sys
// https://github.com/giladaya/arduino-particle-sys
// https://www.youtube.com/watch?v=S6novCRlHV8&t=51s

//при попытке вытащить из этой библиотеки только минимально необходимое выяснилось, что там очередной (третий) вариант реализации субпиксельной графики.
//ну его нафиг. лучше будет повторить визуал имеющимися в прошивке средствами.

void EffectFairy::particlesUpdate(uint8_t i) {
  trackingObjectState[i] -= 1 * speedFactor; 

  //apply velocity
  trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedFactor;
  trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedFactor;
  if(trackingObjectState[i] == 0 || trackingObjectPosX[i] <= -1 || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= -1 || trackingObjectPosY[i] >= HEIGHT) 
    trackingObjectIsShift[i] = false;
}

// ============= ЭФФЕКТ ИСТОЧНИК ===============
// (c) SottNick
// выглядит как https://github.com/fuse314/arduino-particle-sys/blob/master/examples/StarfieldFastLED/StarfieldFastLED.ino

void EffectFairy::fountEmit(uint8_t i) {
  if (hue++ & 0x01)
    hue2++;

  trackingObjectPosX[i] = WIDTH * 0.5;
  trackingObjectPosY[i] = HEIGHT * 0.5;


  trackingObjectSpeedX[i] = (((float)random8()-127.)/512.); 
  trackingObjectSpeedY[i] = EffectMath::sqrt(0.0626-trackingObjectSpeedX[i] * trackingObjectSpeedX[i]); 
  
  if(random8(2U)) trackingObjectSpeedY[i]=-trackingObjectSpeedY[i];

  trackingObjectState[i] = EffectMath::randomf(50, 250); 
#ifdef MIC_EFFECTS
  if (type)
    trackingObjectHue[i] = isMicOn() ? getMicMapFreq() : hue2;
  else 
    trackingObjectHue[i] = random8(getMicMapFreq(), 255);
#else
  if (type)
    trackingObjectHue[i] = hue2;
  else 
    trackingObjectHue[i] = random8(255);
#endif
  trackingObjectIsShift[i] = true; 
}

void EffectFairy::fount(){
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле

  fb.dim(EffectMath::fmap(speed, 1, 255, 180, 127)); //ахах-ха. очередной эффект, к которому нужно будет "подобрать коэффициенты"

  //go over particles and update matrix cells on the way
  for (int i = 0; i < enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      fountEmit(i);
      step--;
    }
    if (trackingObjectIsShift[i]) { 
      particlesUpdate(i);

      //generate RGB values for particle
      CRGB baseRGB;
        baseRGB = CHSV(trackingObjectHue[i], 255, _video); 

      baseRGB.nscale8(trackingObjectState[i]);
      EffectMath::drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB, fb, 0);
    }
  }
  if (blur) EffectMath::blur2d(fb, blur * 10); // Размытие 
}

// ============= ЭФФЕКТ ФЕЯ ===============
// (c) SottNick
#define FAIRY_BEHAVIOR //типа сложное поведение

void EffectFairy::fairyEmit(uint8_t i) {
    if (deltaHue++ & 0x01)
      if (hue++ & 0x01)
        hue2++;//counter++;
    trackingObjectPosX[i] = boids[0].location.x;
    trackingObjectPosY[i] = boids[0].location.y;

    //хотите навставлять speedFactor? - тут не забудьте
    //trackingObjectSpeedX[i] = ((float)random8()-127.)/512./0.25*speedFactor; 
    trackingObjectSpeedX[i] = ((float)random8()-127.)/512.; 
    trackingObjectSpeedY[i] = EffectMath::sqrt(0.0626-trackingObjectSpeedX[i]*trackingObjectSpeedX[i]); 
    if(random8(2U)) { trackingObjectSpeedY[i]=-trackingObjectSpeedY[i]; }

    trackingObjectState[i] = random8(20, 80); 
    trackingObjectHue[i] = hue2;
    trackingObjectIsShift[i] = true; 
}

bool EffectFairy::fairy() {
  step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле
  
#ifdef FAIRY_BEHAVIOR
  if (!deltaHue && deltaHue2 && fabs(boids[0].velocity.x) + fabs(boids[0].velocity.y) < 0.15){ 
    deltaHue2 = 0U;
    
    boids[1].velocity.x = (((float)random8()+255.) / 4080.);
    boids[1].velocity.y = (((float)random8()+255.) / 2040.);
    if (boids[0].location.x > WIDTH * 0.5) boids[1].velocity.x = -boids[1].velocity.x;
    if (boids[0].location.y > HEIGHT * 0.5) boids[1].velocity.y = -boids[1].velocity.y;
  }
  if (!deltaHue2){
    step = 1U;
    
    boids[0].location.x += boids[1].velocity.x * speedFactor;
    boids[0].location.y += boids[1].velocity.y * speedFactor;
    deltaHue2 = (boids[0].location.x <= 0 || boids[0].location.x >= fb.cfg.maxWidthIndex() || boids[0].location.y <= 0 || boids[0].location.y >= fb.cfg.maxHeightIndex());
  }
  else
#endif // FAIRY_BEHAVIOR
  {  
    PVector attractLocation = PVector(WIDTH * 0.5, HEIGHT * 0.5);
    //float attractMass = 10;
    //float attractG = .5;
    // перемножаем и получаем 5.
    Boid boid = boids[0];
    PVector force = attractLocation - boid.location;      // Calculate direction of force
    float d = force.mag();                                // Distance between objects
    d = constrain(d, 5.0f, HEIGHT);//видео снято на 5.0f  // Limiting the distance to eliminate "extreme" results for very close or very far objects
//d = constrain(d, modes[currentMode].Scale / 10.0, HEIGHT);

    force.normalize();                                    // Normalize vector (distance doesn't matter here, we just want this vector for direction)
    float strength = (5. * boid.mass) / (d * d);          // Calculate gravitional force magnitude 5.=attractG*attractMass
//float attractMass = (modes[currentMode].Scale) / 10.0 * .5;
    force *= strength * speedFactor;                                    // Get force vector --> magnitude * direction
    boid.applyForce(force);
    boid.update();
    
    if (boid.location.x <= -1) boid.location.x = -boid.location.x;
    else if (boid.location.x >= WIDTH) boid.location.x = -boid.location.x+WIDTH+WIDTH;
    if (boid.location.y <= -1) boid.location.y = -boid.location.y;
    else if (boid.location.y >= HEIGHT) boid.location.y = -boid.location.y+HEIGHT+HEIGHT;
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

  fb.dim(EffectMath::fmap(speed, 1, 255, 180, 127));

  //go over particles and update matrix cells on the way
  for(int i = 0; i<enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i] && step) {
      fairyEmit(i);
      step--;
    }
    if (trackingObjectIsShift[i]){ 
      // вернуться и поглядеть, что это
      if (type && trackingObjectSpeedY[i] > -1) trackingObjectSpeedY[i] -= 0.05; //apply acceleration
      particlesUpdate(i);

      //generate RGB values for particle
      CRGB baseRGB = CHSV(trackingObjectHue[i], 255,255); 

      baseRGB.nscale8(trackingObjectState[i]);//эквивалент
      EffectMath::drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB, fb, 0);
    }
  }

  EffectMath::drawPixelXYF(boids[0].location.x, boids[0].location.y, CHSV(hue, 160U, 255U) /*temp*/, fb, 0);  
  return true;
}

bool EffectFairy::run() {
#ifdef MIC_EFFECTS
   _video = isMicOn() ? constrain(getMicMaxPeak() * EffectMath::fmap(gain, 1.0f, 255.0f, 1.25f, 5.0f), 48U, 255U) : 255;
#endif

  switch (effect)
  {
  case EFF_FAIRY:
    return fairy();
    break;
  case EFF_FOUNT:
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
  if(effect==EFF_FAIRY)
    deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
  else
    deltaValue = enlargedObjectNUM / (EffectMath::sqrt(CENTER_X_MAJOR * CENTER_X_MAJOR + CENTER_Y_MAJOR * CENTER_Y_MAJOR) * 4U) + 1U; // 4 - это потому что за 1 цикл частица пролетает ровно четверть расстояния между 2мя соседними пикселями

  enlargedObjectNUM = map(scale, 1, 255, 4, trackingOBJECT_MAX_COUNT);
  if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT)
    enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
  for (uint16_t i = 0; i < enlargedOBJECT_MAX_COUNT; i++)
    trackingObjectIsShift[i] = false; 

  //---- Только для эффекта Фея
  // лень было придумывать алгоритм для таектории феи, поэтому это будет нулевой "бойд" из эффекта Притяжение
  boids[0] = Boid(random8(WIDTH), random8(HEIGHT));
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
String EffectFairy::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    if (effect == EFF_FAIRY) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.05, .25) * EffectCalc::speedfactor;
    else speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.2, 1.) * EffectCalc::speedfactor;
  } else if(_val->getId()==2) {
    enlargedObjectNUM = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 4, trackingOBJECT_MAX_COUNT);
    if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT)
      enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
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
String EffectCircles::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 5) * EffectCalc::speedfactor;
  else if(_val->getId()==2) count = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 3, NUMBER_OF_CIRCLES);
  else if(_val->getId()==4) gain = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectCircles::load() {
  palettesload();
  for (uint8_t i = 0; i < NUMBER_OF_CIRCLES; i++) {
    circles[i].reset();
  }
}

void EffectCircles::drawCircle(LedFB &fb, Circle circle) {
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
      fb.pixel(x, y) += ColorFromPalette(*curPalette, hue, brightness);
    }
  }
}

bool EffectCircles::run() {
#ifdef MIC_EFFECTS
  _video = isMicOn() ? constrain(getMicMaxPeak() * EffectMath::fmap(gain, 1.0f, 255.0f, 1.25f, 5.0f), 48U, 255U) : 255;
#endif
  randomSeed(millis());
  FastLED.clear();
  for (uint8_t i = 0; i < count; i++) {
    circles[i].bpm += speedFactor;
    if (circles[i].radius() < 0.001) {
#ifdef MIC_EFFECTS
      circles[i].hue = isMicOn() ? getMicMapFreq() : random(0, NUMBER_OF_CIRCLES) * 255 / count;
#else
      circles[i].hue = random(0, NUMBER_OF_CIRCLES) * 255 / count;
#endif
      circles[i].move();
    }
    drawCircle(fb, circles[i]);
  }
  return true;
}

// ----------- Эффект "Бенгальские Огни"
// (c) stepko https://wokwi.com/arduino/projects/289797125785520649
// 06.02.2021
void EffectBengalL::load() {
  for (byte i = 0; i < sparksNum; i++) {
    regen(i);
  }
  //speedFactor = EffectMath::fmap(speed, 1, 255, 0.1, 1)*EffectCalc::speedfactor;
}

//!++
String EffectBengalL::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.1, 1) * EffectCalc::speedfactor;
  else if(_val->getId()==3) centerRun = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectBengalL::phisics(byte id) {
  sparksPos[0][id] += sparksSpeed[0][id] * speedFactor;
  sparksPos[1][id] += sparksSpeed[1][id] * speedFactor;
  sparksSpeed[1][id] -= .98 * speedFactor;
  sparksSat[id] += (255. / (float)WIDTH) * speedFactor;            // остывание искор
  sparksFade[id] -= (255. / (float)(HEIGHT*1.5)) * speedFactor;    // угасание искор
  if (sparksSpeed[0][id] > 0)
    sparksSpeed[0][id] -= 0.1 * speedFactor;
  else
    sparksSpeed[0][id] += 0.1 * speedFactor;
  if (sparksPos[0][id] <= 0 || sparksPos[0][id] >= WIDTH * 10 || sparksPos[1][id] < 0) {
    regen(id);
  }
}

void EffectBengalL::regen(byte id) {
  sparksPos[0][id] = gPos[0];
  sparksPos[1][id] = gPos[1];
  sparksSpeed[0][id] = random(-10, 10);
  sparksSpeed[1][id] = random(-5, 20);
  sparksColor[id] = random8();
  sparksSat[id] = 10;
  sparksFade[id] = 255;
}


bool EffectBengalL::run() {
  fb.fade(beatsin8(5, 20, 100));
  if (centerRun) {
    gPos[0] = beatsin16(_x, 0, fb.cfg.maxWidthIndex() * 10);
    gPos[1] = beatsin16(_y, 0, fb.cfg.maxHeightIndex() * 10);
  } else {
    gPos[0] = WIDTH / 2 * 10;
    gPos[1] = HEIGHT / 2 * 10;
  }
  for (byte i = 0; i < map(scale, 1, 255, 8, sparksNum); i++) {
    phisics(i);
    if (sparksPos[1][i] < (fb.cfg.maxHeightIndex() * 10) and sparksPos[1][i] >= 0)
      if (sparksPos[0][i] < (fb.cfg.maxWidthIndex() * 10) and sparksPos[0][i] >= 0)
        EffectMath::drawPixelXYF(sparksPos[0][i] / 10,  sparksPos[1][i] / 10, CHSV(sparksColor[i], constrain(sparksSat[i], 5, 255), constrain(sparksFade[i], 32, 255)), fb);
  }

  EVERY_N_SECONDS(period) {
    for (byte i = 0; i < map(scale, 1, 255, 1, sparksNum); i++) regen(i);
    period = random(10, 60);
    _x = random(1, 10);
    _y = random(1, 10);
  }
  return true;
}

// ----------- Эффект "Шары"
// (c) stepko and kostyamat https://wokwi.com/arduino/projects/289839434049782281
// 07.02.2021
void EffectBalls::load() {
  randomSeed(millis());
  palettesload();

  speedFactor = EffectMath::fmap(speed, 1, 255, 0.15, 0.5) * EffectCalc::speedfactor;

  for (byte i = 0; i < ballsAmount; i++) {
    radius[i] = EffectMath::randomf(0.5, radiusMax);
    ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
    ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
    ball[i][0] = random(0, WIDTH);
    ball[i][1] = random(0, HEIGHT);
    color[i] = random(0, 255);
  }
}


// !++
String EffectBalls::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.15, 0.5) * EffectCalc::speedfactor;
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectBalls::run() {
  fb.fade(map(speed, 1, 255, 5, 20));

  for (byte i = 0; i < map(scale, 1, 255, 2, ballsAmount); i++) {
    if (rrad[i]) {  // тут у нас шарики надуваются\сдуваются по ходу движения
      radius[i] += (fabs(ball[i][2]) > fabs(ball[i][3])? fabs(ball[i][2]) : fabs(ball[i][3])) * 0.1 * speedFactor;
      if (radius[i] >= radiusMax) {
        rrad[i] = false;
      }
    } else {
      radius[i] -= (fabs(ball[i][2]) > fabs(ball[i][3])? fabs(ball[i][2]) : fabs(ball[i][3])) * 0.1 * speedFactor;
      if (radius[i] < 1.) {
        rrad[i] = true;
        color[i] = random(0, 255);
      }
    }


    //EffectMath::drawCircleF(ball[i][1], ball[i][0], radius[i], ColorFromPalette(*curPalette, color[i]), 0.5);
    if (radius[i] > 1) 
      EffectMath::fill_circleF(ball[i][1], ball[i][0], radius[i], ColorFromPalette(*curPalette, color[i]), fb);
    else 
      EffectMath::drawPixelXYF(ball[i][1], ball[i][0], ColorFromPalette(*curPalette, color[i]), fb);


    if (ball[i][0] + radius[i] >= fb.cfg.maxHeightIndex())
      ball[i][0] += (ball[i][2] * ((fb.cfg.maxHeightIndex() - ball[i][0]) / radius[i] + 0.005));
    else if (ball[i][0] - radius[i] <= 0)
      ball[i][0] += (ball[i][2] * (ball[i][0] / radius[i] + 0.005));
    else
      ball[i][0] += ball[i][2];
    //-----------------------
    if (ball[i][1] + radius[i] >= fb.cfg.maxWidthIndex())
      ball[i][1] += (ball[i][3] * ((fb.cfg.maxWidthIndex() - ball[i][1]) / radius[i] + 0.005));
    else if (ball[i][1] - radius[i] <= 0)
      ball[i][1] += (ball[i][3] * (ball[i][1] / radius[i] + 0.005));
    else
      ball[i][1] += ball[i][3];
    //------------------------
    if (ball[i][0] < 0.01) {
      ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][0] = 0.01;
    }
    else if (ball[i][0] > (float)fb.cfg.maxHeightIndex()) {
      ball[i][2] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][2] = -ball[i][2];
      ball[i][0] = (float)fb.cfg.maxHeightIndex();
    }
    //----------------------
    if (ball[i][1] < 0.01) {
      ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][1] = 0.01;
    }
    else if (ball[i][1] > fb.cfg.maxWidthIndex()) {
      ball[i][3] = EffectMath::randomf(0.5, 1.1) * speedFactor;
      ball[i][3] = -ball[i][3];
      ball[i][1] = fb.cfg.maxWidthIndex();
    }
  }
  EffectMath::blur2d(fb, 48);
  return true;
}

// ---------- Эффект-игра "Лабиринт"
EffectMaze::EffectMaze(LedFB &framebuffer) : EffectCalc(framebuffer),
  _mwidth(fb.cfg.w()%2 ? fb.cfg.w() : fb.cfg.w()-1),
  _mheight(fb.cfg.h()%2 ? fb.cfg.h() : fb.cfg.h()-1),
  maxSolves(_mwidth*_mheight*5),
  maze(std::vector<uint8_t>(_mwidth*_mheight))
  {};

void EffectMaze::newGameMaze() {
  playerPos[0] = !MAZE_SHIFT;
  playerPos[1] = !MAZE_SHIFT;

  gameOverFlag = false;
  buttons = 4;

  GenerateMaze(maze, _mwidth, _mheight);    // генерировать лабиринт обычным способом
  SolveMaze(maze, _mwidth, _mheight);       // найти путь

  if (!(MAZE_GAMEMODE || mazeMode)) {
    for (byte y = 0; y < _mheight; y++) {
      for (byte x = 0; x < _mwidth; x++) {
        switch (maze[(y + MAZE_SHIFT) * _mwidth + (x + MAZE_SHIFT)]) {
          case 1:  fb.pixel(x, y)= color; break;
          case 2:
            fb.pixel(x, y) = 0x000000;
            break;
          default: fb.pixel(x, y) = 0x000000; break;
        }
      }
      // Отображаем сгенерированный лабиринт строка за строкой
      //FastLED.show();  // стоит убрать, и все начинает мерцать
      //delay(50);
    }

  } else {
    for (byte y = 0; y < MAZE_FOV; y++) {
      for (byte x = 0; x < MAZE_FOV; x++) {
        switch (maze[(y + MAZE_SHIFT) * _mwidth + (x + MAZE_SHIFT)]) {
          case 1:  fb.pixel(x, y) = color;  break;
          default:
            fb.pixel(x, y) = 0x000000;
        }
      }
    }
  }

  // Отрисовка - с видимыми границами по периметру (настройки MAZE_SHIFT выше)
  // Слева от начальной позиции делаем дыру - это вход
  if (playerPos[0]>0) {
    playerPos[0] = playerPos[0] - 1;
    fb.pixel(playerPos[0], playerPos[1]) = 0x000000;
  }
  
  fb.pixel(playerPos[0], playerPos[1]) = playerColor;

  mazeStarted = false;  
}

bool EffectMaze::run() {
  if (loadingFlag || gameOverFlag) {  
    if (loadingFlag) fb.clear();
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
      if (newPos >= 0 && newPos <= fb.cfg.maxWidthIndex())
        if (!fb.pixel(newPos, playerPos[1])) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 1) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[0] + 1;
      if (newPos >= 0 && newPos <= fb.cfg.maxWidthIndex())
        if (!fb.pixel(newPos, playerPos[1])) {
          movePlayer(newPos, playerPos[1], playerPos[0], playerPos[1]);
          playerPos[0] = newPos;
        }
    }
    if (buttons == 0) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] + 1;
      if (newPos >= 0 && newPos <= fb.cfg.maxHeightIndex())
        if (!fb.pixel(playerPos[0], newPos)) {
          movePlayer(playerPos[0], newPos, playerPos[0], playerPos[1]);
          playerPos[1] = newPos;
        }
    }
    if (buttons == 2) {   // кнопка нажата
      btnPressed = true;
      int8_t newPos = playerPos[1] - 1;
      if (newPos >= 0 && newPos <= fb.cfg.maxHeightIndex())
        if (!fb.pixel(playerPos[0], newPos)) {
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
  if (!track) fb.pixel(prevX, prevY) = 0x000000;
  fb.pixel(nowX, nowY) = playerColor;

  if ((nowX == (_mwidth - 2) - MAZE_SHIFT) && (nowY == (_mheight - 1) - MAZE_SHIFT)) {
    gameOverFlag = true;
    return;
  }

  if (MAZE_GAMEMODE || mazeMode) {
    for (int8_t y = nowY - MAZE_FOV; y < nowY + MAZE_FOV; y++) {
      for (int8_t x = nowX - MAZE_FOV; x < nowX + MAZE_FOV; x++) {
        if (x < 0 || x > fb.cfg.maxWidthIndex() || y < 0 || y > fb.cfg.maxHeightIndex()) continue;
        if (maze[(y + MAZE_SHIFT) * _mwidth + (x + MAZE_SHIFT)] == 1) {
          fb.pixel(x, y) = CRGB::Aqua;
        }
      }
      //FastLED.show();
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
  if ( (maze[(playerPos[1] + y + MAZE_SHIFT) * _mwidth + (playerPos[0] + x + MAZE_SHIFT)]) == 2) {
    maze[(playerPos[1] + MAZE_SHIFT) * _mwidth + (playerPos[0] + MAZE_SHIFT)] = 4;   // убираем текущую клетку из пути (2 - метка пути, ставим любое число, например 4)
    return true;
  }
  return false;
}

// копаем лабиринт
void EffectMaze::CarveMaze(std::vector<uint8_t> &maze, int width, int height, int x, int y) {
  int x1, y1;
  int x2, y2;
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
    if (   x2 > 0 && x2 < width && y2 > 0 && y2 < height
           && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
      maze[y1 * width + x1] = 0;
      maze[y2 * width + x2] = 0;
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
void EffectMaze::GenerateMaze(std::vector<uint8_t> &maze, int width, int height) {
  int x, y;
  maze.assign(maze.size(), 1);

  maze[1 * width + 1] = 0;
  for (y = 1; y < height; y += 2) {
    for (x = 1; x < width; x += 2) {
      CarveMaze(maze, width, height, x, y);
    }
  }
  // вход и выход
  maze[0 * width + 1] = 0;
  maze[(height - 1) * width + (width - 2)] = 0;

  track = random8(0,2);
  color = CHSV(hue += 8, random8(192, 255), 192);
  
 /* CHSV tmp = rgb2hsv_approximate(color);
  tmp.s = 255;
  tmp.hue += 100;
  tmp.val = 255; */

  playerColor = CHSV(hue + random(63, 127), random8(127, 200), 255);;
  
}

// решатель (ищет путь)
void EffectMaze::SolveMaze(std::vector<uint8_t> &maze, int width, int height) {
  int dir{0}, count{0};
  int x{1}, y{1};
  int dx, dy;
  int forward = 1;
  // Remove the entry and exit. 
  maze[0 * width + 1] = 1;
  maze[(height - 1) * width + (width - 2)] = 1;

  unsigned int attempts = 0;
  while (x != width - 2 || y != height - 2) {
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
    if (   (forward  && maze[(y + dy) * width + (x + dx)] == 0)
           || (!forward && maze[(y + dy) * width + (x + dx)] == 2)) {
      maze[y * width + x] = forward ? 2 : 3;
      x += dx;
      y += dy;
      forward = 1;
      count = 0;
      dir = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
      if (count > 3) {
        forward = 0;
        count = 0;
      }
    }
  }
  
  // Replace the entry and exit.
  maze[(height - 2) * width + (width - 2)] = 2;
  maze[(height - 1) * width + (width - 2)] = 2;
}

// !++
String EffectMaze::setDynCtrl(UIControl*_val){
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
    fb.pixel(beatsin8(12. * _speed + i * _speed, 0, fb.cfg.maxWidthIndex()), beatsin8(15. * _speed + i * _speed, 0, fb.cfg.maxHeightIndex())) = CHSV(beatsin8(12. * _speed, 0, 255), scale > 127 ? 255 - i*8 : 255, scale > 127 ? 127 + i*8 : 255);
  EffectMath::blur2d(fb, 16);
  return true;
}

// --------- Эффект "Северное Сияние"
// (c) kostyamat 05.02.2021
// идеи подсмотрены тут https://www.reddit.com/r/FastLED/comments/jyly1e/challenge_fastled_sketch_that_fits_entirely_in_a/
void EffectPolarL::load() {
  adjastHeight = EffectMath::fmap((float)HEIGHT, 8, 32, 28, 12);
  adjScale = map((int)WIDTH, 8, 64, 310, 63);
  palettesload();
}

void EffectPolarL::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(numpalettes);
  palettes.push_back(&AuroraColors_p); 
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&PartyColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&GreenAuroraColors_p); 
  palettes.push_back(&BlueAuroraColors_p);
  palettes.push_back(&NeonAuroraColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&WaterfallColors_p); 
   
  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
}

// нахрена кастомный мапинг тут???
// А потому, что мне нужно переменную flag получить! Есть варианты? Вариант ничем не хуже setDynCtrl

void EffectPolarL::palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min, const uint8_t _max){
  std::size_t idx = (_val-1); // т.к. сюда передается точное значение контрола, то приводим его к 0
  if (!_pals.size() || idx>=_pals.size()) {
    LOG(println,F("No palettes loaded or wrong value!"));
    return;
  }
  flag = idx;
  curPalette = _pals.at(idx);
}

// !++
String EffectPolarL::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) _speed = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 128, 16);
  else if(_val->getId()==2) _scale = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 30, adjScale);
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectPolarL::run() {
  
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      timer++;
      fb.pixel(x, y) =
          ColorFromPalette(*curPalette,
            qsub8(
              inoise8(/*i*/timer % 2 + x * _scale,
                y * 16 + timer % 16,
                timer / _speed
              ),
              fabs((float)HEIGHT/2 - (float)y) * adjastHeight
            )
          );
      if (flag == 1) { // Тут я модифицирую стандартные палитры 
        CRGB tmpColor = fb.pixel(x, y);
        CRGB led = tmpColor;
        led.g = tmpColor.r;
        led.r = tmpColor.g;
        led.g /= 6;
        led.r += led.r < 206 ? 48 : 0;
        fb.pixel(x, y) = led;
      } else if (flag == 3) {
        CRGB led = fb.pixel(x, y);
        led.b += 48;
        led.g += led.g < 206 ? 48 : 0;
        fb.pixel(x, y) = led;
      }
    }
  }

  return true;
}

// --------- Эффект "Космо-Гонщик"
// (c) Stepko + kostyamat https://editor.soulmatelights.com/my-patterns/655
// void EffectRacer::setspd(const byte _spd) {
//   EffectCalc::setspd(_spd);
//   speedFactor = _speed * EffectMath::fmap(speed, 1, 255, 0.33, 2);
//   addRadius = _addRadius * EffectMath::fmap(speed, 1, 255, 0.33, 2);
// }
// !++
String EffectRacer::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    uint8_t sp = EffectCalc::setDynCtrl(_val).toInt();
    speedFactor = EffectMath::fmap(sp, 1, 255, 0.33, 2) * EffectCalc::speedfactor;
    addRadius = _addRadius * EffectMath::fmap(sp, 1, 255, 0.33, 2);
  } else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectRacer::run() {
  fb.fade(16. * speedFactor);

  if (round(posX / 4) > aimX) {
    posX -= speedFactor;
  }
  if (round(posY / 4) > aimY) {
    posY -= speedFactor;
  }
  if (round(posX / 4) < aimX) {
    posX += speedFactor;
  }
  if (round(posY / 4) < aimY) {
    posY += speedFactor;
  }
  if (round(posX / 4) == aimX && round(posY / 4) == aimY) {
    aimChange();
  }
  radius += addRadius;
  angle += radius;
  switch (hue%3)
  {
  case 0:
    EffectMath::drawCircleF(aimX, aimY, radius, color, fb); // рисуем круг
    break;  
  case 1:
    drawStarF(aimX, aimY, 1.3 * radius, radius, 4, angle, color); // рисуем квадрат
    break;
  case 2:
    drawStarF(aimX, aimY, 2 * radius, radius, starPoints, angle, color); // рисуем звезду
    break;
  }
  
  EffectMath::drawPixelXYF(posX / 4, posY / 4, CHSV(0, 0, 255), fb); // отрисовываем бегуна

  return true;
}

void EffectRacer::load() {
  palettesload();
}

void EffectRacer::aimChange() {
  aimX = random(0, fb.cfg.maxWidthIndex());  // позиция цели 
  aimY = random(0, fb.cfg.maxHeightIndex());
  radius = 1; // начальный размер цели = 1 пиксель
  hue = millis()>>1; //random(0, 255);
  color = ColorFromPalette(*curPalette, hue, 180);
  starPoints = random(3, 7); // количество лучей у звезды
}

void EffectRacer::drawStarF(float x, float y, float biggy, float little, int16_t points, float dangle, CRGB color) {
  float radius2 = 255.0 / points;
  for (int i = 0; i < points; i++) {
    EffectMath::drawLineF(x + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), y + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), x + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), y + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), color, fb);
    EffectMath::drawLineF(x + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), y + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), x + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), y + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), color, fb);
  }
}

// ----------------- Эффект "Магма"
// (c) Сотнег (SottNick) 2021
// адаптация и доводка до ума - kostyamat
void EffectMagma::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&MagmaColor_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&HeatColors2_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&NormalFire3_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&NormalFire2_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&WaterfallColors_p);
  
  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
}

void EffectMagma::load() {
  palettesload();
  regen();
}

// !++
String EffectMagma::setDynCtrl(UIControl*_val){
  if (_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.075, .5) * EffectCalc::speedfactor;
  else if(_val->getId()==3) {
    long scale = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 100, MAGMA_MIN_OBJ, MAGMA_MAX_OBJ);
    particles.assign(scale, Magma());
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  regen();
  return String();
}

void EffectMagma::regen() {
  for (uint8_t j = 0; j != shiftHue.size(); ++j){
    shiftHue[j] = map(j, 0, HEIGHT+HEIGHT/4, 255, 0);// init colorfade table
  }

  for (auto &i : particles){
    leapersRestart_leaper(i);
    i.hue = 50U;
  }
}

bool EffectMagma::run() {
  fb.fade(50);
  

  for (auto &i : particles){
    leapersMove_leaper(i);
    i.hue = 50U;
    EffectMath::drawPixelXYF(i.posX, i.posY, ColorFromPalette(*curPalette, i.hue), fb, 0);
  }

  for (uint8_t i = 0; i < WIDTH; i++) {
    for (uint8_t j = 0; j < HEIGHT; j++) {
     fb.pixel(i, fb.cfg.maxHeightIndex() - j) += ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j + ff_y + random8(2)) * deltaHue, ff_z), shiftHue[j]), 127U);
    }
  }

  ff_y += speedFactor * 2;
  ff_z += speedFactor;
  //EffectMath::blur2d(fb.data(), fb.cfg.w, fb.cfg.h, 32);
  return true;
}

void EffectMagma::leapersMove_leaper(Magma &l) {

  l.posX += l.speedX * speedFactor;
  l.posY += l.shift * speedFactor;

  // bounce off the ceiling?
  if (l.posY > HEIGHT + HEIGHT/4) {
    l.shift *= -1;
  }
  
  // settled on the floor?
  if (l.posY <= (HEIGHT/8-1)) {
    leapersRestart_leaper(l);
  }

  // bounce off the sides of the screen?
  if (l.posX < 0 || l.posX > fb.cfg.maxWidthIndex()) {
    leapersRestart_leaper(l);
  }
  
  l.shift -= gravity * speedFactor;
}

void EffectMagma::leapersRestart_leaper(Magma &l) {
  randomSeed(millis());
  // leap up and to the side with some random component
  l.speedX = EffectMath::randomf(-0.75, 0.75);
  l.shift = EffectMath::randomf(0.50, 0.85);
  l.posX = EffectMath::randomf(0, WIDTH);
  l.posY = EffectMath::randomf(0, (float)HEIGHT/4-1);

  // for variety, sometimes go 100% faster
  if (random8() < 12) {
    l.shift += l.shift * EffectMath::randomf(1.5, 2.5);
  }
}


// --------------------- Эффект "Звездный Десант"
// Starship Troopers https://editor.soulmatelights.com/gallery/839-starship-troopers
// Based on (c) stepko`s codes https://editor.soulmatelights.com/gallery/639-space-ships
// reworked (c) kostyamat (subpixel, shift speed control, etc)  08.04.2021

// !++
String EffectStarShips::setDynCtrl(UIControl*_val){
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
  FastLED.clear();
}

bool EffectStarShips::run() {
  fb.fade(_fade);
  switch (dir) {
    case 0: // Up
      for (byte x = 0; x < WIDTH; x++) {
		    if (!_dir and x > WIDTH/2 and random8(chance) == DIR_CHARGE) {count++; break;}
        for (float y = 0; y < HEIGHT; y+=speedFactor) {
          fb.pixel(x, y) = (((int)y == fb.cfg.maxHeightIndex()) ? CRGB::Black : fb.pixel(x, y + 1));
        }
      }
      break;
    case 1: // Up - Right 
      for (float x = 0; x < WIDTH; x+=speedFactor) {
        if (!_dir and (uint8_t)x > WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (byte y = 0; y < HEIGHT; y++) {
          fb.pixel(x, y) = ((y == fb.cfg.maxHeightIndex() or (int)x == fb.cfg.maxWidthIndex()) ? CRGB::Black : fb.pixel(x + 1, y + 1));
        }
      }
      break;
    case 2: // Right
      for (float x = 0; x < WIDTH; x+=speedFactor) {
        if (!_dir and (uint8_t)x > WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb.cfg.maxHeightIndex(); y > 0; y--) {
          fb.pixel(x, y) = (((int)x == fb.cfg.maxWidthIndex()) ? CRGB::Black : fb.pixel(x + 1, y));
        }
      }
      break;
    case 3: // Down - Right 
      for (float x = 0; x < WIDTH; x+=speedFactor) {
        if (!_dir and (uint8_t)x > WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb.cfg.maxHeightIndex(); y > 0; y--) {
          fb.pixel(x, y) = (((int)x == fb.cfg.maxWidthIndex() or y == 0) ? CRGB::Black : fb.pixel(x + 1, y - 1));
        }
      }
      break;
    case 4: // Down
      for (byte x = 0; x < WIDTH; x++) {
		    if (!_dir and x < WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (float y = fb.cfg.maxHeightIndex(); y > 0; y-=speedFactor) {
          fb.pixel(x, y) = (((int)y == 0) ? CRGB::Black : fb.pixel(x, y - 1));
        }
      }
      break;
    case 5: // Down - Left
      for (float x = fb.cfg.maxWidthIndex(); x > 0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb.cfg.maxHeightIndex(); y > 0; y--) {
          fb.pixel(x, y) = ((y == 0 or (int)x == 0) ? CRGB::Black : fb.pixel(x - 1, y - 1));
        }
      }
      break;
    case 6: // Left
      for (float x = fb.cfg.maxWidthIndex(); x > 0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb.cfg.maxHeightIndex(); y > 0; y--) {
          fb.pixel(x, y) = ((int)x == 0 ? CRGB::Black : fb.pixel(x - 1, y));
        }
      }
      break;
    case 7: // Up - Left 
      for (float x = WIDTH -1; x >0; x-=speedFactor) {
        if (!_dir and (uint8_t)x < WIDTH/2 and random(chance) == DIR_CHARGE) {count++; break;}
        for (uint8_t y = fb.cfg.maxHeightIndex(); y > 0; y--) {
          fb.pixel(x, y) = ((y == fb.cfg.maxHeightIndex() or (int)x == 0) ? CRGB::Black : fb.pixel(x - 1, y + 1));
        }
      }
      break;
  }

  for (byte i = 0; i < _scale; i++) {
    float x = (float)beatsin88(3840*speedFactor + i*256, 0, fb.cfg.maxWidthIndex() *4, 0, _scale*i*256) /4;
    float y = (float)beatsin88(3072*speedFactor + i*256, 0, fb.cfg.maxWidthIndex() *4, 0, 0) /4;
    if ((x >= 0 and x <= fb.cfg.maxWidthIndex()) and (y >= 0 and y <= fb.cfg.maxHeightIndex())) draw(x, y, ColorFromPalette(*curPalette, beatsin88(256*12.*speedFactor + i*256, 0, 255), 255));
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
  if (WIDTH > 24 || HEIGHT > 24) {
    if (x < WIDTH - 2) EffectMath::drawPixelXYF(x + 1, y, color, fb);
    if (x > 1) EffectMath::drawPixelXYF(x - 1, y, color, fb);
    if (y < HEIGHT - 2) EffectMath::drawPixelXYF(x, y + 1, color, fb);
    if (y > 1) EffectMath::drawPixelXYF(x, y - 1, color, fb);
  }
}

// ------------- Эффект "Флаги"
// (c) Stepko + kostyamat
// 17.03.21
// https://editor.soulmatelights.com/gallery/739-flags
String EffectFlags::setDynCtrl(UIControl*_val){
  if (_val->getId()==1)
    _speed = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 1, 16);
  else if (_val->getId()==3) _flag = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFlags::run() {
  changeFlags();
  fb.fade(32);
  for (uint8_t i = 0; i < WIDTH; i++) {
    thisVal = inoise8((float) i * DEVIATOR, counter, (int)count/*(float)i * SPEED_ADJ/2*/);
    thisMax = map(thisVal, 0, 255, 0, fb.cfg.maxHeightIndex());
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
      break;
    }

  }
  EffectMath::blur2d(fb, 32);
  counter += (float)_speed * SPEED_ADJ;
  return true;
}

void EffectFlags::changeFlags() {
  if (!_flag) {
    EVERY_N_SECONDS(CHANGE_FLAG) {
      count++;
      flag = count % 10;
    }
  }
  else
    flag = _flag - 1;
}

#ifdef MIC_EFFECTS
/* -------------- эффект "VU-Meter"
    (c) G6EJD, https://www.youtube.com/watch?v=OStljy_sUVg&t=0s
    reworked by s-marley https://github.com/s-marley/ESP32_FFT_VU
    adopted for FireLamp_jeeUI by kostyamat, kDn
    reworked and updated (c) kostyamat 24.04.2021
*/
String EffectVU::setDynCtrl(UIControl*_val){
  if (_val->getId()==1) amplitude = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.025, 0.5);
  else if (_val->getId()==2) threshold = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0, 30);
  else if (_val->getId()==3) {
    effId = EffectCalc::setDynCtrl(_val).toInt() - 1;
    NUM_BANDS = effId & 01 ? (WIDTH/2 + (WIDTH & 01 ? 1:0)) : WIDTH;
    BAR_WIDTH =  (WIDTH  / (NUM_BANDS - 1));

    //memset(oldBarHeights,0,sizeof(oldBarHeights));
    for(uint16_t i = 0; i < WIDTH; i++) {
      oldBarHeights[i] = 0.;
      bandValues[i] = 0.;
    }
  }
  else if (_val->getId()==4) colorType = EffectCalc::setDynCtrl(_val).toInt() - 1;
  else if (_val->getId()==5) {
    colorTimer = EffectCalc::setDynCtrl(_val).toInt();
    if (!colorTimer) colorShifting = true;
    else colorShifting = false;
  }
  else if (_val->getId()==6) type = EffectCalc::setDynCtrl(_val).toInt();
  else if (_val->getId()==7) averaging = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectVU::load() {
#ifdef MIC_EFFECTS
  setMicAnalyseDivider(0); // отключить авто-работу микрофона, т.к. тут все анализируется отдельно, т.е. не нужно выполнять одну и ту же работу дважды
#endif
    NUM_BANDS = effId & 01 ? (WIDTH/2 + (WIDTH & 01 ? 1:0)) : WIDTH;
    BAR_WIDTH =  (WIDTH  / (NUM_BANDS - 1));

  //memset(oldBarHeights,0,sizeof(oldBarHeights));
  for(uint16_t i = 0; i < WIDTH; i++) {
    oldBarHeights[i] = 0.0;
    bandValues[i] = 0.0;
  }

}

bool EffectVU::run() {
#ifdef MIC_EFFECTS
  setMicAnalyseDivider(0); // отключить авто-работу микрофона, т.к. тут все анализируется отдельно, т.е. не нужно выполнять одну и ту же работу дважды
#endif
  // Оставлю себе напоминалку как все это работает https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/post-85649
  //bool ready = false;
  tickCounter++;
#ifdef MIC_EFFECTS
  if(isMicOn()){ // вот этот блок медленный, особенно нагружающим будет вызов заполенния массива
    //EVERY_N_MILLIS(100){ // обсчет тяжелый, так что желательно не дергать его чаще 10 раз в секунду, лучеш реже
    if (!(tickCounter%3)) {
      bool withAnalyse = !(++calcArray%3);
      MicWorker *mw = new MicWorker(getMicScale(),getMicNoise(), withAnalyse);

      if(mw!=nullptr){
        samp_freq = mw->process(getMicNoiseRdcLevel()); // частота семплирования
        last_min_peak = mw->getMinPeak();
        last_max_peak = mw->getMaxPeak()*2;
        // for(uint16_t i=0; i<sizeof(bandValues)/(sizeof(*bandValues));i++)
        //   bandValues[i]=0.0f;
        if(withAnalyse){
          maxVal=mw->fillSizeScaledArray(bandValues, NUM_BANDS);
          last_freq=mw->getFreq();
          //ready = true; // рассчет готов. Выводить будем в следующей итерации эффекта. Выводить сразу == длинный цикл итерации эффекта.
          calcArray=1;
        }
        samp_freq = samp_freq; last_min_peak=last_min_peak; last_freq=last_freq; // давим варнинги
        delete mw;
      }
    }
    if (!(tickCounter%3)) return false; // не будем заставлять бедный контроллер еще и выводить инфу в том же цикле, что и рассчеты. Это режет ФПС. Но без новых рассчетов - ФПС просто спам.
  } else 
  #endif
  {
    //EVERY_N_MILLIS(random(50,300)) {
    if (!(tickCounter%random(2,11))) {
      last_max_peak=random(0,HEIGHT);
      maxVal=random(0,last_max_peak);
      for (uint16_t i = 0; i < (sizeof(bandValues) / sizeof(float)); i++) {
        bandValues[i] = random(2)? random(0, HEIGHT) : bandValues[i];
      }
      last_freq = random(100,20000);
    }
  }

  float _scale = (maxVal==0? 0 : last_max_peak/maxVal) * amplitude;
/*
#ifdef LAMP_DEBUG
 EVERY_N_SECONDS(1){
  for(uint16_t i=0; i<(sizeof(bandValues)/sizeof(float));i++)
    LOG(printf_P,PSTR("%7.2f"),bandValues[i]*_scale);
    LOG(printf_P,PSTR(" F: %8.2f SC: %5.2f\n"),last_freq, _scale);
  }
#endif
*/
  FastLED.clear();

  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {

    // Scale the bars for the display
    float barHeight = bandValues[band] * _scale > threshold ? (bandValues[band] * _scale) : 0.;
    if (barHeight > TOP) barHeight = TOP;

    // Small amount of averaging between frames
    if (averaging) barHeight = (oldBarHeights[band] + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min((float)TOP, barHeight);
    }

  // EVERY_N_SECONDS(1){
  //     LOG(printf_P,PSTR("%d: %d %d %d %d,\n"),band, (int)bandValues[band], peak[band], (int)barHeight, oldBarHeights[band]);
  // }

    // Draw bars
    switch (effId) {
      case 0:
      case 1:
        horizontalColoredBars(band, barHeight, colorType, colorTimer);
        break;
      case 2:
      case 3:
        outrunPeak(band, gradPal[colorType], colorTimer);
        break;
      case 4:
      case 5:
        paletteBars(band, barHeight, gradPal[colorType], colorTimer);
        break;
      case 6:
      case 7:
        centerBars(band, barHeight, gradPal[colorType], colorTimer);
        break;
      case 8:
      case 9:
        verticalColoredBars(band, barHeight, colorType, colorTimer);
        break;
    }

    if(type) 
      switch (effId)
      {
      case 2:
      case 3:
      case 6:
      case 7:
        /* code */
        break;      
      default:
        whitePeak(band);
        break;
      }

    // Save oldBarHeights for averaging later
    if (averaging) oldBarHeights[band] = barHeight;
  }

// Decay peak
 // EVERY_N_MILLISECONDS(EFFECTS_RUN_TIMER +1) {
    for (byte band = 0; band < NUM_BANDS; band++)
      if (peak[band] > 0) peak[band] -= 0.25 * speedFactorVertical;
    //colorTimer++;
  //}

  // Used in some of the patterns
if (colorShifting) {
  //EVERY_N_MILLISECONDS(EFFECTS_RUN_TIMER*4) {
    colorTimer++;
  //}
}
/*
  EVERY_N_SECONDS(10) {
    if (autoChangePatterns) effId = (effId + 1) % 6;
  }
*/
  return true;
}

// PATTERNS BELOW //

void EffectVU::horizontalColoredBars(uint8_t band, float barHeight, uint8_t type, uint8_t colorShift) {
  colorShift--;
  uint8_t xStart = BAR_WIDTH * band;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (float y = TOP; y >= (float)TOP - barHeight; y-= 0.5) {
      switch (type) {
      case 0: // Только цвет по высоте
        EffectMath::drawPixelXYF_Y(x, (float)TOP - y, CHSV(band * (232 / NUM_BANDS) + colorShift, 255, 255), fb);
        break;
      case 1: // Цвет и насыщенность
        EffectMath::drawPixelXYF_Y(x, (float)TOP - y, CHSV(band * (232 / NUM_BANDS) + colorShift, colorDev * (uint8_t)y, 255), fb);
        break;
      case 2: // Цвет и яркость
        EffectMath::drawPixelXYF_Y(x, (float)TOP - y, CHSV(band * (232 / NUM_BANDS) + colorShift, 255, (uint8_t)255 - constrain(colorDev * (uint8_t)y, 0, 200)), fb);
        break;
      case 3: // Цвет, насыщенность и яркость
        EffectMath::drawPixelXYF_Y(x, (float)TOP - y, CHSV(band * (232 / NUM_BANDS) + colorShift, colorDev * (uint8_t)y, (uint8_t)255 - constrain(colorDev * (uint8_t)y, 0, 200)), fb);
        break;
      case 4: // Вертикальная радуга
        EffectMath::drawPixelXYF_Y(x, (float)TOP - y, ColorFromPalette(RainbowColors_p, colorDev * (uint8_t)y + colorShift, 255), fb);
        break;
      }
    }
  }
}

void EffectVU::paletteBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift) {
  colorShift--;
  uint8_t xStart = BAR_WIDTH * band;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (float y = TOP; y >= (float)TOP - barHeight; y-= 0.5) {
      EffectMath::drawPixelXYF_Y(x, (float)TOP - y, ColorFromPalette(palette, (uint8_t)y * (255 / (barHeight + 1)) + colorShift), fb);
    }
  }
}

void EffectVU::verticalColoredBars(uint8_t band, float barHeight, uint8_t type, uint8_t colorShift) {
  colorShift--;
  uint8_t xStart = BAR_WIDTH * band;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (float y = TOP; y >= (float)TOP - barHeight; y-= 0.5) {
      switch (type) {
      case 0: // Только цвет по высоте
        EffectMath::drawPixelXYF_Y(x, TOP - y, CHSV((uint8_t)y * colorDev + colorShift, 255, 255), fb);
        break;
      case 1: // Цвет и насыщенность
        EffectMath::drawPixelXYF_Y(x, TOP - y, CHSV((uint8_t)y * colorDev + colorShift, colorDev * (uint8_t)y, 255), fb);
        break;
      case 2: // Цвет и яркость
        EffectMath::drawPixelXYF_Y(x, TOP - y, CHSV((uint8_t)y * colorDev + colorShift, 255, (uint8_t)255 - constrain(colorDev * (uint8_t)y, 0, 200)), fb);
        break;
      case 3: // Цвет, насыщенность и яркость
        EffectMath::drawPixelXYF_Y(x, TOP - y, CHSV((uint8_t)y * colorDev + colorShift, colorDev * (uint8_t)y, (uint8_t)255 - constrain(colorDev * (uint8_t)y, 0, 200)), fb);
        break;
      case 4: // Радуга с палитры
        EffectMath::drawPixelXYF_Y(x, TOP - y, ColorFromPalette(RainbowColors_p, colorDev * x + colorShift, 255), fb);
        break;
      }

    }
  }
}

void EffectVU::centerBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift) {
  colorShift--;
  uint8_t xStart = BAR_WIDTH * band;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    if ((int)barHeight % 2 == 0) barHeight--;
    float yStart = (((float)HEIGHT - barHeight) / 2 );
    for (float y = yStart; y <= (yStart + barHeight); y+= 0.25) {
      uint8_t colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      EffectMath::drawPixelXYF_Y(x, y, ColorFromPalette(palette, colorIndex + colorShift), fb);
    }
  }
}

void EffectVU::whitePeak(uint8_t band) {
  uint8_t xStart = BAR_WIDTH * band;
  float peakHeight = (float)TOP - peak[band] - 1;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    EffectMath::drawPixelXYF_Y(x, (float)TOP - peakHeight, CHSV(0,0,255), fb);
  }
}

void EffectVU::outrunPeak(uint8_t band, CRGBPalette16& palette, uint8_t colorShift) {
  colorShift--;
  uint8_t xStart = BAR_WIDTH * band;
  float peakHeight = (float)TOP - peak[band] - 1;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
    EffectMath::drawPixelXYF_Y(x, (float)TOP - peakHeight, type ? ColorFromPalette(palette, (uint8_t)(peakHeight * colorDev) + colorShift) : CHSV(colorShift, 255, 255), fb);
  }
}

void EffectVU::waterfall(uint8_t band, uint8_t barHeight) {
  int xStart = BAR_WIDTH * band;
  for (uint8_t x = xStart; x < xStart + BAR_WIDTH; x++) {
      fb.pixel(x, 0) = CHSV((x / BAR_WIDTH) * (255 / NUM_BANDS), 255, 255);
  }

  // Move screen up starting at 2nd row from top
  if (band == NUM_BANDS - 1){
    for (byte x = 0; x < WIDTH; x++) {
      for (byte y = 0; y < HEIGHT; y++) {
        fb.pixel(x, y) = (((int)y == fb.cfg.maxHeightIndex()) ? CRGB::Black : fb.pixel(x, y + 1));
      }
    }
  }
}
#endif

// ----------- Эффект "Огненная Лампа"
// https://editor.soulmatelights.com/gallery/546-fire
// (c) Stepko 17.06.21
// sparks (c) kostyamat 10.01.2022 https://editor.soulmatelights.com/gallery/1619-fire-with-sparks
void EffectFire2021::load() {
  palettesload();    // подгружаем палитры
}

void EffectFire2021::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&NormalFire2_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&NormalFire3_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&MagmaColor_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&AlcoholFireColors_p); 
  palettes.push_back(&WaterfallColors_p);

  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
  
  sparks.resize(sparksCount);
  for (byte i = 0; i < sparksCount; i++) 
    sparks[i].reset(fb);
}


// !++
String EffectFire2021::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 20, 100) * speedfactor;
  else if(_val->getId()==3) _scale = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 100, 32, 132);
  else if(_val->getId()==5) withSparks = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectFire2021::run() {
  t += speedFactor;

  if (withSparks)
    for (byte i = 0; i < sparksCount; i++) {
      sparks[i].addXY((float)random(-1, 2) / 2, 0.5 * speedfactor, fb);
      if (sparks[i].getY() > HEIGHT && !random(0, 50))
        sparks[i].reset(fb);
      else
        sparks[i].draw(fb);
    }

  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
     
      int16_t bri= inoise8(x * _scale, (y * _scale) - t) - ((withSparks ? y + spacer : y) * (256 / WIDTH));
      byte col = bri;
      if(bri<0){bri= 0;} if(bri!=0) {bri= 256 - (bri* 0.2);}
      nblend(fb.pixel(x, y), ColorFromPalette(*curPalette, col, bri), speedFactor);}
  }
  return true;
}

// ----------- Эффект "Пятнашки"
// https://editor.soulmatelights.com/gallery/1471-puzzles-subpixel
// (c) Stepko 10.12.21
String EffectPuzzles::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.05, 0.5);
  else if(_val->getId()==3) {
    psizeX = psizeY = EffectCalc::setDynCtrl(_val).toInt();
    regen();
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectPuzzles::load() {
  palettesload();    // подгружаем палитры
  regen();
}

void EffectPuzzles::regen() {
  pcols = fb.cfg.w() / psizeX + !!(fb.cfg.w() % psizeX);
  prows = fb.cfg.h() / psizeY + !!(fb.cfg.w() % psizeY);

  step = 0;
  puzzle = std::vector< std::vector<uint8_t> >(pcols, std::vector<uint8_t>(prows));

  byte n = 0;
  for (auto &c : puzzle) {
    for (auto &r : c) { 
      r = (255/ (pcols*prows)) * ++n; 
    }
  }
  z_dot[0] = random(0, pcols);
  z_dot[1] = random(0, prows);
}

void EffectPuzzles::draw_square(byte x1, byte y1, byte x2, byte y2, byte col) {
  for (byte x = x1; x < x2; x++) {
    for (byte y = y1; y < y2; y++) {
      if (col == 0) { fb.pixel(x, y) = CRGB(0, 0, 0); } 
      else if ((x == x1 || x == x2 - 1) || (y == y1 || y == y2 - 1))
        fb.pixel(x, y) = ColorFromPalette(*curPalette, col);
      else fb.pixel(x, y) = CHSV(0, 0, 96);
    }
  }
}

void EffectPuzzles::draw_squareF(float x1, float y1, float x2, float y2, byte col) {
  for (float x = x1; x < x2; x++) {
    for (float y = y1; y < y2; y++) {
      if ((x == x1 || x == x2 - 1) || (y == y1 || y == y2 - 1))
        EffectMath::drawPixelXYF(x, y, ColorFromPalette(*curPalette, col), fb);
      else EffectMath::drawPixelXYF(x, y, CHSV(0, 0, 96), fb);
    }
  }
}

bool EffectPuzzles::run() { 
  for (byte x = 0; x < pcols; x++) {
    for (byte y = 0; y < prows; y++) {
      draw_square(x * psizeX, y * psizeY, (x + 1) * psizeX, (y + 1) * psizeY, puzzle[x][y]);
    }
  }

  switch (step) {
    case 0:
      XorY = !XorY;
      if (XorY) {
        if (z_dot[0] == pcols - 1)
          move[0] = -1;
        else if (z_dot[0] == 0) move[0] = 1;
        else move[0] = (move[0] == 0) ? (random8() % 2) * 2 - 1 : move[0];
      } else {
        if (z_dot[1] == prows - 1)
          move[1] = -1;
        else if (z_dot[1] == 0) move[1] = 1;
        else move[1] = (move[1] == 0) ? (random8() % 2) * 2 - 1 : move[1];
      }
      move[(XorY) ? 1 : 0] = 0;
      step = 1;
      break;
    case 1:
     color = puzzle[z_dot[0] + move[0]][z_dot[1] + move[1]];
      puzzle[z_dot[0] + move[0]][z_dot[1] + move[1]] = 0;
      step = 2;
      break;
    case 2:
      draw_square(((z_dot[0] + move[0]) * psizeX) + shift[0], ((z_dot[1] + move[1]) * psizeY) + shift[1], ((z_dot[0] + move[0] + 1) * psizeX) + shift[0], (z_dot[1] + move[1] + 1) * psizeY + shift[1], color);
      shift[0] -= (move[0] * speedFactor);
      shift[1] -= (move[1] * speedFactor);
      if ((fabs(shift[0]) >= fb.cfg.w() / pcols) || (fabs(shift[1]) >= fb.cfg.h() / prows)) {
        shift[0] = 0;
        shift[1] = 0;
        puzzle[z_dot[0]][z_dot[1]] = color;
        step = 3;
      }
      break;
    case 3:
      z_dot[0] += move[0];
      z_dot[1] += move[1];
      step = 0;
      break;
    default :
      step = 0;
  }
  return true;
}

// ============= Эффект Цветные драже ===============
// (c) SottNick
//по мотивам визуала эффекта by Yaroslaw Turbin 14.12.2020
//https://vk.com/ldirko программный код которого он запретил брать
// !++
String EffectPile::setDynCtrl(UIControl*_val) {
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
  if (pcnt >= map8(temp, 2U, HEIGHT - 3U)) {
    //temp = 255U - temp + 2;
    //if (temp < 2) temp = 255;
    temp = HEIGHT + 1U - pcnt;
    if (!random(4U)) {// иногда песка осыпается до половины разом
      if (random(2U)) {
        temp = 2U;
      } else {
        temp = 3U;
      }
    }
    //for (uint16_t i = 0U; i < num_leds; i++)
    for (uint8_t y = 0; y < pcnt; y++)
      for (uint8_t x = 0; x < WIDTH; x++)
        if (!random(temp))
          fb.pixel(x,y) = 0;
  }

  pcnt = 0U;
  // осыпаем всё, что есть на экране
  for (uint8_t y = 1; y < HEIGHT; y++)
    for (uint8_t x = 0; x < WIDTH; x++)
      if (fb.pixel(x, y))
      { // проверяем для каждой песчинки
        if (!fb.pixel(x, y - 1))
        { // если под нами пусто, просто падаем
          fb.pixel(x, y - 1) = fb.pixel(x, y);
          fb.pixel(x, y) = 0;
        }
        else if (x > 0U && !fb.pixel(x - 1, y - 1) && x < WIDTH - 1 && !fb.pixel(x + 1, y - 1))
        { // если под нами пик
          if (random8(2U))
            fb.pixel(x - 1, y - 1) = fb.pixel(x, y);
          else
            fb.pixel(x - 1, y - 1) = fb.pixel(x, y);
          fb.pixel(x, y) = 0;
          pcnt = y - 1;
        }
        else if (x > 0U && !fb.pixel(x - 1, y - 1))
        { // если под нами склон налево
          fb.pixel(x - 1, y - 1) = fb.pixel(x, y);
          fb.pixel(x, y) = 0;
          pcnt = y - 1;
        }
        else if (x < WIDTH - 1 && !fb.pixel(x + 1, y - 1))
        { // если под нами склон направо
          fb.pixel(x + 1, y - 1) = fb.pixel(x, y);
          fb.pixel(x, y) = 0;
          pcnt = y - 1;
        }
        else // если под нами плато
          pcnt = y;
      }
  // эмиттер новых песчинок
  if (!fb.pixel(CENTER_X_MINOR, HEIGHT - 2) && !fb.pixel(CENTER_X_MAJOR, HEIGHT - 2) && !random(3))
  {
    temp = random(2) ? CENTER_X_MINOR : CENTER_X_MAJOR;
    fb.pixel(temp, HEIGHT - 1) = ColorFromPalette(*curPalette, random8());
  }
  return true;
}

// ============= Эффект ДНК ===============
// (c) Stepko
// https://editor.soulmatelights.com/gallery/1520-dna
//по мотивам визуала эффекта by Yaroslaw Turbin
//https://vk.com/ldirko программный код которого он запретил брать
// !++
String EffectDNA::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor  = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.5, 5) * speedfactor;
  else if(_val->getId()==3)  type = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==4)  _scale = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==5) bals = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectDNA::run() {
  fb.fade(32);
  t += speedFactor; // (float)millis()/10;
  if (type == 0) {
    EVERY_N_SECONDS(30) {
      _type ++;
      if (_type > 3)
        _type = 1;
    }
  } else _type = type;

  if (_type == 1 or _type == 4) a = (256.0 / (float)WIDTH);
  else a = (256.0 / (float)HEIGHT);

  for (byte i = 0; i < ((_type == 1) ? HEIGHT : WIDTH); i++) {
    uint16_t shift = (i * _scale);
    float sin1 = (1.0 + sin(radians(t + shift))) * 128.0;
    byte brightFront =  constrain(112 * (1 + sin(radians(t + shift + 90))) + 30, 96, 255); 
    byte brightBack =  constrain(112 * (1 + sin(radians(t + shift + 270))) + 30, 96, 255);

    float x = 0, y = 0, x1 = 0, y1 = 0;
    uint8_t width_height;

    switch (_type)
    {
    case 1: // Вертикальная ДНК
      width_height = WIDTH;
      x = sin1 /a;
      y = i;
      x1 = (float)(width_height - 1) - (sin1 / a);
      y1 = i;
      break;
    case 2: // Вертикально-горизонтальная
      width_height = HEIGHT;
      x = sin1 /a;
      y = i;
      y1 = (float)(width_height - 1) - (sin1 / a);
      x1 = i;
      break;
    case 3: // Горизонтальная ДНК
      width_height = HEIGHT;
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
String EffectSmoker::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 3, 20) * speedfactor;
  else if(_val->getId()==2) color = EffectCalc::setDynCtrl(_val).toInt();
  else if(_val->getId()==3) saturation = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

bool EffectSmoker::run() {
  t += speedFactor;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) { 
      uint8_t bri= inoise8(x * beatsin8(glitch, 20, 40), (y * _scale) - t);
      nblend(fb.pixel(x, y), CHSV(color, saturation, bri), speedFactor);}
  }
  
  EVERY_N_SECONDS(random8(10, 31)) {
    glitch = random(1, 3);
  }
  return true;
}

// ----------- Эффект "Мираж"
// based on cod by @Stepko (c) 23/12/2021

// !++
String EffectMirage::setDynCtrl(UIControl*_val) {
  if(_val->getId()==1) _speed = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 64, 512) * speedfactor;
  else if(_val->getId()==2) {
    color = EffectCalc::setDynCtrl(_val).toInt();
    if (color == 1) colorShift = true;
    else colorShift = false;
  }
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectMirage::drawDot(float x, float y, byte a){
uint8_t xx = (x - (int) x) * 255, yy = (y - (int) y) * 255, ix = 255 - xx, iy = 255 - yy;
#define WU(a, b)((uint8_t)(((a) * (b) + (a) + (b)) >> 8))
  uint8_t wu[4] = {
    WU(ix, iy),
    WU(xx, iy),
    WU(ix, yy),
    WU(xx, yy)
  };
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    byte clr = buff[xn][yn];
    clr = constrain(qadd8(clr, (a * wu[i]) >> 8), 0, 240);
    buff[xn][yn] = clr;
  }
}

void EffectMirage::blur() {
  uint16_t sum;
  for (byte x = 1; x < WIDTH + 1; x++) {
    for (byte y = 1; y < HEIGHT + 1; y++) {
      sum = buff[x][y];
      sum += buff[x + 1][y];
      sum += buff[x][y - 1];
      sum += buff[x][y + 1];
      sum += buff[x - 1][y];
      sum /= 5;
      buff[x][y] = sum;
    }
  }
}

bool EffectMirage::run() {
  blur();
  float x1 = (float)beatsin88(15UL * _speed, div, width) / div;
  float y1 = (float)beatsin88(20UL * _speed, div, height) / div;
  float x2 = (float)beatsin88(16UL * _speed, div, width) / div;
  float y2 = (float)beatsin88(14UL * _speed, div, height) / div;
  float x3 = (float)beatsin88(12UL * _speed, div, width) / div;
  float y3 = (float)beatsin88(16UL * _speed, div, height) / div;
  drawDot(x1, y1, 200);
  drawDot(x1 + 1, y1, 200);
  drawDot(x2, y2, 200);
  drawDot(x2 + 1, y2, 200);
  drawDot(x3, y3, 200);
  drawDot(x3 + 1, y3, 200);
  for (byte x = 1; x < WIDTH + 1; x++) {
    for (byte y = 1; y < HEIGHT + 1; y++) {
      fb.pixel(x - 1, y - 1) = CHSV(colorShift ? color++ : color, buff[x][y], 255);
    }
  }
  return true;
}

// -------------------- Эффект "Акварель"
// (c) kostyamat 26.12.2021
// https://editor.soulmatelights.com/gallery/1587-oil
// !++
String EffectWcolor::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    speedFactor = EffectMath::fmap(EffectCalc::setDynCtrl(_val).toInt(), 1, 255, 0.1, 0.5);
    blur = 64.f * speedFactor;
    speedFactor *= EffectCalc::speedfactor;
  }  else if(_val->getId()==3) {
    bCounts = map(EffectCalc::setDynCtrl(_val).toInt(), 1, 8, HEIGHT/4, HEIGHT);
    blots.resize(bCounts);
    load();
  }
  else if(_val->getId()==4) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectWcolor::load() {
  for (byte i = 0; i < bCounts; i++) {
    blots[i].reset(i, bCounts);
  }
}

bool EffectWcolor::run() {
  fb.fade(blur);
  for (byte i = 0; i < bCounts; i++) {
    blots[i].drawing(fb);
    blots[i].appendXY( mode ? ((float)inoise8(t+= speedFactor, 0, i * 100) / 256) - 0.5f : 0, -speedFactor);
    if(blots[i].getY() < -0.1) {
      blots[i].reset(i, bCounts);
      random16_set_seed(millis());
    }
  }
  //EffectMath::blur2d(fb.data(), fb.cfg.w, fb.cfg.h, 32); 
  return true;
}

// ----------- Эффект "Неопалимая купина"
//RadialFire
// (c) Stepko and Sutaburosu https://editor.soulmatelights.com/gallery/1570-radialfire
//23/12/21
// !++
String EffectRadialFire::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    speedFactor = EffectMath::fmap(speed, 1, 255, 2., 20.);
  } else if(_val->getId()==3) {_scale = EffectCalc::setDynCtrl(_val).toInt();
  } else if(_val->getId()==5) mode = EffectCalc::setDynCtrl(_val).toInt();
  else EffectCalc::setDynCtrl(_val).toInt(); // для всех других не перечисленных контролов просто дергаем функцию базового класса (если это контролы палитр, микрофона и т.д.)
  return String();
}

void EffectRadialFire::load() {
  for (int8_t x = -centre; x < centre + (int8_t)(WIDTH % 2); x++) {
    for (int8_t y = -centre; y < centre + (int8_t)(HEIGHT % 2); y++) {
      XY_angle[x + centre][y + centre] = atan2(y, x) * (180. / 2. / PI) * maximum;
      XY_radius[x + centre][y + centre] = hypotf(x, y); // thanks Sutaburosu
    }
  }
  palettesload();

}

void EffectRadialFire::palettesload(){
  // собираем свой набор палитр для эффекта
  palettes.reserve(NUMPALETTES);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&NormalFire2_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&NormalFire3_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&MagmaColor_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&AlcoholFireColors_p); 
  palettes.push_back(&WaterfallColors_p);

  usepalettes = true; // включаем флаг палитр
  scale2pallete();    // выставляем текущую палитру
}

bool EffectRadialFire::run() {
  t += speedFactor;
  for (uint8_t x = 0; x < maximum; x++) {
    for (uint8_t y = 0; y < maximum; y++) {
      float angle = XY_angle[x][y];
      uint16_t radius = mode ? maximum - 3 - XY_radius[x][y] : XY_radius[x][y];
      int16_t Bri = inoise8(angle, radius * _scale - t, x * _scale) - radius * (256 /maximum);
      byte Col = Bri;
      if (Bri < 0) Bri = 0; 
      if(Bri != 0) Bri = 256 - (Bri * 0.2);
        nblend(fb.pixel(x+X, y+Y), ColorFromPalette(*curPalette, Col, Bri), speed);
    }
  }
  return true;
}

// 
String EffectSplashBals::setDynCtrl(UIControl*_val){
  if(_val->getId()==1) {
    speed = EffectCalc::setDynCtrl(_val).toInt();
    speedFactor = EffectMath::fmap(speed, 1, 255, 1, 3) * speedfactor;
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
  fb.fade(100);
  hue++;

  for (auto &b : balls){
    b.x1 = (float)beatsin88(((10UL + b.iniX1) * 256) * speedFactor, 0, (WIDTH - 1) * dev) / dev;
    b.y1 = (float)beatsin88(((10UL + b.iniY1) * 256) * speedFactor, 0, (HEIGHT - 1) * dev) / dev;
    b.x2 = (float)beatsin88(((10UL + b.iniX2) * 256) * speedFactor, 0, (WIDTH - 1) * dev) / dev;
    b.y2 = (float)beatsin88(((10UL + b.iniY2) * 256) * speedFactor, 0, (HEIGHT - 1) * dev) / dev;
      float a = dist(b.x1, b.y1, b.x2, b.y2);
      if (a <= float(min(WIDTH, HEIGHT) / 2)) {
        EffectMath::drawLineF(b.x1, b.y1, b.x2, b.y2, CHSV(0, 0, EffectMath::fmap(a, min(WIDTH, HEIGHT), 0, 48, 255)), fb);
      }
    EffectMath::fill_circleF(b.x1, b.y1, EffectMath::fmap(fabs(float(WIDTH / 2) - b.x1), 0, WIDTH / 2, R, 0.2), ColorFromPalette(*curPalette, 256 - 256/HEIGHT * fabs(float(HEIGHT/2) - b.y1)), fb);
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
    // 0,0
    fb.pixel(0,0) = fb.pixel(1,0) = fb.pixel(2,0) = fb.pixel(2,0) = fb.pixel(0,1) = CRGB::Red;

    // w,0
    fb.pixel(mx.cfg.w()-3,0) = fb.pixel(mx.cfg.w()-2,0) = fb.pixel(mx.cfg.w()-1,0) = fb.pixel(mx.cfg.w()-1,1) = CRGB::Green;

    // 0,h
    fb.pixel(1,mx.cfg.h()-2) = fb.pixel(1,mx.cfg.h()-1) = CRGB::Red;

    // w,h
    fb.pixel(mx.cfg.w()-2,mx.cfg.h()-2) = fb.pixel(mx.cfg.w()-2,mx.cfg.h()-1) = CRGB::Green;
}

