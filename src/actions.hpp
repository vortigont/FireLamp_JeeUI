/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023 Emil Muratov (vortigont)
    Copyright © 2020 Dmytro Korniienko (kDn)

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

/*

here we declare a set of actions used to control the Lamp
those could be called either internaly or as a callbacks from WebUI/MQTT/HTTP

*/

#pragma once
#include <ArduinoJson.h>
#include "char_const.h"

#define ACTION_PARAM_SIZE   256     // Static json document size to pass the parameters to actions

/**
 * @brief enumeration of the availbale named actions
 * 
 */
enum class ra:uint8_t {
  off = 0,            // switch lamp Off
  on,                 // switch lamp On
  aux,                // aux pin control
  aux_flip,           // flip AUX pin
  demo,               // demo mode on/off
  demo_next,          // switch effect in demo mode
  eff_next,           // switch to next effect
  eff_prev,           // switch to previous effect
  eff_rnd,            // switch to random effect
  eff_switch,         // switch effect to specific number
  miconoff,           // Mike: On/Off
  mp3_eff,            // MP3: play sound for specific effect
  mp3_enable,         // MP3: enable/disable mp3 sounds
  mp3_next,           // MP3: play next track?
  mp3_prev,           // MP3: play previous track?
  mp3_vol,            // MP3: set volume
  reboot,             // reboot MCU
  sendtext,           // send text to lamp
  warn,               // show warning 
  end                 // not an action actually
};

/**
 * @brief execute action with optional JsonObject data
 * it could be either no parameters,
 * a generic data["value"] param
 * or a set of key:val pairs in *data that could be injected into EmbUI 'post' action lookup method
 * 
 * @param action enum of various actions
 * @param data a ref to json obj
 */
void run_action(ra act, JsonObject *data);

/**
 * @brief a stub for really simple actions with no params
 * NOTE: will pass empty JsonObject to run_action()
 * @param action 
 */
void run_action(ra act);

/**
 * @brief execute action with a single parameter
 * 
 * @tparam T parameter type - any that is supported by JsonDocument container
 * @param action to run
 * @param param parameter is added into JsonDocument and passed to run_action handler
 */
template<typename T>
void run_action(ra act, const T& param) {
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  obj[TCONST_value] = param;
  run_action(act, &obj);
}

/**
 * @brief execute action with 
 * 
 * @tparam T 
 * @param key 
 * @param val 
 * @return true 
 * @return false 
 */
template<typename T>
void run_action(ra act, const String &key, const T& val) {
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  obj[key] = val;
  run_action(act, &obj);
}


