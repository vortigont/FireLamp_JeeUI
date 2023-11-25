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
#include "filehelpers.hpp"
#include "LList.h"
#include "effects_types.h"
#include "ledfb.hpp"
#include "luma_curves.hpp"

#ifdef MIC_EFFECTS
#include "micFFT.h"
#endif

#include "ts.h"

// Вывод значка микрофона в списке эффектов
#ifdef MIC_EFFECTS
    #define MIC_SYMBOL(N) (pgm_read_byte(T_EFFVER + (uint8_t)N) % 2 ? "" : " \U0001F399")
    //#define MIC_SYMBOL (micSymb ? (pgm_read_byte(T_EFFVER + (uint8_t)eff->eff_nb) % 2 == 0 ? " \U0001F399" : "") : "")
    //#define MIC_SYMB bool micSymb = myLamp.getLampSettings().effHasMic
#else
    #define MIC_SYMBOL(N) ""
    //#define MIC_SYMB
#endif

// Вывод номеров эффектов в списке, в WebUI
#define EFF_NUMBER(N)   N <= 255 ? (String(N) + ". ") : (String((byte)(N & 0xFF)) + "." + String((byte)(N >> 8) - 1U) + " ")
// depend on option to disable numbers in list names 
//#define EFF_NUMBER   (numList ? (eff->eff_nb <= 255 ? (String(eff->eff_nb) + ". ") : (String((byte)(eff->eff_nb & 0xFF)) + "." + String((byte)(eff->eff_nb >> 8) - 1U) + ". ")) : "")



typedef struct {
    union {
        uint32_t flags;
        struct {
            bool isInitCompleted:1; // завершилась ли инициализация лампы
            bool isOptPass:1;       // введен ли пароль для опций
            bool isMicOn:1;
            bool isDebug:1;
            bool isRandDemo:1;

            bool dawnFlag:1; // флаг устанавливается будильником "рассвет"
            bool isStringPrinting:1; // печатается ли прямо сейчас строка?
            bool isEffectsDisabledUntilText:1; // признак отключения эффектов, пока выводится текст
            bool isOffAfterText:1; // признак нужно ли выключать после вывода текста
            bool isCalibrationRequest:1; // находимся ли в режиме калибровки микрофона
            bool isWarning:1; // выводится ли индикация предупреждения
            uint8_t micAnalyseDivider:2; // делитель анализа микрофона 0 - выключен, 1 - каждый раз, 2 - каждый четвертый раз, 3 - каждый восьмой раз
            uint8_t warnType:2; // тип предупреждения 0 - цвет, 1 - цвет + счетчик,  1 - цвет + счетчик обратным цветом,  3 - счетчик цветом
        };
    };
    float speedfactor;
    //uint8_t brightness;

#ifdef MIC_EFFECTS
    float mic_noise = 0.0; // уровень шума в ед.
    float mic_scale = 1.0; // коэф. смещения
    float last_freq = 0.0; // последняя измеренная часота
    float samp_freq = 0.0; // часота семплирования
    float cur_val = 0.0;   // текущее значение
    uint8_t last_max_peak = 0; // последнее максимальное амплитудное значение (по модулю)
    uint8_t last_min_peak = 0; // последнее минимальное амплитудное значение (по модулю)
    mic_noise_reduce_level_t noise_reduce = mic_noise_reduce_level_t::NR_NONE; // уровень шумодава

    float getCurVal() {return cur_val;}
    void setMicAnalyseDivider(uint8_t val) {micAnalyseDivider = val&3;}
    float getMicScale() {return mic_scale;}
    void setMicScale(float scale) {mic_scale = scale;}
    float getMicNoise() {return mic_noise;}
    void setMicNoise(float noise) {mic_noise = noise;}
    void setMicNoiseRdcLevel(mic_noise_reduce_level_t lvl) {noise_reduce = lvl;}
    mic_noise_reduce_level_t getMicNoiseRdcLevel() {return noise_reduce;}
    uint8_t getMicMaxPeak() {return isMicOn?last_max_peak:0;}
    uint8_t getMicMapMaxPeak() {return isMicOn?((last_max_peak>(uint8_t)mic_noise)?(last_max_peak-(uint8_t)mic_noise)*2:1):0;}
    float getMicFreq() {return isMicOn?last_freq:0;}
    uint8_t getMicMapFreq() {
        float minFreq=(log((float)(SAMPLING_FREQ>>1)/MicWorker::samples));
        float scale = 255.0 / (log((float)HIGH_MAP_FREQ) - minFreq); 
        return (uint8_t)(isMicOn?(log(last_freq)-minFreq)*scale:0);
    }
#endif

    uint32_t freeHeap;
    uint8_t HeapFragmentation;
    int32_t rssi;
    uint32_t fsfreespace;

} LAMPSTATE;

typedef union {
    uint8_t mask;
    struct {
        bool canBeSelected:1;
        bool isFavorite:1;
        bool renamed:1;
    };
} EFFFLAGS;

typedef enum : uint8_t {ST_BASE=0,ST_END, ST_IDX, ST_AB, ST_AB2, ST_MIC} SORT_TYPE; // виды сортировки

class UIControl{
private:
    uint8_t id;
    CONTROL_TYPE ctype;
    String control_name;
    String val;
    String min;
    String max;
    String step;
public:
    UIControl(
        uint8_t _id=0,
        CONTROL_TYPE _ctype=CONTROL_TYPE::RANGE,
        const String _name="ctrl",
        const String _val= "128",
        const String _min= "1",
        const String _max= "255",
        const String _step= "1"
        ) : 
        id(_id), ctype(_ctype),
        control_name(_name),
        val(_val), min(_min), max(_max), step(_step) {}

    /**
     * @brief copy constructor
     * 
     */
    UIControl(const UIControl&rhs) : id(rhs.id), ctype(rhs.ctype), control_name(rhs.control_name), val(rhs.val), min(rhs.min), max(rhs.max), step(rhs.step) {};
    UIControl& operator =(const UIControl&rhs);

    const uint8_t getId() {return id;}
    const CONTROL_TYPE getType() {return ctype;}
    const String &getName() {return control_name;}
    const String &getVal() {return val;}
    const String &getMin() {return min;}
    const String &getMax() {return max;}
    const String &getStep() {return step;}

    void setVal(const String &_val);
};

/**
 * @brief effect configuration data structure
 * holds info about effect engine, like name,num
 * controls, etc...
 */
class Effcfg {
    Task *tConfigSave = nullptr;       // динамическая таска, задержки при сохранении текущего конфига эффекта в файл

    /**
     * получить версию эффекта из "прошивки" по его ENUM
     */
    inline static uint8_t geteffcodeversion(uint8_t id) { return pgm_read_byte(T_EFFVER + id); };

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
    bool _eff_cfg_deserialize(DynamicJsonDocument &doc, const char *folder = NULL);

    /**
     * @brief serialize and write struct to json file
     * 
     * @param folder 
     */
    void _savecfg(char *folder=NULL);

    /**
     * @brief load effect controls from JsonDocument to a list
     * 
     * @param effcfg - deserialized JsonDocument with effect config (should come from a file)
     * @param ctrls - destination list to load controls (all existing controls will be cleared)
     * @return true - on success
     * @return false - on error
     */
    bool _eff_ctrls_load_from_jdoc(DynamicJsonDocument &effcfg, LList<std::shared_ptr<UIControl>> &ctrls);

public:
    uint16_t num = 0;       // номер эффекта
    uint8_t version = 0;    // версия эффекта
    EFFFLAGS flags;         // effect flags
    uint8_t brt{0};         // effect's private brightness
    luma::curve curve{luma::curve::cie1931};
    String effectName;      // имя эффекта (предварительно заданное или из конфига)
    String soundfile;       // имя/путь к звуковому файлу (DF Player Mini)
    // список контроллов эффекта
    LList<std::shared_ptr<UIControl>> controls;

    Effcfg(){};
    // constructor loads or creates default configuration for effect with specified ID
    Effcfg(uint16_t effid);
    ~Effcfg();

    // copy not yet implemented
    Effcfg(const Effcfg&) = delete;
    Effcfg& operator=(const Effcfg &) = delete;
    Effcfg(Effcfg &&) = delete;
    Effcfg & operator=(Effcfg &&) = delete;

    /**
     * @brief load effect's configuration from a json file
     * apply saved configuration to current worker instance
     * @param cfg - struct to load data into
     * @param nb - effect number
     * @param folder - folder to look for config files
     * @return int 
     */
    bool loadeffconfig(uint16_t nb, const char *folder=NULL);

    /**
     * @brief create Effect's default configuration json file
     * it (over)writes json file with effect's default configuration
     * 
     * @param nb - eff enum
     * @param filename - filename to write
     */
    static void create_eff_default_cfg_file(uint16_t nb, String &filename);

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
    String getSerializedEffConfig(uint8_t replaceBright = 0) const;

    /**
     * @brief flush pending config data to file on disk
     * it's a temporary workaround method
     * it writes cfg data ONLY if some changes are pending in delayed task
     */
    void flushcfg(){ if(tConfigSave) autosave(true); };
};


class EffectListElem{
private:
    uint8_t ms = micros()|0xFF; // момент создания элемента, для сортировки в порядке следования (естественно, что байта мало, но экономим память)

public:
    uint16_t eff_nb; // номер эффекта, для копий наращиваем старший байт
    EFFFLAGS flags; // флаги эффекта

    EffectListElem(uint16_t nb = 0, uint8_t mask = 0) : eff_nb(nb) { flags.mask = mask; }

    EffectListElem(const EffectListElem *base) {
        eff_nb = ((((base->eff_nb >> 8) + 1 ) << 8 ) | (base->eff_nb&0xFF)); // в старшем байте увеличиваем значение на 1
        flags =base->flags; 
    }

    bool canBeSelected() const { return flags.canBeSelected; }
    void canBeSelected(bool val){ flags.canBeSelected = val; }
    bool isFavorite() const { return flags.isFavorite; }
    void isFavorite(bool val){ flags.isFavorite = val; }
    bool renamed() const { return flags.renamed; }
    void renamed(bool v){ flags.renamed = v; }
    uint8_t getMS() const { return ms; }
};


// forward declaration
class EffectWorker;

/**
 * Базовый класс эффекта с основными переменными и методами общими для всех эффектов
 * методы переопределяются каждым эффектом по необходимости
*/
class EffectCalc {
private:
    //EffectWorker *_pworker = nullptr; // указатель на воркер
    LAMPSTATE *_lampstate = nullptr;
    LList<std::shared_ptr<UIControl>> *ctrls;
    String dummy; // дефолтная затычка для отсутствующего контролла, в случае приведения к целому получится "0"
    bool active = false;          /**< работает ли воркер и был ли обсчет кадров с момента последнего вызова, пока нужно чтобы пропускать холостые кадры */
    bool isCtrlPallete = false; // признак наличия контрола палитры
    bool isMicActive = false; // признак включенного микрофона
    bool isMicOnState() {return _lampstate ? _lampstate->isMicOn : false;}
protected:
    LedFB<CRGB> *fb;          // Framebuffer to work on
    EFF_ENUM effect;        /**< энумератор эффекта */
    bool isDebug() {return _lampstate ? _lampstate->isDebug : false;}
    bool isRandDemo() {return _lampstate ? _lampstate->isRandDemo : false;}
    float getSpeedFactor() {return _lampstate ? _lampstate->speedfactor : 1.0;}
    //float getBrightness() {return _lampstate ? _lampstate->brightness : 127;}

#ifdef MIC_EFFECTS
    void setMicAnalyseDivider(uint8_t val) {if(_lampstate) _lampstate->micAnalyseDivider = val&3;}
    uint8_t getMicMapMaxPeak() {return _lampstate ? _lampstate->getMicMapMaxPeak() : 0;}
    uint8_t getMicMapFreq() {return _lampstate ? _lampstate->getMicMapFreq() : 0;}
    uint8_t getMicMaxPeak() {return _lampstate ? _lampstate->getMicMaxPeak() : 0;}
    
    float getCurVal() {return _lampstate ? _lampstate->getCurVal() : 0;}
    float getMicFreq() {return _lampstate ? _lampstate->getMicFreq() : 0;}
    float getMicScale() {return _lampstate ? _lampstate->getMicScale() : 1;}
    float getMicNoise() {return _lampstate ? _lampstate->getMicNoise() : 0;}
    mic_noise_reduce_level_t getMicNoiseRdcLevel() {return _lampstate ? _lampstate->getMicNoiseRdcLevel() : mic_noise_reduce_level_t::NR_NONE;}
    
#endif
    bool isActive() {return active;}
    void setActive(bool flag) {active=flag;}
    uint32_t lastrun=0;     /**< счетчик времени для эффектов с "задержкой" */
    static constexpr byte brightness{128};    // рудиментная "яркость", должна быть удалена из кода эффектов
    byte speed=1;
    byte scale=1;
    float speedfactor=1.0;      // коэффициент скорости эффекта

    uint8_t palettescale=1.0;     // внутренний масштаб для палитр, т.е. при 22 палитрах на нее будет приходится около 11 пунктов, при 8 палитрах - около 31 пункта
    float ptPallete=1.0;          // сколько пунктов приходится на одну палитру; 255.1 - диапазон ползунка, не включая 255, т.к. растягиваем только нужное :)
    uint8_t palettepos=0;         // позиция в массиве указателей паллитр
    uint8_t paletteIdx=0;         // индекс палитры переданный с UI

    /** флаг, включает использование палитр в эффекте.
     *  влияет на:
     *  - подгрузку дефолтовых палитр при init()
     *  - переключение палитры при изменении ползунка "шкалы"
     */
    bool usepalettes=false;
    std::vector<PGMPalette*> palettes;          /**< набор используемых палитр (пустой)*/
    TProgmemRGBPalette16 const *curPalette = &RainbowColors_p;     /**< указатель на текущую палитру */

    const String &getCtrlVal(unsigned idx);

public:
    EffectCalc(LedFB<CRGB> *framebuffer) : fb(framebuffer) {}

    /**
     * деструктор по-умолчанию
     */
    virtual ~EffectCalc() = default;

    bool isMicOn() {return isMicActive;}

    /**
     * intit метод, вызывается отдельно после создания экземпляра эффекта для установки базовых переменных
     * в конце выполнения вызывает метод load() который может быть переопределен в дочернем классе
     * @param _eff - энумератор эффекта
     * @param _controls - контролы эффекта
     * @param _state - текущее состояние лампы
     *
    */
    void init(EFF_ENUM eff, LList<std::shared_ptr<UIControl>> *controls, LAMPSTATE* state);

    /**
     * load метод, по умолчанию пустой. Вызывается автоматом из init(), в дочернем классе можно заменять на процедуру первой загрузки эффекта (вместо того что выполняется под флагом load)
     *
    */
    virtual void load();

    /**
     * run метод, Вызывается для прохода одного цикла эффекта, можно переопределять либо фунцией обсчета смого эффекта,
     * либо вспомогательной оберткой, вызывающей приватный метод.
     * Метод должет вернуть true если обсчет эффекта был завершен успешно или false если обсчет был пропущен и кадр не менялся
     * @param ledarr - указатель на массив, пока не используется
     * @param opt - опция, пока не используется, вероятно нужно заменить на какую-нибудь расширяемую структуру
    */
    virtual bool run();

    /**
     * drynrun метод, всеми любимая затычка-проверка на "пустой" вызов
     * возвращает false если еще не прошло достаточно времени с EFFECTS_RUN_TIMER
     */
    bool dryrun(float n=1.0, uint8_t delay = EFFECTS_RUN_TIMER);

    /**
     * status - статус воркера, если работает и загружен эффект, отдает true
     */
    virtual bool status();

    /**
     * setDynCtrl - обработка для динамических контролов idx=3+
     * https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/page-112#post-48848
     */
    virtual String setDynCtrl(UIControl*_val);

    /**
     * загрузка дефолтных палитр в массив и установка текущей палитры
     * в соответствие в "бегунком" шкала/R
     */
    virtual void palettesload();

    /**
     * palletemap - меняет указатель на текущую палитру из набора в соответствие с "ползунком"
     * @param _val - байт "ползунка"
     * @param _pals - набор с палитрами
     */
    virtual void palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min=1,  const uint8_t _max=255);

    /**
     * метод выбирает текущую палитру '*curPalette' из набора дотупных палитр 'palettes'
     * в соответствии со значением "бегунка" шкалы. В случае если задана паременная rval -
     * метод использует значение R,  иначе используется значение scale
     * (палитры меняются автоматом при изменении значения шкалы/R, метод оставлен для совместимости
     * и для первоначальной загрузки эффекта)
     */
    void scale2pallete();

};

class EffectWorker {
private:
    LAMPSTATE *lampstate;   // ссылка на состояние лампы
    //LedFB *fb;              // framebuffer to run EffectCalcs
    SORT_TYPE effSort;      // порядок сортировки в UI

    Effcfg curEff;          // конфигурация текущего эффекта, имя/версия и т.п.
    Effcfg pendingEff;      // конфигурация эффекта следующего на очереди переключения (на время работы затухания)
    
    // список эффектов с флагами из индекса
    LList<EffectListElem> effects;

    // указатель на экземпляр класса текущего эффекта
    std::unique_ptr<EffectCalc> worker;

    // effect instance mutex
    std::mutex _mtx;

    volatile bool _status = false;                  // if worker is in active (enabled and running state)
    TaskHandle_t    _runnerTask_h=nullptr;          // effect calculator task

    /**
     * @brief WTF???
     * update LAMP class instance with FileSystem stat info
     * todo: move it to a proper place
     */
    void fsinforenew();

    /**
     * создает и инициализирует экземпляр класса выбранного эффекта
     *
    */
    void workerset(uint16_t effect);

    void effectsReSort(SORT_TYPE st=(SORT_TYPE)(255));

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
    void _load_eff_list_from_idx_file(const char *folder = NULL);

    /**
     * @brief rebuild list of effects based on json configs on filesystem
     * loads a list of default effects from firmware, then apply per effect
     * configs from fs (if present)
     * 
     * @param folder 
     */
    void _rebuild_eff_list(const char *folder = NULL);

    // static wrapper for _runner Task to call handling class member
    static inline void _runnerTask(void* pvParams){ ((EffectWorker*)pvParams)->_runnerHndlr(); }

    // worker Task method that runs periodic Effect calculation
    void _runnerHndlr();

    // start a task that periodically runs effect calculation
    void _start_runner();

public:
    // дефолтный конструктор
    EffectWorker(LAMPSTATE *_lampstate);
    ~EffectWorker(){ if(_runnerTask_h) vTaskDelete(_runnerTask_h); _runnerTask_h = nullptr; };

    // noncopyable
    EffectWorker (const EffectWorker&) = delete;
    EffectWorker& operator= (const EffectWorker&) = delete;

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
     * @brief set a new ledbuffer for worker
     * it will pass it further on effects creation, etc...
     */
   //void setLEDbuffer(LedFB *buff);

    /**
     * @brief Set the Luma Curve value for the current effect configuration
     * 
     * @param c luma curve enum
     */
    void setLumaCurve(luma::curve c);

    // уделение списков из ФС
    void removeLists();

    void initDefault(const char *folder = NULL); // пусть вызывается позже и явно

    /**
     * @brief Get const reference to current Effects List
     * 
     */
    LList<EffectListElem> const &getEffectsList() const { return effects; };

    LList<std::shared_ptr<UIControl>>&getControls() { return isEffSwPending() ? pendingEff.controls : curEff.controls; }

    // тип сортировки
    void setEffSortType(SORT_TYPE type) {if(effSort != type) { effectsReSort(type); } effSort = type;}
    SORT_TYPE getEffSortType() {return effSort;}

    // удалить конфиг переданного эффекта
    void removeConfig(const uint16_t nb, const char *folder=NULL);

    /**
     * @brief пересоздает индекс из текущего списка эффектов
     * 
     * @param folder 
     * @param forceRemove - удалить ВСЕ списки, в.т.ч. списки для выпадающих меню
     */
    void makeIndexFileFromList(const char *folder = NULL, bool forceRemove = false);

    /**
     * @brief Get total number of effects in a list 
     */
    unsigned getEffectsListSize() const {return effects.size();}

    const String &getEffectName() const {return curEff.effectName;}

    // если текущий, то просто пишем имя, если другой - создаем экземпляр, пишем, удаляем
    void setEffectName(const String &name, EffectListElem*to);

    const String &getSoundfile() const {return curEff.soundfile;}

    // если текущий, то просто пишем имя звукового файла, если другой - создаем экземпляр, пишем, удаляем
    void setSoundfile(const String &_soundfile, EffectListElem*to);

    /**
    * вычитать только имя эффекта из конфиг-файла и записать в предоставленную строку
    * в случае отсутствия/повреждения взять имя эффекта из флеш-таблицы, если есть
    * для работы метода не требуется экземпляра класса effectCalc'а
    * @param effectName - String куда записать результат
    * @param nb  - айди эффекта
    * @param folder - какой-то префикс для каталога
    */
    void loadeffname(String& effectName, const uint16_t nb, const char *folder=NULL);

    /**
    * вычитать только имя\путь звука из конфиг-файла и записать в предоставленную строку
    * в случае отсутствия/повреждения возвращает пустую строку
    * @param effectName - String куда записать результат
    * @param nb  - айди эффекта
    * @param folder - какой-то префикс для каталога
    */
    void loadsoundfile(String& effectName, const uint16_t nb, const char *folder=NULL);

    // следующий эффект, кроме canBeSelected==false
    uint16_t getNext();
    // предыдущий эффект, кроме canBeSelected==false
    uint16_t getPrev();

    // перейти на количество шагов, к ближйшему большему (для DEMO)
    void moveByCnt(byte cnt){ switchEffect(getByCnt(cnt)); }

    // получить номер эффекта смещенного на количество шагов (для DEMO)
    uint16_t getByCnt(byte cnt);

    bool validByList(int val);

    // получить реальный номер эффекта по номеру элемента списка (для плагинов)
    uint16_t realEffNumdByList(uint16_t val) { return effects[val].eff_nb; }

    // получить индекс эффекта по номеру (для плагинов)
    uint16_t effIndexByList(uint16_t val);

    // получить флаг canBeSelected по номеру элемента списка (для плагинов)
    bool effCanBeSelected(uint16_t val) { return effects.exist(val) ? effects[val].canBeSelected() : false; }

    // вернуть первый элемент списка
    EffectListElem *getFirstEffect();
    // вернуть следующий эффект
    EffectListElem *getNextEffect(EffectListElem *current);
    // вернуть выбранный элемент списка
    EffectListElem *getEffect(uint16_t select);
    // вернуть номер текущего эффекта
    uint16_t getCurrent() const {return curEff.num; }
    // вернуть номер следущиего эффекта отложенного переключения (на время работы затухания)
    uint16_t getSelected() const { return pendingEff.num; }
    /**
     * @brief  вернуть актуальный номер эффекта
     * в случае если работает затухание, возвращает номер эффекта на очереди
     */
    uint16_t getEffnum() const { return isEffSwPending() ? pendingEff.num : curEff.num; }
    // вернуть текущий элемент списка
    EffectListElem *getCurrentListElement();
    // вернуть выбранный элемент списка
    EffectListElem *getSelectedListElement();

    /**
     * @brief return current effect config object
     */
    Effcfg const &getCurrEffCfg() const { return curEff; }

    /**
     * @brief return pending effect config object
     */
    Effcfg const &getPendingEffCfg() const { return pendingEff; }

    /**
     * @brief return a ref to effect config depending on if switching in pending or not
     * if fade is progress, than a ref to pending config will be returned
     */
    Effcfg const &getEffCfg() const { return isEffSwPending() ? pendingEff : curEff; }

    /**
     * @brief autosave current effect configuration to json file
     * 
     */
    void autoSaveConfig(){ curEff.autosave(); }

    /**
     * @brief switch to the specified effect
     * two-stage switch required for fading effect, first time call will only preload controls for a new effect,
     * second one does the switching
     * 
     * @param effnb - effect to switch to
     * @param twostate - use two staged switching
     */
    void switchEffect(uint16_t effnb, bool twostate = false);

    /**
     * @brief returns true if effect switching is pending for fader
     */
    bool isEffSwPending() const { return (curEff.num != pendingEff.num); }

    // копирование эффекта
    void copyEffect(const EffectListElem *base);
    // удалить эффект
    void deleteEffect(const EffectListElem *eff, bool isCfgRemove = false);


    // COMPAT methods

    /**
     * @brief a wrapper for EffectCalc's setDynCtrl method
     * (exist for compatibility for the time of refactoring control's code)
     */
    String setDynCtrl(UIControl*_val){ return worker ? worker->setDynCtrl(_val) : String(); };  // damn String()

    bool isMicOn(){ return worker ? worker->isMicOn() : false; }


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