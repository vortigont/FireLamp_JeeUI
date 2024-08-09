/*
Copyright © 2023-2024 Emil Muratov (Vortigont)
Copyright © 2020 Dmytro Korniienko (kDn)

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
#include "evtloop.h"

#ifndef DEFAULT_MQTTPUB_INTERVAL
    #define DEFAULT_MQTTPUB_INTERVAL 30
#endif

#define MAX_BRIGHTNESS          255U                    // максимальная яркость LED
#define DEF_BRT_SCALE           20                      // шкала регулировки яркости по-умолчанию
#ifndef FADE_TIME
#define FADE_TIME               2000U                   // Default fade time, ms
#endif

// смена эффекта
enum class effswitch_t : uint8_t {
    none = 0,   // переключить на пустой эффект, он же темнота
    next,       // следующий
    prev,       // предыдущий
    rnd,        // случайный
    num        // переход на конкретный эффект по индексу
};  // ex EFFSWITCH;

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
    bool restoreState:1;            // restore lamp on/off/demo state on restart
    bool pwrState:1;                // флаг включения/выключения
    bool fadeEffects:1;             // признак использования затухания при смене эффектов/яркости
    bool demoMode:1;                // demo state
    bool demoRndOrderSwitching:1;   // переключать эффекты в случайном порядке в Демо
    bool demoRndEffControls:1;      // в демо тассовать настройки эффекта случайным образом
    bool reserved6:1;
    bool wipeOnEffChange:1;         // признак очистки экрана при переходе с одного эффекта на другой
    bool reserved8:1;               // признак режима отладки
    bool reserved9:1;               // случайный порядок демо
    bool reserved10:1;              // отображение имени в демо
    bool isMicOn:1;                 // включение/выключение микрофона
    bool effHasMic:1;               // микрофон для эффекта
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

// this struct keeps volatile flags that are cleared on MCU reset
struct VolatileFlags {
    bool pwrState:1;        // флаг включения/выключения
    bool demoMode:1;        // running demo
    bool isMicOn:1;         // включение/выключение микрофона
    bool debug:1;           // some debug flag
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

    // used auxilary GPIOs 
    struct GPIO_pins {
        // Analog pin for microphone
        int32_t mic{GPIO_NUM_NC};
        // matrix power switch FET
        int32_t fet{GPIO_NUM_NC};
        // some uknown AUX pin
        int32_t aux{GPIO_NUM_NC};
        // active logic levels
        bool fet_ll;
        bool aux_ll;
    };

    // effect switching state
    EffSwitch_state_t _swState;
    GPIO_pins _pins;

    std::shared_ptr<LedFB<uint16_t> > _overlay;     // буфер для оверлея

    // a set of lamp options (flags)
    LampFlagsPack opts;
    VolatileFlags vopts{};
    // текущее состояние лампы, которое передается в класс эффектпроцессора
    LampState lampState;

    uint8_t _brightnessScale{DEF_BRT_SCALE};
    // default luma correction curve for PWM driven LEDs
    luma::curve _curve = luma::curve::cie1931;
    uint8_t globalBrightness{DEF_BRT_SCALE/2};     // глобальная яркость

    uint16_t storedEffect = (uint16_t)EFF_ENUM::EFF_NONE;

    // Microphone
    MicWorker *mw = nullptr;
    void micHandler();

    // Таймер смены эффектов в ДЕМО
    Task *demoTask = nullptr;
    // Demo change period
    uint32_t demoTime{DEFAULT_DEMO_TIMER};



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

    LampState &getLampState(){ return lampState; }

    std::vector<std::shared_ptr<UIControl>>&getEffControls() { return effwrkr.getControls(); }

    void setMicOnOff(bool val);

    // return if Microphone enabled state
    bool getMicState() const { return vopts.isMicOn; }

    void setSpeedFactor(float val) {
        lampState.speedfactor = val;
        // speed is control number 1, so check the size of vector, must be >1
        if ( effwrkr.getControls().size() >1 ) effwrkr.setDynCtrl(effwrkr.getControls().at(1).get());
        //if(effwrkr.getControls().exist(1)) effwrkr.setDynCtrl(effwrkr.getControls()[1].get());
    }

    // Lamp brightness control
    /**
     * @brief - Change global brightness with or without fade effect
     * if fade flag for the lamp is set, than fade applied in non-blocking way unless skipfade param is set to 'true'
     * brightness is mapped to a current lamp's luma curve value
     * 
     * @param uint8_t tgtbrt - target scaled brigtness level 0-255
     * @param fade_t fade - use/skip or use default fade effect
     * @param bool bypass - set brightness value as-is directly to backend device, skipping fader, scaling and do NOT save new value to NVS
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
    void setBrightnessScale(uint8_t scale);

    /**
     * @brief Get the Brightness Scale
     * see setBrightnessScale()
     */
    uint8_t getBrightnessScale() const { return _brightnessScale; };

    /**
     * @brief initiate gradual brightness fade
     * will fade brightness without reporting intermediate values and saving result to NVS
     * could be user for effects during power-on/off, sunrise, dusk, etc...
     * 
     * @param arg 
     */
    void gradualFade(evt::gradual_fade_t arg);


    // Loop cycle
    void handle();

    // === flag get/set methods ===

    // возвращает упакованные в целое флаги лампы
    uint32_t getLampFlags() {return opts.pack; }
    // возвращает структуру СОХРАНЁННЫХ флагов лампы
    const LampFlags &getLampFlagsStuct() const {return opts.flag; }

    // saves flags to NVS
    void save_flags();
    void setFaderFlag(bool flag) {opts.flag.fadeEffects = flag; save_flags(); }
    bool getFaderFlag() const { return opts.flag.fadeEffects; }
    void setClearingFlag(bool flag) {opts.flag.wipeOnEffChange = flag; save_flags(); }
    bool getClearingFlag() const {return opts.flag.wipeOnEffChange; }

    bool isLampOn() {return vopts.pwrState;}

    // get lamp's power state
    bool getPwr() const { return vopts.pwrState; }

    bool isDebugOn() {return vopts.debug;}
    bool isDebug() {return lampState.isDebug;}
    void setDebug(bool flag) { vopts.debug = flag; lampState.isDebug=flag; }

    // set/clear "restore on/off/demo" state on boot
    void setRestoreState(bool flag){ opts.flag.restoreState = flag; save_flags(); }


    // Drawing methods
    bool isDrawOn() const { return _overlay.get(); }

    // activate/disable overlay for drawing
    //void enableDrawing(bool state){ _overlay_buffer(state); };

    // draw a pixel in overlay
    //void writeDrawBuf(CRGB color, uint16_t x, uint16_t y);
    //void writeDrawBuf(CRGB color, uint16_t num);

    /**
     * @brief fill DrawBuffer with solid color
     * 
     * @param color 
     */
    //void fillDrawBuf(CRGB color);

    /**
     * @brief fill DrawBuf with solid black (make it transparent)
     * 
     */
    //void clearDrawBuf() { CRGB c = CRGB::Black; fillDrawBuf(c); }

    /**
     * @brief enable/disable demo mode
     * 
     * @param avtive 
     */
    void demoMode(bool active);

    // return demo mode status
    bool getDemoMode() const { return vopts.demoMode; }

    // reset demo timer
    void demoReset(){ if (demoTask) demoTask->restartDelayed(); }

    /**
     * @brief switch to next effect in demo mode
     * 
     */
    void demoNext();

    /**
     * @brief Set the Demo Timer period
     * 
     * @param seconds 
     */
    void setDemoTime(uint32_t seconds);

    // get demo period
    uint32_t getDemoTime() const { return demoTime; }

    /**
     * @brief set random order when switching effects in Demo mode
     * 
     * @param flag 
     */
    void setDemoRndSwitch(bool flag){ opts.flag.demoRndOrderSwitching = flag; }

    /**
     * @brief shuffle effect controls randomly when in demo mode
     * 
     * @param v 
     */
    void setDemoRndEffControls(bool v){ opts.flag.demoRndEffControls = lampState.demoRndEffControls = v; };


    void setEffHasMic(bool flag){ opts.flag.effHasMic = flag; }


    // ---------- служебные функции -------------

    /**
     * @brief вкл./выкл лампу
     * логическое включение/выключение лампы
     * generates events:
     *  lampEvtId_t::pwron
     *  lampEvtId_t::pwroff
     * 
     */
    void power(bool pwr, bool restore_state = true);

    /**
     * @brief toggle logical power state for the lamp
     * generates events:
     *  lampEvtId_t::pwron
     *  lampEvtId_t::pwroff
     * 
     */
    void power(){ power(!vopts.pwrState); };

    /**
     * @brief общий переключатель эффектов лампы
     * в зависимости от режима лампы может переключать эффекты с затуханием или без
     * @param action - тип переключения на эффект, предыдущий, следующий, конкретный и т.п.
     * @param effnb - опциональный параметр номер переключаемого эффекта
     */
    void switcheffect(effswitch_t action, uint16_t effnb = EFF_ENUM::EFF_NONE);

    /*
     * включает/выключает "эффект"-таймер
     * @param SCHEDULER action - enable/disable/reset
     */
    void effectsTimer(bool action);

private:
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

    /**
     * @brief event picker method, processes incoming get events from a event_hndlr wrapper
     * 
     * @param base 
     * @param id 
     * @param event_data 
     */
    void _event_picker_get(esp_event_base_t base, int32_t id, void* data);

};

/**
 * @brief asynchronously fade lamp's brightness
 * 
 */
class LEDFader {
    Lamp *lmp = nullptr;
    int16_t _brt;                               // transient brightness
    int16_t  _tgtbrt{0};                        // target brightness
    int16_t _brtincrement;                      // change step
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
     * @param uint8_t _targetbrightness - scaled end value for the brighness to fade to
     * @param uint32_t _duration - fade effect duraion, ms
     */
    void fadelight(int targetbrightness, uint32_t duration = FADE_TIME);

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
