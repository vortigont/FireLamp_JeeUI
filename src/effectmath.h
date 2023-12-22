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

#include "ledfb.hpp"
// Общий набор мат. функций и примитивов для обсчета эффектов

#define M_PI_2	1.57079632679489661923

#define MAP_SIN 1
#define MAP_COS 0
#define MOVE_X 1
#define MOVE_Y 0

namespace EffectMath {
    /*    Наложение эффектов на буфер, рисование, работа с цветами     */

    // затенение
    void nightMode(LedFB<CRGB> *ledarr);

    /**
   * @brief добавить пиксель случайного цвета в случайном месте
   * 
   * @param density lesser number -> higher chance
   */
    void confetti(LedFB<CRGB> *leds, byte density);

    /**
   * @brief с некоторой вероятностью добавляет вспышку в случайном месте 
   * 
   * @param leds 
   * @param chanceOfGlitter lesser number -> higher chance
   */
    void addGlitter(LedFB<CRGB> *leds, uint8_t chanceOfGlitter = 127);

    CRGB makeBrighter( const CRGB& color, fract8 howMuchBrighter = 5);
    CRGB makeDarker( const CRGB& color, fract8 howMuchDarker = 5);


    /**
     * @brief FastLED's blur2d function over LedFB<CRGB>
     * 
     * @param leds framebuffer
     * @param blur_amount 
     */
    void blur2d(LedFB<CRGB> *leds, fract8 blur_amount);
    // blurRows: perform a blur1d on every row of a rectangular matrix
    void blurRows(LedFB<CRGB> *leds, fract8 blur_amount);
    // blurColumns: perform a blur1d on each column of a rectangular matrix
    void blurColumns(LedFB<CRGB> *leds, fract8 blur_amount);

    // нарисовать линию в буфере
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const CRGB &color, LedFB<CRGB> *fb);

    // нарисовать окружность в буфере
    void drawCircle(int x0, int y0, int radius, const CRGB &color, LedFB<CRGB> *fb);

    /*
     * AA plotting and drawing
     * wu aa sutaburosu code  https://wokwi.com/projects/306764419316056642
     * https://www.reddit.com/r/FastLED/comments/p2twhw/supersample_crgb_array/
     */

    // wu weights for AA
    constexpr uint8_t wu_weight(uint8_t a, uint8_t b){return (a*b+a+b)>>8;};

    // plot a wu pixel in buffer
    void wu_pixel(uint32_t x, uint32_t y, CRGB col, LedFB<CRGB> *fb);

    /**
     * @brief draw a dot in a framebuffer defined by non integer coordinates
     * will dissolve into four surrounding pixels
     * @param x 
     * @param y 
     * @param color 
     * @param fb 
     * @param darklevel - насколько затемнять картинку
     */
    void drawPixelXYF(float x, float y, const CRGB &color, LedFB<CRGB> *fb, uint8_t darklevel=25);

    void drawPixelXYF_Y(int16_t x, float y, const CRGB &color, LedFB<CRGB> *fb, uint8_t darklevel=50);
    void drawPixelXYF_X(float x, int16_t y, const CRGB &color, LedFB<CRGB> *fb, uint8_t darklevel=50);

    // Вариант субпикселя от @stepko, в некоторых случаях работает лучше, но в некоторых хуже
    void sDrawPixelXYF(float x, float y, const CRGB &color, LedFB<CRGB> *fb); 
    void sDrawPixelXYF_Y(int16_t x, float y, const CRGB &color, LedFB<CRGB> *fb);
    void sDrawPixelXYF_X(float x, int16_t y, const CRGB &color, LedFB<CRGB> *fb);

    /**
     * @brief draw a line in a framebuffer defined by non integer coordinates
     * 
     * @param x1 
     * @param y1 
     * @param x2 
     * @param y2 
     * @param color 
     */
    void drawLineF(float x1, float y1, float x2, float y2, const CRGB &color, LedFB<CRGB> *fb);

    /**
     * @brief draw a circle in a framebuffer defined by non integer coordinates
     * 
     * @param x0 
     * @param y0 
     * @param radius 
     * @param color 
     * @param fb 
     * @param step 
     */
    void drawCircleF(float x0, float y0, float radius, const CRGB &color, LedFB<CRGB> *fb, float step = 0.25);

    void fill_circleF(float cx, float cy, float radius, CRGB col, LedFB<CRGB> *fb);

    void drawSquareF(float x, float y, float leg, CRGB color, LedFB<CRGB> *fb);



    // ***************************
    // ***  Математические     ***
    // ***************************

    uint16_t mapsincos8(bool map, uint8_t theta, uint16_t lowest = 0, uint16_t highest = 65535);

    /**
     * Возвращает частное от а,б округленное до большего целого
     */
    uint8_t ceil8(uint8_t a, uint8_t b);

    /**
     функция возвращает рандомное значение float между min и max с шагом 1/1024
     kostyamat добавил
    */
    float randomf(float min, float max);

    // finds distance between two points defined by floating coordinates
    float distance(float x1, float y1, float x2, float y2);

    /** аналог ардуино функции map(), но только для float   **/
    float fmap(const float x, const float in_min, const float in_max, const float out_min, const float out_max);
    double dmap(const double x, const double in_min, const double in_max, const double out_min, const double out_max);


    // чуть менее точная, зато в 3 раза быстрее
    float sqrt(float x);
    float tan2pi_fast(float x);
    float atan2_fast(float y, float x);
    float atan_fast(float x);


    /*      UNUSED or obsolete      */
    // функция возвращает true, если float ~= целое (первая цифра после запятой == 0)
    //bool isInteger(float val);
    //  some gamma curve, not sure why need it if brightness control is done with dim8() already
    //  void gammaCorrection();
    /* Функция возврашает "вес" яркости пикселя от 0 (черный) до 765 (белый). Может использоваться для проверки не "пустое ли место"
    для этого есть FastLED CRGB::getLuma
    uint16_t RGBweight (CRGB *leds, uint16_t idx);  */

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

    Boid(float x, float y);

    static float randomf() {
      return mapfloat(random(0, 255), 0, 255, -.5, .5);
    }

    static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void run(std::vector<Boid> boids) {
      flock(boids);
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
    void flock(std::vector<Boid> &boids);

    // Separation
    // Method checks for nearby boids and steers away
    PVector separate(std::vector<Boid> &boids);

    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    PVector align(std::vector<Boid> &boids);

    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    PVector cohesion(std::vector<Boid> &boids);

    // A method that calculates and applies a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    PVector seek(PVector target);

    // A method that calculates a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    void arrive(PVector target);

    void wrapAroundBorders(uint16_t w, uint16_t h);

    void avoidBorders(uint16_t w, uint16_t h);

    bool bounceOffBorders(float bounce, uint16_t w, uint16_t h);

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

    /**
     * @brief spawn Boids at random location
     * 
     * @param boids 
     */
    static void spawn(std::vector<Boid> &boids, uint16_t w, uint16_t h);
};

/**
 * @brief a 1D vector mapped to 2D array
 * 
 * @tparam T template type
 */
template <class T>
class Vector2D {
    size_t _w, _h;
    std::vector<T> v;

public:
    Vector2D(size_t w, size_t h, T const &val=T()) : _w(w), _h(h), v(std::vector<T>(w*h, val)) {}

    size_t idx(size_t x, size_t y) const { return _w*y + x; }
    size_t w() const { return _w; };
    size_t h() const { return _h; };
    size_t size() const { return v.size(); };

    T& at(size_t idx){ return v.at( idx ); };
    T& at(size_t x, size_t y){ return v.at( idx(x,y) ); };
    T* getData(){ return v.data(); }

    void reset(size_t width, size_t height, T const &val = T()){
        _w = width, _h = height;
        v.assign(_w*_h, val);
        v.shrink_to_fit();
    };
};

// 3D Noise map structure
struct Noise3dMap {
    struct Deviation
    {
        uint32_t e_x, e_y, e_z;
        uint32_t e_scaleX, e_scaleY;
    };
    
    const uint8_t w, h;
    const uint8_t e_centerX = w / 2 + !!(w % 2);
    const uint8_t e_centerY = h / 2 + !!(h % 2);
    std::vector<Deviation> opt;
    std::vector< Vector2D<uint8_t> > map;

    Noise3dMap(uint8_t layers, uint8_t w, uint8_t h) : w(w), h(h),
                opt(std::vector<Deviation>(layers)),
                map(std::vector< Vector2D<uint8_t> >(layers, Vector2D<uint8_t>(w,h))) {}
    // turn x,y into array index
    inline size_t idx(uint8_t x, uint8_t y) const { return w*y + x; }

    // return a reference to map element via layer,x,y coordinates
    uint8_t &lxy(uint8_t layer, uint8_t x, uint8_t y){ return map.at(layer).at(x, y); }

    /**
     * @brief fill noise map
     * 
     * @param smooth if > 0 apply smooth 
     */
    void fillNoise(uint8_t smooth = 0);
};
