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
#include "sensors.hpp"
#include "bm8563.hpp"
#include "uRTCLib.h"

// sensors types enum
enum class rtc_chip_t{
  na,           // not available
  bm8563,
  ds32xx
};

class GenericRTC {

public:
  GenericRTC(){};
  virtual bool init() = 0;
  virtual void get_tm(tm* time) = 0;
  virtual void set(tm* time) = 0;

};

class RTC_8563 : public GenericRTC {
  arduino::bm8563 _rtc;

public:
  RTC_8563(){};
  bool init() override { return _rtc.initialize(); };
  void get_tm(tm* time) override { return _rtc.now(time); };
  void set(tm* time) override { _rtc.set(*time); };
};


class RTC_DS32 : public GenericRTC {
  uRTCLib _rtc;
public:
  RTC_DS32(){};
  bool init() override { return _rtc.refresh(); };
  void get_tm(tm* time) override;
  void set(tm* time) override;
};



class RTC_Clock : public GenericSensor {
  rtc_chip_t _chip;
  std::unique_ptr <GenericRTC> _rtc;

public:
  RTC_Clock(int32_t id);
  ~RTC_Clock();

	void load_cfg(JsonVariantConst cfg) override;

  bool init() override;

  void poll() override {};

private:
  void _set_time_cb();
  void _init_from_utc_rtc();
  void _set_rtc_to_utc();
};