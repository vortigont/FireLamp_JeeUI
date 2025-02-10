/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (vortigont)

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

#pragma once
#include "modules/mod_manager.hpp"

class HTTP_API_Poller : public Task {
  uint32_t _refresh; // ms
  bool _retry{false};

protected:
  virtual void http_fetch() = 0;


};

/**
 * @brief OpenWeather source
 * obtains weather forcast via API and pulishes it to the text message queue
 * 
 */
class ModWeatherSource : public GenericModuleProfiles, public Task {

struct WeatherCfg {
  String apikey;
  uint32_t city_id, refresh; // ms
  bool retry{false};
};

  WeatherCfg _weathercfg;

  // msg unique id
  uint32_t _msg_id;
  // scroller destination
  uint8_t _scroller_id;
  // how many times to repeat message (-1 - forever)
  int32_t _repeat_cnt;
  // interval between message displays in seconds
  int32_t _repeat_interval;

  // formatting
  bool _show_sunrise;

  // update weather
  void _getOpenWeather();

  //void _send_msg();

public:
  ModWeatherSource();
  ~ModWeatherSource();

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override;

  // load class configuration into JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void start() override { enable(); };
  void stop() override { disable(); };
};


class ModNarodMonSource : public GenericModule, public Task {

  struct NarodMonAPICfg {
    String apikey, sensorid, uuid, lang{"en"};
    uint32_t refresh; // ms
  };

  NarodMonAPICfg _sourceCfg;

  bool _retry{false};


  // String Scroller params
  // msg unique id
  uint32_t _msg_id;
  // scroller destination
  uint8_t _scroller_id;
  // how many times to repeat message (-1 - forever)
  int32_t _repeat_cnt;
  // interval between message displays in seconds
  int32_t _repeat_interval;

public:
  ModNarodMonSource();
  ~ModNarodMonSource();

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override;

  // load class configuration from JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void start() override { enable(); };
  void stop() override { disable(); };

  // update data from a remote source
  void getData();

};
