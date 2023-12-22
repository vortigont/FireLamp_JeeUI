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

#pragma once

#include "color_palette.h"
#include "effectworker.h"
#include "effectmath.h"
#include <TetrisMatrixDraw.h>

#define NUMPALETTES 10
#define NUM_LAYERS  1       // layers for noice effetcs

//-------------- Специально обученный пустой эффект :)
class EffectNone : public EffectCalc {
private:
    void load() override { fb->clear(); };
public:
    EffectNone(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override { fb->clear(); return false; };
};

//-------------- Эффект "Часы"
class EffectTime : public EffectCalc {
private:
    bool timeShiftDir; // направление сдвига
    float curTimePos; // текущая позиция вывода
    float color_idx; // индекс цвета
    CRGB hColor[1]; // цвет часов и минут
    CRGB mColor[1]; // цвет часов и минут
    uint32_t lastrun=0;     /**< счетчик времени для эффектов с "задержкой" */
    bool isMinute=false;

    bool timePrintRoutine();
    bool palleteTest();
    void load() override;
public:
    EffectTime(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

/*
 ***** METABALLS / МЕТАСФЕРЫ *****
Metaballs proof of concept by Stefan Petrick 
https://gist.github.com/StefanPetrick/170fbf141390fafb9c0c76b8a0d34e54
*/
class EffectMetaBalls : public EffectCalc {
private:
	float speedFactor;
	const float hormap = (256 / fb->w());
    const float vermap = (256 / fb->h());
	String setDynCtrl(UIControl*_val) override;
    void load() override;
public:
    EffectMetaBalls(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ***** SINUSOID3 / СИНУСОИД3 *****
/*
  Sinusoid3 by Stefan Petrick (mod by Palpalych for GyverLamp 27/02/2020)
  read more about the concept: https://www.youtube.com/watch?v=mubH-w_gwdA
*/
class EffectSinusoid3 : public EffectCalc {
private:
	const uint8_t semiHeightMajor =  fb->h() / 2 + (fb->h() % 2);
	const uint8_t semiWidthMajor =  fb->w() / 2  + (fb->w() % 2);
	float e_s3_speed;
	float e_s3_size;
	uint8_t _scale;
	uint8_t type;
	
	String setDynCtrl(UIControl*_val) override;

public:
    EffectSinusoid3(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override { fb->clear(); }; 
    bool run() override;
};



//----- Эффект "Прыгающие Мячики"
// перевод на субпиксельную графику kostyamat
class EffectBBalls : public EffectCalc {
    struct Ball {
        uint8_t color;              // прикручено при адаптации для разноцветных мячиков
        uint8_t brightness{156};
        int8_t x;                   // прикручено при адаптации для распределения мячиков по радиусу лампы
        float pos{0};               // The integer position of the dot on the strip (LED index) /yeah, integer.../
        float vimpact{0};           // As time goes on the impact velocity will change, so make an array to store those values
        float cor{0};               // Coefficient of Restitution (bounce damping)
        long unsigned tlast{millis()};      // The clock time of the last ground strike
        float shift{0};
    };

    float bballsHi = 0.0;                               // An array of heights
    uint32_t bballsTCycle = 0;                        // The time since the last time the ball struck the ground
    float hue{0};
    bool halo = false;                                  // ореол
    uint8_t _scale=1;
    uint16_t _speed;
    std::vector<Ball> balls = std::vector<Ball>(1, Ball());

    bool bBallsRoutine();
    void load() override;
	String setDynCtrl(UIControl*_val) override;
public:
    EffectBBalls(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ------------- Эффект "Пейнтбол" -------------
class EffectLightBalls : public EffectCalc {
private:
	#define BORDERTHICKNESS       (1U)   // глубина бордюра для размытия яркой частицы: 0U - без границы (резкие края); 1U - 1 пиксель (среднее размытие) ; 2U - 2 пикселя (глубокое размытие)
	const uint8_t paintWidth = fb->w() - BORDERTHICKNESS * 2;
	const uint8_t paintHeight = fb->h() - BORDERTHICKNESS * 2;
	float speedFactor;
	
	String setDynCtrl(UIControl*_val) override;

public:
    EffectLightBalls(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ------- Эффект "Пульс"
class EffectPulse : public EffectCalc {
    uint8_t pulse_hue;
    float pulse_step = 0;
    uint8_t centerX = random8(fb->w() - 5U) + 3U;
    uint8_t centerY = random8(fb->h() - 5U) + 3U;
    uint8_t currentRadius = 4;
    float _pulse_hue = 0;
    uint8_t _pulse_hueall = 0;
    float speedFactor;
    String setDynCtrl(UIControl*_val) override;
public:
    EffectPulse(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ------------- эффект "Блуждающий кубик" -------------
class EffectBall : public EffectCalc {
private:
    uint8_t ballSize;
    CRGB ballColor;
    float vectorB[2U];
    float coordB[2U];
	bool flag[2] = {true, true};
	float speedFactor;

	String setDynCtrl(UIControl*_val);
	
public:
    EffectBall(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// -------- Эффект "Светлячки со шлейфом"
#define _AMOUNT 16U
class EffectLighterTracers : public EffectCalc {
private:
    uint8_t cnt = 1;
    float vector[_AMOUNT][2U];
    float coord[_AMOUNT][2U];
    int16_t ballColors[_AMOUNT];
    byte light[_AMOUNT];
    float speedFactor;
    bool lighterTracersRoutine();

public:
    EffectLighterTracers(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
    String setDynCtrl(UIControl*_val) override;
};

class EffectRainbow : public EffectCalc {
private:
    float hue; // вещественное для малых скоростей, нужно приведение к uint8_t по месту
    float twirlFactor;
    float micCoef;

    bool rainbowHorVertRoutine(bool isVertical);
    bool rainbowDiagonalRoutine();

public:
    EffectRainbow(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

class EffectColors : public EffectCalc {
private:
    uint8_t ihue;
    uint8_t mode;
    uint8_t modeColor;
    unsigned int step = 0; // доп. задержка

    bool colorsRoutine();
    //void setscl(const byte _scl) override;
    String setDynCtrl(UIControl*_val) override;
public:
    EffectColors(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------ Эффект "Белая Лампа"
class EffectWhiteColorStripe : public EffectCalc {
private:
    uint8_t shift=0;
    bool whiteColorStripeRoutine();
    String setDynCtrl(UIControl*_val) override;
public:
    EffectWhiteColorStripe(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ---- Эффект "Конфетти"
class EffectSparcles : public EffectCalc {
private:
    uint8_t eff = 1;
    bool sparklesRoutine();

public:
    EffectSparcles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    String setDynCtrl(UIControl*_val) override;
};

// ========== Эффект "Эффектопад"
// совместное творчество юзеров форума https://community.alexgyver.ru/
class EffectEverythingFall : public EffectCalc {
private:
    Vector2D<uint8_t> heat{ Vector2D<uint8_t>(fb->w(),fb->h()) };

public:
    EffectEverythingFall(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= FIRE 2012 /  ОГОНЬ 2012 ===============
// based on FastLED example Fire2012WithPalette: https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
// v1.0 - Updating for GuverLamp v1.7 by SottNick 17.04.2020
/*
 * Эффект "Огонь 2012"
 */
class EffectFire2012 : public EffectCalc {
private:
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
    uint8_t cooling = 80U; // 70
  // SPARKING: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
    uint8_t sparking = 90U; // 130
  // SMOOTHING; How much blending should be done between frames
  // Lower = more blending and smoother flames. Higher = less blending and flickery flames
    uint8_t _scale = 1;
    const uint8_t fireSmoothing = 60U; // 90
    Vector2D<uint8_t> noise;
    bool fire2012Routine();
    String setDynCtrl(UIControl*_val) override;

    /** полезные обертки **/
    uint8_t wrapX(int8_t x){ return (x + fb->w()) % fb->w(); }
    uint8_t wrapY(int8_t y){ return (y + fb->h()) % fb->h(); }

public:
    EffectFire2012(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), noise(fb->w(), fb->h()) {}
    void load() override;
    bool run() override;
};

// ------------- класс Светлячки -------------
// нужен для некоторых эффектов
#define LIGHTERS_MAX    10
class EffectLighters : public EffectCalc {
protected:
struct Lighter {
    uint8_t color;
    uint8_t light;
    float spdX, spdY;
    float posX, posY;
};

    bool subPix = false;
    //uint16_t lightersIdx;
    float speedFactor{0.1};
    std::vector<Lighter> lighters;
private:
    String setDynCtrl(UIControl*_val) override;
public:
    EffectLighters(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), lighters(std::vector<Lighter>(10)) {}
    void load() override;
    bool run() override;
};

// ------------- Эффект "New Матрица" ---------------
class EffectMatrix : public EffectLighters {
private:
    bool matrixRoutine();
    uint8_t _scale = 1;
    byte gluk = 1;
    uint8_t hue, _hue;
    bool randColor = false;
    bool white = false;
    float count{0};
    float _speed{1};
    String setDynCtrl(UIControl*_val) override;
public:
    EffectMatrix(LedFB<CRGB> *framebuffer) : EffectLighters(framebuffer){}
    void load() override;
    bool run() override;
};

// ------------- звездопад/метель -------------
class EffectStarFall : public EffectLighters {
private:
    uint8_t _scale = 1;
    uint8_t effId = 1;
    bool isNew = true;
    float fade;
    float _speed{1};
    bool snowStormStarfallRoutine();
    String setDynCtrl(UIControl*_val) override;

public:
    EffectStarFall(LedFB<CRGB> *framebuffer) : EffectLighters(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эффекты "Лава, Зебра, etc"
class Effect3DNoise : public EffectCalc {
private:
    void fillNoiseLED();
    void fillnoise8();

    uint8_t ihue;
    bool colorLoop;
	bool blurIm;
    float _speed{200};             // speed is set dynamically once we've started up
    float _scale{10};             // scale is set dynamically once we've started up
    float x{1}, y{1}, z{1};
    Vector2D<uint8_t> noise;

public:
    Effect3DNoise(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), noise(2*fb->w(), fb->h()) {}
    void load() override;
    bool run() override;
    String setDynCtrl(UIControl*_val) override;
};

// ***** Эффект "Спираль"     ****
/*
 * Aurora: https://github.com/pixelmatix/aurora
 * https://github.com/pixelmatix/aurora/blob/sm3.0-64x64/PatternSpiro.h
 * Copyright (c) 2014 Jason Coon
 * Неполная адаптация SottNick
 */
class EffectSpiro : public EffectCalc {
private:
  const uint8_t spiroradiusx = fb->w() /4;
  const uint8_t spiroradiusy = fb->h() /4;

  const uint8_t spirocenterX = fb->w() /2;
  const uint8_t spirocenterY = fb->h() /2;

  const uint8_t spirominx = spirocenterX - spiroradiusx;
  const uint8_t spiromaxx = spirocenterX + spiroradiusx - (fb->w()%2 == 0 ? 1:0);// + 1;
  const uint8_t spirominy = spirocenterY - spiroradiusy;
  const uint8_t spiromaxy = spirocenterY + spiroradiusy - (fb->h()%2 == 0 ? 1:0); // + 1;

  bool spiroincrement = false;
  bool spirohandledChange = false;
  float spirohueoffset = 0;
  uint8_t spirocount = 1;
  float spirotheta1 = 0;
  float spirotheta2 = 0;
  uint8_t internalCnt = 0;
  float speedFactor;

  String setDynCtrl(UIControl*_val) override;

public:
    EffectSpiro(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= ЭФФЕКТ ПРИЗМАТА ===============
// Prismata Loading Animation
class EffectPrismata : public EffectCalc {
private:
    byte spirohueoffset = 0;
    uint8_t fadelvl=1;
	float speedFactor;
    
    String setDynCtrl(UIControl*_val) override;
public:
    EffectPrismata(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= ЭФФЕКТ СТАЯ ===============
// Адаптация от (c) SottNick
class EffectFlock : public EffectCalc {
private:
  std::vector<Boid> boids;
  Boid predator;
  PVector wind;
  float speedFactor;

  bool predatorPresent;
  float hueoffset;

  bool flockRoutine();
  String setDynCtrl(UIControl*_val) override;
  //void setspd(const byte _spd) override;
public:
    EffectFlock(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer),
        boids( std::vector<Boid>(AVAILABLE_BOID_COUNT) ) {}
    void load() override;
    bool run() override;
};

// ***** RAINBOW COMET / РАДУЖНАЯ КОМЕТА *****
// ***** Парящий огонь, Кровавые Небеса, Радужный Змей и т.п.
// базис (c) Stefan Petrick
#define COMET_NOISE_LAYERS  1
class EffectComet : public EffectCalc {
    byte hue, hue2;
    uint8_t eNs_noisesmooth = 200;
    uint8_t count;
    uint8_t speedy{100};
    float spiral, spiral2;
    float _speed{1};
    uint8_t _scale = 4;     // effect switcher
    uint8_t effId = 1;      // 2, 1-6
    uint8_t colorId;        // 3, 1-255
    uint8_t smooth = 1;     // 4, 1-12
    uint8_t blur{10};           // 5, 1-64
    // 3D Noise map
    Noise3dMap noise3d;

    const uint8_t e_centerX = (fb->w() / 2) - ((fb->w() - 1) & 0x01);
    const uint8_t e_centerY = (fb->h() / 2) - ((fb->h() - 1) & 0x01);


    void drawFillRect2_fast(int8_t x1, int8_t y1, int8_t x2, int8_t y2, CRGB color);
    void moveFractionalNoise(bool direction, int8_t amplitude, float shift = 0);

    bool rainbowCometRoutine();
    bool rainbowComet3Routine();
    bool firelineRoutine();
    bool fractfireRoutine();
    bool flsnakeRoutine();
    bool smokeRoutine();
    String setDynCtrl(UIControl*_val) override;

public:
    EffectComet(LedFB<CRGB> *framebuffer) :  EffectCalc(framebuffer), noise3d(COMET_NOISE_LAYERS, framebuffer->w(), framebuffer->h()) {}
    void load() override;
    bool run() override;
};

// ============= SWIRL /  ВОДОВОРОТ ===============
// https://gist.github.com/kriegsman/5adca44e14ad025e6d3b
// Copyright (c) 2014 Mark Kriegsman
class EffectSwirl : public EffectCalc {
private:
    bool swirlRoutine();

public:
    EffectSwirl(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= DRIFT / ДРИФТ ===============
// v1.0 - Updating for GuverLamp v1.7 by SottNick 12.04.2020
// v1.1 - +dither, +phase shifting by PalPalych 12.04.2020
// https://github.com/pixelmatix/aurora/blob/master/PatternIncrementalDrift.h
class EffectDrift : public EffectCalc {
private:
	byte maxDim_steps(){ return 256 / fb->maxDim(); }
	uint8_t dri_phase;
	float _dri_speed;
	uint8_t _dri_delta;
	byte driftType = 0;

	String setDynCtrl(UIControl*_val) override;
	bool incrementalDriftRoutine();
	bool incrementalDriftRoutine2();
    // some adjustments
    int width_adj(){ return (fb->w() < fb->h() ? (fb->h() - fb->w()) /2 : 0); };
    int height_adj(){ return (fb->h() < fb->w() ? (fb->w() - fb->h()) /2: 0); };

public:
    EffectDrift(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------------------------------ ЭФФЕКТ МЕРЦАНИЕ ----------------------
// (c) SottNick
class EffectTwinkles : public EffectCalc {
private:
  uint8_t thue = 0U;
  uint8_t tnum;
  PixelDataBuffer<CRGB> ledsbuff;
  float speedFactor;
  bool twinklesRoutine();
  String setDynCtrl(UIControl*_val) override;
  //void setscl(const byte _scl) override;
public:
    EffectTwinkles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), ledsbuff(fb->size()) {}
    void load() override;
    void setup();
    bool run() override { return twinklesRoutine(); };
};

class EffectWaves : public EffectCalc {
private:
  float whue;
  float waveTheta;
  uint8_t _scale=1;
  float speedFactor;
  bool wavesRoutine();
  String setDynCtrl(UIControl*_val) override;
public:
    EffectWaves(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= RADAR / РАДАР ===============
// Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternRadar.h
// Copyright(c) 2014 Jason Coon
class EffectRadar : public EffectCalc {
private:
    float eff_offset; 
    float eff_theta;  // глобальная переменная угла для работы эффектов
    bool subPix = false;
    byte hue;
    const float width_adj_f = (float)(fb->w() < fb->h() ? (fb->h() - fb->w()) / 2. : 0);
    const float height_adj_f= (float)(fb->h() < fb->w() ? (fb->w() - fb->h()) / 2. : 0);
    int width_adj(){ return (fb->w() < fb->h() ? (fb->h() - fb->w()) /2 : 0); };
    int height_adj(){ return (fb->h() < fb->w() ? (fb->w() - fb->h()) /2: 0); };

    bool radarRoutine();
    String setDynCtrl(UIControl *_val) override;

public:
    EffectRadar(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

class EffectFire2018 : public EffectCalc {
#define FIRE_NUM_LAYERS     2
private:
  bool isLinSpeed = true;
  uint8_t fade_amount = 15;      // this could be a new control for flame size/height
  // use vector of vectors to take benefit of swap operations
  std::vector<std::vector<uint8_t>> fire18heat;
  Noise3dMap noise;

  String setDynCtrl(UIControl*_val) override;

public:
    EffectFire2018(LedFB<CRGB> *framebuffer) : 
        EffectCalc(framebuffer),
        fire18heat(std::vector<std::vector<uint8_t>>(framebuffer->h(), std::vector<uint8_t>(framebuffer->w()))),
        noise(FIRE_NUM_LAYERS, framebuffer->w(), framebuffer->h()) { fb->clear(); }
    bool run() override;
};

// -------------- Эффект "Кодовый замок"
// (c) SottNick
//uint8_t ringColor[fb->h()]; // начальный оттенок каждого кольца (оттенка из палитры) 0-255
//uint8_t huePos[fb->h()]; // местоположение начального оттенка кольца 0-fb->maxWidthIndex()
//uint8_t shiftHueDir[fb->h()]; // 4 бита на ringHueShift, 4 на ringHueShift2
////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число
//uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
//uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от fb->w()/5 до fb->w()-3
class EffectRingsLock : public EffectCalc {
private:
    struct LockRing {
        uint8_t color;      // начальный оттенок каждого кольца (оттенка из палитры) 0-255
        uint8_t huePos;     // местоположение начального оттенка кольца 0-w-1
        uint8_t shiftHueDir;    // 4 бита на ringHueShift, 4 на ringHueShift2
    };

    uint8_t ringWidth;      // максимальне количество пикселей в кольце (толщина кольца) от 1 до height / 2 + 1
    //uint8_t ringNb; // количество колец от 2 до height
    uint8_t lowerRingWidth, upperRingWidth; // количество пикселей в нижнем (lowerRingWidth) и верхнем (upperRingWidth) кольцах
    uint8_t currentRing; // кольцо, которое в настоящий момент нужно провернуть
    uint8_t stepCount; // оставшееся количество шагов, на которое нужно провернуть активное кольцо - случайное от w/5 до w-3

    // набор колец
    std::vector<LockRing> rings{std::vector<LockRing>(2, LockRing())};
  //std::vector<uint8_t> ringColor{std::vector<uint8_t>(fb->h())};    // начальный оттенок каждого кольца (оттенка из палитры) 0-255
  //std::vector<uint8_t> huePos{std::vector<uint8_t>(fb->h())};       // местоположение начального оттенка кольца 0-w-1
  //std::vector<uint8_t> shiftHueDir{std::vector<uint8_t>(fb->h())};  

  ////ringHueShift[ringsCount]; // шаг градиета оттенка внутри кольца -8 - +8 случайное число
  ////ringHueShift2[ringsCount]; // обычная скорость переливания оттенка всего кольца -8 - +8 случайное число



  void ringsSet();
  bool ringsRoutine();
  String setDynCtrl(UIControl*_val) override;

public:
    EffectRingsLock(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------------------------------ ЭФФЕКТ КУБИК 2D ----------------------
// (c) SottNick
// refactored by Vortigont
class EffectCube2d : public EffectCalc {
private:
  bool classic = false;
  uint8_t sizeX, sizeY; // размеры ячеек по горизонтали / вертикали
  uint8_t cntX, cntY; // количество ячеек по горизонтали / вертикали
  uint8_t fieldX, fieldY; // размер всего поля блоков по горизонтали / вертикали (в том числе 1 дополнительная пустая дорожка-разделитель с какой-то из сторон)
  uint8_t currentStep;
  uint8_t pauseSteps; // осталось шагов паузы
  uint8_t shiftSteps; // всего шагов сдвига
  bool direction = false; // направление вращения в текущем цикле (вертикаль/горизонталь)
  std::vector<int8_t> moveItems;     // индекс перемещаемого элемента
  std::vector< std::vector<uint8_t> > storage;
  int8_t globalShiftX, globalShiftY;
  uint8_t gX, gY;
  bool seamlessX = true;

  LedFB<CRGB> ledbuff;        // виртуальй холст

  void swapBuff();
  void cubesize();
  bool cube2dRoutine();
  bool cube2dClassicRoutine();
  void cube2dmoveCols(uint8_t moveItem, bool movedirection);
  void cube2dmoveRows(uint8_t moveItem, bool movedirection);
  String setDynCtrl(UIControl*_val) override;

public:
    EffectCube2d(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), sizeX(4), sizeY(4), ledbuff(1,1)  { cubesize(); moveItems = std::vector<int8_t>(direction ? cntX : cntY, 0); }
    void load() override;
    bool run() override;
};

// ------ Эффекты "Пикассо"
// (c) obliterator
#define PICASSO_MIN_PARTICLES   6U
#define PICASSO_MAX_PARTICLES   20U
class EffectPicasso : public EffectCalc {
private:
    struct Particle {
        float position_x{0}, position_y{0};
        float speed_x{0}, speed_y{0};
        CHSV color;
        uint8_t hue_next = 0;
        int8_t hue_step = 0;
    };
    uint8_t effId=0;
    uint8_t pidx = 0;
	float speedFactor{1};
    std::vector<Particle> particles{std::vector<Particle>(PICASSO_MIN_PARTICLES, Particle())};

    void generate(bool reset = false);
    void position();
    bool picassoRoutine();
    bool metaBallsRoutine();
    GradientPaletteList palettes;
public:
    EffectPicasso(LedFB<CRGB> *framebuffer);
    virtual ~EffectPicasso(){}
    bool run() override;
    String setDynCtrl(UIControl*_val) override;
};

// ------ Эффект "Прыгуны" (c) obliterator
#define LEAPERS_MIN     5
#define LEAPERS_MAX     20
class EffectLeapers : public EffectCalc {
    struct Leaper {
        float x{0}, y{0};
        float xd{0}, yd{0};
        byte color;
    };
private:
    unsigned numParticles = 0;
    uint8_t _rv{0};
	float speedFactor{1};
    std::vector<Leaper> leapers{std::vector<Leaper>(LEAPERS_MIN, Leaper())};

    void generate();
    void restart_leaper(Leaper &l);
    void move_leaper(Leaper &l);
	String setDynCtrl(UIControl*_val) override;
public:
    EffectLeapers(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
	void load() override;
    bool run() override;

};

// ------ Эффект "Лавовая Лампа"
// (c) obliterator
#define LIQLAMP_MASS_MIN    10
#define LIQLAMP_MASS_MAX    50
#define LIQLAMP_MIN_PARTICLES   10
#define LIQLAMP_MAX_PARTICLES   20
class EffectLiquidLamp : public EffectCalc {
    struct Particle{
        float position_x = 0;
        float position_y = 0;
        float speed_x = 0;
        float speed_y = 0;
        float rad = 0;
        float hot = 0;
        float spf = 0;
        int mass = 0;
        unsigned mx = 0;
        unsigned sc = 0;
        unsigned tr = 0;
    };

    uint8_t pidx = 0;
    bool physic_on = 1;
    uint8_t filter = 0;
	float speedFactor = 1.0;
    GradientPaletteList palettes;

    std::vector<Particle> particles{std::vector<Particle>(LIQLAMP_MIN_PARTICLES, Particle())};
    std::unique_ptr< Vector2D<uint8_t> > buff;
    std::unique_ptr< Vector2D<float> > buff2;

    void generate(bool reset = false);
    void position();
    void physic();
    bool routine();

public:
    EffectLiquidLamp(LedFB<CRGB> *framebuffer);
    void load() override { generate(true); };
    bool run() override {return routine();};
    String setDynCtrl(UIControl*_val) override;
};

// ------- Эффект "Вихри"
// Based on Aurora : https://github.com/pixelmatix/aurora/blob/master/PatternFlowField.h
// Copyright(c) 2014 Jason Coon
//адаптация SottNick
class EffectWhirl : public EffectCalc {
private:
    float ff_x;
    float ff_y;
    float ff_z;
    float hue;
    std::vector<Boid> boids;
    uint8_t micPick = 0;
	float speedFactor;

    const uint8_t ff_speed = 1; // чем выше этот параметр, тем короче переходы (градиенты) между цветами. 1 - это самое красивое
    const uint8_t ff_scale = 26; // чем больше этот параметр, тем больше "языков пламени" или как-то так. 26 - это норм

    bool whirlRoutine();
    String setDynCtrl(UIControl*_val) override;
public:
    EffectWhirl(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), boids( std::vector<Boid>(AVAILABLE_BOID_COUNT) ) {}
    void load() override;
    bool run() override;
};

// ------------- Эффект "Блики на воде Цвета"
// Идея SottNick
// переписал на программные блики + паттерны - (c) kostyamat
// Генератор бликов (c) stepko
class EffectAquarium : public EffectCalc {
private:

struct Drop{
    uint8_t posX, posY;
    float radius;
};

    CRGBPalette16 currentPalette;
    const uint8_t _scale = 25;
    const uint8_t _speed = 3;

    float hue = 0.;
    uint16_t x{0}, y{0}, z{0};
    Vector2D<uint8_t> noise;

    inline uint8_t maxRadius(){return fb->w() + fb->h();};
    std::vector<Drop> drops;
    uint8_t satur;
    uint8_t glare = 0;
    uint8_t iconIdx = 0;
	float speedFactor{1};

    void nGlare(uint8_t bri);
    void nDrops(uint8_t bri);
    void fillNoiseLED();

public:
    EffectAquarium(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer),
        noise(framebuffer->w(), framebuffer->h()),
        drops(std::vector<Drop>((fb->h() + fb->w()) / 6)) {}

    void load() override;
    String setDynCtrl(UIControl*_val) override;
    bool run() override;
};


// ----------- Эффект "Звезды"
// (c) SottNick
#define STARS_NUM (5)
class EffectStar : public EffectCalc {
private:

    struct Star{
        uint8_t color;                        // цвет звезды
        uint8_t points;                     // количество углов в звезде
        unsigned cntdelay;                  // задержка пуска звезды относительно счётчика
    };

    float driftx, drifty;
    float cangle, sangle;
    float radius2;
    float counter = 0;                                // счетчик для реализации смещений, нарастания и т.д.
	float _speedFactor{1};
	float _speed;
    bool setup = true;
    uint8_t micPick = 0;
    const uint8_t spirocenterX = fb->w() / 2;
    const uint8_t spirocenterY = fb->h() / 2;
    std::vector<Star> stars{std::vector<Star>(STARS_NUM)};

    void drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler);
	String setDynCtrl(UIControl*_val) override;

public:
    EffectStar(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

//---------- Эффект "Фейерверк"
// адаптация и переписал - kostyamat
// https://gist.github.com/jasoncoon/0cccc5ba7ab108c0a373
// vortigont переписал за kostyamat'ом

#define MIN_RCKTS   2U
#define MAX_RCKTS   8U     // максимальное количество снарядов
#define NUM_SPARKS  16U    // количество разлитающихся петард (частей снаряда)

struct DotsStore {
    accum88 gBurstx;
    accum88 gBursty;
    saccum78 gBurstxv;
    saccum78 gBurstyv;
    CRGB gBurstcolor;
    bool gSkyburst = false;
};

struct Dot {    // класс для создания снарядов и петард

  static constexpr saccum78 gGravity = 10;
  static constexpr fract8  gBounce = 127;
  static constexpr fract8  gDrag = 255;

  byte    show{0};
  byte    theType{0};
  accum88 x{0}, y{0};
  saccum78 xv{0}, yv{0};
  accum88 r{0};
  CRGB color{CRGB::Black};
  uint16_t cntdown{0};

  //Dot()
  void Move(DotsStore &store, bool flashing);
  void GroundLaunch(DotsStore &store, uint16_t h);
  void Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor, uint8_t dim);
  int16_t scale15by8_local( int16_t i, fract8 _scale ){ return (i * _scale / 256); };
};

class EffectFireworks : public EffectCalc {

    DotsStore store;
    byte dim;
    uint8_t valDim;
    bool flashing = false;
    bool fireworksRoutine();
    void sparkGen();
    std::vector<Dot> gDot;
    std::vector<Dot> gSparks;
    String setDynCtrl(UIControl*_val) override;
    void draw(Dot &d);
    int16_t _model_w(){ return 2*(fb->w() - 4) + fb->w(); };  // как далеко за экран может вылетить снаряд, если снаряд вылетает за экран, то всышка белого света (не особо логично)
    int16_t _model_h(){ return 2*(fb->h() - 4) + fb->h(); };
    int16_t _x_offset(){ return (_model_w()-fb->w())/2; };
    int16_t _y_offset(){ return (_model_h()-fb->h())/2; };
    void _screenscale(accum88 a, uint16_t N, uint16_t &screen, uint16_t &screenerr);

public:
    EffectFireworks(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), gDot(std::vector<Dot>(MIN_RCKTS)), gSparks(std::vector<Dot>(NUM_SPARKS)) {}
    //void load() override;
    bool run() override;
};

// ------------ Эффект "Тихий Океан"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
// https://raw.githubusercontent.com/FastLED/FastLED/master/examples/Pacifica/Pacifica.ino
class EffectPacific : public EffectCalc {
private:
	uint32_t speedFactor = 100;

    uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;    // "color index start" counters
    uint32_t sLastms = 0;

	void pacifica_one_layer(const TProgmemRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff);
	void pacifica_deepen_colors();
	void pacifica_add_whitecaps();
	String setDynCtrl(UIControl*_val) override;

public:
    EffectPacific(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    //void load() override;
    bool run() override;
};

#ifdef MIC_EFFECTS
//----- Эффект "Осциллограф" 
// (c) kostyamat
class EffectOsc : public EffectCalc {
private:
    int oscHV, oscilLimit;
#ifdef ESP32
    int pointer{2048};
#else
    int pointer{512};
#endif
    CRGB color;
    int div{1}, gain{1};
    int y[2] = {0, 0};
    String setDynCtrl(UIControl*_val) override;
    

public:
    EffectOsc(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer), oscHV(fb->w()), oscilLimit(fb->h()) {}
    bool run() override;
};
#endif

// ------ Эффект "Вышиванка" 
// (с) проект Aurora "Munch"
class EffectMunch : public EffectCalc {
private:
    byte count = 0;
    int8_t dir = 1;
    byte flip = 0;
    byte generation = 0;
    byte mic[2];
    byte rand;
    bool flag = false;
    uint8_t minDimLocal = fb->maxDim() > 32 ? 32 : 16;

    String setDynCtrl(UIControl*_val) override;
    bool munchRoutine();

public:
    EffectMunch(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------ Эффект "Цветной шум" 
// (с) https://gist.github.com/StefanPetrick/c856b6d681ec3122e5551403aabfcc68
class EffectNoise : public EffectCalc {
private:

    const uint8_t centreX = (fb->w() / 2) - 1;
    const uint8_t centreY = (fb->h() / 2) - 1;
    Noise3dMap  noise{Noise3dMap(1, fb->w(), fb->h())};
	uint8_t speedFactor;
    bool type = false;

	String setDynCtrl(UIControl*_val) override;

public:
    EffectNoise(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ---- Эффект "Мотыльки"
// (с) SottNick, https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/post-49262
class EffectButterfly : public EffectCalc {
private:
    float butterflysPosX[BUTTERFLY_MAX_COUNT];
    float butterflysPosY[BUTTERFLY_MAX_COUNT];
    float butterflysSpeedX[BUTTERFLY_MAX_COUNT];
    float butterflysSpeedY[BUTTERFLY_MAX_COUNT];
    float butterflysTurn[BUTTERFLY_MAX_COUNT];
    uint8_t butterflysColor[BUTTERFLY_MAX_COUNT];
    uint8_t butterflysBrightness[BUTTERFLY_MAX_COUNT];
    uint8_t deltaValue;
    uint8_t deltaHue;
    uint8_t hue;
    uint8_t hue2;
    byte step = 0;
    byte csum = 0;
    uint8_t cnt;
    bool wings = false;
    bool isColored = true;
	float speedFactor;
	String setDynCtrl(UIControl*_val) override;

public:
    EffectButterfly(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ---- Эффект "Тени"
// https://github.com/vvip-68/GyverPanelWiFi/blob/master/firmware/GyverPanelWiFi_v1.02/effects.ino
class EffectShadows : public EffectCalc {
private:
    uint16_t sPseudotime = 0;
    uint16_t sLastMillis = 0;
    uint16_t sHue16 = 0;
    bool linear = true;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectShadows(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ---- Эффект "Узоры"
// (c) kostyamat (Kostyantyn Matviyevskyy) 2020
// переделано kDn
// идея отсюда https://github.com/vvip-68/GyverPanelWiFi/
#define PATTERNS_BUFFSIZE   10
class EffectPatterns : public EffectCalc {
private:
    int8_t patternIdx = -1;
    int8_t lineIdx = 0;
    bool _subpixel = false;
    bool _sinMove = false;
    int8_t _speed = 1, _scale = 1;
    bool dir = false;
    byte csum = 0;
    byte _bri = 255U;
    std::vector< std::vector<uint8_t> > buff{ std::vector< std::vector<uint8_t> >(PATTERNS_BUFFSIZE, std::vector<uint8_t>(PATTERNS_BUFFSIZE)) };
    //LedFB<CRGB> buff(PATTERNS_BUFFSIZE, PATTERNS_BUFFSIZE);
    float xsin, ysin;
    unsigned long lastrun2;
    byte _sc = 0;
    float _speedX, _speedY;

    CHSV colorMR[12] = {
        CHSV(0, 0, 0),              // 0 - Black
        CHSV(HUE_RED, 255, 255),    // 1 - Red
        CHSV(HUE_GREEN , 255, 255),  // 2 - Green
        CHSV(HUE_BLUE, 255, 255),   // 3 - Blue
        CHSV(HUE_YELLOW, 255, 255), // 4 - Yellow
        CHSV(0, 0, 220),            // 5 - White
        CHSV(0, 255, 255),              // 6 - плавно меняеться в цикле (фон)
        CHSV(0, 255, 255),              // 7 - цвет равен 6 но +64
        CHSV(HUE_ORANGE, 255, 255),
        CHSV(HUE_PINK, 255, 255),
        CHSV(HUE_PURPLE, 255, 255),
        CHSV(HUE_AQUA, 255, 255),
    };

    String setDynCtrl(UIControl*_val) override;
    void drawPicture_XY();
    bool patternsRoutine();

public:
    EffectPatterns(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};


// ***************************** "Стрелки" *****************************
// взято отсюда https://github.com/vvip-68/GyverPanelWiFi/
class EffectArrows : public EffectCalc {
private:
    float arrow_x[4], arrow_y[4], stop_x[4], stop_y[4];
    byte arrow_direction;             // 0x01 - слева направо; 0x02 - снизу вверх; 0х04 - справа налево; 0х08 - сверху вниз
    byte arrow_mode, arrow_mode_orig; // 0 - по очереди все варианты
                                      // 1 - по очереди от края до края экрана;
                                      // 2 - одновременно по горизонтали навстречу к ентру, затем одновременно по вертикали навстречу к центру
                                      // 3 - одновременно все к центру
                                      // 4 - по два (горизонталь / вертикаль) все от своего края к противоположному, стрелки смещены от центра на 1/3
                                      // 5 - одновременно все от своего края к противоположному, стрелки смещены от центра на 1/3
    bool arrow_complete, arrow_change_mode;
    byte arrow_hue[4];
    byte arrow_play_mode_count[6];      // Сколько раз проигрывать полностью каждый режим если вариант 0 - текущий счетчик
    byte arrow_play_mode_count_orig[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - исходные настройки
    uint8_t _scale;
    float speedFactor;
    void arrowSetupForMode(byte mode, bool change);
    void arrowSetup_mode1();
    void arrowSetup_mode2();
    //void arrowSetup_mode3(;)
    void arrowSetup_mode4();

    String setDynCtrl(UIControl*_val) override;
public:
    EffectArrows(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------ Эффект "Дикие шарики"
// (с) https://gist.github.com/bonjurroughs/9c107fa5f428fb01d484#file-noise-balls
class EffectNBals : public EffectCalc {
private:
    uint8_t lastSecond = 99;
    uint16_t speedy;// speed is set dynamically once we've started up
    uint16_t _scale;
    byte beat1, beat2 = 0;
    byte balls = 4;
    void balls_timer();
    void blur();
    bool nballsRoutine();

    String setDynCtrl(UIControl*_val) override;

public:
    EffectNBals(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ------ Эффект "Притяжение"
// project Aurora
// доведено до ума - kostyamat
class EffectAttract : public EffectCalc {
private:
    const uint8_t spirocenterX = fb->w() / 2;
    const uint8_t spirocenterY = fb->h() / 2;
    float speedFactor;
    float mass{10};    // Mass, tied to size
    float G{0.5};      // Gravitational Constant
    uint8_t _mass = 127;
    uint8_t _energy = 127;
    bool loadingFlag = true;
    byte csum = 0;
    std::vector<Boid> boids;
    PVector location;   // Location

    PVector attract(Boid &m);
    void setup();
    String setDynCtrl(UIControl*_val) override;

public:
    EffectAttract(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer) {
        boids.assign(fb->h() *2 - fb->w() /2, Boid());
        location = PVector(spirocenterX, spirocenterY);
    }
    void load() override;
    bool run() override;
};

//------------ Эффект "Змейки"
// вариант субпикселя и поведения от kDn
#define MAX_SNAKES    (16U) 
class EffectSnake : public EffectCalc {
private:
    const int snake_len{fb->h()/2};
    float hue;
    float speedFactor;
    bool subPix = false;
    bool onecolor = false;
    enum class dir_t:uint8_t {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    struct Pixel{
        float x, y;
    };

    std::vector<CRGB> colors{std::vector<CRGB>(snake_len)};

    struct Snake {
        float internal_counter = 0.0;
        float internal_speedf = 1.0;
        std::vector<Pixel> pixels;

        dir_t direction;

        Snake(uint8_t len) : pixels(std::vector<Pixel>(len)) {}

        void newDirection();
        void shuffleDown(float speedy, bool subpix);
        void reset();
        void move(float speedy, uint16_t w,  uint16_t h);
        void draw(std::vector<CRGB> &colors, int snakenb, bool subpix, LedFB<CRGB> *fb, bool isDebug=false);
    };

    std::vector<Snake> snakes{ std::vector<Snake>(2, Snake(snake_len)) };
    String setDynCtrl(UIControl*_val) override;
    void reset();
public:
    EffectSnake(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

//------------ Эффект "Nexus"
// База паттерн "Змейка" из проекта Аврора, 
// перенос и переписан - kostyamat
#define NEXUS_MIN   5
class EffectNexus: public EffectCalc {
  struct Nexus{
    float posX{0};
    float posY{0};
    int8_t direct{0};           // направление точки 
    CRGB color{CRGB::Black};    // цвет точки
    float accel{0};             // персональное ускорение каждой точки
  };

    bool white = false;
    byte type = 1;
    uint8_t _scale = 1;
    bool randColor = false;
    //float windProgress{1};
	float speedFactor{1};
    std::vector<Nexus> nxdots{std::vector<Nexus>(NEXUS_MIN, Nexus())};

    void reload();
    void resetDot(Nexus &nx);
    String setDynCtrl(UIControl*_val) override;

  public:
    EffectNexus(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    void load() override;
};

// ----------- Эфеект "Змеиный Остров"
// (c) Сотнег
// адаптация и доработки kostyamat
#define MAX_SNAKES    (16U)
class EffectSnakeIsland : public EffectCalc {
    const uint8_t snake_len = fb->h()/2;
    struct Snake {
        long  last;            // тут будет траектория тела червяка
        float posX, posY;      // тут будет позиция головы
        float speedX, speedY;  // тут будет скорость червяка
        uint8_t color;         // тут будет начальный цвет червяка
        uint8_t direct;        //тут будет направление червяка
    };

    std::vector<Snake> snakes{std::vector<Snake>(1)};
	float speedFactor;

    String setDynCtrl(UIControl*_val) override;
    void regen();

public:
    EffectSnakeIsland(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эфеект "Попкорн"
// (C) Aaron Gotwalt (Soulmate)
// адаптация и доработки kostyamat
#define POPCORN_ROCKETS 10
class EffectPopcorn : public EffectCalc {
private:
    uint8_t numRockets = POPCORN_ROCKETS;
    bool blurred = false;
    bool revCol = false;
    //bool tiltDirec;
    float speedFactor;
    float center = (float)fb->w() / 2.;

    struct Rocket {
        float x, y, xd, yd;
        byte hue;
    };

    std::vector<Rocket> rockets{std::vector<Rocket>(POPCORN_ROCKETS, Rocket())};

    void restart_rocket(uint8_t r);
    void reload();

    String setDynCtrl(UIControl*_val) override;
    //void setscl(const byte _scl) override; // перегрузка для масштаба

public:
    EffectPopcorn(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

//-------- Эффект "Детские сны"
// (c) Stepko https://editor.soulmatelights.com/gallery/505
class EffectSmokeballs: public EffectCalc {
  private:
    struct Wave {
        uint16_t reg;
        uint16_t pos;
        float sSpeed;
        uint8_t maxMin;
        uint8_t waveColors;
    };
    uint8_t dimming = 240;
    uint8_t _scale = 1;
    float speedFactor = 0.1;
    std::vector<Wave> waves{std::vector<Wave>(fb->w()/4)};      // allow max w/4 waves to run simultaneously

    void shiftUp();
    void regen();
    String setDynCtrl(UIControl*_val) override;
  public:
    EffectSmokeballs(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эффект "Ёлки-Палки"
// "Cell" (C) Elliott Kember из примеров программы Soulmate
// Spaider и Spruce (c) stepko
class EffectCell: public EffectCalc {
  private:
    const uint8_t Lines = 5;
	const bool glitch = abs((int)fb->w()-(int)fb->h()) >= fb->minDim()/4;
	const byte density = 50;
    uint8_t Scale = 6;
    uint8_t _scale = 1;
    int16_t offsetX = 0;
    int16_t offsetY = 0;
    float x;
    uint8_t effId = 1;
    uint8_t hue;
    int16_t a;

	float speedFactor;
    void cell();
    void spider();
    void spruce();
    void vals();

    int width_adj(){ return (fb->w() < fb->h() ? (fb->h() - fb->w()) /2 : 0); };
    int height_adj(){ return (fb->h() < fb->w() ? (fb->w() - fb->h()) /2: 0); };

  public:
    EffectCell(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    String setDynCtrl(UIControl*_val) override;
};

// ---------- Эффект "Тикси Ленд"
// (c)  Martin Kleppe @aemkei, https://github.com/owenmcateer/tixy.land-display
class EffectTLand: public EffectCalc {
  private:
    bool isSeq = false;
    byte animation = 0;
    bool ishue;
    bool ishue2;
    byte hue = 0;
    byte hue2 = 128;
    byte shift = 0;
    byte fine = 1;
    double t;
    void processFrame(LedFB<CRGB> *fb, double t, double x, double y);
    float code(double i, double x, double y);
    String setDynCtrl(UIControl*_val);
  public:
    EffectTLand(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// ----------- Эффект "Осциллятор"
// (c) Сотнег (SottNick)
class EffectOscillator: public EffectCalc {
  private:
    uint8_t hue, hue2;                                 // постепенный сдвиг оттенка или какой-нибудь другой цикличный счётчик
    uint8_t deltaHue, deltaHue2;                       // ещё пара таких же, когда нужно много
    uint8_t step;                                      // какой-нибудь счётчик кадров или последовательностей операций
    uint8_t deltaValue;  
    unsigned long timer;

    struct OscillatingCell {
        byte red, green, blue;
        byte color;
    };

    Vector2D<OscillatingCell> oscillatingWorld{ Vector2D<OscillatingCell>(fb->w(), fb->h()) };

    void drawPixelXYFseamless(float x, float y, CRGB color);
    int redNeighbours(uint8_t x, uint8_t y);
    int blueNeighbours(uint8_t x, uint8_t y);
    int greenNeighbours(uint8_t x, uint8_t y);
    void setCellColors(uint8_t x, uint8_t y);
    //String setDynCtrl(UIControl*_val) override;
  public:
    EffectOscillator(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer) {}
    bool run() override;
    void load() override;
};

//------------ Эффект "Шторм" 
// (с) kostyamat 1.12.2020
class EffectWrain: public EffectCalc {
  private:

    struct Drop {
        float posX{0};
        float posY{0};
        uint8_t color{0};    // цвет капли
        float accell{0};     // персональное ускорение каждой капли
        uint8_t bri{0};      // яркость капли
    };

    const uint8_t cloudHeight = fb->h() / 5 + 1;
    float dotChaos;         // сила ветра
    int8_t dotDirect;       // направление ветра 
    bool clouds = false;
    bool storm = false;
    bool white = false;
    uint8_t _scale=1;
    byte type = 1;
    bool _flash;
    bool randColor = false;
    float windProgress = 0;
    float speedFactor = 0.5;
    uint32_t timer = 0;
    Vector2D<uint8_t> _noise {Vector2D<uint8_t>(fb->w(), cloudHeight)};
    std::vector<Drop> drops {std::vector<Drop>(fb->w() * 3)};

    void reload();
    String setDynCtrl(UIControl*_val) override;
    bool Lightning(uint16_t chanse);
    void Clouds(bool flash);

  public:
    EffectWrain(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    void load() override;
};

//-------- по мотивам Эффектов Particle System -------------------------
// https://github.com/fuse314/arduino-particle-sys
// https://github.com/giladaya/arduino-particle-sys
// https://www.youtube.com/watch?v=S6novCRlHV8&t=51s
//при попытке вытащить из этой библиотеки только минимально необходимое выяснилось, что там очередной (третий) вариант реализации субпиксельной графики.
//ну его нафиг. лучше будет повторить визуал имеющимися в прошивке средствами.

// ============= ЭФФЕКТ Фея/Источник ===============
// (c) SottNick

#define FAIRY_MIN_COUNT    4            // минимальное число объектов

class EffectFairy : public EffectCalc {
private:
struct TObject {
    float   posX, posY;
    float   speedX, speedY;
    float   shift;
    float   state;
    uint8_t hue;
    bool    isShift;
};

    std::vector<TObject> units{std::vector<TObject>(FAIRY_MIN_COUNT)};
    Boid boids[2];

    uint8_t hue, hue2;
    uint8_t step;
    uint8_t deltaValue;
    uint8_t deltaHue, deltaHue2;
    float speedFactor;
    bool type = false;
    byte blur;
    uint8_t _video = 255;
    uint8_t gain;

    int _max_units(){ return fb->minDim()*3; }
    void particlesUpdate(TObject &i);
    void fairyEmit(TObject &i);
    void fountEmit(TObject &i);
    bool fairy();
    void fount();
    //void setscl(const byte _scl) override; // перегрузка для масштаба
    String setDynCtrl(UIControl*_val) override;

public:
    EffectFairy(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ---------- Эффект "Бульбулятор"
// "Circles" (C) Elliott Kember https://editor.soulmatelights.com/gallery/11
// адаптация и переделка - kostyamat
#define CIRCLES_MIN 3
class EffectCircles : public EffectCalc {
private:
    struct Circle {
        int16_t centerX, centerY;
        byte hue;
        float bpm;

        float radius() {
            return EffectMath::fmap(triwave8(bpm), 0, 254, 0, 6); //beatsin16(bpm, 0, 500, 0, offset) / 100.0;
        }
    };

    byte color;
    float speedFactor{1};
    byte _video = 255;
    byte gain;
    std::vector<Circle> circles{ std::vector<Circle>(CIRCLES_MIN) };

    void move(Circle &c) {
        c.centerX = random(0, fb->maxWidthIndex());
        c.centerY = random(0, fb->maxHeightIndex());
        c.bpm = random(0, 255);
    }
    
    void reset(Circle &c) {
        move(c);
        c.hue = random(0, 255);
    }

    void drawCircle(LedFB<CRGB> *fb, Circle &circle);
    String setDynCtrl(UIControl*_val) override;

public:
    EffectCircles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;

};

// ----------- Эффект "Бенгальские Огни"
// (c) stepko https://wokwi.com/arduino/projects/289797125785520649
// 06.02.2021
class EffectBengalL : public EffectCalc {
    struct Spark{
        float posx, posy;
        float speedx, speedy;
        float sat;
        float fade;
        byte color;
    };

    const uint8_t minSparks = 4;
    const uint8_t maxSparks = fb->w() * 4;
    uint8_t gPosx, gPosy;

    bool centerRun = true;
    byte period = 10;
    byte _x = fb->w()/2;
    byte _y = fb->h()/2;
    std::vector<Spark> sparks{ std::vector<Spark>(minSparks) };

    void regen(Spark &s);
    void physics(Spark &s);
    String setDynCtrl(UIControl*_val) override;


public:
    EffectBengalL(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эффект "Шары"
// (c) stepko and kostyamat https://wokwi.com/arduino/projects/289839434049782281
// 07.02.2021
#define BALLS_MIN   3
class EffectBalls : public EffectCalc {
private:
    struct Ball {
        float x, y;
        float spdx, spdy;
        float radius;
        uint8_t color;
        bool rrad;
    };

    const float radiusMax = (float)fb->maxDim() /5;
    std::vector<Ball> balls{ std::vector<Ball>(BALLS_MIN) };
    float speedFactor;
    String setDynCtrl(UIControl*_val) override;
    void reset();

public:
    EffectBalls(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ---------- Эффект-игра "Лабиринт"
class EffectMaze : public EffectCalc {
private:
    const uint8_t _mwidth, _mheight;
    const uint16_t maxSolves;
    Vector2D<uint8_t> maze;
    int8_t playerPos[2];
    uint32_t labTimer;
    bool mazeMode = false;
    bool mazeStarted = false;
    uint8_t hue;
    CRGB color;
    CRGB playerColor = CRGB::White;

    bool loadingFlag = true;
    bool gameOverFlag = false;
    bool gameDemo = true;
    bool gamePaused = false;
    bool track = random8(0,2);  // будет ли трек игрока
    uint8_t buttons;

    unsigned long timer = millis(), gameTimer = 200;         // Таймер скорости игр

    void newGameMaze();
    void buttonsTickMaze();
    void movePlayer(int8_t nowX, int8_t nowY, int8_t prevX, int8_t prevY);
    void demoMaze();
    bool checkPath(int8_t x, int8_t y);
    void CarveMaze(int x, int y);
    void GenerateMaze();
    void SolveMaze();

    bool checkButtons(){ return (buttons != 4); }

    String setDynCtrl(UIControl*_val) override;
    //void setspd(const byte _spd) override; // перегрузка для скорости
public:
    EffectMaze(LedFB<CRGB> *framebuffer);
    bool run() override;

}; 

// --------- Эффект "Вьющийся Цвет"
// (c) Stepko https://wokwi.com/arduino/projects/283705656027906572
class EffectFrizzles : public EffectCalc {
private:
    float _speed;
    float _scale;
    //String setDynCtrl(UIControl*_val) override;

public:
    EffectFrizzles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    //void load() override;
    bool run() override;
};

// --------- Эффект "Северное Сияние"
// (c) kostyamat 05.02.2021
// идеи подсмотрены тут https://www.reddit.com/r/FastLED/comments/jyly1e/challenge_fastled_sketch_that_fits_entirely_in_a/

class EffectPolarL : public EffectCalc {
private:
    const byte numpalettes = 14;
    unsigned long timer = 0;
    float adjastHeight = 1;
    uint16_t adjScale = 1;
    byte pal = 0;
    uint16_t _scale = 30;
    byte flag = 0;
	byte _speed = 16;

    //void setscl(const byte _scl) override;
    String setDynCtrl(UIControl*_val) override;
    void palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min, const uint8_t _max) override;
    void palettesload() override;

public:
    EffectPolarL(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// --------- Эффект "Космо-Гонщик"
// (c) Stepko + kostyamat https://editor.soulmatelights.com/my-patterns/655
class EffectRacer: public EffectCalc {
private:
    float posX = random(0, fb->w()-1);
    float posY = random(0, fb->h()-1);
    uint8_t aimX = random(0, fb->w())-1;
    uint8_t aimY = random(0, fb->h()-1);
    float radius = 0;
    byte hue = millis()>>1; //random(0, 255);
    CRGB color;
    float speedFactor;
    float addRadius;
    float angle;
    byte starPoints = random(3, 7);

    const float _speed = (float)fb->size() / 256; // Нормализация скорости для разных размеров матриц
    const float _addRadius = (float)fb->size() / 4000;   // Нормализация скорости увеличения радиуса круга для разных матриц


    void aimChange();
    void drawStarF(float x, float y, float biggy, float little, int16_t points, float dangle, CRGB color);
    //void setspd(const byte _spd) override;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectRacer(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------------- Эффект "Магма"
// (c) Сотнег (SottNick) 2021
// адаптация и доводка до ума - kostyamat
#define MAGMA_MIN_OBJ   (fb->w()/2)
#define MAGMA_MAX_OBJ   (fb->w()*3)
class EffectMagma: public EffectCalc {
private:

    struct Magma {
        float posX{0}, posY{0};
        float speedX{0};
        float shift{0};
        uint8_t hue{0};
    };

    float ff_y{0}, ff_z{0};         // большие счётчики
    //control magma bursts
    const byte deltaValue = 6U;     // 2-12 
    const byte deltaHue = 8U;       // высота языков пламени должна уменьшаться не так быстро, как ширина
    const float gravity = 0.1;
    uint8_t step = fb->w();
    float speedFactor{0.1};
    std::vector<uint8_t> shiftHue{std::vector<uint8_t>(fb->h())};
    std::vector<Magma> particles{std::vector<Magma>(fb->w(), Magma())};

    void palettesload();
    void regen();
    void leapersMove_leaper(Magma &l);
    void leapersRestart_leaper(Magma &l);
    String setDynCtrl(UIControl*_val) override;

public:
    EffectMagma(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ------------- Эффект "Флаги"
// (c) Stepko + kostyamat
// 17.03.21
// https://editor.soulmatelights.com/gallery/739-flags
class EffectFlags: public EffectCalc {
private:
    const float DEVIATOR = 512. / fb->w();
    float counter;
    uint8_t flag = 0;
    uint8_t _flag;
    uint8_t count;
    uint8_t _speed; // 1 - 16
    const uint8_t CHANGE_FLAG = 30; // >= 10 Autochange

    uint8_t thisVal;
    uint8_t thisMax;

    //Germany
    void germany(uint8_t i);
    //Ukraine
    void ukraine(uint8_t i);
    //Belarus
    void belarus(uint8_t i);
    //Russia
    void russia(uint8_t i);
    //Poland
    void poland(uint8_t i);
    //The USA
    void usa(uint8_t i);
    //Italy
    void italy(uint8_t i);
    //France
    void france(uint8_t i);
    //UK
    void uk(uint8_t i);
    //Spain
    void spain(uint8_t i);

    void changeFlags();
    String setDynCtrl(UIControl*_val) override;

public:
    EffectFlags(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    //void load () override;
    bool run() override;
};

// --------------------- Эффект "Звездный Десант"
// Starship Troopers https://editor.soulmatelights.com/gallery/839-starship-troopers
// Based on (c) stepko`s codes https://editor.soulmatelights.com/gallery/639-space-ships
// reworked (c) kostyamat (subpixel, shift speed control, etc) 08.04.2021
class EffectStarShips: public EffectCalc {
private:
    byte _scale = 8;
    const byte DIR_CHARGE = 2; // Chance to change direction 1-5
    uint16_t chance = 4096;

    byte dir = 3;
    int8_t _dir = 0;
    byte count = 0;
    uint8_t _fade = 20;

	float speedFactor = 0.5;

    void draw(float x, float y, CRGB color);
    String setDynCtrl(UIControl*_val) override;

public:
    EffectStarShips(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
    void load() override;
};
#ifdef MIC_EFFECTS
/* -------------- эффект "VU-meter"
    (c) G6EJD, https://www.youtube.com/watch?v=OStljy_sUVg&t=0s
    reworked by s-marley https://github.com/s-marley/ESP32_FFT_VU
    adopted for FireLamp_jeeUI by kostyamat, kDn
    reworked and updated (c) kostyamat 24.04.2021
*/
class EffectVU: public EffectCalc {
private:
    CRGBPalette16 gradPal[5] = {
        purple_gp,    rainbowsherbet_gp, 
        redyellow_gp, Colorfull_gp, es_ocean_breeze_068_gp
    };
    uint8_t bands = fb->w();
    uint8_t bar_width{1};
    uint8_t calcArray = 1;          // уменьшение частоты пересчета массива
    uint8_t colorTimer = 0;
    const uint8_t colorDev = 256/fb->maxHeightIndex();
    // Sampling and FFT stuff
    std::vector<float> peak{std::vector<float>(fb->w())};              // The length of these arrays must be >= bands
    std::vector<float> oldBarHeights{std::vector<float>(fb->w())};
    std::vector<float> bandValues{std::vector<float>(fb->w())};
    MicWorker *mw = nullptr;

    float samp_freq;
    double last_freq = 0;
    uint8_t last_min_peak, last_max_peak;
    float maxVal;
    float threshold;
    byte tickCounter;
    byte colorType;

    float amplitude = 1.0;
    int effId = 0;
    bool type = false;
    bool colorShifting = false;
    const float speedFactorVertical = (float)fb->h() / 16;
    bool averaging = true;

    String setDynCtrl(UIControl*_val) override;
    void horizontalColoredBars(uint8_t band, float barHeight, uint8_t type = 0, uint8_t colorShift = 0);
    void paletteBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift = 0);
    void verticalColoredBars(uint8_t band, float barHeight, uint8_t type = 0, uint8_t colorShift = 0);
    void centerBars(uint8_t band, float barHeight, CRGBPalette16& palette, uint8_t colorShift = 0);
    void whitePeak(uint8_t band);
    void outrunPeak(uint8_t band, CRGBPalette16& palette, uint8_t colorShift = 0);
    void waterfall(uint8_t band, uint8_t barHeight);

public:
    EffectVU(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    ~EffectVU(){ delete mw; }
    bool run() override;
    void load() override;
};
#endif

// ----------- Эффект "Огненная лампа"
// https://editor.soulmatelights.com/gallery/546-fire
// (c) Stepko 17.06.21
// sparks (c) kostyamat 10.01.2022 https://editor.soulmatelights.com/gallery/1619-fire-with-sparks
class EffectFire2021 : public EffectCalc {
private:
    byte _pal = 8;
    byte _scale = 32;
	byte speedFactor;
    uint32_t t;
    bool withSparks = false;

    const uint8_t sparksCount = fb->w() / 4;
    const uint8_t spacer = fb->h()/4;

    class Spark {
    private:
        CRGB color;
        uint8_t Bri;
        uint8_t Hue;
        float x, y, speedy = 1;
    
    public:
        void addXY(float nx, float ny, LedFB<CRGB> *fb);

        float getY() { return y; }

        void reset(LedFB<CRGB> *fb);

        void draw(LedFB<CRGB> *fb);
    }; 

    std::vector<Spark> sparks{std::vector<Spark>(sparksCount, Spark())};

    String setDynCtrl(UIControl*_val) override;
    void palettesload() override;

public:
    EffectFire2021(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эффект "Пятнашки"
// https://editor.soulmatelights.com/gallery/1471-puzzles-subpixel
// (c) Stepko 10.12.21
class EffectPuzzles : public EffectCalc {
private:
private:
    byte psizeX = 4;
    byte psizeY = 4;
    uint8_t pcols, prows;
    float speedFactor = 0.1;
    std::vector< std::vector<uint8_t> > puzzle;
    byte color;
    Vector2<int8_t> z_dot;
    byte step;
    Vector2<int8_t> move{0, 0};
    Vector2<float> shift{ 0, 0 };

    void draw_square(byte x1, byte y1, byte x2, byte y2, byte col);
    void draw_squareF(float x1, float y1, float x2, float y2, byte col);
    void regen();

    String setDynCtrl(UIControl*_val) override;

public:
    EffectPuzzles(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){ regen(); }
    void load() override;
    bool run() override;
};

// ============= Эффект Цветные драже ===============
// (c) SottNick
//по мотивам визуала эффекта by Yaroslaw Turbin 14.12.2020
//https://vk.com/ldirko программный код которого он запретил брать
class EffectPile : public EffectCalc {
private:
    uint8_t pcnt = 0U, _scale;
    String setDynCtrl(UIControl*_val) override;

public:
    EffectPile(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ============= Эффект ДНК ===============
// (c) Stepko
// https://editor.soulmatelights.com/gallery/1520-dna
//по мотивам визуала эффекта by Yaroslaw Turbin
//https://vk.com/ldirko программный код которого он запретил брать
class EffectDNA : public EffectCalc {
private:
    float a = (256.0 / (float)fb->w());
    float t = 0.0;
    float speedFactor = 0.5;
    bool flag = true; 
    bool bals = false;
    uint8_t type = 0, _type = 1, _scale = 16;
    
    String setDynCtrl(UIControl*_val) override;

public:
    EffectDNA(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    //void load() override;
    bool run() override;
};

// ----------- Эффект "Дым"
// based on cod by @Stepko (c) 23/12/2021
class EffectSmoker : public EffectCalc {
private:
    byte color, saturation;
    byte _scale = 30, glitch;
	float speedFactor;
    float t;

    String setDynCtrl(UIControl*_val) override;

public:
    EffectSmoker(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}

    bool run() override;
};

// ----------- Эффект "Мираж"
// based on cod by @Stepko (c) 23/12/2021
class EffectMirage : public EffectCalc {
private:
    const float div = 10.;
    const uint16_t width = (fb->w() - 1) * div, height = fb->h() * div;
    uint16_t _speed;
    byte color;
    bool colorShift = false;
    Noise3dMap buff{Noise3dMap(1, fb->w()+2, fb->h()+2)};
    bool a = true;
	float speedFactor;

    String setDynCtrl(UIControl*_val) override;
    void drawDot(float x, float y, byte a);
    void blur();

public:
    EffectMirage(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    bool run() override;
};

// -------------------- Эффект "Акварель"
// (c) kostyamat 26.12.2021
// https://editor.soulmatelights.com/gallery/1587-oil
#define BLOT_COUNT 1U
class EffectWcolor : public EffectCalc {
private:
    float speedFactor;
    uint8_t blur;
    bool mode = false;
    float t;

    class Blot {
    private:
        byte hue, sat, bri;
        int x0, y0;
        std::vector<float> x;
        std::vector<float> y;

    public:
        Blot(int size) : x(std::vector<float>(size)), y(std::vector<float>(size)) {}

        void appendXY(float nx, float ny) {
            for (auto &i : x) i += nx;
            for (auto &i : y) i += ny;
        }
        
        void reset(int w, int h);
        double getY();
        void drawing(LedFB<CRGB> *fb);
    };


    std::vector<Blot> blots = std::vector<Blot>(BLOT_COUNT, fb->w()/2);

    String setDynCtrl(UIControl*_val) override;

public:
    EffectWcolor(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

// ----------- Эффект "Неопалимая купина"
// RadialFire
// (c) Stepko and Sutaburosu https://editor.soulmatelights.com/gallery/1570-radialfire
// refactoring, stretching for any dimension Vortigont
class EffectRadialFire : public EffectCalc {
    Vector2D<float> xy_angle{ Vector2D<float>(fb->w(), fb->h()) };
    Vector2D<float> xy_radius{ Vector2D<float>(fb->w(), fb->h()) };
    float t{0};
    uint8_t _scale{1};
    bool mode = false;

    String setDynCtrl(UIControl*_val) override;
    void palettesload() override;

public:
    EffectRadialFire(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer) {}
    void load() override;
    bool run() override;
};


class EffectSplashBals : public EffectCalc {
private:
    uint8_t count = 3;
    uint8_t hue;
    const uint8_t dev = 5;
    const float R = (float)fb->size()/128;

    struct Ball{
        float x1{0}, y1{0};
        float x2{0}, y2{0};
        uint8_t iniX1{0}, iniX2{0};
        uint8_t iniY1{0}, iniY2{0};
    };

    std::array<Ball, 3> balls;

    float speedFactor{1};

    float dist(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

    String setDynCtrl(UIControl*_val) override;

public:
    EffectSplashBals(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;
};

/* Настроечная мира */
class EffectMira : public EffectCalc {
    unsigned cnt{0}, x{0}, y{0};
public:
    EffectMira(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
    void load() override;
    bool run() override;// { return false; };
};

/* Эффект "Цветение" */
class EffectFlower : public EffectCalc {
	private:
        uint8_t effTimer;
        float ZVoffset = 0;
        const float COLS_HALF = fb->w() * .5;
        const float ROWS_HALF = fb->h() * .5;
        int16_t ZVcalcDist(uint8_t x, uint8_t y, float center_x, float center_y);
	public:
    EffectFlower(LedFB<CRGB> *framebuffer) : EffectCalc(framebuffer){}
        bool run() override;
};
#include "log.h"
/*
    Effect "Tetsrik clock"
    based on https://github.com/witnessmenow/WiFi-Tetris-Clock
*/
class TetrisClock : public EffectCalc {
    LedFB_GFX   screen;
    TetrisMatrixDraw t_clk;     // Main clock
    TetrisMatrixDraw t_m;       // The "M" of AM/PM
    TetrisMatrixDraw t_ap;      // The "P" or "A" of AM/PM

    Task seconds;               // seconds pulse
    Task animatic;              // animation task

    bool animation_idle;        // animation in progress
    bool hour24{1};             // 12/24 hour mode
    bool showColon{0};          // hh:mm sepparator
    bool forceRefresh = true;   // redraw all numbers
    bool redraw{0};             // flag that triggers screen refresh

    String lastDisplayedTime;
    uint8_t lastDisplayedAmPm;


    void _clock_animation();
    void _gettime();
    void _handleColonAfterAnimation();
    String setDynCtrl(UIControl*_val) override;

	public:
    TetrisClock(std::shared_ptr< LedFB<CRGB> > framebuffer) : EffectCalc(framebuffer.get()), screen(framebuffer), t_clk(screen), t_m(screen), t_ap(screen) {
        screen.setRotation(2);
    }
    ~TetrisClock(){ ts.deleteTask(seconds); }
    void load() override; 
    bool run() override;
};
