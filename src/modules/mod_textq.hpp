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
#include "mod_manager.hpp"
//#include <string>
//#include <ctime>


struct TextMessage {
    std::string msg;
    int32_t cnt;
    time_t interval;
    time_t last_displayed;
};

class ModTextScroller : public GenericModuleProfiles, public Task {
  TextBitMapCfg _bitmapcfg;

  std::unique_ptr<Arduino_Canvas_Mono> _textmask;

  int _cur_offset{0};
  int _scrollrate;
  uint32_t _last_redraw;
  uint16_t _txt_pixlen;

  overlay_cb_t _renderer;

  std::string _txtstr{"обновление погоды..."};

  static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override;

  // load class configuration into JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  // hook to check/update text sroller
  void _scroll_line(LedFB_GFX *gfx);

public:
  ModTextScroller();
  ~ModTextScroller();

  void moduleRunner();

  void start() override;
  void stop() override;
};

/*
class TextScrollerWgdt : public GenericModuleProfiles, public Task {

struct WeatherCfg {
  String apikey;
  uint32_t city_id, refresh; // ms
  bool retry{false};
};

  TextBitMapCfg _bitmapcfg;
  WeatherCfg _weathercfg;

  std::unique_ptr<Arduino_Canvas_Mono> _textmask;

  int _cur_offset{0};
  int _scrollrate;
  uint32_t _last_redraw;
  uint16_t _txt_pixlen;
  bool _wupd{false};

  overlay_cb_t _renderer;

  std::string _txtstr{"обновление погоды..."};

  static void _event_hndlr(void* handler, esp_event_base_t base, int32_t id, void* event_data);

  // pack class configuration into JsonObject
  void generate_cfg(JsonVariant cfg) const override;

  // load class configuration into JsonObject
  void load_cfg(JsonVariantConst cfg) override;

  void _getOpenWeather();

  // hook to check/update text sroller
  void _scroll_line(LedFB_GFX *gfx);

public:
  TextScrollerWgdt();
  ~TextScrollerWgdt();

  void moduleRunner();

  void start() override;
  void stop() override;
};

*/