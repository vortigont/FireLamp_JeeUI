/*
    ESP32 Light Manager library

This code implements a library for ESP32-xx family chips and provides an
API for controling Lighting applications, mostly (but not limited to) LEDs,
LED strips, PWM drivers, RGB LED strips etc...


Copyright (C) Emil Muratov, 2022
GitHub: https://github.com/vortigont/ESP32-LightManager

 *  This program or library is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General
 *  Public License along with this library; if not, get one at
 *  https://opensource.org/licenses/LGPL-3.0
*/


/*
The curves below created basing on the articles and code examples from
 * PWM Exponential LED Fading on Arduino
https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms

 * Light Projects - A collection LED control examples for Arduino 
https://tigoe.github.io/LightProjects/fading.html


*** Some more references:

 * a small lib for generating Cie1931 tables
https://github.com/nitz/Cie1931 
https://gist.github.com/mathiasvr/19ce1d7b6caeab230934080ae1f1380e

 * Formula to determine perceived brightness of RGB color
https://stackoverflow.com/questions/596216/formula-to-determine-perceived-brightness-of-rgb-color

 * Scaling brightness using PWM - difference threshold approach
https://www.eevblog.com/forum/programming/scaling-brightness-using-pwm/

 * Psychometric Lightness and Gamma - CIE 1931
https://www.photonstophotos.net/GeneralTopics/Exposure/Psychometric_Lightness_and_Gamma.htm
*/

#pragma once
#include <cmath>


// returned value won't round floats but just truncates fractional part
// assume we are OK with such precision
namespace luma {

enum class curve:uint8_t { binary, linear, cie1931, exponent, sine, square };

/**
 * @brief map luma value scaled by specified curve
 * 
 * @param c - curve to apply to luma scaling
 * @param luma - unscaled luma value
 * @param max_duty - dynamic range for the curve, i.e. 256 for 8 bit resolution, etc...
 * @param max_luma - max luma scale, i.e. 100, means 0-100 scale (same as percents)
 * @return uint32_t - scaled luma value
 */
uint32_t curveMap(curve c, uint32_t luma, uint32_t max_duty, uint32_t max_luma = 100);
uint32_t curveUnMap(curve c, uint32_t duty, uint32_t max_duty, uint32_t max_luma = 100);

uint32_t map_linear(uint32_t l, uint32_t max_duty, uint32_t max_l = 100);
uint32_t unmap_linear(uint32_t duty, uint32_t max_duty, uint32_t max_l = 100);

/**
 * @brief CIE1931 curve scaling
 * 
 * @param l - perceived lightness value L* between 0 and 100
 * @param max_value - max PWM value, i.e. (1<<10)-1 for 10 bit pwm
 * @return uint32_t mapped pwm value
 */
uint32_t map_cie1931(uint32_t l, uint32_t max_duty, uint32_t max_l = 100);

uint32_t unmap_cie1931(uint32_t duty, uint32_t max_duty, uint32_t max_l = 100);

uint32_t map_exponent(uint32_t l, uint32_t max_duty, uint32_t max_l = 100);

uint32_t unmap_exponent(uint32_t duty, uint32_t max_duty, uint32_t max_l = 100);

uint32_t map_sine(uint32_t l, uint32_t max_duty, uint32_t max_l = 100);

uint32_t unmap_sine(uint32_t duty, uint32_t max_duty, uint32_t max_l = 100);

uint32_t map_square(uint32_t l, uint32_t max_duty, uint32_t max_l = 100);

uint32_t unmap_square(uint32_t duty, uint32_t max_duty, uint32_t max_l = 100);

inline uint32_t map_binary(uint32_t l, uint32_t max_duty, uint32_t max_l = 1){ return (l*2 >= max_l); };
inline uint32_t unmap_binary(uint32_t duty, uint32_t max_duty, uint32_t max_l = 1){ return (max_duty*(bool)duty); };

}   // end of namespace luma