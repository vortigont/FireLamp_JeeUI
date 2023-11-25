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

#define MAX_BRIGHTNESS            (255U)                    // максимальная яркость LED
#define DEF_BRT_SCALE               20                      // шкала регулировки яркости по-умолчанию

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

/**
 * @brief if method should use a fader when changing brightness
 * 
 */
enum class fade_t {
    off=0,
    on,
    preset
};

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
    bool restoreState:1;    // restore lamp on/off/demo state on restart
    bool ONflag:1;          // флаг включения/выключения
    bool isFaderON:1;       // признак того, что фейдер используется для эффектов
    bool tm24:1;            // 24х часовой формат
    bool tmZero:1;          // ведущий 0
    bool limitAlarmVolume:1; // ограничивать громкость будильника
    bool isEventsHandled:1; // глобальный признак обработки событий
    bool isEffClearing:1;   // признак очистки эффектов при переходе с одного на другой
    bool isDebug:1;         // признак режима отладки
    bool dRand:1;           // случайный порядок демо
    bool showName:1;        // отображение имени в демо
    bool isMicOn:1;         // глобальное включение/выключение микрофона
    bool effHasMic:1;       // микрофон для эффекта
    bool isOnMP3:1;         // включен ли плеер?
    bool isBtn:1;           // включена ли кнопка?
    bool playName:1;        // воспроизводить имя?
    //--------16 штук граница-------------------
    bool playEffect:1;      // воспроизводить эффект?
    bool playMP3:1;         // режим mp3-плеера
    uint8_t alarmSound:3;   // звук будильника ALARM_SOUND_TYPE
    uint8_t playTime:3;     // воспроизводить время?
    uint8_t GaugeType:2;    // тип индикатора
    uint8_t MP3eq:3;        // вид эквалайзера
};
uint32_t lampflags; // набор битов для конфига
_LAMPFLAGS(){
    restoreState = false;
    ONflag = false;
    isDebug = false;
    isFaderON = true;
    isEffClearing = false;
    isEventsHandled = false;
    isMicOn = false;
    effHasMic = false;
    dRand = false;
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
    tm24 = true;
    tmZero = false;
    GaugeType = GAUGETYPE::GT_VERT;
}
} LAMPFLAGS;
#pragma pack(pop)

class LAMP {
    friend class LEDFader;
    friend class ALARMTASK;        // будильник ходит сюда за MOSFET и AUX пином, todo: переписать будильник целиком
private:
    std::shared_ptr<LedFB<CRGB> > _overlay;     // буфер для оверлея

    LAMPFLAGS flags;
    LAMPSTATE lampState;                // текущее состояние лампы, которое передается эффектам

    uint8_t _brightnessScale = DEF_BRT_SCALE;
    luma::curve _curve = luma::curve::cie1931;       // default luma correction curve for PWM driven LEDs
    uint8_t globalBrightness = 127;     // глобальная яркость
    uint8_t storedBright;               // "запасное" значение яркости
    uint8_t BFade;                      // затенение фона под текстом

    uint8_t txtOffset = 0; // смещение текста относительно края матрицы

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
    uint8_t tmBright; // яркость дисплея при вкл - старшие 4 бита и яркость дисплея при выкл - младшие 4 бита

    DynamicJsonDocument *docArrMessages = nullptr; // массив сообщений для вывода на лампу

    timerMinim tmStringStepTime;    // шаг смещения строки, в мс
    timerMinim tmNewYearMessage;    // период вывода новогоднего сообщения

    time_t NEWYEAR_UNIXDATETIME=1609459200U;    // дата/время в UNIX формате, см. https://www.cy-pr.com/tools/time/ , 1609459200 => Fri, 01 Jan 2021 00:00:00 GMT

    Task *demoTask = nullptr;    // динамический планировщик Смены эффектов в ДЕМО
    Task *effectsTask= nullptr;  // динамический планировщик обработки эффектов
    WarningTask *warningTask = nullptr; // динамический планировщик переключалки флага lampState.isWarning
    Task *tmqtt_pub = nullptr;   // динамический планировщик публикации через mqtt

    /**
     * @brief set brightness value to Display backend
     * method uses curve mapping to the applied value by default
     * 
     * @param _brt - brighntess value
     * @param absolute - if true, than do not apply curve mapping
     */
    void _brightness(uint8_t brt, bool absolute=false);

    /**
     * @brief get actual matrix led brightness
     * returns either scaled and curve-mapped or absolute value 
     * @param absolute - get absolute brightness
     * @return uint8_t - brightness value
     */
    uint8_t _get_brightness(bool absolute=false);

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

    /**
     * @brief effectiveley wipes LedBuffers and fills LED Strip with Black
     * 
     */
    //void _wipe_screen();

public:
    // c-tor
    LAMP();

    // noncopyable
    LAMP (const LAMP&) = delete;
    LAMP& operator= (const LAMP&) = delete;

    void lamp_init();       // первичная инициализация Лампы

    // wipe all screen buffers to black
    //void reset_led_buffs();


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

    EffectWorker effects; // объект реализующий доступ к эффектам
    EVENT_MANAGER events; // Объект реализующий доступ к событиям

    void setbPin(uint8_t val) {bPin = val;}
    uint8_t getbPin() {return bPin;}
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
        if(effects.getControls().exist(1)) effects.setDynCtrl(effects.getControls()[1].get());
    }

    // Lamp brightness control
    /**
     * @brief - Change global brightness with or without fade effect
     * if fade flag for the lamp is set, than fade applied in non-blocking way unless skipfade param is set to 'true'
     * brightness is mapped to a current lamp's luma curve value
     * 
     * @param uint8_t tgtbrt - target brigtness level 0-255
     * @param fade_t fade - use/skip or use default fade effect
     * @param bool bypass - set brightness as-as directly to backend, skipping fader, scaling and do NOT save new value
     */
    void setBrightness(uint8_t tgtbrt, fade_t fade=fade_t::preset, bool bypass = false);

    /**
     * @brief - Get configured lamp's brightness
     * returns scaled brightness according to the defined luma curve
     */
    uint8_t getBrightness() const { return globalBrightness; };

    /**
     * @brief Set the Luma Curve for brightness correction
     * 
     * @param c luma curve enum
     */
    void setLumaCurve(luma::curve c);

    /**
     * @brief Get current Luma Curve value
     * 
     * @return luma::curve 
     */
    luma::curve getLumaCurve() const { return _curve; };

    /**
     * @brief Set the Brightness Scale range
     * i.e. 100 equals to 0-100% sclae
     * default is DEF_BRT_SCALE
     * 
     * @param scale 
     */
    void setBrightnessScale(uint8_t scale){ _brightnessScale = scale ? scale : DEF_BRT_SCALE; };

    /**
     * @brief Get the Brightness Scale
     * see setBrightnessScale()
     */
    uint8_t getBrightnessScale() const { return _brightnessScale; };

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

    // === flag get/set methods ===

    // возвращает упакованные в целое флаги лампы
    uint32_t getLampFlags() {return flags.lampflags;}
    // возвращает структуру флагов лампы
    const LAMPFLAGS &getLampFlagsStuct() const {return flags;}
    // saves flags to EmbUI config
    void save_flags();
    void setFaderFlag(bool flag) {flags.isFaderON = flag; save_flags(); }
    bool getFaderFlag() {return flags.isFaderON; save_flags(); }
    void setClearingFlag(bool flag) {flags.isEffClearing = flag; save_flags(); }
    bool getClearingFlag() {return flags.isEffClearing; }
    void disableEffectsUntilText() {lampState.isEffectsDisabledUntilText = true; display.clear(); save_flags(); }
    void setOffAfterText() {lampState.isOffAfterText = true; save_flags(); }
    void setIsEventsHandled(bool flag) {flags.isEventsHandled = flag; save_flags(); }
    bool IsEventsHandled() {return flags.isEventsHandled;} // LOG(printf_P,PSTR("flags.isEventsHandled=%d\n"), flags.isEventsHandled);
    bool isLampOn() {return flags.ONflag;}
    bool isDebugOn() {return flags.isDebug;}
    bool isDebug() {return lampState.isDebug;}
    void setDebug(bool flag) {flags.isDebug=flag; lampState.isDebug=flag; save_flags(); }
    void setButton(bool flag) {flags.isBtn=flag; save_flags(); }

    // set/clear "restore on/off/demo" state on boot
    void setRestoreState(bool flag){ flags.restoreState = flag; save_flags(); }


    // Drawing methods
    bool isDrawOn() const { return _overlay.get(); }

    // activate/disable overlay for drawing
    void enableDrawing(bool state){ _overlay_buffer(state); };

    // draw a pixel in overlay
    void writeDrawBuf(CRGB color, uint16_t x, uint16_t y);
    //void writeDrawBuf(CRGB color, uint16_t num);

    /**
     * @brief fill DrawBuffer with solid color
     * 
     * @param color 
     */
    void fillDrawBuf(CRGB color);

    /**
     * @brief fill DrawBuf with solid black (make it transparent)
     * 
     */
    void clearDrawBuf() { CRGB c = CRGB::Black; fillDrawBuf(c); }

    bool isONMP3() {return flags.isOnMP3;}
    void setONMP3(bool flag) {flags.isOnMP3=flag;}
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

    // TM1637_CLOCK
    void settm24 (bool flag) {flags.tm24 = flag;}
    void settmZero (bool flag) {flags.tmZero = flag;}
    bool isTm24() const {return flags.tm24;}
    bool isTmZero() const {return flags.tmZero;}
    void setTmBright(uint8_t val) {tmBright = val; embui.var(TCONST_tmBright, val); }
    uint8_t getBrightOn() const { return tmBright>>4; }
    uint8_t getBrightOff() const { return tmBright&0x0F; }

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
     * @brief общий переключатель эффектов лампы
     * в зависимости от режима лампы может переключать эффекты с затуханием или без
     * @param action - тип переключения на эффект, предыдущий, следующий, конкретный и т.п.
     * @param effnb - опциональный параметр номер переключаемого эффекта
     */
    void switcheffect(EFFSWITCH action, uint16_t effnb = EFF_ENUM::EFF_NONE);

    /**
     * @brief - переключатель эффектов для других методов,
     * может использовать фейдер, выбирать случайный эффект для демо
     * @param EFFSWITCH action - вид переключения (пред, след, случ.)
     * @param fade - переключаться через фейдер или сразу
     * @param effnb - номер эффекта
     * @param skip - системное поле - пропуск фейдера
     */
    void switcheffect(EFFSWITCH action, bool fade, uint16_t effnb = EFF_ENUM::EFF_NONE, bool skip = false);

    /*
     * включает/выключает "демо"-таймер
     * @param SCHEDULER action - enable/disable/reset
     */
    void demoTimer(SCHEDULER action, uint8_t tmout = DEFAULT_DEMO_TIMER); // дефолтное значение, настраивается из UI

    /*
     * включает/выключает "эффект"-таймер
     * @param SCHEDULER action - enable/disable/reset
     */
    void effectsTimer(SCHEDULER action);

private:
    /**
     * @brief creates/destroys buffer for "drawing, etc..."
     * 
     * @param active - if 'true' creates new buffer, otherwise destory/release buffer
     */
    void _overlay_buffer(bool activate);


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
     * @brief - Non-blocking light fader, uses scheduler to globaly fade display brightness within specified duration
     * @param uint8_t _targetbrightness - end value for the brighness to fade to
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
