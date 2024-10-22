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
//#include "filehelpers.hpp"
#include "effects_types.h"
#include "ledfb.hpp"
#include "luma_curves.hpp"
#include "ts.h"



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
    bool _locked{false};                       // config is locked, no changes possible

    Task *tConfigSave = nullptr;        // динамическая таска, задержки при сохранении текущего конфига эффекта в файл

    // Effect's controls
    std::vector<EffectControl> _controls;
    // index for control presets
    size_t _profile_idx{0};


    /**
     * @brief deserialise effect configuration from a file based on eff number
     * if file is missing/damaged or it's versions is older than firmware's default
     * it will be reset to defaults
     * 
     * @param nb - effect number
     * @param folder - folder to load effects from, must be absolute path with leading/trailing slashes, default is '/eff/'
     * @param jdoc - document to place deserialized obj
     * @return true - on success
     * @return false - on failure
     */
    //bool _eff_cfg_deserialize(JsonDocument &doc, const char *folder = NULL);

    /**
     * @brief serialize and write struct to json file
     * 
     * @param folder 
     */
    void _savecfg(char *folder=NULL);

    /**
     * @brief load effect controls from JsonDocument to a vector
     * 
     * @param effcfg - deserialized JsonDocument with effect config (should come from a file)
     * @param ctrls - destination list to load controls (all existing controls will be cleared)
     * @return true - on success
     * @return false - on error
     */
    //bool _eff_ctrls_load_from_jdoc(JsonDocument &effcfg, std::vector<std::shared_ptr<EffectControl>> &ctrls);

    /**
     * @brief load controls configuration from manifest file
     * 
     * @return true on success 
     * @return false on error
     */
    bool _load_manifest();

    /**
     * @brief load control's values for specified preset from file
     * 
     * @param seq - profile to load, if <0 then load last used from config
     */
    void _load_preset(int seq = -1);

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
     * @brief lock the configuration
     * no changes are possible for controls while it is locked
     * loadEffconfig() call will reset and unlocks the controls
     * 
     */
    void lock();

    // unlock configuration
    void unlock(){ _locked = false; };

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
     * @brief create Effect's default configuration json file
     * it (over)writes json file with effect's default configuration
     * 
     * @param nb - eff enum
     * @param filename - filename to write
     */
    //static void create_eff_default_cfg_file(effect_t nb, String &filename);

    /**
     * @brief write configuration to json file on FS
     * it used delayed save (CFG_AUTOSAVE_TIMEOUT) to reduce writing to flash
     * @param force write immidiately, wo delay
     */
    void autosave(bool force = false);

    /**
     * @brief Get the json string with Serialized Eff Config object
     * 
     * @param nb 
     * @param replaceBright 
     * @return String 
     */
    //String getSerializedEffConfig(uint8_t replaceBright = 0) const;

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

};


// forward declaration
class EffectWorker;

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

    /**
     * @brief rebuild list of effects based on json configs on filesystem
     * loads a list of default effects from firmware, then apply per effect
     * configs from fs (if present)
     * 
     * @param folder 
     */
    //void _rebuild_eff_list(const char *folder = NULL);

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
    ~EffectWorker(){ if(_runnerTask_h) vTaskDelete(_runnerTask_h); _runnerTask_h = nullptr; };

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

    // уделение списков из ФС
    void removeLists();

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

    //std::vector<std::shared_ptr<UIControl>>&getControls() { return curEff.controls; }

    // удалить конфиг переданного эффекта
    void removeConfig(const uint16_t nb, const char *folder=NULL);

    /**
     * @brief создает json индекс файл на ФС из текущего списка эффектов
     * 
     * @param forceRemove - удалить ВСЕ списки, в.т.ч. списки для выпадающих меню
     */
    void makeIndexFileFromList(bool forceRemove = false);

    /**
     * @brief Get total number of effects in a list 
     */
    size_t getEffectsListSize() const {return effects.size();}

    //const String &getEffectName() const {return curEff.effectName;}

    // если текущий, то просто пишем имя, если другой - создаем экземпляр, пишем, удаляем
    //void setEffectName(const String &name, EffectsListItem_t*to){};

    /**
    * вычитать только имя эффекта из конфиг-файла и записать в предоставленную строку
    * в случае отсутствия/повреждения взять имя эффекта из флеш-таблицы, если есть
    * для работы метода не требуется экземпляра класса effectCalc'а
    * @param effectName - String куда записать результат
    * @param nb  - айди эффекта
    * @param folder - какой-то префикс для каталога
    */
    void loadeffname(String& effectName, const uint16_t nb, const char *folder=NULL);

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


    // вернуть первый элемент списка
    //EffectsListItem_t *getFirstEffect();
    // вернуть следующий эффект
    //EffectsListItem_t *getNextEffect(EffectsListItem_t *current);
    // вернуть выбранный элемент списка
    //EffectsListItem_t *getEffect(effect_t select);

    // вернуть номер текущего эффекта
    effect_t getCurrentEffectNumber() const { return _effItem.eid; }

    // вернуть текущий элемент списка
    //EffectsListItem_t *getCurrentListElement();

    // вернуть выбранный элемент списка
    //EffectsListItem_t *getSelectedListElement();

    /**
     * @brief return current effect config object
     */
    //Effcfg const &getCurrEffCfg() const { return curEff; }

    /**
     * @brief return a ref to effect config depending on if switching in pending or not
     * if fade is progress, than a ref to pending config will be returned
     */
    //Effcfg const &getEffCfg() const { return curEff; }

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

    // копирование эффекта
    void copyEffect(const EffectsListItem_t *base){};


    /**
     * @brief удалить эффект  или из списка выбора или конфиг эффекта с ФС
     * 
     * @param eff 
     * @param onlyCfgFile - удалить только конфиг файл с ФС (сбрасывает настройки эффекта на дефолтные)
     */
    void deleteEffect(const EffectsListItem_t *eff, bool onlyCfgFile = false){};


    // COMPAT methods

    /**
     * @brief a wrapper for EffectCalc's setDynCtrl method
     * (exist for compatibility for the time of refactoring control's code)
     */
    //String setDynCtrl(UIControl*_val){ return worker ? worker->setDynCtrl(_val) : String(); };  // damn String()


};

/**
 * @brief creates a json file with a list of effects names
 * list files are fetched from WebUI to create dropdown lists
 * on a main page and in "effects configuration" page
 * 
 * @param w reference to current Effects worker object
 * @param full - if true, build full list of all efects (/eff_fulllist.json), used in "effects configuration" page,
 *                 otherwise build (/eff_list.json) a list of only those effects that are not "hidden", used on a main page
 */
void build_eff_names_list_file(EffectWorker &w, bool full = false);