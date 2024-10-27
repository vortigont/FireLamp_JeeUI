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

#include "freertos/FreeRTOS.h"
#include <mutex>
#include "effects_types.h"
#include "ledfb.hpp"
#include "luma_curves.hpp"
#include "ArduinoJson.h"
#include "EmbUI.h"



/**
 * @brief effect flags
 * denotes different markers for effect, like if it is hidden, it's features, etc...
 * 
 */
struct eff_flags_t {
    bool hidden:1;              // скрыт из списка выбора эффектов, не может быть запущен
    bool disabledInDemo:1;      // не доступен в демо-режиме
};

struct EffectsListItem_t {
    effect_t eid;               // effect's enumerator
    eff_flags_t flags;          // флаги эффекта
    luma::curve curve;

    EffectsListItem_t(effect_t nb = effect_t::empty, eff_flags_t flags = {}, luma::curve curve = luma::curve::cie1931) : eid(nb), flags(flags), curve(curve) {}

    // return in-firmware effect's label based on enum index
    const char* getLbl() const { return getLbl(eid); };
    // return in-firmware effect's label based on enum index
    static const char* getLbl(effect_t eid);
};

class EffectControl {

    const size_t _idx;
    String _name;
    int32_t _val, _minv, _maxv, _scale_min, _scale_max;

public:
    EffectControl(
        size_t idx,
        const char* name,
        int32_t val,
        int32_t min,
        int32_t max,
        int32_t scale_min,
        int32_t scale_max
        );

    /**
     * @brief set value for the control according to scale and min/max mapping
     * value would be limited withing scale range and mapped to _minv, _maxv range
     * returns Scaled Value (same as getScaledVal())
     * @param v 
     * @return int32_t 
     */
    int32_t setVal(int32_t v);

    /**
     * @brief returns clamped raw value
     * 
     * @return int32_t 
     */
    int32_t getVal() const { return _val; }

    /**
     * @brief returns Scaled Value
     * 
     * @return int32_t 
     */
    int32_t getScaledVal() const;

    size_t getIdx() const {return _idx;};
    const char* getName() const { return _name.c_str(); };
};

/**
 * @brief effect configuration class
 * it respond for loading/saving effect's presets from json files on FS
 */
class EffConfiguration {

    effect_t _eid;                      // энумератор эффекта
    bool _locked{false};                // config is locked, no setValue possible
    int32_t _preset_idx{0};             // profile index
    size_t _presets_total{1};           // max number of presets in saved json file
    String _profile_lbl;                // profile's name label
    Task *tConfigSave = nullptr;        // динамическая таска, задержки при сохранении текущего конфига эффекта в файл

    // Effect's controls
    std::vector<EffectControl> _controls;

    DeserializationError _load_cfg(JsonDocument& doc);

    /**
     * @brief serialize and write struct to json file
     * 
     * @param folder 
     */
    void _savecfg();
    void _savecfg(JsonVariant doc);

    void _switchPreset(int32_t idx, JsonVariant doc);

    /**
     * @brief load controls configuration from manifest file
     * 
     * @return true on success 
     * @return false on error
     */
    bool _load_manifest();

    /**
     * @brief lock the configuration
     * no changes are possible for controls while it is locked
     * loadEffconfig() call will reset and unlocks the controls
     * 
     */
    void _lock();

    // unlock configuration
    void _unlock(){ _locked = false; };

    void _jscall_preset_list_rebuild(Interface *interf);

public:

    // constructor loads or creates default configuration for effect with specified ID
    EffConfiguration() : _eid(effect_t::empty) {};
    EffConfiguration(effect_t effid);
    ~EffConfiguration();

    // copy not yet implemented
    EffConfiguration(const EffConfiguration&) = delete;
    EffConfiguration& operator=(const EffConfiguration &) = delete;
    EffConfiguration(EffConfiguration &&) = delete;
    EffConfiguration & operator=(EffConfiguration &&) = delete;

    // return lock state
    bool locked() const { return _locked; }


    /**
     * @brief load effect's configuration from a json file
     * apply saved configuration to current worker instance
     * @param cfg - struct to load data into
     * @param nb - effect number
     * @param folder - folder to look for config files
     * @return int 
     */
    bool loadEffconfig(effect_t effid);

    /**
     * @brief switch to specific profile number
     * loads profile config from file, if specified argument is <0 or wrong, loads last used profile
     * @param keepvalues - if true, then keep control values just change the preset number
     */
    void switchPreset(int32_t idx = -1);

    /**
     * @brief write configuration to json file on FS
     * it used delayed save (CFG_AUTOSAVE_TIMEOUT) to reduce writing to flash
     * @param force write immidiately, wo delay
     */
    void autosave(bool force = false);

    /**
     * @brief flush pending config data to file on disk
     * it's a temporary workaround method
     * it writes cfg data ONLY if some changes are pending in delayed task
     */
    void flushcfg(){ if(tConfigSave) autosave(true); };

    /**
     * @brief Set the value for control undex index
     * 
     * @param idx 
     * @param v 
     * @return return scaled value, or -1 for any non-existing controls
     */
    int32_t setValue(size_t idx, int32_t v);

    /**
     * @brief Get control's Value by index
     * 
     * @param idx 
     * @return int32_t 
     */
    int32_t getValue(size_t idx) const;

    // get access to controls container
    const std::vector<EffectControl> &getControls() const { return _controls; }

    /**
     * @brief Construct an EmbUI section with effect's controls
     * it generates an overriding section that must be placed on "Effects" page
     */
    void mkEmbUIpage(Interface *interf);

    /**
     * @brief post current control values to EmbUI feeders
     * 
     * @param interf 
     */
    void embui_control_vals(Interface *interf);

    /**
     * @brief action handler that renames current preset
     * 
     */
    void embui_preset_rename(Interface *interf, JsonObjectConst data, const char* action);

    /**
     * @brief action handler that clones current preset into new one
     * 
     */
    void embui_preset_clone(Interface *interf);

    void embui_preset_delete(Interface *interf);

};



/**
 * Базовый класс эффекта с основными переменными и методами общими для всех эффектов
 * методы переопределяются каждым эффектом по необходимости
*/
class EffectCalc {
private:


protected:
    LedFB<CRGB> *fb;          // Framebuffer to work on
    // a flag that indicates that effect is using framebuffer memory to keep data between frame calculation, it must persist unmodified accross runs
    const bool _canvasProtect;
    
    uint32_t lastrun{0};         /**< счетчик времени для эффектов с "задержкой" */

    int32_t speed{1}, scale{1};
    // inheritable effect speedfactor variable
    float speedFactor{1.0};

    /**< набор используемых палитр*/
    std::vector<const TProgmemRGBPalette16*> palettes;
    /**< указатель на текущую палитру */
    TProgmemRGBPalette16 const *curPalette = &RainbowColors_p;


public:
    EffectCalc(LedFB<CRGB> *framebuffer, bool canvasProtect = false) : fb(framebuffer), _canvasProtect(canvasProtect) {}

    /**
     * деструктор по-умолчанию
     */
    virtual ~EffectCalc() = default;


    /**
     * load метод, по умолчанию пустой. Вызывается автоматом из init(), в дочернем классе можно заменять на процедуру первой загрузки эффекта (вместо того что выполняется под флагом load)
     *
    */
    virtual void load(){};

    /**
     * run метод, Вызывается для прохода одного цикла эффекта, можно переопределять либо фунцией обсчета смого эффекта,
     * либо вспомогательной оберткой, вызывающей приватный метод.
     * Метод должет вернуть true если обсчет эффекта был завершен успешно или false если обсчет был пропущен и кадр не менялся
     * @param ledarr - указатель на массив, пока не используется
     * @param opt - опция, пока не используется, вероятно нужно заменить на какую-нибудь расширяемую структуру
    */
    virtual bool run() = 0;

    /**
     * drynrun метод, всеми любимая затычка-проверка на "пустой" вызов
     * возвращает false если еще не прошло достаточно времени с EFFECTS_RUN_TIMER
     */
    bool dryrun(float n=1.0, uint8_t delay = EFFECTS_RUN_TIMER);

    /**
     * status - статус воркера, если работает и загружен эффект, отдает true
     */
    //virtual bool status();

    /**
     * загрузка дефолтных палитр в массив и установка текущей палитры
     * 
     */
    virtual void palettesload();

    // returns a flag that indicates that efefct is using framebuffer memory to keep data between frame calculation
    bool getCanvasProtect() const { return _canvasProtect; }

    /**
     * @brief Set the Control's value
     * 
     * @param idx control's index
     * @param value control's value
     */
    virtual void setControl(size_t idx, int32_t value);
};



class EffectWorker {
private:
    EffectsListItem_t _effItem;        // current effect item and flags
    EffConfiguration _effCfg;          // конфигурация текущего эффекта
    
    // список эффектов с флагами из индекса
    std::vector<EffectsListItem_t> effects;

    // указатель на экземпляр класса текущего эффекта
    std::unique_ptr<EffectCalc> worker;

    // effect instance mutex
    std::mutex _mtx;

    volatile bool _status = false;                  // if worker is in active (enabled and running state)
    TaskHandle_t    _runnerTask_h=nullptr;          // effect calculator task

    /**
     * создает и инициализирует экземпляр класса требуемого эффекта
     *
    */
    void _spawn(effect_t eid);

    /**
     * @brief load a list of default effects from firmware tables
     * it loads an 'effects' list with default flags
     * used when no on-flash index file is present 
     */
    void _load_default_fweff_list();

    /**
     * @brief try to load effects list from index file on fs
     * loads an 'effects' list from 'eff_index.json' file
     * if file is missing or corrupted, loads default list from firmware
     * 
     * @param folder - where to look for idx file, must end with a '/'
     */
    void _load_eff_list_from_idx_file();

    // static wrapper for _runner Task to call handling class member
    static inline void _runnerTask(void* pvParams){ ((EffectWorker*)pvParams)->_runnerHndlr(); }

    // worker Task method that runs periodic Effect calculation
    void _runnerHndlr();

    // start a task that periodically runs effect calculation
    void _start_runner();

    // updates _effItem to match eid copy in a list
    void _switch_current_effect_item(effect_t eid);


public:
    // дефолтный конструктор
    EffectWorker();
    ~EffectWorker();

    // noncopyable
    EffectWorker (const EffectWorker&) = delete;
    EffectWorker& operator= (const EffectWorker&) = delete;

    /**
     * @brief loads effect index
     * enumerates effects in firmware and merge parameters from an index on FS
     * this method allows to always have a fresh copy of effects despite if FW and index file are out of sync
     * @note should be called early after calss instantiantion buf after FS is awailable
     */
    void loadIndex();

    // start effect calculator and renderer
    void start();

    // stop effect calculator and release resources
    void stop();

    /**
     * @brief reset worker by recreating current EffectCalc instance
     * it will destroy and recreate effect instance and reload it's config
     * effectively reloading it
     * 
     */
    void reset();

    /**
     * @brief worker status
     * 
     * @return true if workier is running
     * @return false if worker is idle
     */
    bool status() const { return _status; };

    /**
     * @brief Set the Luma Curve value for the current effect configuration
     * 
     * @param c luma curve enum
     */
    void setLumaCurve(luma::curve c);

    /**
     * @brief apply all controls to the current effect
     * should be called after loading new worker/control configs
     */
    void applyControls();

    /**
     * @brief Set the Control Value object
     * 
     * @param idx 
     * @param v 
     */
    void setControlValue(size_t idx, int32_t v);

    /**
     * @brief Get a reference to current effect element
     * 
     * @return const EffectsListItem_t& 
     */
    const EffectsListItem_t& getCurrentEffectItem() const { return _effItem; }

    /**
     * @brief access effects controls container
     * needed to build conrol values for WebUI
     * 
     * @return const std::vector<EffectControl>& 
     */
    const std::vector<EffectControl> &getEffControls() const { return _effCfg.getControls(); }

    /**
     * @brief Get const reference to current Effects List
     * 
     */
    std::vector<EffectsListItem_t> const &getEffectsList() const { return effects; };

    /**
     * @brief создает json индекс файл на ФС из текущего списка эффектов
     * 
     */
    void makeIndexFileFromList();

    /**
     * @brief Get total number of effects in a list 
     */
    size_t getEffectsListSize() const {return effects.size();}

    // следующий эффект, кроме enabled==false
    effect_t getNext();
    // предыдущий эффект, кроме enabled==false
    effect_t getPrev();

    /**
     * @brief найти следующий номер эффекта для демо режима
     * 
     * @return uint16_t 
     */
    effect_t getNextEffIndexForDemo(bool rnd = false);

    // вернуть номер текущего эффекта
    effect_t getCurrentEffectNumber() const { return _effItem.eid; }

    /**
     * @brief autosave current effect configuration to json file
     * 
     */
    void autoSaveConfig(){ _effCfg.autosave(); }

    /**
     * @brief switch to the specified effect
     * two-stage switch required for fading effect, first time call will only preload controls for a new effect,
     * second one does the switching
     * 
     * @param effnb - effect to switch to
     */
    void switchEffect(effect_t eid);

    // switch current effect's preset
    void switchEffectPreset(int32_t preset);

    /**
     * @brief Get the Serialized Controls for current effect
     * 
     * @param obj object to add control k:v pairs
     */
    //void getSerializedControls(JsonObject obj){ _effCfg.makeJson(obj); };

    /**
     * @brief Construct an EmbUI section with effect's controls
     * it generates an overriding section that must be placed on "Effects" page
     */
    void mkEmbUIpage(Interface *interf){ _effCfg.mkEmbUIpage(interf); };

};
