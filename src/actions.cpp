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

void run_action(ra action){
  StaticJsonDocument<ACTION_PARAM_SIZE> jdoc;
  JsonObject obj = jdoc.to<JsonObject>();
  run_action(action, &obj);
}

void run_action(ra action, JsonObject *data){
  switch (action){
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

      (*data)[FPSTR(TCONST_effListMain)] = myLamp.effects.getEffnum();   // call switch effect as in GUI/main page
      break;
    }

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
      if ( action == ra::mp3_prev) offset *= -1;
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
      embui.post(*data, true);
      return;
    }

    // send text to lamp
    case ra::sendtext : {
      if (!data || !(*data)[TCONST_value]) return;
      String tmpStr( embui.param(FPSTR(TCONST_txtColor)) );
      tmpStr.replace(F("#"),F("0x"));
      CRGB::HTMLColorCode color = (CRGB::HTMLColorCode)strtol(tmpStr.c_str(), NULL, 0);
      myLamp.sendString((*data)[TCONST_value], color);
      return;
    }

    default:
      return;
  }

  embui.post(*data, true);                    // inject packet back into EmbUI action selector
}