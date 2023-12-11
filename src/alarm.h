/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

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

    Будильник-рассвет


*/

#pragma once
#include "lamp.h"

class ALARMTASK : public Task {
private:
    CHSV dawnColorMinus[6];                                            // цвет "рассвета"
    uint8_t dawnCounter = 0;                                           // счётчик первых шагов будильника
    time_t startmillis;

    typedef struct {
        uint8_t alarmP;
        uint8_t alarmT;
        String msg;
        bool isStartSnd;
        bool isLimitVol;
        ALARM_SOUND_TYPE type;

        void clear() { alarmP = 5; alarmT = 5; msg=""; isStartSnd = true; isLimitVol = true; type = ALARM_SOUND_TYPE::AT_RANDOM; }
    } ALARM_DATA;
    
    ALARM_DATA curAlarm;
    Lamp *lamp;             // куда же без лампы, блин 8-0
    static ALARMTASK *alarmTask;
    ALARMTASK() = delete;

    void initAlarm(const char *value = nullptr);

public:
    ALARMTASK(Scheduler* aS, Lamp *_l, const char *value = nullptr)
        : Task(TASK_SECOND, TASK_FOREVER, [](){ ALARMTASK::alarmWorker(); }, aS, false, nullptr,[](){ alarmTask = nullptr;}, true)
    {
        lamp = _l;
        alarmTask = this;
        initAlarm(value);
        enableDelayed();
    }

    static inline ALARMTASK *getInstance() {return alarmTask;}
    static void startAlarm(Lamp *_lamp, const char *value = nullptr);

    static void stopAlarm();

    // обработчик будильника "рассвет"
    static void alarmWorker();
};
