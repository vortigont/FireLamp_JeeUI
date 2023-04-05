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

#include "config.h"
#include "ledmatrix.hpp"
// Общий набор мат. функций и примитивов для обсчета эффектов

#define M_PI_2	1.57079632679489661923

#define MAP_SIN 1
#define MAP_COS 0
#define MOVE_X 1
#define MOVE_Y 0

#define NUM_LAYERS             (1U)                 // The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS2            (2U)                 // The coordinates for 3 16-bit noise spaces.

namespace EffectMath {
  constexpr uint16_t maxDim = ((WIDTH>HEIGHT)?WIDTH:HEIGHT);
  constexpr uint16_t minDim = ((WIDTH<HEIGHT)?WIDTH:HEIGHT);
  constexpr uint16_t maxHeightIndex = (int16_t)HEIGHT-1;
  constexpr uint16_t maxWidthIndex = (int16_t)WIDTH-1;
  constexpr uint16_t getmaxDim() {return maxDim;}
  constexpr uint16_t getminDim() {return minDim;}
  constexpr int16_t getmaxWidthIndex() {return maxWidthIndex;}
  constexpr int16_t getmaxHeightIndex() {return maxHeightIndex;}

  /** полезные обертки **/
  constexpr uint8_t wrapX(int8_t x){ return (x + WIDTH) % WIDTH; }
  constexpr uint8_t wrapY(int8_t y){ return (y + HEIGHT) % HEIGHT; }



  void blur1d( CRGB* leds, uint16_t numLeds, fract8 blur_amount);
  void blur2d( CRGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);
  // blurRows: perform a blur1d on every row of a rectangular matrix
  void blurRows( CRGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);
  // blurColumns: perform a blur1d on each column of a rectangular matrix
  void blurColumns(CRGB* leds, uint8_t width, uint8_t height, fract8 blur_amount);

  // для работы с буфером
  uint32_t getPixelNumberBuff(uint16_t x, uint16_t y, uint8_t W , uint8_t H); // получить номер пикселя в буфере по координатам
  
  CRGB &getPixel(uint16_t x, uint16_t y);
  uint8_t mapsincos8(bool map, uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255);
  void MoveFractionalNoise(bool scale, const uint8_t noise3d[][WIDTH][HEIGHT], int8_t amplitude, float shift = 0);
  void fadePixel(uint8_t i, uint8_t j, uint8_t step);
  void fader(uint8_t step);
  uint8_t ceil8(const uint8_t a, const uint8_t b);
  CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter = 5);
  CRGB makeDarker( const CRGB& color, fract8 howMuchDarker = 5);
  float randomf(float min, float max);
  bool isInteger(float val);
  // Функция возврашает "вес" яркости пикселя от 0 (черный) до 765 (белый). Может использоваться для проверки не "пустое ли место"
  uint16_t RGBweight (CRGB *leds, uint16_t idx);
  void confetti(byte density);
  void addGlitter(uint8_t chanceOfGlitter = 127);
  void nightMode(CRGB *leds);
  void gammaCorrection();

    // функция получения цвета пикселя по его номеру
    uint32_t getPixColor(uint32_t thisSegm); 
    // функция получения цвета пикселя в матрице по его координатам
    uint32_t getPixColorXY(int16_t x, int16_t y); 
    // залить все
    void fillAll(const CRGB &color); 
    
    void drawPixelXY(int16_t x, int16_t y, const CRGB &color); // функция отрисовки точки по координатам X Y
    void wu_pixel(uint32_t x, uint32_t y, CRGB col);
    void drawPixelXYF(float x, float y, const CRGB &color, uint8_t darklevel=25); // darklevel - насколько затемнять картинку
    void drawPixelXYF_Y(int16_t x, float y, const CRGB &color, uint8_t darklevel=50);
    void drawPixelXYF_X(float x, int16_t y, const CRGB &color, uint8_t darklevel=50);
    
    // Вариант субпикселя от @stepko, в некоторых случаях работает лучше, но в некоторых хуже
    void sDrawPixelXYF(float x, float y, const CRGB &color); 
    void sDrawPixelXYF_Y(int16_t x, float y, const CRGB &color);
    void sDrawPixelXYF_X(float x, int16_t y, const CRGB &color);

    CRGB getPixColorXYF_X(float x, int16_t y);
    CRGB getPixColorXYF_Y(int16_t x, float y);
    CRGB getPixColorXYF(float x, float y);

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const CRGB &color);
    void drawLineF(float x1, float y1, float x2, float y2, const CRGB &color);
	void drawSquareF(float x, float y, float leg, CRGB color);
    void drawCircle(int x0, int y0, int radius, const CRGB &color);
    void drawCircleF(float x0, float y0, float radius, const CRGB &color, float step = 0.25);
    void fill_circleF(float cx, float cy, float radius, CRGB col);
    void setLedsNscale8(uint16_t idx, uint8_t val);
    void dimAll(uint8_t value);
    CRGB &getLed(uint16_t idx);
    void blur2d(uint8_t val);

    /** аналог ардуино функции map(), но только для float   */
    double fmap(const double x, const double in_min, const double in_max, const double out_min, const double out_max);

    // finds distance between two points defined by floating coordinates
    float distance(float x1, float y1, float x2, float y2);
    // чуть менее точная, зато в 3 раза быстрее
    float sqrt(float x);
    float tan2pi_fast(float x);
    float atan2_fast(float y, float x);
    float atan_fast(float x);

    // аналог fmap, но не линейная. (linear == fmap)
    float mapcurve(const float x, const float in_min, const float in_max, const float out_min, const float out_max, float (*curve)(float,float,float,float));
    float linear(float t, float b, float c, float d);
    float InQuad(float t, float b, float c, float d);
    float OutQuad(float t, float b, float c, float d);
    float InOutQuad(float t, float b, float c, float d);
    float InCubic(float t, float b, float c, float d);
    float OutCubic(float t, float b, float c, float d);
    float InOutCubic(float t, float b, float c, float d);
    float InQuart(float t, float b, float c, float d);
    float OutQuart(float t, float b, float c, float d);
    float InOutQuart(float t, float b, float c, float d);
    float InQuint(float t, float b, float c, float d);
    float fixed_to_float(int input);
    int float_to_fixed(float input);
    float OutQuint(float t, float b, float c, float d);
    float InOutQuint(float t, float b, float c, float d);
    // static float InSine(float t, float b, float c, float d) { return -c * Math.cos(t/d * (Math.PI/2)) + c + b; }
    // static float OutSine(float t, float b, float c, float d) { return c * Math.sin(t/d * (Math.PI/2)) + b; }
    // static float InOutSine(float t, float b, float c, float d) { return -c/2 * (Math.cos(Math.PI*t/d) - 1) + b; }
    float InExpo(float t, float b, float c, float d);
    float OutExpo(float t, float b, float c, float d);
    float InOutExpo(float t, float b, float c, float d);
    float InCirc(float t, float b, float c, float d);
    float OutCirc(float t, float b, float c, float d);
    float InOutCirc(float t, float b, float c, float d);
};  //  namespace EffectMath


template <class T>
class Vector2 {
public:
    T x, y;

    Vector2() :x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    Vector2(const Vector2& v) : x(v.x), y(v.y) {}

    Vector2& operator=(const Vector2& v) {
        x = v.x;
        y = v.y;
        return *this;
    }

    bool isEmpty() {
        return x == 0 && y == 0;
    }

    bool operator==(Vector2& v) {
        return x == v.x && y == v.y;
    }

    bool operator!=(Vector2& v) {
        return !(x == y);
    }

    Vector2 operator+(Vector2& v) {
        return Vector2(x + v.x, y + v.y);
    }
    Vector2 operator-(Vector2& v) {
        return Vector2(x - v.x, y - v.y);
    }

    Vector2& operator+=(Vector2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    Vector2& operator-=(Vector2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2 operator+(double s) {
        return Vector2(x + s, y + s);
    }
    Vector2 operator-(double s) {
        return Vector2(x - s, y - s);
    }
    Vector2 operator*(double s) {
        return Vector2(x * s, y * s);
    }
    Vector2 operator/(double s) {
        return Vector2(x / s, y / s);
    }

    Vector2& operator+=(double s) {
        x += s;
        y += s;
        return *this;
    }
    Vector2& operator-=(double s) {
        x -= s;
        y -= s;
        return *this;
    }
    Vector2& operator*=(double s) {
        x *= s;
        y *= s;
        return *this;
    }
    Vector2& operator/=(double s) {
        x /= s;
        y /= s;
        return *this;
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    void rotate(double deg) {
        double theta = deg / 180.0 * M_PI;
        double c = cos(theta);
        double s = sin(theta);
        double tx = x * c - y * s;
        double ty = x * s + y * c;
        x = tx;
        y = ty;
    }

    Vector2& normalize() {
        if (length() == 0) return *this;
        *this *= (1.0 / length());
        return *this;
    }

    float dist(Vector2 v) const {
        Vector2 d(v.x - x, v.y - y);
        return d.length();
    }
    float length() const {
        return sqrt(x * x + y * y);
    }

    float mag() const {
        return length();
    }

    float magSq() {
        return (x * x + y * y);
    }

    void truncate(double length) {
        double angle = atan2f(y, x);
        x = length * cos(angle);
        y = length * sin(angle);
    }

    Vector2 ortho() const {
        return Vector2(y, -x);
    }

    static float dot(Vector2 v1, Vector2 v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }
    static float cross(Vector2 v1, Vector2 v2) {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }

    void limit(float max) {
        if (magSq() > max*max) {
            normalize();
            *this *= max;
        }
    }
};

typedef Vector2<float> PVector;

// Flocking
// Daniel Shiffman <http://www.shiffman.net>
// The Nature of Code, Spring 2009

// Boid class
// Methods for Separation, Cohesion, Alignment added
class Boid {
  public:

    PVector location;
    PVector velocity;
    PVector acceleration;
    float maxforce;    // Maximum steering force
    float maxspeed;    // Maximum speed

    float desiredseparation = 4;
    float neighbordist = 8;
    byte colorIndex = 0;
    float mass;

    boolean enabled = true;

    Boid() {}

    Boid(float x, float y) {
      acceleration = PVector(0, 0);
      velocity = PVector(randomf(), randomf());
      location = PVector(x, y);
      maxspeed = 1.5;
      maxforce = 0.05;
    }

    static float randomf() {
      return mapfloat(random(0, 255), 0, 255, -.5, .5);
    }

    static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void run(Boid boids [], uint8_t boidCount) {
      flock(boids, boidCount);
      update();
      // wrapAroundBorders();
      // render();
    }

    // Method to update location
    void update();

    inline void applyForce(PVector force) {
      // We could add mass here if we want A = F / M
      acceleration += force;
    }

    void repelForce(PVector obstacle, float radius);

    // We accumulate a new acceleration each time based on three rules
    void flock(Boid boids [], uint8_t boidCount);

    // Separation
    // Method checks for nearby boids and steers away
    PVector separate(Boid boids [], uint8_t boidCount);

    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    PVector align(Boid boids [], uint8_t boidCount);

    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    PVector cohesion(Boid boids [], uint8_t boidCount);

    // A method that calculates and applies a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    PVector seek(PVector target);

    // A method that calculates a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    void arrive(PVector target);

    void wrapAroundBorders();

    void avoidBorders();

    bool bounceOffBorders(float bounce);

    void render() {
      // // Draw a triangle rotated in the direction of velocity
      // float theta = velocity.heading2D() + radians(90);
      // fill(175);
      // stroke(0);
      // pushMatrix();
      // translate(location.x,location.y);
      // rotate(theta);
      // beginShape(TRIANGLES);
      // vertex(0, -r*2);
      // vertex(-r, r*2);
      // vertex(r, r*2);
      // endShape();
      // popMatrix();
      // backgroundLayer.drawPixel(location.x, location.y, CRGB::Blue);
    }
};
