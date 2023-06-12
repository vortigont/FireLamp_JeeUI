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

#include "actions.hpp"
#include "lamp.h"

void run_action(ra act){
  // here we catch some really simple action that do not need reflecting any data to other components
  switch (act){
    case ra::reboot : {
      // make warning signaling
      StaticJsonDocument<128> warn;
      deserializeJson(warn, F("{\"event\":[\"#ec21ee\",3000,500,true,\"Reboot...\"]}"));
      JsonObject j = warn.as<JsonObject>();
      run_action(ra::warn, &j);
      Task *t = new Task(5 * TASK_SECOND, TASK_ONCE, nullptr, &ts, false, nullptr, [](){ ESP.restart(); });
      t->enableDelayed();
      return;
    }
    default:;
  }
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  run_action(act, &obj);
}

void run_action(ra act, JsonObject *data){
  LOG(printf_P, PSTR("run_action: %d\n"), static_cast<int>(act));
  switch (act){
    // AUX PIN On/Off
    case ra::aux : {
      (*data)[FPSTR(TCONST_AUX)] = (*data)[FPSTR(TCONST_value)];   // change key name
      (*data).remove(FPSTR(TCONST_value));
      break;
    }

    // AUX PIN flip
    case ra::aux_flip : {
      if ( embui.paramVariant(FPSTR(TCONST_aux_gpio)) == -1) return;    // if AUX pin is not set, than quit
      (*data)[FPSTR(TCONST_AUX)] = !digitalRead(embui.paramVariant(FPSTR(TCONST_aux_gpio)));  // we simply flip the state here
      break;
    }

    // demo mode On/Off
    case ra::demo : {
      (*data)[FPSTR(TCONST_Demo)] = (*data)[FPSTR(TCONST_value)];   // change key name
      break;
    }

    // demo next effect
    case ra::demo_next : {
      if (myLamp.getLampSettings().dRand)
          myLamp.switcheffect(SW_RND, myLamp.getFaderFlag());
      else myLamp.switcheffect(SW_NEXT_DEMO, myLamp.getFaderFlag());

      (*data)[FPSTR(TCONST_eff_run)] = myLamp.effects.getEffnum();   // call switch effect as in GUI/main page
      break;
    }

    // switch to next effect
    case ra::eff_next : {
      // pick next available effect (considering it is enabled)
      (*data)[FPSTR(TCONST_eff_run)] = myLamp.effects.getNext();
      break;
    }

    // switch to previous effect
    case ra::eff_prev : {
      // pick previous available effect (considering it is enabled)
      (*data)[FPSTR(TCONST_eff_run)] = myLamp.effects.getPrev();
      break;
    }

    // switch to previous effect
    case ra::eff_rnd : {
      // pick previous available effect (considering it is enabled)
      (*data)[FPSTR(TCONST_eff_run)] = myLamp.effects.getByCnt(random(0, myLamp.effects.getEffectsListSize()));
      break;
    }

    // switch effect to specific number
    case ra::eff_switch : {
      (*data)[FPSTR(TCONST_eff_run)] = (*data)[FPSTR(TCONST_value)];        // change key name and inject data to EmbUI action selector
      break;
    }
#ifdef MIC_EFFECTS
    // simple actions with provided key:value
    case ra::miconoff : {
      (*data)[FPSTR(TCONST_Mic)] = (*data)[FPSTR(TCONST_value)];   // change key name
      break;
    }
#endif  //#ifdef MIC_EFFECTS
#ifdef MP3PLAYER
    //MP3: play specific track
    case ra::mp3_eff : {
      if(!myLamp.isONMP3()) return;
      mp3->playEffect((*data)[FPSTR(TCONST_value)], "");
      return; // no need to execute any UI action
    }
    //MP3: enable/disable
    case ra::mp3_enable : {
      if(!myLamp.isONMP3()) return;
      (*data)[FPSTR(TCONST_isOnMP3)] = (*data)[FPSTR(TCONST_value)];   // change key name
      break;
    }
    //MP3: play previous/next track?
    case ra::mp3_next :
    case ra::mp3_prev : {
      if(!myLamp.isONMP3()) return;
      int offset = (*data)[FPSTR(TCONST_value)];
      if ( act == ra::mp3_prev) offset *= -1;
      mp3->playEffect(mp3->getCurPlayingNb() + offset, "");
      return; // no need to execute any UI action
    }
    //MP3: set volume
    case ra::mp3_vol : {
      if(!myLamp.isONMP3()) return;
      (*data)[FPSTR(TCONST_mp3volume)] = (*data)[FPSTR(TCONST_value)];   // change key name
      break;
    }
#endif  //#ifdef MP3PLAYER

    // turn lamp ON
    case ra::on : {
      (*data)[FPSTR(TCONST_ONflag)] = true;
      break;
    }

    // turn lamp OFF
    case ra::off : {
      myLamp.stopRGB(); // выключение RGB-режима
      (*data)[FPSTR(TCONST_ONflag)] = false;
      break;
    }

    // send text to lamp
    case ra::sendtext : {
      if (!(*data)[TCONST_value]) return;
      String tmpStr( embui.param(FPSTR(TCONST_txtColor)) );
      tmpStr.replace(F("#"),F("0x"));
      CRGB::HTMLColorCode color = (CRGB::HTMLColorCode)strtol(tmpStr.c_str(), NULL, 0);
      myLamp.sendString((*data)[TCONST_value], color);
      return;
    }

    // show warning on a lamp
    case ra::warn : {
      if ( !(*data).containsKey(TCONST_event) ) return;   // invalid object

      // here we receive JsonArray with alert params
      JsonArray arr = (*data)[TCONST_event];
      if (arr.size() < 3 ) return;    // some malformed warning config

      // color
      String tmpStr(arr[0].as<const char*>());
      tmpStr.replace(F("#"), F("0x"));
      uint32_t col = strtol(tmpStr.c_str(), NULL, 0);
      myLamp.showWarning(col, // color
             arr[1],          // duration
             arr[2],          // period
             arr[3],          // type
             arr[4],          // overwrite
             arr[5]);         // text message
      break; 
    }
    default:
      return;
  }

  embui.post(*data, true);                    // inject packet back into EmbUI action selector
}

