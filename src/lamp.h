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
#include "char_const.h"
#include "luma_curves.hpp"
#include "micFFT.h"

#ifndef DEFAULT_MQTTPUB_INTERVAL
    #define DEFAULT_MQTTPUB_INTERVAL 30
#endif

#define MAX_BRIGHTNESS            (255U)                    // максимальная яркость LED
#define DEF_BRT_SCALE               20                      // шкала регулировки яркости по-умолчанию

typedef enum _LAMPMODE {
  MODE_NORMAL = 0,
  MODE_DEMO,
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


struct LampFlags {
    // ВНИМАНИЕ: порядок следования не менять, флаги не исключать, переводить в reserved!!! используется как битовый массив в конфиге!
    bool restoreState:1;    // restore lamp on/off/demo state on restart
    bool pwrState:1;        // флаг включения/выключения
    bool fadeEffects:1;     // признак использования затухания при смене эффектов/яркости
    bool demoMode:1;
    bool demoRandom:1;
    bool reserved5:1;
    bool reserved6:1;
    bool wipeOnEffChange:1; // признак очистки экрана при переходе с одного эффекта на другой
    bool debug:1;           // признак режима отладки
    bool reserved9:1;       // случайный порядок демо
    bool reserved10:1;      // отображение имени в демо
    bool isMicOn:1;         // включение/выключение микрофона
    bool effHasMic:1;       // микрофон для эффекта
    bool reserved13:1;
    bool reserved14:1;
    bool reserved15:1;      //
    //--------16 штук граница-------------------
};

union LampFlagsPack {
    uint32_t pack;          // vars packed into unsigned
    LampFlags flag;
    LampFlagsPack() : pack(4){}     // set fade
};



/**
 * @brief Lamp class
 * 
 */
class Lamp {
    friend class LEDFader;
private:
    // struct keep state during switching effects with fader
    struct EffSwitch_state_t {
        int fadeState{0};       // 0 not fading, -1 - fadeout, 1 - fadein
        uint16_t pendingEffectNum{0};
    };

    // effect switching state
    EffSwitch_state_t _swState;

    std::shared_ptr<LedFB<CRGB> > _overlay;     // буфер для оверлея

    // a set of lamp options (flags)
    LampFlagsPack opts;
    // текущее состояние лампы, которое передается в класс эффектпроцессора
    LampState lampState;

    uint8_t _brightnessScale = DEF_BRT_SCALE;
    // default luma correction curve for PWM driven LEDs
    luma::curve _curve = luma::curve::cie1931;
    uint8_t globalBrightness = 127;     // глобальная яркость
    //uint8_t storedBright;               // "запасное" значение яркости
    //uint8_t BFade;                      // затенение фона под текстом

    // GPIO's
    int32_t fet_gpio, aux_gpio, _mic_gpio;
    bool fet_ll, aux_ll;

    LAMPMODE mode = LAMPMODE::MODE_NORMAL; // текущий режим
    LAMPMODE storedMode = LAMPMODE::MODE_NORMAL; // предыдущий режим
    uint16_t storedEffect = (uint16_t)EFF_ENUM::EFF_NONE;

    // Microphone
    MicWorker *mw = nullptr;
    void micHandler();

    // аймер смены эффектов в ДЕМО
    Task *demoTask = nullptr;

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

    LampState &getLampState() {return lampState;}
    LList<std::shared_ptr<UIControl>>&getEffControls() { return effwrkr.getControls(); }

    void setMicOnOff(bool val);
    //bool isMicOnOff() const {return opts.flag.isMicOn;}

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
     * @param bool bypass - set brightness as-шs directly to backend, skipping fader, scaling and do NOT save new value to NVS
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

    LAMPMODE getMode() {return mode;}
    LAMPMODE getStoredMode() {return storedMode;}
    void setMode(LAMPMODE _mode) { storedMode = ((mode == _mode) ? storedMode: mode); mode=_mode;}

    // Loop cycle
    void handle();

    // === flag get/set methods ===

    // возвращает упакованные в целое флаги лампы
    uint32_t getLampFlags() {return opts.pack; }
    // возвращает структуру флагов лампы
    const LampFlags &getLampFlagsStuct() const {return opts.flag; }
    // saves flags to EmbUI config
    void save_flags();
    void setFaderFlag(bool flag) {opts.flag.fadeEffects = flag; save_flags(); }
    bool getFaderFlag() const { return opts.flag.fadeEffects; }
    void setClearingFlag(bool flag) {opts.flag.wipeOnEffChange = flag; save_flags(); }
    bool getClearingFlag() const {return opts.flag.wipeOnEffChange; }

    bool isLampOn() {return opts.flag.pwrState;}
    bool isDebugOn() {return opts.flag.debug;}
    bool isDebug() {return lampState.isDebug;}
    void setDebug(bool flag) {opts.flag.debug = flag; lampState.isDebug=flag; save_flags(); }

    // set/clear "restore on/off/demo" state on boot
    void setRestoreState(bool flag){ opts.flag.restoreState = flag; save_flags(); }


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

    void startDemoMode(uint8_t tmout = DEFAULT_DEMO_TIMER); // дефолтное значение, настраивается из UI
    void startNormalMode(bool forceOff=false);

    void setEffHasMic(bool flag) {opts.flag.effHasMic = flag;}
    void setDRand(bool flag) {opts.flag.demoRandom = flag; lampState.isRandDemo = (flag && mode==LAMPMODE::MODE_DEMO); }


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

private:
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

    /**
     * @brief - переключатель эффектов для других методов,
     * может использовать фейдер, выбирать случайный эффект для демо
     * @param effswitch_t action - вид переключения (пред, след, случ.)
     * @param fade - переключаться через фейдер или сразу
     * @param effnb - номер эффекта на который переключаться (при переключении по конкретному номеру)
     */
    void _switcheffect(effswitch_t action, bool fade, uint16_t effnb = EFF_ENUM::EFF_NONE);

    /**
     * @brief get effect number relative to fader state
     * when not in transition state it returns current effect number
     * when in transition state, it returns pending effect number
     * @return uint16_t 
     */
    uint16_t _getRealativeEffectNum();


    /**
     * @brief - обработка событий fadeEnd
     * отслеживает стадии переключения эффектов через затухание
     * запускает 2ю стадию переключения эффекта после окончания затухания
     */
    void _fadeEventHandler();

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
     */
    void fadelight(int _targetbrightness=0, uint32_t _duration=FADE_TIME);

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
