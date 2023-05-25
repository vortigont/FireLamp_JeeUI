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
      embui.post(*data, true);                                      // inject "demo" packet
      return;
    }

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
}