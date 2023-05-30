/*
    This file is part of FireLamp EmbUI project
    https://github.com/vortigont/FireLamp_JeeUI

    a fork of https://github.com/DmytroKorniienko/FireLamp_EmbUI

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
*/

#include "filehelpers.hpp"
#include "StreamUtils.h"
#include "char_const.h"

namespace fshlpr{

    const String getEffectCfgPath(const uint16_t nb, const char *folder) {
        uint16_t swapnb = nb>>8|nb<<8; // меняю местами 2 байта, так чтобы копии/верисии эффекта оказалась в имени файла позади
        // todo: check if supplied alternative path starts with '/'
        String filename(folder ? folder : "/eff/");
        char buffer[10];
        sprintf_P(buffer,PSTR("%04x.json"), swapnb);
        filename.concat(buffer);
        return filename;
    }

    bool getfseffconfig(uint16_t nb, String &result){
        String filename = fshlpr::getEffectCfgPath(nb);
        File jfile = LittleFS.open(filename, "r");
        if(jfile){
            result = jfile.readString();
            jfile.close();
            return true;
        }
        return false;
    }

    /**
     * процедура открывает индекс-файл на запись в переданный хендл,
     * возвращает хендл
     */
    File& openIndexFile(File& fhandle, const char *folder){
        String filename;
        if (folder && folder[0]){ // если указан каталог и первый символ не пустой, то берем как есть
            filename.concat(folder);
        } else
            filename.concat(FPSTR(TCONST_eff_index));
        
        fhandle = LittleFS.open(filename, "w");
        return fhandle;
    }

}