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
#include "EmbUI.h"
#include "char_const.h"
#include "main.h"

#define ACTION_PARAM_SIZE   256     // Static json document size to pass the parameters to actions


/**
 * @brief enumeration of the availbale named actions
 * 
 */
enum class ra:uint8_t {
  noop = 0,            // switch lamp Off
//  on,                 // switch lamp On
  aux,                // aux pin control
  aux_flip,           // flip AUX pin
  brt,                // brightness control, param int
  brt_nofade,         // change brightness without fading effect, param int
  brt_lcurve,         // set brightness luma curve
  demo,               // demo mode on/off, param bool
  demo_next,          // switch effect in demo mode, param void
  eff_ctrl,           // apply effect control value
  eff_next,           // switch to next effect
  eff_prev,           // switch to previous effect
  eff_rnd,            // switch to random effect
  eff_switch,         // switch effect to specific number
  miconoff,           // Mike: On/Off
  mp3_enable,         // MP3: enable/disable mp3 sounds
  mp3_next,           // MP3: play next track?
  mp3_prev,           // MP3: play previous track?
  mp3_vol,            // MP3: set volume
  reboot,             // reboot MCU
//  sendtext,           // send text to lamp
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
 * here we catch some really simple action that do not need reflecting any data to other components
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
void run_action(ra act, const T& param);

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
void run_action(const String &key, const T& val);



/**     IMPLEMENTATIONS FOLLOW        **/

template<typename T>
void run_action(ra act, const T& param) {
  LOG(printf_P, PSTR("run_action_p: %d\n"), static_cast<int>(act));
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  JsonObject data = obj.createNestedObject(P_data);

  // action specific key:value setup
  switch (act){
    // AUX PIN On/Off
    case ra::aux : {
      obj[P_action] = TCONST_AUX;
      data[TCONST_AUX] = param;
      break;
    }

    case ra::brt_lcurve : {
      obj[P_action] = A_dev_lcurve;
      data[A_dev_lcurve] = param;
      break;
    }

    // brightness control
    case ra::brt :
    case ra::brt_nofade : {
      obj[P_action] = A_dev_brightness;
      data[A_dev_brightness] = param;
      if (act == ra::brt_nofade) data[TCONST_nofade] = true;     // disable fader
      break;
    }

    // switch to specified effect number
    case ra::eff_switch : {
      obj[P_action] = A_effect_switch_idx;
      data[A_effect_switch_idx] = param;
      break;
    }

    // demo mode On/Off
    case ra::demo : {
      obj[P_action] = K_demo;
      data[K_demo] = param;
      break;
    }

#ifdef MIC_EFFECTS
    // simple actions with provided key:value
    case ra::miconoff : {
      obj[P_action] = TCONST_Mic;
      data[TCONST_Mic] = param;
      break;
    }
#endif  //#ifdef MIC_EFFECTS

#ifdef MP3PLAYER
    //MP3: enable/disable
    case ra::mp3_enable : {
      obj[P_action] = TCONST_Mic;
      data[TCONST_Mic] = param;
      break;
    }
    //MP3: set volume
    case ra::mp3_vol : {
      obj[P_action] = TCONST_mp3volume;
      data[TCONST_mp3volume] = param;
      break;
    }

    //MP3: play previous/next track?
    case ra::mp3_next :
    case ra::mp3_prev : {
      //if(!myLamp.isONMP3()) return;
      int offset = param;
      if ( act == ra::mp3_prev) offset *= -1;
      mp3->playEffect(mp3->getCurPlayingNb() + offset, "");
      return; // no need to execute any UI action
    }
#endif  //#ifdef MP3PLAYER


    // all the rest we just assign "action": 'value' pair
    default:
      obj[P_action] = param;
  }

  embui.post(obj, true);                    // inject packet back into EmbUI action selector
  //run_action(act, &obj);
}

template<typename T>
void run_action(const String &key, const T& val) {
  LOG(printf_P, PSTR("run_action_kv: %s\n"), key.c_str());
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  obj[P_action] = key;
  JsonObject data = obj.createNestedObject(P_data);
  data[key] = val;

  embui.post(obj, true);                    // inject packet back into EmbUI action selector
}


