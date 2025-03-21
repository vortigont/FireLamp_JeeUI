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

#include "rtc_clock.hpp"
#include "timeProcessor.h"
#include <chrono>
#include "log.h"

// RAM:   [=         ]  14.0% (used 45876 bytes from 327680 bytes)                                                                                               
// Flash: [======    ]  63.8% (used 1338884 bytes from 2097152 bytes) 

// w chrono
//RAM:   [=         ]  14.0% (used 45876 bytes from 327680 bytes)                                                                                               
//Flash: [======    ]  63.9% (used 1339264 bytes from 2097152 bytes)      

// https://stackoverflow.com/a/75714048
static std::time_t timegm(std::tm const& t){
  using namespace std::chrono;
  return system_clock::to_time_t(
      sys_days{year{t.tm_year+1900}/(t.tm_mon+1)/t.tm_mday} +
      hours{t.tm_hour} + minutes{t.tm_min} + seconds{t.tm_sec});
}

RTC_Clock::RTC_Clock(int32_t id) : GenericSensor(id){
  poll_rate = 3600;
  TimeProcessor::getInstance().attach_callback([this](){ _set_rtc_to_utc(); });
}

RTC_Clock::~RTC_Clock(){
  TimeProcessor::getInstance().attach_callback(nullptr);
}

void RTC_Clock::load_cfg(JsonVariantConst cfg){
  _chip = static_cast<rtc_chip_t>( cfg[T_model].as<uint32_t>() );

  switch (_chip){
    case rtc_chip_t::bm8563 :
      _rtc = std::make_unique<RTC_8563>();
      break;
    case rtc_chip_t::ds32xx :
      _rtc = std::make_unique<RTC_8563>();
      break;
    default:;
  }

}

bool RTC_Clock::init(){
  online =_rtc->init();

  if (online){
    _init_from_utc_rtc();
  }
  return online;
}

void RTC_Clock::_init_from_utc_rtc(){
  char buff[std::size("yyyy-mm-ddThh:mm:ss")];
  std::time_t t = std::time({});
  std::strftime(std::data(buff), std::size(buff), "%FT%T", std::localtime(&t));
  LOGD(T_clock, printf, "Sys clock:%s\n", buff);
  tm rtc_tm;
  _rtc->get_tm(&rtc_tm);
  // RTC clock is in UTC, as it is not capable to keep timezone rules and DST changes
  std::strftime(std::data(buff), std::size(buff), "%FT%T", &rtc_tm); //std::gmtime(&rtc_clk));
  LOGD(T_clock, printf, "RTC UTC clock:%s\n", buff);
    // if sys time >5 sec behind RTC time - reset system time to RTC's
  if ( difftime(timegm(rtc_tm), t) > 5){
    // convert RTC's UTC to localtime
    std::time_t rtc_time = timegm(rtc_tm);
    timeval tv{rtc_time, 0};
    settimeofday(&tv, NULL);

    std::strftime(std::data(buff), std::size(buff), "%FT%T", std::localtime(&rtc_time));
    LOGI(T_clock, printf, "Sync local clock to RTC:%s\n", buff);
  }
}

void RTC_Clock::_set_rtc_to_utc(){
  if (!online) return;

  std::time_t t = std::time({});
  // update RTC to UTC time
  _rtc->set(std::gmtime(&t));

  char buff[std::size("yyyy-mm-ddThh:mm:ss")];
  std::strftime(std::data(buff), std::size(buff), "%FT%T",   std::gmtime(&t));
  LOGI(T_clock, printf, "Update RTC to UTC:%s\n", buff);

  tm rtc_tm;
  _rtc->get_tm(&rtc_tm);

  //std::strftime(std::data(buff), std::size(buff), "%FT%T",   &rtc_tm );
  //LOGI(T_clock, printf, "GMTime in RTC is:%s\n", buff);
}


void RTC_DS32::get_tm(tm* time) {
  _rtc.refresh();
  time->tm_year = _rtc.year();
  time->tm_mon = _rtc.month();
  time->tm_mday = _rtc.day();
  time->tm_wday = _rtc.dayOfWeek();
  time->tm_hour = _rtc.hour();
  time->tm_min = _rtc.minute();
  time->tm_sec = _rtc.second();
  time->tm_isdst = -1;
}

void RTC_DS32::set(tm* time){
  _rtc.set(time->tm_sec, time->tm_min, time->tm_hour, time->tm_wday, time->tm_mday, time->tm_mon, time->tm_year);
}
