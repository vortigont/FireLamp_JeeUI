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

#include "luma_curves.hpp"

constexpr float CIE1931_Y{8.856};        // 216/24389
constexpr double PI{3.1415926535897932384626433832795};
constexpr double HALF_PI{1.5707963267948966192313216916398};
constexpr double TWO_PI{6.283185307179586476925286766559};
//#define DEG_TO_RAD 0.017453292519943295769236907684886
//#define RAD_TO_DEG 57.295779513082320876798154814105
//#define EULER 2.718281828459045235360287471352

namespace luma {

uint32_t curveMap(curve c, uint32_t luma, uint32_t max_duty, uint32_t max_luma){
    switch(c){
    case curve::binary:
        return map_binary(luma, max_duty, max_luma);
    case curve::cie1931:
        return map_cie1931(luma, max_duty, max_luma);
    case curve::exponent:
        return map_exponent(luma, max_duty, max_luma);
    case curve::sine:
        return map_sine(luma, max_duty, max_luma);
    case curve::square:
        return map_square(luma, max_duty, max_luma);
    default:
        return map_linear(luma, max_duty, max_luma);
    }
}

uint32_t curveUnMap(curve c, uint32_t duty, uint32_t max_duty, uint32_t max_luma){
    switch(c){
    case curve::binary:
        return unmap_binary(duty, max_duty, max_luma);
    case curve::cie1931:
        return unmap_cie1931(duty, max_duty, max_luma);
    case curve::exponent:
        return unmap_exponent(duty, max_duty, max_luma);
    case curve::sine:
        return unmap_sine(duty, max_duty, max_luma);
    case curve::square:
        return unmap_square(duty, max_duty, max_luma);
    default:
        return unmap_linear(duty, max_duty, max_luma);
    }
}

uint32_t map_linear(uint32_t l, uint32_t max_duty, uint32_t max_l){
    if (!l)
        return 0;

    if (l >= max_l)
        return max_duty;

    return max_duty * l / max_l;
    //return round(max_duty * l / max_l);
}

uint32_t unmap_linear(uint32_t duty, uint32_t max_duty, uint32_t max_l){
    if (duty >= max_duty)
        return max_l;

    return round(duty * max_l / max_duty);
}

uint32_t map_cie1931(uint32_t l, uint32_t max_duty, uint32_t max_l){
    if (l >= max_l)
        return max_duty;

    if (!l)
        return 0;

    float l_scaled = float(l) / max_l * 100;
    if (l_scaled > 8.0)
        return pow((l_scaled + 16.0)/116.0, 3) * max_duty;
    else
        return l_scaled / 902.3 * max_duty;
}

uint32_t unmap_cie1931(uint32_t duty, uint32_t max_duty, uint32_t max_l){
    if (duty >= max_duty)
        return max_l;

    if (!duty)
        return 0;

    float x = float(duty) / max_duty;

    if (x * 902.3 <= 8.0){
        return round(x * 902.3 * max_l / 100);
    }
    else {
        return round((cbrtf(x) * 116.0 - 16.0) * max_l / 100);
    }
}

uint32_t map_exponent(uint32_t l, uint32_t max_duty, uint32_t max_l){
    if (l >= max_l)
        return max_duty;

    if (!l)
        return 0;

    float scalingFactor = float(max_l) * log10(2) / log10(max_duty);
    return (pow(2, (l / scalingFactor)) - 1);
}

uint32_t unmap_exponent(uint32_t duty, uint32_t max_duty, uint32_t max_l){
    if (duty >= max_duty)
        return max_l;

    if (!duty)
        return 0;

    float scalingFactor = float(max_l) * log10(2) / log10(max_duty);
    return round(scalingFactor * log2(duty + 1));
}

uint32_t map_sine(uint32_t l, uint32_t max_duty, uint32_t max_l){
    if (l >= max_l)
        return max_duty;

    if (!l)
        return 0;

    float duty = l * max_duty / float(max_l);
    return (sinf((duty * PI / max_duty) - HALF_PI) + 1) * max_duty/2;
}

uint32_t unmap_sine(uint32_t duty, uint32_t max_duty, uint32_t max_l){
    if (duty >= max_duty)
        return max_l;

    if (!duty)
        return 0;

    float uduty = (asinf(float(duty) * 2 / max_duty - 1) + HALF_PI) * max_duty / PI;
    return round(uduty * max_l / max_duty);
}

uint32_t map_square(uint32_t l, uint32_t max_duty, uint32_t max_l){
    if (l >= max_l)
        return max_duty;

    if (!l)
        return 0;

    uint32_t duty = l * max_duty / max_l;
    return duty*duty  / max_duty;
}

uint32_t unmap_square(uint32_t duty, uint32_t max_duty, uint32_t max_l){
    if (duty >= max_duty)
        return max_l;

    if (!duty)
        return 0;

    return round(sqrt( duty * max_duty ) * max_l / max_duty);
}

} // namespace luma
