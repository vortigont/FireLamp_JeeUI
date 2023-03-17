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

namespace fshlpr{

    /**
     *  метод загружает и пробует десериализовать джейсон из файла в предоставленный документ,
     *  возвращает true если загрузка и десериализация прошла успешно
     *  @param doc - DynamicJsonDocument куда будет загружен джейсон
     *  @param jsonfile - файл, для загрузки
     */
    bool deserializeFile(DynamicJsonDocument& doc, const char* filepath){
        if (!filepath || !*filepath)
            return false;

        LOG(printf_P, PSTR("Load file: %s\n"), filepath);
        File jfile = LittleFS.open(filepath, "r");
        DeserializationError error;
        if (jfile){
            error = deserializeJson(doc, jfile);
            jfile.close();
        } else {
            LOG(printf_P, PSTR("Can't open File: %s"), filepath);
            return false;
        }

        if (error) {
            LOG(printf_P, PSTR("File: failed to load json file: %s, deserialize error: "), filepath);
            LOG(println, error.code());
            return false;
        }
        return true;
    }

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
        //LOG(print, F("index:"));
        //LOG(println, filename.c_str());
    } else
        filename.concat(F("/eff_index.json"));
    
    fhandle = LittleFS.open(filename, "w");
    return fhandle;
    }


}