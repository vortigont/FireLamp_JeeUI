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
#include "EmbUI.h"
#include "extra_tasks.h"
#include "char_const.h"
#include "mp3player.h"
#include "luma_curves.hpp"

#ifdef MIC_EFFECTS
#include "micFFT.h"
#endif

#ifndef DEFAULT_MQTTPUB_INTERVAL
    #define DEFAULT_MQTTPUB_INTERVAL 30
#endif

#define MAX_BRIGHTNESS            (255U)                    // максимальная яркость LED
#define DEF_BRT_SCALE               20                      // шкала регулировки яркости по-умолчанию

typedef enum _LAMPMODE {
  MODE_NORMAL = 0,
  MODE_DEMO,
  MODE_RGBLAMP,
  MODE_ALARMCLOCK,
  MODE_OTA
} LAMPMODE;

// смена эффекта
enum class effswitch_t : uint8_t {
    none = 0,   // переключить на пустой эффект, он же темнота
    next,       // следующий
    prev,       // предыдущий
    rnd,        // случайный
    num        // переход на конкретный эффект по индексу
};  // ex EFFSWITCH;

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
    bool reserved3:1;
    bool reserved4:1;
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
    GaugeType = GAUGETYPE::GT_VERT;
}
} LAMPFLAGS;
#pragma pack(pop)

class Lamp {
    friend class LEDFader;
private:
    std::shared_ptr<LedFB<CRGB> > _overlay;     // буфер для оверлея

    LAMPFLAGS flags;
    LAMPSTATE lampState;                // текущее состояние лампы, которое передается эффектам

    uint8_t _brightnessScale = DEF_BRT_SCALE;
    luma::curve _curve = luma::curve::cie1931;       // default luma correction curve for PWM driven LEDs
    uint8_t globalBrightness = 127;     // глобальная яркость
    uint8_t storedBright;               // "запасное" значение яркости
    uint8_t BFade;                      // затенение фона под текстом

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

    Task *demoTask = nullptr;    // динамический планировщик Смены эффектов в ДЕМО

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

#ifdef MP3PLAYER
    // temp disable
    void playEffect(bool isPlayName = false, effswitch_t action = effswitch_t::next){};
#endif

    /**
     * @brief effectiveley wipes LedBuffers and fills LED Strip with Black
     * 
     */
    //void _wipe_screen();

public:
    // c-tor
    Lamp();
    // d-tor
    ~Lamp();


    // noncopyable
    Lamp (const Lamp&) = delete;
    Lamp& operator= (const Lamp&) = delete;

    EffectWorker effwrkr; // объект реализующий доступ к эффектам

    // инициализация Лампы
    void lamp_init();

    void setbPin(uint8_t val) {bPin = val;}
    uint8_t getbPin() {return bPin;}
    LAMPSTATE &getLampState() {return lampState;}
    LList<std::shared_ptr<UIControl>>&getEffControls() { return effwrkr.getControls(); }

#ifdef MIC_EFFECTS
    void setMicCalibration() {lampState.isCalibrationRequest = true;}
    bool isMicCalibration() const {return lampState.isCalibrationRequest;}

    void setMicOnOff(bool val);
    
    bool isMicOnOff() const {return flags.isMicOn;}
#endif

    void setSpeedFactor(float val) {
        lampState.speedfactor = val;
        if(effwrkr.getControls().exist(1)) effwrkr.setDynCtrl(effwrkr.getControls()[1].get());
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

    void setPlayTime(uint8_t val) {flags.playTime = val;}
    void setPlayName(bool flag) {flags.playName = flag;}
    void setPlayEffect(bool flag) {flags.playEffect = flag;}
    void setPlayMP3(bool flag) {flags.playMP3 = flag;}
    void setLimitAlarmVolume(bool flag) {flags.limitAlarmVolume = flag;}
    void setEqType(uint8_t val) {flags.MP3eq = val;}

    /**
     * @brief prints current time on screen
     * 
     * @param value - some ugly json string with opts   {'isShowOff':false,'isPlayTime':true}
     * @param force - print even if lamp is off
     */
    void showTimeOnScreen(const char *value, bool force=false);

    bool getGaugeType() {return flags.GaugeType;}
    void setGaugeType(GAUGETYPE val) {flags.GaugeType = val;}
    void startRGB(CRGB &val);
    void stopRGB();
    bool isRGB() {return mode == LAMPMODE::MODE_RGBLAMP;}
    void startDemoMode(uint8_t tmout = DEFAULT_DEMO_TIMER); // дефолтное значение, настраивается из UI
    void startNormalMode(bool forceOff=false);
    void restoreStored();
    void storeEffect();
    void setBFade(uint8_t val){ BFade = val; }
    uint8_t getBFade(){ return BFade; }
    void setEffHasMic(bool flag) {flags.effHasMic = flag;}
    void setDRand(bool flag) {flags.dRand = flag; lampState.isRandDemo = (flag && mode==LAMPMODE::MODE_DEMO); }
    void setShowName(bool flag) {flags.showName = flag;}


    // ---------- служебные функции -------------

    /**
     * @brief вкл./выкл лампу
     * логическое включение/выключение лампы
     * generates events:
     *  lampEvtId_t::pwron
     *  lampEvtId_t::pwroff
     * 
     */
    void power(bool);

    /**
     * @brief toggle logical power state for the lamp
     * generates events:
     *  lampEvtId_t::pwron
     *  lampEvtId_t::pwroff
     * 
     */
    void power();

    /**
     * @brief общий переключатель эффектов лампы
     * в зависимости от режима лампы может переключать эффекты с затуханием или без
     * @param action - тип переключения на эффект, предыдущий, следующий, конкретный и т.п.
     * @param effnb - опциональный параметр номер переключаемого эффекта
     */
    void switcheffect(effswitch_t action, uint16_t effnb = EFF_ENUM::EFF_NONE);

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

    /**
     * @brief static event handler
     * wraps class members access for event loop
     * 
     * @param handler_args 
     * @param base 
     * @param id 
     * @param event_data 
     */
    static void event_hndlr(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);

private:
    /**
     * @brief - переключатель эффектов для других методов,
     * может использовать фейдер, выбирать случайный эффект для демо
     * @param effswitch_t action - вид переключения (пред, след, случ.)
     * @param fade - переключаться через фейдер или сразу
     * @param effnb - номер эффекта
     * @param skip - системное поле - пропуск фейдера
     */
    void _switcheffect(effswitch_t action, bool fade, uint16_t effnb = EFF_ENUM::EFF_NONE, bool skip = false);

    /**
     * @brief creates/destroys buffer for "drawing, etc..."
     * 
     * @param active - if 'true' creates new buffer, otherwise destory/release buffer
     */
    void _overlay_buffer(bool activate);


    // *** Event bus members    ***

    // instance that holds lamp command events handler
    esp_event_handler_instance_t _events_lamp_cmd;

    /**
     * @brief subscribe lamp objects to interesting events
     * 
     */
    void _events_subsribe();

    /**
     * @brief unregister from event loop
     * 
     */
    void events_unsubsribe();

    /**
     * @brief event picker method, processes incoming command events from a event_hndlr wrapper
     * 
     * @param base 
     * @param id 
     * @param event_data 
     */
    void _event_picker_cmd(esp_event_base_t base, int32_t id, void* data);

    /**
     * @brief event picker method, processes incoming notification events from a event_hndlr wrapper
     * 
     * @param base 
     * @param id 
     * @param event_data 
     */
    void _event_picker_state(esp_event_base_t base, int32_t id, void* data);


};

/**
 * @brief asynchronously fade lamp's brightness
 * 
 */
class LEDFader {
    Lamp *lmp = nullptr;
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
    void setLamp(Lamp *l){ if(l) lmp = l;}

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
extern Lamp myLamp; // Объект лампы
