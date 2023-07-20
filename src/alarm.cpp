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

#include "alarm.h"
#include "main.h"

void ALARMTASK::initAlarm(const char *value){
    String buf(value);
    buf.replace("'","\"");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc,buf);
    curAlarm.alarmP = doc.containsKey(TCONST_alarmP) ? doc[TCONST_alarmP] : lamp->getAlarmP();
    curAlarm.alarmT = doc.containsKey(TCONST_alarmT) ? doc[TCONST_alarmT] : lamp->getAlarmT();
    curAlarm.msg = doc.containsKey(TCONST_msg) ? doc[TCONST_msg] : String("");
    curAlarm.isLimitVol = doc.containsKey(TCONST_lV) ? doc[TCONST_lV].as<String>()=="1" : lamp->getLampFlagsStuct().limitAlarmVolume;
    curAlarm.isStartSnd = doc.containsKey(TCONST_afS) ? doc[TCONST_afS].as<String>()=="1" : true;
    curAlarm.type = (ALARM_SOUND_TYPE)(doc.containsKey(TCONST_sT) ? doc[TCONST_sT].as<uint8_t>() : lamp->getLampFlagsStuct().alarmSound);

    lamp->setMode(LAMPMODE::MODE_ALARMCLOCK);
    lamp->demoTimer(T_DISABLE);     // гасим Демо-таймер
    #ifdef USE_STREAMING
    if(!lamp->getLampFlagsStuct().isDirect || !lamp->getLampFlagsStuct().isStream)
    #endif
    lamp->effectsTimer(T_ENABLE);
    #ifdef MP3PLAYER
    if(curAlarm.isStartSnd){
        mp3->setAlarm(true);
        mp3->StartAlarmSoundAtVol(curAlarm.type, 1); // запуск звука будильника c минимальной громкости
    } else {
        mp3->setAlarm(false); // здесь будет стоп музыки
    }
    #endif

    // gpio that controls FET pin
    if (lamp->fet_gpio > static_cast<int>(GPIO_NUM_NC)){
        digitalWrite(lamp->fet_gpio, lamp->fet_ll);
    }
    // gpio that controls AUX/Alarm pin
    if (lamp->aux_gpio > static_cast<int>(GPIO_NUM_NC)){
        digitalWrite(lamp->aux_gpio, lamp->aux_ll);
    }
}

void ALARMTASK::startAlarm(LAMP *_lamp, const char *value){
    if(ALARMTASK::getInstance()){
        ALARMTASK::getInstance()->stopAlarm();      // отменяем предыдущий будильник
    }
    new ALARMTASK(&ts, _lamp, value);
}

void ALARMTASK::stopAlarm(){
    if(!ALARMTASK::getInstance()) return;
    LAMP *lamp = ALARMTASK::getInstance()->lamp;

    lamp->getLampState().dawnFlag = false;
    if (lamp->getMode() != LAMPMODE::MODE_ALARMCLOCK) return;

    lamp->setMode(lamp->getStoredMode() != LAMPMODE::MODE_ALARMCLOCK ? lamp->getStoredMode() : LAMPMODE::MODE_NORMAL); // возвращаем предыдущий режим
    #ifdef MP3PLAYER
    mp3->setAlarm(false);
    Task *_t = new Task(300, TASK_ONCE, nullptr, &ts, false, nullptr, [lamp](){
        mp3->RestoreVolume(); // восстановить уровень громкости
        if(lamp->isLampOn())
            mp3->playEffect(mp3->getCurPlayingNb(),"");
    }, true);
    _t->enableDelayed();
    ALARMTASK::getInstance()->curAlarm.clear(); // очистить сообщение выводимое на лампу в будильнике
    #endif

    // gpio that controls FET pin
    if (lamp->fet_gpio > static_cast<int>(GPIO_NUM_NC)){
        digitalWrite(lamp->fet_gpio, lamp->fet_ll);
    }
    // gpio that controls AUX/Alarm pin
    if (lamp->aux_gpio > static_cast<int>(GPIO_NUM_NC)){
        digitalWrite(lamp->aux_gpio, lamp->aux_ll);
    }

    LOG(printf_P, PSTR("Отключение будильника рассвет, ONflag=%d\n"), lamp->isLampOn());
    //lamp->brightness(lamp->getNormalizedLampBrightness());
    //lamp->setBrightness(lamp->getLampBrightness(), false, false);
    if (!lamp->isLampOn()) {
        lamp->effectsTimer(T_DISABLE);
        FastLED.clear();
        FastLED.show();
    } else if(lamp->getMode()==LAMPMODE::MODE_DEMO)
        lamp->demoTimer(T_ENABLE);     // вернуть демо-таймер
}

void ALARMTASK::alarmWorker(){
    if(!ALARMTASK::getInstance()) return;
    LAMP *lamp = ALARMTASK::getInstance()->lamp;

    if (lamp->getMode() != LAMPMODE::MODE_ALARMCLOCK){
        lamp->getLampState().dawnFlag = false;
        return;
    }

    // проверка рассвета, первый вход в функцию
    if (!lamp->getLampState().dawnFlag){
        ALARMTASK::getInstance()->startmillis = millis();
        memset(ALARMTASK::getInstance()->dawnColorMinus,0,sizeof(dawnColorMinus));
        ALARMTASK::getInstance()->dawnCounter = 0;
        FastLED.clear();
        lamp->setBrightness(MAX_BRIGHTNESS, fade_t::off, true);
        // величина рассвета 0-255
        int16_t dawnPosition = map((millis()-ALARMTASK::getInstance()->startmillis)/1000,0,ALARMTASK::getInstance()->curAlarm.alarmP*60,0,255); // 0...curAlarm.alarmP*60 секунд приведенные к 0...255
        dawnPosition = constrain(dawnPosition, 0, 255);
        ALARMTASK::getInstance()->dawnColorMinus[0] = CHSV(map(dawnPosition, 0, 255, 10, 35),
            map(dawnPosition, 0, 255, 255, 170),
            map(dawnPosition, 0, 255, 10, DAWN_BRIGHT)
        );
        lamp->getLampState().dawnFlag = true;
    }

    if (((millis() - ALARMTASK::getInstance()->startmillis) / 1000 > (((uint32_t)(ALARMTASK::getInstance()->curAlarm.alarmP) + ALARMTASK::getInstance()->curAlarm.alarmT) * 60UL+30U))) {
        // рассвет закончился
        stopAlarm();
        return;
    }

    //EVERY_N_SECONDS(1){
    if (TimeProcessor::getInstance().seconds00()) {
        CRGB letterColor;
        hsv2rgb_rainbow(ALARMTASK::getInstance()->dawnColorMinus[0], letterColor); // конвертация цвета времени, с учетом текущей точки рассвета
        if(!ALARMTASK::getInstance()->curAlarm.msg.isEmpty() && ALARMTASK::getInstance()->curAlarm.msg != "-") {
            lamp->sendStringToLamp(ALARMTASK::getInstance()->curAlarm.msg.c_str(), letterColor, true);
        } else {
#ifdef PRINT_ALARM_TIME
#ifdef MP3PLAYER
        if(mp3->isAlarm()) // если отложенный звук будильника, то время тоже не выводим, т.к. может быть включено озвучивание
#endif
            if(ALARMTASK::getInstance()->curAlarm.msg != "-") // отключение вывода по спец. символу "минус"
                lamp->sendStringToLamp(String("%TM").c_str(), letterColor, true);
#endif
        }
    } else if(!(localtime(TimeProcessor::now())->tm_sec%6)){ // проверка рассвета каждые 6 секунд, кроме 0 секунды
        // величина рассвета 0-255
        int16_t dawnPosition = map((millis()-ALARMTASK::getInstance()->startmillis)/1000,0,ALARMTASK::getInstance()->curAlarm.alarmP*60,0,255); // 0...300 секунд приведенные к 0...255
        dawnPosition = constrain(dawnPosition, 0, 255);

#ifdef MP3PLAYER
        //LOG(println, dawnPosition);
        if(ALARMTASK::getInstance()->curAlarm.isStartSnd)
            mp3->setTempVolume(map(dawnPosition,0,255,1,(ALARMTASK::getInstance()->curAlarm.isLimitVol ? mp3->getVolume() : 30))); // наростание громкости
        else if(dawnPosition==255 && !ALARMTASK::getInstance()->curAlarm.isStartSnd && !mp3->isAlarm()){
            mp3->setAlarm(true);
            mp3->StartAlarmSoundAtVol(ALARMTASK::getInstance()->curAlarm.type, mp3->getVolume()); // запуск звука будильника
        }
#endif
        
        ALARMTASK::getInstance()->dawnColorMinus[0] = CHSV(map(dawnPosition, 0, 255, 10, 35),
        map(dawnPosition, 0, 255, 255, 170),
        map(dawnPosition, 0, 255, 10, DAWN_BRIGHT)
        );
        ALARMTASK::getInstance()->dawnCounter++; //=dawnCounter%(sizeof(dawnColorMinus)/sizeof(CHSV))+1;

        for (uint8_t i = sizeof(dawnColorMinus) / sizeof(CHSV) - 1; i > 0U; i--){
            ALARMTASK::getInstance()->dawnColorMinus[i]=((ALARMTASK::getInstance()->dawnCounter > i)?ALARMTASK::getInstance()->dawnColorMinus[i-1]:ALARMTASK::getInstance()->dawnColorMinus[i]);
        }
    }
    //}

    int cnt{0};
    for (auto i = mx->begin(); i != mx->end(); ++i) {
        *i = ALARMTASK::getInstance()->dawnColorMinus[cnt%(sizeof(dawnColorMinus)/sizeof(CHSV))];
        ++cnt;
    }

}