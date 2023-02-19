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

#ifndef __EFFECTWORKER_H
#define __EFFECTWORKER_H

#include "misc.h"
#include "LList.h"
#include <ArduinoJson.h>
#include "effects_types.h"
#include <LittleFS.h>

#ifdef MIC_EFFECTS
#include "micFFT.h"
#endif

#include "ts.h"
// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

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
    uint8_t brightness;

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
        const String _val="128",
        const String _min="1",
        const String _max="255",
        const String _step="1"
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



class EffectListElem{
private:
    uint8_t ms = micros()|0xFF; // момент создания элемента, для сортировки в порядке следования (естественно, что байта мало, но экономим память)

public:
    uint16_t eff_nb; // номер эффекта, для копий наращиваем старший байт
    EFFFLAGS flags; // флаги эффекта

    EffectListElem(uint16_t nb, uint8_t mask) : eff_nb(nb) { flags.mask = mask; }

    EffectListElem(const EffectListElem *base) {
        eff_nb = ((((base->eff_nb >> 8) + 1 ) << 8 ) | (base->eff_nb&0xFF)); // в старшем байте увеличиваем значение на 1
        flags =base->flags; 
    }

    bool canBeSelected(){ return flags.canBeSelected; }
    void canBeSelected(bool val){ flags.canBeSelected = val; }
    bool isFavorite(){ return flags.isFavorite; }
    void isFavorite(bool val){ flags.isFavorite = val; }
    uint8_t getMS(){ return ms; }
};


// forward declaration
class EffectWorker;

//! Basic Effect Calc class
/**
 * Базовый класс эффекта с основными переменными и методами общими для всех эффектов
 * методы переопределяются каждым эффектом по необходимости
*/
class EffectCalc {
private:
    EffectWorker *pworker = nullptr; // указатель на воркер
    LAMPSTATE *lampstate = nullptr;
    LList<std::shared_ptr<UIControl>> *ctrls;
    String dummy; // дефолтная затычка для отсутствующего контролла, в случае приведения к целому получится "0"
    bool active = false;          /**< работает ли воркер и был ли обсчет кадров с момента последнего вызова, пока нужно чтобы пропускать холостые кадры */
    bool isCtrlPallete = false; // признак наличия контрола палитры
    bool isMicActive = false; // признак включенного микрофона
    bool isMicOnState() {return lampstate!=nullptr ? lampstate->isMicOn : false;}
protected:
    EFF_ENUM effect;        /**< энумератор эффекта */
    bool isDebug() {return lampstate!=nullptr ? lampstate->isDebug : false;}
    bool isRandDemo() {return lampstate!=nullptr ? lampstate->isRandDemo : false;}
    float getSpeedFactor() {return lampstate!=nullptr ? lampstate->speedfactor : 1.0;}
    float getBrightness() {return lampstate!=nullptr ? lampstate->brightness : 127;}

#ifdef MIC_EFFECTS
    void setMicAnalyseDivider(uint8_t val) {if(lampstate!=nullptr) lampstate->micAnalyseDivider = val&3;}
    uint8_t getMicMapMaxPeak() {return lampstate!=nullptr ? lampstate->getMicMapMaxPeak() : 0;}
    uint8_t getMicMapFreq() {return lampstate!=nullptr ? lampstate->getMicMapFreq() : 0;}
    uint8_t getMicMaxPeak() {return lampstate!=nullptr ? lampstate->getMicMaxPeak() : 0;}
    
    float getCurVal() {return lampstate!=nullptr ? lampstate->getCurVal() : 0;}
    float getMicFreq() {return lampstate!=nullptr ? lampstate->getMicFreq() : 0;}
    float getMicScale() {return lampstate!=nullptr ? lampstate->getMicScale() : 0;}
    float getMicNoise() {return lampstate!=nullptr ? lampstate->getMicNoise() : 0;}
    mic_noise_reduce_level_t getMicNoiseRdcLevel() {return lampstate!=nullptr ? lampstate->getMicNoiseRdcLevel() : mic_noise_reduce_level_t::NR_NONE;}
    
#endif
    bool isActive() {return active;}
    void setActive(bool flag) {active=flag;}
    uint32_t lastrun=0;     /**< счетчик времени для эффектов с "задержкой" */
    byte brightness=1;
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
    /**
     * деструктор по-умолчанию пустой, может быть переопределен
     */
    virtual ~EffectCalc() = default;
    //virtual ~EffectCalc(){ LOG(println,PSTR("DEGUG: Effect was destroyed\n")); } // отладка, можно будет затем закомментировать

    bool isMicOn() {return isMicActive;}

    /** полезные обертки **/
    static uint8_t wrapX(int8_t x){ return (x + WIDTH) % WIDTH; }
    static uint8_t wrapY(int8_t y){ return (y + HEIGHT) % HEIGHT; }

    /**
     * pre_init метод, вызывается отдельно после создания экземпляра эффекта до каких либо иных инициализаций
     * это нужно чтобы объект понимал кто он и возможно было вычитать конфиг для мультиэфектов, никаких иных действий здесь не предполагается
    */
    void pre_init(EFF_ENUM _eff, EffectWorker *_pworker, LList<std::shared_ptr<UIControl>> *_ctrls, LAMPSTATE* _state) {effect = _eff; pworker = _pworker; ctrls = _ctrls; lampstate = _state;}

    /**
     * intit метод, вызывается отдельно после создания экземпляра эффекта для установки базовых переменных
     * в конце выполнения вызывает метод load() который может быть переопределен в дочернем классе
     * @param _eff - энумератор эффекта
     * @param _controls - контролы эффекта
     * @param _state - текущее состояние лампы
     *
    */
    void init(EFF_ENUM _eff, LList<std::shared_ptr<UIControl>> *_controls, LAMPSTATE* _state);

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
    virtual bool run(CRGB* ledarr, EffectWorker *opt=nullptr);

    /**
     * drynrun метод, всеми любимая затычка-проверка на "пустой" вызов
     * возвращает false если еще не прошло достаточно времени с EFFECTS_RUN_TIMER
     */
    bool dryrun(float n=1.0, uint8_t delay = EFFECTS_RUN_TIMER);

    /**
     * status - статус воркера, если работает и загружен эффект, отдает true
     */
    virtual bool status();

    ///
    /// следующие методы дублируют устранку "яркости", "скорости", "шкалы" для эффекта.
    /// Сейчас это не используется, но соображения "за" следующие:
    ///  - эффекты можно программить со своими локальными переменными, не дергая конкретный
    ///    экземпляр myLamp.effects.getXXX
    ///  - эффекты могут по необходимости масштабировать параметры из байта в свою размерность, или можно расширить базовый класс
    ///  - эфекты могут переопределять методы установки параметров и корректировать их с учетом микрофона, например
    ///


    // /**
    //  * setBrt - установка яркости для воркера
    //  */
    // virtual void setbrt(const byte _brt);

    // /**
    //  * setSpd - установка скорости для воркера
    //  */
    // virtual void setspd(const byte _spd);

    // /**
    //  * setBrt - установка шкалы для воркера
    //  */
    // virtual void setscl(const byte _scl);

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
    //time_t listsuffix = 0; // суффикс используемый для обновления списков
    LAMPSTATE *lampstate; // ссылка на состояние лампы
    SORT_TYPE effSort; // порядок сортировки в UI

    uint16_t curEff = (uint16_t)EFF_NONE;     ///< энумератор текущего эффекта
    uint16_t selEff = (uint16_t)EFF_NONE;     ///< энумератор выбранного эффекта (для отложенного перехода)
    
    String originalName;    // имя эффекта дефолтное
    String effectName;      // имя эффекта (предварительно заданное или из конфига)
    String soundfile;       // имя/путь к звуковому файлу (DF Player Mini)
    uint8_t version;        // версия эффекта

    LList<EffectListElem*> effects; // список эффектов с флагами из индекса
    // список контроллов текущего эффекта
    LList<std::shared_ptr<UIControl>> controls;
    // список контроллов следующего эффекта (используется на время работы фейдера)
    LList<std::shared_ptr<UIControl>> selcontrols;

    Task *tConfigSave = nullptr;       // динамическая таска, задержки при сохранении текущего конфига эффекта в файл

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
    void workerset(uint16_t effect, const bool isCfgProceed = true);

    EffectWorker(const EffectWorker&);  // noncopyable
    EffectWorker& operator=(const EffectWorker&);  // noncopyable

    void clearEffectList(); // очистка списка эффектов, вызываетсяч в initDefault

    /**
     * @brief очистка списка контроллов
     * 
     * @param list list to clear
     */
    void _clearControlsList(LList<std::shared_ptr<UIControl>> &list);

    void effectsReSort(SORT_TYPE st=(SORT_TYPE)(255));

    /**
     * @brief load effect's configuration from a json file
     * apply saved configuration to current worker instance
     * @param nb - effect number
     * @param folder - folder to look for config files
     * @return int 
     */
    bool loadeffconfig(const uint16_t nb, const char *folder=NULL);

    /**
     * @brief получение пути и имени файла конфига эффекта по его номеру
     * 
     * @param nb - номер эффекта
     * @param folder -  абсолютный путь к каталогу с конфигами, должен начинаться и заканчиваться '/', по-умолчанию испльзуется '/eff/'
     * @return const String - полный путь до файла с конфигом
     */
    const String geteffectpathname(const uint16_t nb, const char *folder=NULL) const;

    /**
     * проверка на существование "дефолтных" конфигов для всех статичных эффектов
     *
     */
    void chckdefconfigs(const char *folder);

    void savedefaulteffconfig(uint16_t nb, String &filename);
    void saveeffconfig(uint16_t nb, char *folder=NULL);
    void makeIndexFile(const char *folder = NULL);
    // создать или обновить текущий индекс эффекта
    void updateIndexFile();
    // удалить эффект из индексного файла
    void deleteFromIndexFile(const uint16_t effect);

    /**
     * получить версию эффекта из "прошивки" по его ENUM
     */
    const uint8_t geteffcodeversion(const uint8_t id);

    /**
     *  метод загружает и пробует десериализовать джейсон из файла в предоставленный документ,
     *  возвращает true если загрузка и десериализация прошла успешно
     *  @param doc - DynamicJsonDocument куда будет загружен джейсон
     *  @param jsonfile - файл, для загрузки
     */
    bool deserializeFile(DynamicJsonDocument& doc, const char* filepath);

    /**
     * процедура открывает индекс-файл на запись в переданный хендл,
     * возвращает хендл
     */
    File& openIndexFile(File& fhandle, const char *folder);

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
    bool _eff_cfg_deserialize(DynamicJsonDocument &doc, uint16_t nb, const char *folder = NULL);

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
    // дефолтный конструктор
    EffectWorker(LAMPSTATE *_lampstate);
    // конструктор копий эффектов
    EffectWorker(const EffectListElem* base, const EffectListElem* copy);
    // Конструктор для отложенного эффекта
    EffectWorker(uint16_t delayeffnb);
    // конструктор текущего эффекта, для fast=true вычитываетсяч только имя
    EffectWorker(const EffectListElem* eff, bool fast=false);


    void removeLists(); // уделение списков из ФС
    //time_t getlistsuffix() {return listsuffix ? listsuffix : (listsuffix=micros());}      // obsolete, server could handle caching
    //void setlistsuffix(time_t val) {listsuffix=val;}
    std::unique_ptr<EffectCalc> worker = nullptr;           ///< указатель-класс обработчик текущего эффекта
    void initDefault(const char *folder = NULL); // пусть вызывается позже и явно
    ~EffectWorker();

    LList<std::shared_ptr<UIControl>>&getControls() { return isSelected() ? controls : selcontrols; }

    // тип сортировки
    void setEffSortType(SORT_TYPE type) {if(effSort != type) { effectsReSort(type); } effSort = type;}
    SORT_TYPE getEffSortType() {return effSort;}

    // Получить конфиг текущего эффекта
    /**
     * @brief Get the json string with Serialized Eff Config object
     * 
     * @param nb 
     * @param replaceBright 
     * @return String 
     */
    //getSerializedEffConfig
    String geteffconfig(uint16_t nb, uint8_t replaceBright = 0);

    // Получить конфиг эффекта из ФС
    bool getfseffconfig(uint16_t nb, String &result);

    /**
     *  отложенная запись конфига текущего эффекта, каждый вызов перезапускает счетчик
     *  force - сохраняет без задержки, таймер отключается
     */
    void autoSaveConfig(bool force=false);
    // удалить конфиг переданного эффекта
    void removeConfig(const uint16_t nb, const char *folder=NULL);
    // пересоздает индекс с текущего списка эффектов
    void makeIndexFileFromList(const char *folder = NULL, bool forceRemove = true);
    // пересоздает индекс с конфигов в ФС
    void makeIndexFileFromFS(const char *fromfolder = NULL, const char *tofolder = NULL);

    byte getModeAmount() {return effects.size();}

    const String &getEffectName() {return effectName;}

    // если текущий, то просто пишем имя, если другой - создаем экземпляр, пишем, удаляем
    void setEffectName(const String &name, EffectListElem*to);

    const String &getSoundfile() {return soundfile;}

    // если текущий, то просто пишем имя звукового файла, если другой - создаем экземпляр, пишем, удаляем
    void setSoundfile(const String &_soundfile, EffectListElem*to);

    const String &getOriginalName() {return originalName;}

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

    // текущий эффект или его копия
    const uint16_t getEn() {return curEff;}
    //const uint16_t

    // следующий эффект, кроме canBeSelected==false
    uint16_t getNext();
    // предыдущий эффект, кроме canBeSelected==false
    uint16_t getPrev();
    // получить указанный
    uint16_t getBy(uint16_t select){ return select;}
    // перейти по предворительно выбранному

    void moveSelected();
    // перейти на количество шагов, к ближйшему большему (для DEMO)

    void moveByCnt(byte cnt){ uint16_t eff = getByCnt(cnt); directMoveBy(eff); }
    // получить номер эффекта смещенного на количество шагов (для DEMO)
    uint16_t getByCnt(byte cnt);
    bool validByList(int val);
    // получить реальный номер эффекта по номеру элемента списка (для плагинов)
    uint16_t realEffNumdByList(uint16_t val) { return effects[val]->eff_nb; }
    // получить индекс эффекта по номеру (для плагинов)
    uint16_t effIndexByList(uint16_t val);

    // получить флаг canBeSelected по номеру элемента списка (для плагинов)
    bool effCanBeSelected(uint16_t val) { if (val < effects.size())return effects[val]->canBeSelected(); return false; }

    // перейти на указанный в обход нормального переключения, использовать только понимая что это (нужно для начальной инициализации и переключений выключенной лампы)
    void directMoveBy(uint16_t select);
    // вернуть первый элемент списка
    EffectListElem *getFirstEffect();
    // вернуть следующий эффект
    EffectListElem *getNextEffect(EffectListElem *current);
    // вернуть выбранный элемент списка
    EffectListElem *getEffect(uint16_t select);
    // вернуть текущий
    uint16_t getCurrent() {return curEff;}
    // вернуть текущий элемент списка
    EffectListElem *getCurrentListElement();
    // вернуть выбранный
    uint16_t getSelected() {return selEff;}
    // вернуть выбранный элемент списка
    EffectListElem *getSelectedListElement();
    void setSelected(const uint16_t effnb);
    bool isSelected(){ return (curEff == selEff); }
    // копирование эффекта
    void copyEffect(const EffectListElem *base);
    // удалить эффект
    void deleteEffect(const EffectListElem *eff, bool isCfgRemove = false);
};

#endif

