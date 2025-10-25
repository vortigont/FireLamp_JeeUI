/*
Copyright © 2023 Emil Muratov (vortigont)

    This file is part of FireLamp_JeeUI.

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

    this file holds definitions for functions used to set and configure various periferal devices

*/

#pragma once
#include "ui.h"

/**
 * @brief Read configuration and setup TM1637 Display if required
 * 
 */
void tm1637_setup();

/**
 * @brief (re)configure TM1637 with supplied config object
 * 
 * @param tm JsonObject with configuration params
 */
void tm1637_configure(JsonVariantConst cfg);

// ========== Button

void button_cfg_load();

void button_configure_gpio(JsonVariantConst cfg);

void button_configure_events(JsonVariantConst cfg);

void getset_button_gpio(Interface *interf, const JsonVariantConst data, const char* action  = NULL);

// get/set button lock
void getset_btn_lock(Interface *interf, const JsonVariantConst data, const char* action  = NULL);

// ========== Encoder

// get/set encoder's configuration to/from WebUI
void getset_encoder_gpio(Interface *interf, const JsonVariantConst data, const char* action  = NULL);

// *** DFPlayer

// Load DFPlayer configuration from JSON on flash
void dfplayer_cfg_load();

/**
 * @brief configure hw options for DFPlayer
 * it could be executes either
 * 
 * @param cfg 
 */
void dfplayer_setup_device(JsonVariantConst cfg);

/**
 * @brief configure playback options for player
 * 
 * @param cfg 
 */
void dfplayer_setup_opt(JsonVariantConst cfg);

void getset_dfplayer_device(Interface *interf, const JsonVariantConst data, const char* action  = NULL);

void getset_dfplayer_opt(Interface *interf, const JsonVariantConst data, const char* action  = NULL);
