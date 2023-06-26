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

#pragma once

#include "config.h" // подключаем эффекты, там же их настройки
#include "effectworker.h"
#include "events.h"
#include "LList.h"
#include "interface.h"
#include "extra_tasks.h"
#include "timerminim.hpp"
#include "char_const.h"
#include "mp3player.h"
#include "log.h"
#include "luma_curves.hpp"

#ifdef MIC_EFFECTS
#include "micFFT.h"
#endif

#ifndef DEFAULT_MQTTPUB_INTERVAL
    #define DEFAULT_MQTTPUB_INTERVAL 30
#endif

#define MAX_BRIGHTNESS            (255U)                    // стандартная максимальная яркость (255)

// a stub for 8266
#ifndef GPIO_NUM_NC
#define GPIO_NUM_NC   -1
#endif

typedef enum _LAMPMODE {
  MODE_NORMAL = 0,
  MODE_DEMO,
  MODE_RGBLAMP,
  MODE_ALARMCLOCK,
  MODE_OTA
} LAMPMODE;

// смена эффекта
typedef enum _EFFSWITCH {
    SW_NONE = 0,    // пустой
    SW_NEXT,        // следующий
    SW_PREV,        // предыдущий
    SW_RND,         // случайный
    SW_DELAY,       // сохраненный (для фейдера)
    SW_SPECIFIC,    // переход на конкретный эффект по индексу/имени
    SW_NEXT_DEMO,    // следующий для ДЕМО, исключая отключенные
} EFFSWITCH;

// управление Тикером
typedef enum _SCHEDULER {
    T_DISABLE = 0,    // Выкл
    T_ENABLE,         // Вкл
    T_FRAME_ENABLE,   // Вкл
    T_RESET,          // сброс
} SCHEDULER;

// Timings from FastLED chipsets.h
// WS2812@800kHz - 250ns, 625ns, 375ns
// время "отправки" кадра в матрицу, мс. где 1.5 эмпирический коэффициент
//#define FastLED_SHOW_TIME = WIDTH * HEIGHT * 24 * (0.250 + 0.625) / 1000 * 1.5

/*
 минимальная задержка между обсчетом и выводом кадра, мс
 нужна для обработки других задач в loop() между длинными вызовами
 калькулятора эффектов и его отрисовки. С другой стороны это время будет
 потеряно в любом случае, даже если остальные таски будут обработаны быстрее
 пока оставим тут, это крутилка не для общего конфига
 */
#define LED_SHOW_DELAY 1

#pragma pack(push,4)
typedef union _LAMPFLAGS {
struct {
    // ВНИМАНИЕ: порядок следования не менять, флаги не исключать, переводить в reserved!!! используется как битовый массив в конфиге!
    bool reserved0:1;
    bool reserved1:1;
    bool isDraw:1; // режим рисования
    bool ONflag:1; // флаг включения/выключения
    bool isFaderON:1; // признак того, что фейдер используется для эффектов
    bool isGlobalBrightness:1; // признак использования глобальной яркости для всех режимов
    bool tm24:1;   // 24х часовой формат
    bool tmZero:1;  // ведущий 0
    bool limitAlarmVolume:1; // ограничивать громкость будильника
    bool isEventsHandled:1; // глобальный признак обработки событий
    bool isEffClearing:1; // признак очистки эффектов при переходе с одного на другой
    bool isDebug:1; // признак режима отладки
    bool numInList:1; // нумерация в списке
    bool effHasMic:1; // значек микрофона в списке
    bool dRand:1; // случайный порядок демо
    bool showName:1; // отображение имени в демо
    //--------16 штук граница-------------------
    // ВНИМАНИЕ: порядок следования не менять, флаги не исключать, переводить в reserved!!! используется как битовый массив в конфиге!
    bool isMicOn:1; // глобальное включение/выключение микрофона
    uint8_t MP3eq:3; // вид эквалайзера
    bool reserved18:1;      // бывшее системное меню
    bool isOnMP3:1; // включен ли плеер?
    bool isBtn:1; // включена ли кнопка?
    bool playName:1; // воспроизводить имя?
    bool playEffect:1; // воспроизводить эффект?
    uint8_t alarmSound:3; // звук будильника ALARM_SOUND_TYPE
    bool playMP3:1; // режим mp3-плеера
    uint8_t playTime:3; // воспроизводить время?
    // ВНИМАНИЕ: порядок следования не менять, флаги не исключать, переводить в reserved!!! используется как битовый массив в конфиге!
    //--------16 штук граница-------------------
    uint8_t GaugeType:2; // тип индикатора
    bool isTempOn:1;
    bool isStream:1;
    bool isDirect:1;
    bool isMapping:1;
};
uint64_t lampflags; // набор битов для конфига
_LAMPFLAGS(){
    reserved0 = false;
    reserved1 = false;
    ONflag = false; // флаг включения/выключения
    isDebug = false; // флаг отладки
    isFaderON = true; // признак того, что используется фейдер для смены эффектов
    isEffClearing = false; // нужно ли очищать эффекты при переходах с одного на другой
    isGlobalBrightness = true; // признак использования глобальной яркости для всех режимов
    isEventsHandled = true;
    isMicOn = true; // глобальное испльзование микрофона
    numInList = false;
    effHasMic = false;
    dRand = false;
    reserved18 = false;
    isOnMP3 = false;
    isBtn = true;
    showName = false;
    playTime = TIME_SOUND_TYPE::TS_NONE; // воспроизводить время?
    playName = false; // воспроизводить имя?
    playEffect = false; // воспроизводить эффект?
    alarmSound = ALARM_SOUND_TYPE::AT_NONE;
    MP3eq = 0;
    playMP3 = false;
    limitAlarmVolume = false;
    isDraw = false;
    tm24 = true;
    tmZero = false;
    GaugeType = GAUGETYPE::GT_VERT;
    isTempOn = true;
    isStream = false;
    isDirect = false;
    isMapping = true;
}
} LAMPFLAGS;
#pragma pack(pop)

class LAMP {
    friend class LEDFader;
    friend class ALARMTASK;        // будильник ходит сюда за MOSFET и AUX пином, todo: переписать будильник целиком
private:
    LedFB *mx;           // LED matrix framebuffer object
    LedFB *sledsbuff = nullptr;    // вспомогательный буфер для слоя после эффектов
    LedFB *drawbuff = nullptr;     // буфер для рисования
#if defined(USE_STREAMING) && defined(EXT_STREAM_BUFFER)
    std::vector<CRGB> streambuff;  // буфер для трансляции
#endif

    LAMPFLAGS flags;
    LAMPSTATE lampState;                // текущее состояние лампы, которое передается эффектам

    luma::curve _curve = luma::curve::cie1931;       // default luma correction curve for PWM driven LEDs
    uint8_t globalBrightness = 127;     // глобальная яркость
    uint8_t storedBright;               // "запасное" значение яркости
    uint8_t BFade;                      // затенение фона под текстом

    uint8_t txtOffset = 0; // смещение текста относительно края матрицы
    uint16_t curLimit = CURRENT_LIMIT; // ограничение тока
    uint8_t fps = 0;        // fps counter
#ifdef LAMP_DEBUG
    uint16_t avgfps = 0;    // avarage fps counter
#endif

    // GPIO's
    uint8_t bPin = BTN_PIN;        // пин кнопки
    // это должен быть gpio_num_t в есп32, но пока нужна совместимость с 8266 держим инт
    int8_t fet_gpio = GPIO_NUM_NC, aux_gpio = GPIO_NUM_NC;
    int8_t fet_ll, aux_ll;

    LAMPMODE mode = LAMPMODE::MODE_NORMAL; // текущий режим
    LAMPMODE storedMode = LAMPMODE::MODE_NORMAL; // предыдущий режим
    uint16_t storedEffect = (uint16_t)EFF_ENUM::EFF_NONE;
    CRGB rgbColor = CRGB::White; // дефолтный цвет для RGB-режима

#ifdef MIC_EFFECTS
    MicWorker *mw = nullptr;
    void micHandler();
#endif


    uint8_t alarmPT; // время будильника рассвет - старшие 4 бита и свечения после рассвета - младшие 4 бита
#ifdef TM1637_CLOCK
    uint8_t tmBright; // яркость дисплея при вкл - старшие 4 бита и яркость дисплея при выкл - младшие 4 бита
#endif
    DynamicJsonDocument *docArrMessages = nullptr; // массив сообщений для вывода на лампу

    timerMinim tmStringStepTime;    // шаг смещения строки, в мс
    timerMinim tmNewYearMessage;    // период вывода новогоднего сообщения

    time_t NEWYEAR_UNIXDATETIME=1609459200U;    // дата/время в UNIX формате, см. https://www.cy-pr.com/tools/time/ , 1609459200 => Fri, 01 Jan 2021 00:00:00 GMT

    Task *demoTask = nullptr;    // динамический планировщик Смены эффектов в ДЕМО
    Task *effectsTask= nullptr;  // динамический планировщик обработки эффектов
    WarningTask *warningTask = nullptr; // динамический планировщик переключалки флага lampState.isWarning
    Task *tmqtt_pub = nullptr;   // динамический планировщик публикации через mqtt

    /**
     * @brief set brightness value to FastLED backend
     * method uses curve mapping to aplied value by default
     * 
     * @param _brt - brighntess value
     * @param absolute - if true, than do not apply curve mapping
     */
    void _brightness(uint8_t brt, bool absolute=false);     // низкоуровневая крутилка глобальной яркостью (для других публичных методов)

    void effectsTick(); // обработчик эффектов

    String &prepareText(String &source);
    void doPrintStringToLamp(const char* text = nullptr, CRGB letterColor = CRGB::Black, const int8_t textOffset = -128, const int16_t fixedPos = 0);
    bool fillStringManual(const char* text,  const CRGB &letterColor, bool stopText = false, bool isInverse = false, int32_t pos = 0, int8_t letSpace = LET_SPACE, int8_t txtOffset = TEXT_OFFSET, int8_t letWidth = LET_WIDTH, int8_t letHeight = LET_HEIGHT); // -2147483648
    void drawLetter(uint8_t bcount, uint16_t letter, int16_t offset,  const CRGB &letterColor, uint8_t letSpace, int8_t txtOffset, bool isInverse, int8_t letWidth, int8_t letHeight, uint8_t flSymb=0);
    uint8_t getFont(uint8_t bcount, uint8_t asciiCode, uint8_t row);

    //void alarmWorker();

    /*
     * вывод готового кадра на матрицу,
     * и перезапуск эффект-процессора
     */
    void frameShow(const uint32_t ticktime);

#ifdef MP3PLAYER
    void playEffect(bool isPlayName = false, EFFSWITCH action = EFFSWITCH::SW_NEXT);
#endif

    LAMP(const LAMP&);  // noncopyable
    LAMP& operator=(const LAMP&);  // noncopyable


/***    PUBLIC  ***/
public:
    // c-tor
    LAMP();

    /**
     * @brief set a new ledbuffer for lamp
     * it will pass it further on effects creation, etc...
     * any existing buffer will be destructed!!!
     * Do NOT do this for the buffer that is attached to FASTLED engine
     */
    void setLEDbuffer(LedFB *buff);
    void reset_led_buffs();


    /**
     * @brief show a warning message on a matrix
     * display blinks with specific color
     * and scrolls a text message
     * 
     * @param color - цвет вспышки
     * @param duration - продолжительность отображения предупреждения (общее время)
     * @param blinkHalfPeriod - продолжительность одной вспышки в миллисекундах (полупериод)
     * @param warnType - тип предупреждения 0...3; 0 - цвет, 1 - цвет + счетчик,  1 - цвет + счетчик обратным цветом,  3 - счетчик цветом
     * @param forcerestart - перезапускать, если пришло повторное событие предупреждения
     * @param msg - сообщение для вывода на матрицу
     */
    void showWarning(const CRGB &color, uint32_t duration, uint16_t blinkHalfPeriod, uint8_t warnType=0, bool forcerestart=true, const String &msg = String()); // Неблокирующая мигалка
    void warningHelper();

    void lamp_init(const uint16_t curlimit);       // первичная инициализация Лампы
    EffectWorker effects; // объект реализующий доступ к эффектам
    EVENT_MANAGER events; // Объект реализующий доступ к событиям
    uint64_t getLampFlags() {return flags.lampflags;} // возвращает упакованные флаги лампы
    const LAMPFLAGS &getLampSettings() {return flags;} // возвращает упакованные флаги лампы
    //void setLampFlags(uint32_t _lampflags) {flags.lampflags=_lampflags;} // устананавливает упакованные флаги лампы
    void setbPin(uint8_t val) {bPin = val;}
    uint8_t getbPin() {return bPin;}
    void setcurLimit(uint16_t val) {curLimit = val;}
    uint16_t getcurLimit() {return curLimit;}
    LAMPSTATE &getLampState() {return lampState;}
    LList<std::shared_ptr<UIControl>>&getEffControls() { return effects.getControls(); }

#ifdef MIC_EFFECTS
    void setMicCalibration() {lampState.isCalibrationRequest = true;}
    bool isMicCalibration() const {return lampState.isCalibrationRequest;}

    void setMicOnOff(bool val);
    
    bool isMicOnOff() const {return flags.isMicOn;}
#endif

    void setSpeedFactor(float val) {
        lampState.speedfactor = val;
        if(effects.worker) effects.worker->setDynCtrl(effects.getControls()[1].get());
    }

    // Lamp brightness control
    /**
     * @brief - Change global brightness with or without fade effect
     * if fade flag for the lamp is set, than fade applied in non-blocking way unless skipfade param is set to 'true'
     * brightness is mapped to a current lamp's luma curve value
     * 
     * @param uint8_t tgtbrt - target brigtness level 0-255
     * @param bool skipfade - force skip fade effect on brightness change
     */
    void setBrightness(uint8_t tgtbrt, bool skipfade=false);

    /**
     * @brief - Get current FASTLED brightness
     * FastLED brighten8 function applied internaly for natural brightness compensation
     * @param bool natural - return compensated or absolute brightness
     */
    uint8_t getBrightness(const bool natural=true);

    /**
     * @brief returns only CONFIGURED brightness value, not real FastLED brightness
     * 
     */
    uint8_t getLampBrightness() { return flags.isGlobalBrightness? globalBrightness : (effects.getControls()[0]->getVal()).toInt();}

    // выставляет ТОЛЬКО значение в конфиге! Яркость не меняет!
    void setLampBrightness(uint8_t brt) { lampState.brightness=brt; if (flags.isGlobalBrightness) globalBrightness = brt; else effects.getControls()[0]->setVal(String(brt)); }

    void setIsGlobalBrightness(bool val){};   // {flags.isGlobalBrightness = val; if(effects.worker) { lampState.brightness=getLampBrightness(); effects.worker->setDynCtrl(effects.getControls()[0].get());} }

    // keep it for compatibily reasons
    bool IsGlobalBrightness() const { return true; }

    /**
     * @brief get lamp brightness in percents 0-100
     * 
     * @return uint8_t brightness value in percents
     */
    uint8_t lampBrightnesspct(){ return getLampBrightness() * 100 / 255; };

    /**
     * @brief set lamp brightness in percents
     * 
     * @param brt value 0-100
     * @return uint8_t 
     */
    uint8_t lampBrightnesspct(uint8_t brt);

    bool isAlarm() {return mode == LAMPMODE::MODE_ALARMCLOCK;}
    bool isWarning() {return lampState.isWarning;}

#ifdef EMBUI_USE_MQTT
    void setmqtt_int(int val=DEFAULT_MQTTPUB_INTERVAL);
#endif

    LAMPMODE getMode() {return mode;}
    LAMPMODE getStoredMode() {return storedMode;}
    void setMode(LAMPMODE _mode) { storedMode = ((mode == _mode) ? storedMode: mode); mode=_mode;}

    /**
     * @brief send text to scroll on screen
     * using default color and options
     * @param text - text to scroll
     */
    void sendString(const char* text);

    /**
     * @brief 
     * 
     * @param text 
     * @param letterColor 
     * @param forcePrint - выводить текст при выключенной лампе
     * @param clearQueue 
     */
    void sendString(const char* text, CRGB letterColor, bool forcePrint = true, bool clearQueue = false);
    void sendStringToLamp(const char* text = nullptr, CRGB letterColor = CRGB::Black, bool forcePrint = false, bool clearQueue = false, const int8_t textOffset = -128, const int16_t fixedPos = 0);
    void sendStringToLampDirect(const char* text = nullptr,  CRGB letterColor = CRGB::Black, bool forcePrint = false, bool clearQueue = false, const int8_t textOffset = -128, const int16_t fixedPos = 0);
    bool isPrintingNow() { return lampState.isStringPrinting; }

    void handle();          // главная функция обработки эффектов

    void setFaderFlag(bool flag) {flags.isFaderON = flag;}
    bool getFaderFlag() {return flags.isFaderON;}
    void setClearingFlag(bool flag) {flags.isEffClearing = flag;}
    bool getClearingFlag() {return flags.isEffClearing;}
    void disableEffectsUntilText() {lampState.isEffectsDisabledUntilText = true; FastLED.clear();}
    void setOffAfterText() {lampState.isOffAfterText = true;}
    void setIsEventsHandled(bool flag) {flags.isEventsHandled = flag;}
    bool IsEventsHandled() {return flags.isEventsHandled;} // LOG(printf_P,PSTR("flags.isEventsHandled=%d\n"), flags.isEventsHandled);
    bool isLampOn() {return flags.ONflag;}
    bool isDebugOn() {return flags.isDebug;}
    bool isDebug() {return lampState.isDebug;}
    bool isDrawOn() {return flags.isDraw;}
    void setDebug(bool flag) {flags.isDebug=flag; lampState.isDebug=flag;}
    void setButton(bool flag) {flags.isBtn=flag;}
    void setDraw(bool flag);

    /**
     * @brief creates/destroys buffer for "drawing"
     * 
     * @param active - if 'true' creates new buffer, otherwise destory/release buffer mem
     */
    void setDrawBuff(bool active);
    void writeDrawBuf(CRGB &color, uint16_t x, uint16_t y) { if(drawbuff) { drawbuff->pixel(x,y) = color; } }
    void writeDrawBuf(CRGB &color, uint16_t num) { if(drawbuff) { drawbuff->at(num)=color; } }

    /**
     * @brief fill DrawBuffer with solid color
     * 
     * @param color 
     */
    void fillDrawBuf(CRGB &color);

    /**
     * @brief fill DrawBuf with solid black
     * 
     */
    void clearDrawBuf() { CRGB c = CRGB::Black; fillDrawBuf(c); }

#ifdef USE_STREAMING
    bool isStreamOn() {return flags.isStream;}
    bool isDirect() {return flags.isDirect;}
    bool isMapping() {return flags.isMapping;}
    void setStream(bool flag) {flags.isStream = flag;}
    void setDirect(bool flag) {flags.isDirect = flag;}
    void setMapping(bool flag) {flags.isMapping = flag;}
#ifdef EXT_STREAM_BUFFER
    /**
     * @brief creates/destroys buffer for "streaming"
     * 
     * @param active - if 'true' creates new buffer, otherwise destory/release buffer mem
     */
    void setStreamBuff(bool active);
    void writeStreamBuff(CRGB &color, uint16_t x, uint16_t y) { if(!streambuff.empty()) { streambuff[getPixelNumber(x,y)]=color; } }
    void writeStreamBuff(CRGB &color, uint16_t num) { if(!streambuff.empty()) { streambuff[num]=color; } }
    void fillStreamBuff(CRGB &color) { for(uint16_t i=0; i<streambuff.size(); i++) streambuff[i]=color; }
    void clearStreamBuff() { fillStreamBuff(CRGB::Black); }
#endif
#endif
    bool isONMP3() {return flags.isOnMP3;}
    void setONMP3(bool flag) {flags.isOnMP3=flag;}
    //void setMIRR_V(bool flag) {if (flag!=mx.cfg.vmirror()) { mx.cfg.vmirror(flag); mx.clear();} }
    //void setMIRR_H(bool flag) {if (flag!=mx.cfg.hmirror()) { mx.cfg.hmirror(flag); mx.clear();} }
    void setTextMovingSpeed(uint8_t val) {tmStringStepTime.setInterval(val);}
    uint32_t getTextMovingSpeed() {return tmStringStepTime.getInterval();}
    void setTextOffset(uint8_t val) { txtOffset=val;}

    void setPlayTime(uint8_t val) {flags.playTime = val;}
    void setPlayName(bool flag) {flags.playName = flag;}
    void setPlayEffect(bool flag) {flags.playEffect = flag;}
    void setPlayMP3(bool flag) {flags.playMP3 = flag;}
    void setLimitAlarmVolume(bool flag) {flags.limitAlarmVolume = flag;}
    void setAlatmSound(ALARM_SOUND_TYPE val) {flags.alarmSound = val;}
    void setEqType(uint8_t val) {flags.MP3eq = val;}

    /**
     * @brief prints current time on screen
     * 
     * @param value - some ugly json string with opts   {'isShowOff':false,'isPlayTime':true}
     * @param force - print even if lamp is off
     */
    void showTimeOnScreen(const char *value, bool force=false);

#ifdef TM1637_CLOCK
    void settm24 (bool flag) {flags.tm24 = flag;}
    void settmZero (bool flag) {flags.tmZero = flag;}
    bool isTm24() {return flags.tm24;}
    bool isTmZero() {return flags.tmZero;}
    void setTmBright(uint8_t val) {tmBright = val;}
    uint8_t getBrightOn() { return tmBright>>4; }
    uint8_t getBrightOff() { return tmBright&0x0F; }
#endif
#ifdef DS18B20
bool isTempDisp() {return flags.isTempOn;}
void setTempDisp(bool flag) {flags.isTempOn = flag;}
#endif
    bool getGaugeType() {return flags.GaugeType;}
    void setGaugeType(GAUGETYPE val) {flags.GaugeType = val;}
    void startRGB(CRGB &val);
    void stopRGB();
    bool isRGB() {return mode == LAMPMODE::MODE_RGBLAMP;}
    void startDemoMode(uint8_t tmout = DEFAULT_DEMO_TIMER); // дефолтное значение, настраивается из UI
    void startNormalMode(bool forceOff=false);
    void restoreStored();
    void storeEffect();
    void newYearMessageHandle();
    void setBFade(uint8_t val){ BFade = val; }
    uint8_t getBFade(){ return BFade; }
    void setNYMessageTimer(int in){ tmNewYearMessage.setInterval(in*60*1000); tmNewYearMessage.reset(); }
    void setNYUnixTime(time_t tm){ NEWYEAR_UNIXDATETIME = tm; }
    void setNumInList(bool flag) {flags.numInList = flag;}
    void setEffHasMic(bool flag) {flags.effHasMic = flag;}
    void setDRand(bool flag) {flags.dRand = flag; lampState.isRandDemo = (flag && mode==LAMPMODE::MODE_DEMO); }
    void setShowName(bool flag) {flags.showName = flag;}

    void setAlarmPT(uint8_t val) {alarmPT = val;}
    uint8_t getAlarmP() { return alarmPT>>4; }
    uint8_t getAlarmT() { return alarmPT&0x0F; }

    // ---------- служебные функции -------------

    void changePower(); // плавное включение/выключение
    void changePower(bool);

    /**
     * @brief - переключатель эффектов для других методов,
     * может использовать фейдер, выбирать случайный эффект для демо
     * @param EFFSWITCH action - вид переключения (пред, след, случ.)
     * @param fade - переключаться через фейдер или сразу
     * @param effnb - номер эффекта
     * skip - системное поле - пропуск фейдера
     */
    void switcheffect(EFFSWITCH action = SW_NONE, bool fade = false, uint16_t effnb = EFF_ENUM::EFF_NONE, bool skip = false);

    /*
     * включает/выключает "демо"-таймер
     * @param SCHEDULER action - enable/disable/reset
     */
    void demoTimer(SCHEDULER action, uint8_t tmout = DEFAULT_DEMO_TIMER); // дефолтное значение, настраивается из UI

    /*
     * включает/выключает "эффект"-таймер
     * @param SCHEDULER action - enable/disable/reset
     */
    void effectsTimer(SCHEDULER action, uint32_t _begin = 0);
};

/**
 * @brief asynchronously fade lamp's brightness
 * 
 */
class LEDFader {
    LAMP *lmp = nullptr;
    uint8_t _brt;                               // transient brightness
    uint8_t  _tgtbrt{0};                        // target brightness
    int8_t _brtincrement;                       // change step
    std::function<void(void)> _cb = nullptr;    // callback func to call upon completition
    Task *runner = nullptr;
    LEDFader(){};     // hidden c-tor;
    ~LEDFader(){};    // hidden d-tor;

public:
    // this is a singleton, no copy's
    LEDFader(const LEDFader&) = delete;
    LEDFader& operator=(const LEDFader &) = delete;
    LEDFader(LEDFader &&) = delete;
    LEDFader & operator=(LEDFader &&) = delete;

    /**
     * get singlton instance
    */
    static LEDFader* getInstance(){
        static LEDFader fader;
        return &fader;
    }

    /**
     * @brief aborts fader runner if in progress
     * it will leave lamp brightness as-is on the moment call was made
     * fader callback won't be executed
     */
    void abort();

    /**
     * @brief Set ptr to Lamp instance object
     * todo: get rid of this rudiment
     * @param l ptr to lamp
     */
    void setLamp(LAMP *l){ if(l) lmp = l;}

    /**
     * @brief - Non-blocking light fader, uses scheduler to globaly fade FastLED brighness within specified duration
     * @param uint8_t _targetbrightness - end value for the brighness to fade to. FastLED dim8 function applied internaly for natural dimming
     * @param uint32_t _duration - fade effect duraion, ms
     * @param callback  -  callback-функция, которая будет выполнена после окончания затухания
     */
    void fadelight(const uint8_t _targetbrightness=0, const uint32_t _duration=FADE_TIME, std::function<void()> callback=nullptr);

    /**
     * @brief check if fade is in progress
     * 
     * @return true 
     * @return false 
     */
    bool running() const { return runner; }
};

//-----------------------------------------------
extern LAMP myLamp; // Объект лампы
#ifdef MP3PLAYER
extern MP3PlayerDevice *mp3;
#endif
