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

#pragma once

#include "misc.h"
//#include "LList.h"
#include <ArduinoJson.h>
//#include "effects_types.h"
#include <LittleFS.h>


/**
 * @brief A namespace for various functions to help working with files on LittleFS system
 * 
 */
namespace fshlpr{

    /**
     *  метод загружает и пробует десериализовать джейсон из файла в предоставленный документ,
     *  возвращает true если загрузка и десериализация прошла успешно
     *  @param doc - DynamicJsonDocument куда будет загружен джейсон
     *  @param jsonfile - файл, для загрузки
     */
    bool deserializeFile(DynamicJsonDocument& doc, const char* filepath);

    /**
     * @brief получение пути и имени файла конфига эффекта по его номеру
     * 
     * @param nb - номер эффекта
     * @param folder -  абсолютный путь к каталогу с конфигами, должен начинаться и заканчиваться '/', по-умолчанию испльзуется '/eff/'
     * @return const String - полный путь до файла с конфигом
     */
    const String getEffectCfgPath(uint16_t nb, const char *folder=NULL);

    /**
     * @brief read effect's json config file into string
     * 
     * @param nb - effect enum
     * @param result - string ref where to read file
     * @return true - on success
     * @return false - on error
     */
    bool getfseffconfig(uint16_t nb, String &result);

    /**
     * процедура открывает индекс-файл на запись в переданный хендл,
     * возвращает хендл
     */
    File& openIndexFile(File& fhandle, const char *folder);



}