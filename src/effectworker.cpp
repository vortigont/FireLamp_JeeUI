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
#include "effectworker.h"
#include "effects.h"
#include "char_const.h"
#include "constants.h"        // EmbUI string literals
#include "templates.hpp"
#include "actions.hpp"
#include "evtloop.h"
#include "display.hpp"
#include "log.h"

#define DYNJSON_SIZE_EFF_CFG   2048

// defines buffer size for writing json files
#ifdef ESP32
#define ARR_LIST_SIZE   4096
#else
#define ARR_LIST_SIZE   1024
#endif


#define WRKR_TASK_CORE          CONFIG_ARDUINO_RUNNING_CORE    // task MUST be pinned to the second core to avoid LED glitches (if applicable)
#define WRKR_TASK_PRIO          tskIDLE_PRIORITY+1    // task priority
#ifdef LAMP_DEBUG_LEVEL
#define WRKR_TASK_STACK         2048                  // sprintf could take lot's of stack mem for debug messages
#else
#define WRKR_TASK_STACK         1536                  // effects code should mostly allocate mem on heap
#endif
#define WRKR_TASK_NAME          "EFF_WRKR"

constexpr int target_fps{MAX_FPS};                     // desired FPS rate for effect runner
constexpr int interframe_delay_ms = 1000 / target_fps;


// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

//static constexpr const char c_snd[] = "snd";

Effcfg::Effcfg(uint16_t effid) : num(effid){
  loadeffconfig(effid);
};

Effcfg::~Effcfg(){
  // save config if any changes are pending
  if (tConfigSave){
    delete tConfigSave;
    _savecfg();
  }
}

bool Effcfg::_eff_cfg_deserialize(JsonDocument &doc, const char *folder){
  LOGD(T_EffCfg, printf, "_eff_cfg_deserialize() eff:%u\n", num);
  String filename(fshlpr::getEffectCfgPath(num,folder));

  bool retry = true;
  READALLAGAIN:
  if (embuifs::deserializeFile(doc, filename.c_str() )){
    if ( num>255 || geteffcodeversion((uint8_t)num) == doc["ver"] ){ // только для базовых эффектов эта проверка
      return true;   // we are OK
    }
    LOGW(T_EffCfg, printf, "Wrong version in effect cfg file, reset to default (%d vs %d)\n", doc["ver"].as<uint8_t>(), geteffcodeversion((uint8_t)num));
  }
  // something is wrong with eff config file, recreate it to default
  create_eff_default_cfg_file(num, filename);   // пробуем перегенерировать поврежденный конфиг (todo: remove it and provide default from code)

  if (retry) {
    retry = false;
    goto READALLAGAIN;
  }

  LOGE(T_EffCfg, printf, "Failed to recreate eff config file: %s\n", filename.c_str());
  return false;
}

bool Effcfg::loadeffconfig(uint16_t nb, const char *folder){
  num = nb;
  JsonDocument doc;
  if (!_eff_cfg_deserialize(doc, folder)) return false;   // error loading file

  version = doc["ver"];
  effectName = doc[T_name] ? doc[T_name].as<const char*>() : T_EFFNAMEID[(uint8_t)nb];

  //brt = doc["brt"];
  curve = doc[A_dev_lcurve] ? static_cast<luma::curve>(doc[A_dev_lcurve].as<int>()) : luma::curve::cie1931;


  return _eff_ctrls_load_from_jdoc(doc, controls);
}

void Effcfg::create_eff_default_cfg_file(uint16_t nb, String &filename){

  const char* efname = T_EFFNAMEID[(uint8_t)nb]; // выдергиваем имя эффекта из таблицы
  LOGD(T_EffCfg, printf, "Make default config: %d %s\n", nb, efname);

  String  cfg(T_EFFUICFG[(uint8_t)nb]);    // извлекаем конфиг для UI-эффекта по-умолчанию из флеш-таблицы
  cfg.replace("@name@", efname);
  cfg.replace("@ver@", String(geteffcodeversion((uint8_t)nb)) );
  cfg.replace("@nb@", String(nb));
  
  File configFile = LittleFS.open(filename, "w");
  if (configFile){
    configFile.print(cfg.c_str());
    configFile.close();
  }
}

void Effcfg::_savecfg(char *folder){
  File configFile;
  String filename = fshlpr::getEffectCfgPath(num, folder);
  LOGD(T_EffCfg, printf, "Writing eff #%d cfg: %s\n", num, filename.c_str());
  configFile = LittleFS.open(filename, "w");
  configFile.print(getSerializedEffConfig());
  configFile.close();
}

void Effcfg::autosave(bool force) {
  if (force){
    if(tConfigSave)
      tConfigSave->cancel();
    LOGD(T_EffCfg, printf, "Force save eff cfg: %d\n", num);
    _savecfg();
    return;
  }

  if(!tConfigSave){ // task for delayed config autosave
    tConfigSave = new Task(CFG_AUTOSAVE_TIMEOUT, TASK_ONCE, [this](){
      _savecfg();
      //fsinforenew();
      LOGD(T_EffCfg, printf, "Autosave effect config: %u\n", num);
    }, &ts, false, nullptr, [this](){tConfigSave=nullptr;}, true);
    tConfigSave->enableDelayed();
  } else {
    tConfigSave->restartDelayed();
  }
}

String Effcfg::getSerializedEffConfig(uint8_t replaceBright) const {
  JsonDocument doc;

  doc["nb"] = num;
  doc["flags"] = flags.mask;
  doc[T_name] = effectName;
  doc["ver"] = version;
  //if (brt) doc["brt"] = brt;
  if (curve != luma::curve::cie1931) doc[A_dev_lcurve] = e2int(curve);
  JsonArray arr = doc[T_ctrls].to<JsonArray>();
  for (auto c = controls.cbegin(); c != controls.cend(); ++c){
    auto ctrl = c->get();
    JsonObject var = arr.add<JsonObject>();
    var[P_id]=ctrl->getId();
    var[P_type]=ctrl->getType();
    var[T_name] = ctrl->getName();
    var[T_val]  = ctrl->getVal();
    var[P_min]=ctrl->getMin();
    var[P_max]=ctrl->getMax();
    var[P_step]=ctrl->getStep();
  }

  String cfg_str;
  serializeJson(doc, cfg_str);

  return cfg_str;
}


//  ***** EffectWorker implementation *****

EffectWorker::EffectWorker(LampState *_lampstate) : lampstate(_lampstate) {
  // create 3 'faivored' superusefull controls for 'speed', 'scale'
  for(unsigned id=1; id<2; id++){
    auto c = std::make_shared<UIControl>(
      id,                                     // id
      CONTROL_TYPE::RANGE,                    // type
      id==1 ? String(TINTF_087) : String(TINTF_088)           // name
    );
    curEff.controls.push_back(std::move(c));
  }
  //pendingCtrls = controls;
}

//EffectWorker::~EffectWorker() { clearEffectList(); }

/*
 * Создаем экземпляр класса калькулятора в зависимости от требуемого эффекта
 */
void EffectWorker::workerset(uint16_t effect){
  LOGI(T_EffWrkr, printf, "Switch to eff:%u\n", effect);

  LedFB<CRGB> *canvas = display.getCanvas().get();
  if (!canvas) { LOGW(T_EffWrkr, println, "no canvas buffer!"); return; }

  // load effect configuration from a saved file
  curEff.loadeffconfig(effect);

  // не создаем экземпляр калькулятора если воркер неактивен (лампа выключена и т.п.)
  if (!_status) { LOGI(T_EffWrkr, println, "worker is inactive"); return; }

  // grab mutex
  std::unique_lock<std::mutex> lock(_mtx);
  // create a new instance of effect child
  switch (static_cast<EFF_ENUM>(effect%256)){ // номер может быть больше чем ENUM из-за копирований, находим эффект по модулю

//  case EFF_ENUM::EFF_TETRIS_CLOCK :
//    worker = std::make_unique<TetrisClock>(display.getCanvas());
//    break;
  case EFF_ENUM::EFF_SWIRL :
    worker = std::make_unique<EffectSwirl>(canvas);
    break;
  case EFF_ENUM::EFF_COMET :
    worker = std::make_unique<EffectComet>(canvas);
    break;
  case EFF_ENUM::EFF_FLOCK :
    worker = std::make_unique<EffectFlock>(canvas);
    break;
  case EFF_ENUM::EFF_PRIZMATA :
    worker = std::make_unique<EffectPrismata>(canvas);
    break;
  case EFF_ENUM::EFF_SPIRO :
    worker = std::make_unique<EffectSpiro>(canvas);
    break;
  case EFF_ENUM::EFF_METABALLS :
    worker = std::make_unique<EffectMetaBalls>(canvas);
    break;
  case EFF_ENUM::EFF_SINUSOID3 :
    worker = std::make_unique<EffectSinusoid3>(canvas);
    break;
  case EFF_ENUM::EFF_BBALS :
    worker = std::make_unique<EffectBBalls>(canvas);
    break;
  case EFF_ENUM::EFF_PAINTBALL :
    worker = std::make_unique<EffectLightBalls>(canvas);
    break;
  case EFF_ENUM::EFF_PULSE :
    worker = std::make_unique<EffectPulse>(canvas);
    break;
  case EFF_ENUM::EFF_CUBE :
    worker = std::make_unique<EffectBall>(canvas);
    break;
  case EFF_ENUM::EFF_LIGHTER_TRACES :
    worker = std::make_unique<EffectLighterTracers>(canvas);
    break;
  case EFF_ENUM::EFF_RAINBOW_2D :
    worker = std::make_unique<EffectRainbow>(canvas);
    break;
  case EFF_ENUM::EFF_COLORS :
    worker = std::make_unique<EffectColors>(canvas);
    break;
  case EFF_ENUM::EFF_WHITE_COLOR :
    worker = std::make_unique<EffectWhiteColorStripe>(canvas);
    break;
  case EFF_ENUM::EFF_MATRIX :
    worker = std::make_unique<EffectMatrix>(canvas);
    break;
  case EFF_ENUM::EFF_SPARKLES :
    worker = std::make_unique<EffectSparcles>(canvas);
    break;
  case EFF_ENUM::EFF_EVERYTHINGFALL :
    worker = std::make_unique<EffectMira>(canvas);
    break;
  case EFF_ENUM::EFF_FIRE2012 :
    worker = std::make_unique<EffectFire2012>(canvas);
    break;
  case EFF_ENUM::EFF_SNOWSTORMSTARFALL :
    worker = std::make_unique<EffectStarFall>(canvas);
    break;
  case EFF_ENUM::EFF_3DNOISE :
    worker = std::make_unique<Effect3DNoise>(canvas);
    break;
  case EFF_ENUM::EFF_CELL :
    worker = std::make_unique<EffectCell>(canvas);
    break;
  case EFF_ENUM::EFF_T_LEND :
    worker = std::make_unique<EffectTLand>(canvas);
    break;
  case EFF_ENUM::EFF_OSCIL :
    worker = std::make_unique<EffectOscillator>(canvas);
    break;
  case EFF_ENUM::EFF_WRAIN : 
    worker = std::make_unique<EffectWrain>(canvas);
    break;
  case EFF_ENUM::EFF_FAIRY : 
  case EFF_ENUM::EFF_FOUNT :
    worker = std::make_unique<EffectFairy>(canvas);
    break;
  case EFF_ENUM::EFF_CIRCLES :
    worker = std::make_unique<EffectCircles>(canvas);
    break;
  case EFF_ENUM::EFF_DRIFT :
    worker = std::make_unique<EffectDrift>(canvas);
    break;
  case EFF_ENUM::EFF_POPCORN :
    worker = std::make_unique<EffectPopcorn>(canvas);
    break;
  case EFF_ENUM::EFF_TWINKLES :
    worker = std::make_unique<EffectTwinkles>(canvas);
    break;
  case EFF_ENUM::EFF_WAVES :
    worker = std::make_unique<EffectWaves>(canvas);
    break;
  case EFF_ENUM::EFF_RADAR :
    worker = std::make_unique<EffectRadar>(canvas);
    break;
  case EFF_ENUM::EFF_BENGALL :
    worker = std::make_unique<EffectBengalL>(canvas);
    break;
  case EFF_ENUM::EFF_BALLS :
    worker = std::make_unique<EffectBalls>(canvas);
    break;
  case EFF_ENUM::EFF_FIRE2018 :
    worker = std::make_unique<EffectFire2018>(canvas);
    break;
  case EFF_ENUM::EFF_RINGS :
    worker = std::make_unique<EffectRingsLock>(canvas);
    break;
  case EFF_ENUM::EFF_CUBE2 :
    worker = std::make_unique<EffectCube2d>(canvas);
    break;
  case EFF_ENUM::EFF_PICASSO :
  case EFF_ENUM::EFF_PICASSO4 :
    worker = std::make_unique<EffectPicasso>(canvas);
    break;
  case EFF_ENUM::EFF_STARSHIPS :
    worker = std::make_unique<EffectStarShips>(canvas);
    break;
  case EFF_ENUM::EFF_FLAGS :
    worker = std::make_unique<EffectFlags>(canvas);
    break;
  case EFF_ENUM::EFF_LEAPERS :
    worker = std::make_unique<EffectLeapers>(canvas);
    break;
  case EFF_ENUM::EFF_LIQUIDLAMP :
    worker = std::make_unique<EffectLiquidLamp>(canvas);
    break;
  case EFF_ENUM::EFF_AQUARIUM :
    worker = std::make_unique<EffectAquarium>(canvas);
    break;
  case EFF_ENUM::EFF_WHIRL :
    worker = std::make_unique<EffectWhirl>(canvas);
    break;
  case EFF_ENUM::EFF_STAR :
    worker = std::make_unique<EffectStar>(canvas);
    break;
  case EFF_ENUM::EFF_FIREWORKS :
    worker = std::make_unique<EffectFireworks>(canvas);
    break;
  case EFF_ENUM::EFF_PACIFIC :
    worker = std::make_unique<EffectPacific>(canvas);
    break;
  case EFF_ENUM::EFF_MUNCH :
    worker = std::make_unique<EffectMunch>(canvas);
    break;
  case EFF_ENUM::EFF_NOISE :
    worker = std::make_unique<EffectNoise>(canvas);
    break;
  case EFF_ENUM::EFF_BUTTERFLY :
    worker = std::make_unique<EffectButterfly>(canvas);
    break;
  case EFF_ENUM::EFF_SHADOWS :
    worker = std::make_unique<EffectShadows>(canvas);
    break;
  case EFF_ENUM::EFF_PATTERNS :
    worker = std::make_unique<EffectPatterns>(canvas);
    break;
  case EFF_ENUM::EFF_ARROWS :
    worker = std::make_unique<EffectArrows>(canvas);
    break;
  case EFF_ENUM::EFF_NBALLS :
    worker = std::make_unique<EffectNBals>(canvas);
    break;
  case EFF_ENUM::EFF_ATTRACT :
    worker = std::make_unique<EffectAttract>(canvas);
    break;
  case EFF_ENUM::EFF_SNAKE :
    worker = std::make_unique<EffectSnake>(canvas);
    break;
  case EFF_ENUM::EFF_NEXUS :
    worker = std::make_unique<EffectNexus>(canvas);
    break;
  case EFF_ENUM::EFF_MAZE :
    worker = std::make_unique<EffectMaze>(canvas);
    break;
  case EFF_ENUM::EFF_FRIZZLES :
    worker = std::make_unique<EffectFrizzles>(canvas);
    break;
  case EFF_ENUM::EFF_POLARL :
    worker = std::make_unique<EffectPolarL>(canvas);
    break;
  case EFF_ENUM::EFF_SNAKEISLAND :
    worker = std::make_unique<EffectSnakeIsland>(canvas);
    break;
   case EFF_ENUM::EFF_SMOKEBALLS :
    worker = std::make_unique<EffectSmokeballs>(canvas);
    break;
   case EFF_ENUM::EFF_RACER :
    worker = std::make_unique<EffectRacer>(canvas);
    break;
   case EFF_ENUM::EFF_MAGMA :
    worker = std::make_unique<EffectMagma>(canvas);
    break;
   case EFF_ENUM::EFF_FIRE2021 :
    worker = std::make_unique<EffectFire2021>(canvas);
    break;
   case EFF_ENUM::EFF_PUZZLES :
    worker = std::make_unique<EffectPuzzles>(canvas);
    break;
   case EFF_ENUM::EFF_PILE :
    worker = std::make_unique<EffectPile>(canvas);
    break;
   case EFF_ENUM::EFF_DNA :
    worker = std::make_unique<EffectDNA>(canvas);
    break;
   case EFF_ENUM::EFF_SMOKER :
    worker = std::make_unique<EffectSmoker>(canvas);
    break;
  case EFF_ENUM::EFF_MIRAGE :
    worker = std::make_unique<EffectMirage>(canvas);
    break;
  case EFF_ENUM::EFF_WATERCOLORS :
    worker = std::make_unique<EffectWcolor>(canvas);
    break;
  case EFF_ENUM::EFF_RADIALFIRE :
    worker = std::make_unique<EffectRadialFire>(canvas);
    break;
  case EFF_ENUM::EFF_SPBALS :
    worker = std::make_unique<EffectSplashBals>(canvas);
    break;
  case EFF_ENUM::EFF_FLOWER :
    worker = std::make_unique<EffectFlower>(canvas);
    break;
  case EFF_ENUM::EFF_VU :
    worker = std::make_unique<EffectVU>(canvas, lampstate->mic_gpio);
    break;
  case EFF_ENUM::EFF_OSC :
    worker = std::make_unique<EffectOsc>(canvas, lampstate->mic_gpio);
    break;

  default:
    worker = std::make_unique<EffectNone>(canvas); // std::make_unique<EffectCalc>(new EffectCalc());
  }

  if(worker){
    // apply effect's controls
    worker->init(static_cast<EFF_ENUM>(effect%256), &curEff.controls, lampstate);
  }

  // release mutex after effect init  has complete
  lock.unlock();

  if(worker){
    // set newly loaded luma curve to the lamp
    run_action(ra::brt_lcurve, e2int(curEff.curve));
    display.canvasProtect(eff_persistent_buff[effect%256]);     // set 'persistent' frambuffer flag if effect's manifest demands it
    _start_runner();  // start calculator task IF we are marked as active
    // send event
    uint32_t n = effect;
    EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::effSwitchTo), &n, sizeof(uint32_t));
  }
}

void EffectWorker::initDefault(const char *folder)
{
  if(!LittleFS.exists("/eff")){
    LittleFS.mkdir("/eff");
  }
  if(!LittleFS.exists("/backup")){
    LittleFS.mkdir("/backup");
  }
  if(!LittleFS.exists("/backup/btn")){
    LittleFS.mkdir("/backup/btn");
  }
  if(!LittleFS.exists("/backup/evn")){
    LittleFS.mkdir("/backup/evn");
  }
  if(!LittleFS.exists("/backup/glb")){
    LittleFS.mkdir("/backup/glb");
  }
  if(!LittleFS.exists("/backup/idx")){
    LittleFS.mkdir("/backup/idx");
  }

  // try to load effects index from FS, or default index from FW if FS index is missing or corrupted
  _load_eff_list_from_idx_file();
}

void EffectWorker::removeConfig(const uint16_t nb, const char *folder)
{
  LOGD(T_EffWrkr, printf, "Remove from FS: %s\n", fshlpr::getEffectCfgPath(nb,folder).c_str());
  LittleFS.remove(fshlpr::getEffectCfgPath(nb,folder)); // удаляем файл
}
/*
void EffectWorker::effectsReSort(SORT_TYPE _effSort)
{
  LOG(printf_P,PSTR("*Пересортировка эффектов*: %d\n"), _effSort);
  if(_effSort==255) _effSort=effSort; // Для дефолтного - берем с конфига

  switch(_effSort){
    case SORT_TYPE::ST_BASE :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return (((a.eff_nb&0xFF) - (b.eff_nb&0xFF))<<8) + (((a.eff_nb&0xFF00) - (b.eff_nb&0xFF00))>>8);});
      break;
    case SORT_TYPE::ST_END :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return a.eff_nb - b.eff_nb;}); // сортирую по eff_nb
      //effects.sort([](EffectListElem *&a, EffectListElem *&b){ return ((int32_t)(((a->eff_nb&0xFF)<<8) | ((a->eff_nb&0xFF00)>>8)) - (((b->eff_nb&0xFF)<<8) | ((b->eff_nb&0xFF00)>>8)));});
      //effects.sort([](EffectListElem *&a, EffectListElem *&b){ return (a->eff_nb&0xFF00) - (b->eff_nb&0xFF00) + (((a->eff_nb&0xFF) - (b->eff_nb&0xFF))<<8) + (((a->eff_nb&0xFF00) - (b->eff_nb&0xFF00))>>8);});
      break;
    case SORT_TYPE::ST_IDX :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return (int)(a.getMS() - b.getMS());});
      break;
    case SORT_TYPE::ST_AB2 :
      // крайне медленный вариант, с побочными эффектами, пока отключаю и использую вместо него ST_AB
      //effects.sort([](EffectListElem *&a, EffectListElem *&b){ EffectWorker *tmp = new EffectWorker((uint16_t)0); String tmp1; tmp->loadeffname(tmp1, a->eff_nb); String tmp2; tmp->loadeffname(tmp2,b->eff_nb); delete tmp; return strcmp_P(tmp1.c_str(), tmp2.c_str());});
      //break;
    case SORT_TYPE::ST_AB :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return std::string_view(T_EFFNAMEID[(uint8_t)a.eff_nb]).compare(T_EFFNAMEID[(uint8_t)b.eff_nb]); });
      break;
    case SORT_TYPE::ST_MIC :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return ((int)(pgm_read_byte(T_EFFVER + (a.eff_nb&0xFF))&0x01) - (int)(pgm_read_byte(T_EFFVER + (b.eff_nb&0xFF))&0x01)); });
      break;
    default:
      break;
  }
}
*/

/**
 * вычитать только имя эффекта из конфиг-файла и записать в предоставленную строку
 * в случае отсутствия/повреждения взять имя эффекта из флеш-таблицы, если есть
 * @param effectName - String куда записать результат
 * @param nb  - айди эффекта
 * @param folder - какой-то префикс для каталога
 */
void EffectWorker::loadeffname(String& _effectName, const uint16_t nb, const char *folder)
{
  String filename = fshlpr::getEffectCfgPath(nb,folder);
  JsonDocument doc;
  bool ok = embuifs::deserializeFile(doc, filename.c_str());
  if (ok && doc[T_name]){
    _effectName = doc[T_name].as<const char*>(); // перенакрываем именем из конфига, если есть
  } else {
    _effectName = T_EFFNAMEID[(uint8_t)nb];   // выбираем имя по-умолчанию из флеша если конфиг поврежден
  }
}

void EffectWorker::removeLists(){
  LittleFS.remove(TCONST_eff_list_json);
  LittleFS.remove(TCONST_eff_fulllist_json);
  LittleFS.remove(TCONST_eff_index);
}

void EffectWorker::makeIndexFileFromList(const char *folder, bool forceRemove)
{
  if(forceRemove)
    removeLists();

  std::array<char, ARR_LIST_SIZE> *buff = new(std::nothrow) std::array<char, ARR_LIST_SIZE>;
  if (!buff) return;    // not enough mem

  File hndlr;
  fshlpr::openIndexFile(hndlr, folder);
  //effectsReSort(SORT_TYPE::ST_IDX); // сброс сортировки перед записью

  size_t offset = 0;
  auto itr =  effects.cbegin();  // get const interator
  buff->at(offset++) = (char)0x5b;   // Open json with ASCII '['

  do {
    // {"n":%d,"f":%d},   => 32 bytes is more than enough
    if (ARR_LIST_SIZE - offset < 32){
      // write to file and purge buffer
      hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), offset);
      offset = 0;
    }

    offset += sprintf_P(buff->data()+offset, "{\"n\":%d,\"f\":%d},", itr->eff_nb, itr->flags.mask);
  } while (++itr != effects.cend());

  buff->at(--offset) = (char)0x5d;   // ASCII ']' implaced over last comma
  hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), ++offset);
  hndlr.close();
  delete buff;

  LOGD(T_EffWrkr, println, "Индекс эффектов обновлен" );
  //effectsReSort(); // восстанавливаем сортировку
}

// удалить эффект
void EffectWorker::deleteEffect(const EffectListElem *eff, bool onlyCfgFile)
{
  for (auto i = effects.begin(); i != effects.end(); ++i){
    if ( (*i).eff_nb == eff->eff_nb){
          if(onlyCfgFile)
            removeConfig(eff->eff_nb);    // only remove eff json cfg file
          else
            effects.erase(i);             // remove effect from drop-down selection list
          return;
    }
  }
}

// копирование эффекта
void EffectWorker::copyEffect(const EffectListElem *base)
{
  EffectListElem copy(base); // создать копию переданного эффекта
  uint16_t maxfoundnb=base->eff_nb;
  for(unsigned i=0; i<effects.size();i++){
    if(effects[i].eff_nb>255 && ((effects[i].eff_nb&0x00FF)==(copy.eff_nb&0x00FF))){ // найдены копии
      if(maxfoundnb < effects[i].eff_nb) maxfoundnb=effects[i].eff_nb;
    }
  }

  int16_t newnum =(((((maxfoundnb & 0xFF00)>>8)+1) << 8 ) | (copy.eff_nb&0xFF)); // в старшем байте увеличиваем значение на число имеющихся копий
  copy.eff_nb = newnum;
  effects.push_back(std::move(copy));

  Effcfg copycfg(base->eff_nb);
  copycfg.num = newnum;
  // имя формируем с базового + индекс копии
  copycfg.effectName.concat("_");
  copycfg.effectName.concat(newnum>>8);

  // save new config file
  copycfg.autosave(true);
  removeLists();              // drop cached lists
  makeIndexFileFromList();    // rebuild index (it will be faster than wait to hit _rebuild_eff_list() )
}

// вернуть эффект на очереди из списка эффектов
EffectListElem *EffectWorker::getSelectedListElement()
{
  for(unsigned i=0; i<effects.size(); i++){
    if(effects[i].eff_nb == curEff.num)
      return &effects[i];
  }
  return nullptr;
}

// вернуть текущий элемент списка
EffectListElem *EffectWorker::getCurrentListElement()
{
  for(unsigned i=0; i<effects.size(); i++){
    if(effects[i].eff_nb==curEff.num)
      return &effects[i];
  }
  return nullptr;
}

// вернуть выбранный элемент списка
EffectListElem *EffectWorker::getFirstEffect()
{
  if(effects.size()>0)
    return &effects[0];
  else
    return nullptr; // NONE
}

// вернуть выбранный элемент списка
EffectListElem *EffectWorker::getEffect(uint16_t select){
  for (unsigned i = 0; i < effects.size(); i++) {
      if (effects[i].eff_nb == select) {
          return &effects[i];
      }
  }
  LOGW(T_Effect, printf, "requested eff %u not found\n", select);
  return nullptr; // NONE
}

// вернуть следующий эффект, если передан nullptr, то возвращается первый
EffectListElem *EffectWorker::getNextEffect(EffectListElem *current){
    if(current == nullptr) return getFirstEffect();
    for (unsigned i = 0; i < effects.size(); i++) {
        if (effects[i].eff_nb == current->eff_nb) {
            return i+1<effects.size() ? &effects[i+1] : nullptr;
        }
    }
    return nullptr; // NONE
}

uint16_t EffectWorker::getNextEffIndexForDemo(bool rnd){
  auto idx = curEff.num;
  auto i = effects.begin();

  if (rnd){
    i += random(1, effects.size()-2);   // if need random effect, then shift iterator to random distance
  } else {
    i = std::find_if(effects.begin(), effects.end(), [idx](const EffectListElem &e){ return e.eff_nb == idx; }); // otherwise find current
  }

  // какая-то ошибка, не нашли текущий эффект
  if (i == effects.end())
    return idx;

  // ищем следующий доступный эффект для демо после текущего
  while ( ++i != effects.end()){
    if (i->canBeSelected() && i->enabledInDemo())
      return i->eff_nb;
  }

  // если не нашли, ищем с начала списка
  i = effects.begin();
  while ( ++i != effects.end()){
    if (i->canBeSelected() && i->enabledInDemo())
      return i->eff_nb;
  }

  // if nothing found, then return current effect
  return curEff.num;
}

// предыдущий эффект, кроме canBeSelected==false
uint16_t EffectWorker::getPrev(){
  uint16_t firstfound = curEff.num;
  bool found = false;
  for(unsigned i=0; i<effects.size(); i++){
      if(found && firstfound!=curEff.num) { // нашли эффект перед собой
          break;
      } else {
          found = false; // перед текущим не нашлось подходящих, поэтому возьмем последний из canBeSelected()
      }
      if(effects[i].eff_nb==curEff.num && i!=0){ // нашли себя, но не первым :)
          found = true;
          continue;
      }
      if(effects[i].canBeSelected()){
          firstfound = effects[i].eff_nb; // первый найденный, на случай если следующего после текущего не будет
      }
  }
  return firstfound;
}

// следующий эффект, кроме canBeSelected==false
uint16_t EffectWorker::getNext(){
  uint16_t firstfound = curEff.num;
  bool found = false;
  for(unsigned i=0; i<effects.size(); i++){
      if(effects[i].eff_nb==curEff.num){ // нашли себя
          found = true;
          continue;
      }
      if(effects[i].canBeSelected()){
          if(firstfound == curEff.num)
              firstfound = effects[i].eff_nb; // первый найденный, на случай если следующего после текущего не будет
          if(found) { // нашли эффект после себя
              firstfound = effects[i].eff_nb;
              break;
          }
      }
  }
  return firstfound;
}

void EffectWorker::switchEffect(uint16_t effnb){
  // NOTE: if call has been made to the SAME effect number as the current one, than it MUST be force-switched anyway to recreate EffectCalc object
  // (it's required for a cases like new LedFB has been provided, etc)
  if (effnb == curEff.num) return reset();

  curEff.flushcfg();  // сохраняем конфигурацию предыдущего эффекта если были несохраненные изменения

  LOGD(T_EffWrkr, printf, "switchEffect:%u\n", effnb);
  workerset(effnb);
}

void EffectWorker::setEffectName(const String &name, EffectListElem*to){
  if (name == T_EFFNAMEID[(uint8_t)to->eff_nb]){
    to->flags.renamed = false;
    return;   // имя совпадает с исходным значением во флеше, нечего переименовывать
  } 

  to->flags.renamed = true;   // эффект переименовали

  if(to->eff_nb==curEff.num){
    curEff.effectName =name;
    curEff.autosave();
    return;
  }

  // load specific configuration
  Effcfg cfg(to->eff_nb);
  cfg.effectName = name;
  cfg.autosave(true);
}

uint16_t EffectWorker::effIndexByList(uint16_t val) { 
    for (uint16_t i = 0; i < effects.size(); i++) {
        if (effects[i].eff_nb == val ) {
            return i;
        }
    }
    return 0;
}

bool Effcfg::_eff_ctrls_load_from_jdoc(JsonDocument &effcfg, std::vector<std::shared_ptr<UIControl>> &ctrls){
  LOGD(T_Effect, print, "_eff_ctrls_load_from_jdoc(), ");
  //LOG(printf_P, PSTR("Load MEM: %s - CFG: %s - DEF: %s\n"), effectName.c_str(), doc[T_name].as<String>().c_str(), worker->getName().c_str());
  // вычитываею список контроллов
  // повторные - скипаем, нехватающие - создаем
  // обязательные контролы 0, 1, 2 - яркость, скорость, масштаб, остальные пользовательские
  JsonArray arr = effcfg[T_ctrls].as<JsonArray>();
  if (!arr) return false;
  LOGD(T_Effect, printf, "got arr of %u controls\n", arr.size());

  ctrls.clear();
  ctrls.reserve(arr.size());
  uint8_t id_tst = 0x0; // пустой
  for (JsonObject item : arr) {
      uint8_t id = item[T_id].as<uint8_t>();
      if ( !(id_tst&(1<<id)) ){   // проверка на существование контрола
          id_tst |= 1<<id;        // закладываемся не более чем на 8 контролов, этого хватит более чем :)
          // формируем имя контрола
          String name;
          if (item.containsKey(T_name))
            name = item[T_name].as<const char*>();
          else if (id == 1){
            name = TINTF_087;
          } else if (id == 2)
            name = TINTF_088;
          else {
            name = "Доп.";
            name += id;
          }

          String val( item.containsKey(T_val) ? item[T_val].as<int>() : 128 );
          String min( item.containsKey(T_min) && id>2 ? item[T_min].as<int>() : 1 );
          String max( item.containsKey(T_max) && id>2 ? item[T_max].as<int>() : 255 );
          String step( item.containsKey(T_step) && id>2 ?  item[T_step].as<int>() : 1);
          CONTROL_TYPE type = item["type"].as<CONTROL_TYPE>();
          type = ((type & 0x0F)!=CONTROL_TYPE::RANGE) && id<3 ? CONTROL_TYPE::RANGE : type;
          min = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "0" : min;
          max = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "1" : max;
          step = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "1" : step;
          //auto c = std::make_shared<UIControl>( id, type, name, val, min, max, step );
          ctrls.emplace_back( std::make_shared<UIControl>( id, type, name, val, min, max, step ) );
          //LOG(printf_P,PSTR("%d %d %s %s %s %s %s\n"), id, type, name.c_str(), val.c_str(), min.c_str(), max.c_str(), step.c_str());
      }
  }

  // тест стандартных контроллов
  for(int8_t id=1;id<3;id++){   // 0-й пропускаем, это бывшая "индивидуальная яркость"
      if(!((id_tst>>id)&1)){ // не найден контрол, нужно создать
        auto c = std::make_shared<UIControl>(
              id,                                     // id
              CONTROL_TYPE::RANGE,                    // type
              /*id==0 ? TINTF_00D :*/ id==1 ? TINTF_087 : TINTF_088,           // name
              "127",                            // value
              "1",                              // min
              "255",                            // max
              "1"                               // step
        );
        ctrls.push_back(std::move(c));
      }
  }

  //ctrls.sort([](std::shared_ptr<UIControl> &a, std::shared_ptr<UIControl> &b){ return (*a).getId() - (*b).getId();}); // сортирую по id
  return true;
}

void EffectWorker::_load_default_fweff_list(){
  effects.clear();

  for (uint16_t i = 0; i != 256U; i++){
    if (!strlen(T_EFFNAMEID[i]))   // пропускаем индексы-"пустышки" без названия
      continue;

    //EffectListElem el(i, SET_ALL_EFFFLAGS);
    effects.emplace_back(i, SET_ALL_EFFFLAGS);
  }
  LOGD(T_EffWrkr, printf, "Loaded default list of effects, %u entries\n", effects.size());
}

void EffectWorker::_load_eff_list_from_idx_file(const char *folder){
  // todo: check if supplied alternative path starts with '/'
  String filename(folder ? folder : "");
  filename += TCONST_eff_index; // append 'eff_index.json' filename

  // if index file does not exist - load default list from firmware tables
  if (!LittleFS.exists(filename)){
    LOGD(T_EffWrkr, printf, "eff index file %s missing, loading fw defaults\n", filename.c_str());
    return _rebuild_eff_list();
  }

  JsonDocument doc;  // document for loading effects index from file

  if (!embuifs::deserializeFile(doc, filename.c_str())){
    LittleFS.remove(filename);    // remove corrupted index file
    return _rebuild_eff_list();
  }

  JsonArray arr = doc.as<JsonArray>();
  if(arr.isNull() || arr.size()==0){
    LittleFS.remove(filename);    // remove corrupted index file
    LOGW(T_EffWrkr, println, "eff index file corrupted, loading fw defaults");
    return _rebuild_eff_list();
  }

  effects.clear();
  for (JsonObject item : arr){
      if(item.containsKey("n")){
        effects.emplace_back(item["n"].as<uint16_t>(), item["f"].as<uint8_t>());
      }
      //LOG(printf_P,PSTR("%d : %d\n"),item["n"].as<uint16_t>(), item["f"].as<uint8_t>());
  }

  //effects.sort([](EffectListElem &a, EffectListElem &b){ return a.eff_nb - b.eff_nb;}); // сортирую по eff_nb

  int32_t chk = -1; // удаляю дубликаты
  for (auto i = effects.begin(); i != effects.end(); ++i){
    if((int32_t)(*i).eff_nb==chk){
      effects.erase(i);
      continue;
    }
    chk = (*i).eff_nb;
  }

  //effectsReSort();
  LOGD(T_EffWrkr, printf, "Loaded list of effects, %u entries\n", effects.size());
}

void EffectWorker::_rebuild_eff_list(const char *folder){
  LOGD(T_EffWrkr, println, "_rebuild_eff_list()");
  // load default fw list first
  _load_default_fweff_list();

  String sourcedir;

  if (folder) {
      sourcedir.concat("/");
      sourcedir.concat(folder);
  }
  sourcedir.concat("/eff");

  File dir = LittleFS.open(sourcedir);
  if (!dir || !dir.isDirectory()){
    LOGE(T_EffWrkr, printf, "Can't open dir:%s\n", sourcedir.c_str());
    return;
  }

  String fn;

  JsonDocument doc;

  File _f;
  while(_f = dir.openNextFile()){
    fn = sourcedir + "/" + _f.name();

    if (!embuifs::deserializeFile(doc, fn.c_str())) {
      LittleFS.remove(fn);                // delete corrupted config
      continue;
    }

    uint16_t nb = doc["nb"].as<uint16_t>();
    uint8_t flags = doc["flags"].as<uint8_t>();
    EffectListElem *eff = getEffect(nb);
    if(eff){  // such effect exist in list, apply flags
      flags = eff->flags.mask;
    } else {    // no such eff in list, must be an effect copy
      effects.emplace_back(nb, flags);
    }
    delay(1); // give other tasks some breathe
  }

  makeIndexFileFromList();
}

void EffectWorker::reset(){
  if (worker) workerset(getCurrentEffectNumber());
}

void EffectWorker::setLumaCurve(luma::curve c){
  if (c == curEff.curve) return;  // quit if same value
  curEff.curve = c; curEff.autosave();
};

void EffectWorker::_start_runner(){
  if (_runnerTask_h) return;    // we are already running
  xTaskCreatePinnedToCore(EffectWorker::_runnerTask,
                          WRKR_TASK_NAME,
                          WRKR_TASK_STACK,
                          (void *)this,
                          WRKR_TASK_PRIO,
                          &_runnerTask_h,
                          WRKR_TASK_CORE);
}

void EffectWorker::_runnerHndlr(){
  TickType_t xLastWakeTime = xTaskGetTickCount ();
  // make a defered mutex lock
  std::unique_lock<std::mutex> lock(_mtx, std::defer_lock);

#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL>2
  uint32_t fps{0}, t = millis();
#endif

  for (;;){
    // if task has been delayed, than we can't keep up with desired frame rate, let's give other tasks time to run anyway
    if ( xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(interframe_delay_ms) ) ) taskYIELD();

    if (!worker || !_status){
      worker.reset();
      display.clear();
      _runnerTask_h = nullptr;
      vTaskDelete(NULL);    // if there is no Effect instance spawned, there must be something wrong
      return;
    }

    // aquire mutex, if unseccessful then simply skip this run cycle
    if (!lock.try_lock())
      continue;
    //std::unique_lock<std::mutex> lock(_mtx);

    if (worker->run()){
      // effect has rendered a data in buffer, need to call the engine draw it
      display.show();

    // fps counter in debug mode
#if defined(LAMP_DEBUG_LEVEL) && LAMP_DEBUG_LEVEL>2
      ++fps;
      // once per 10 sec
      if(millis()-t > 10000){
        LOGD(T_lamp, printf, "Eff:%u, FPS: %u\n", getCurrentEffectNumber(), fps/10);
        fps = 0;
        t = millis();
      }
#endif

    }
    // effectcalc returned no data

    // release mutex
    lock.unlock();
  }
  // Task must self-terminate (if ever)
  vTaskDelete(NULL);
}

void EffectWorker::start(){
  _status = true;
  if (_runnerTask_h) return;    // we are already running
  workerset(getCurrentEffectNumber());      // spawn an instance of effect and run the task
}

void EffectWorker::stop(){
  _status = false;                  // task will self destruct on next iteration
  //std::lock_guard<std::mutex> lock(_mtx);
  //worker.reset();
  //display.clear();
  display.canvasProtect(false);     // force clear persistent flag for frambuffer (if any) 
}


/*  *** EffectCalc  implementation  ***   */
void EffectCalc::init(EFF_ENUM eff, std::vector<std::shared_ptr<UIControl>> *controls, LampState* state){
  effect = eff;
  ctrls = controls;
  _lampstate = state;

  for(unsigned i=0; i<controls->size(); i++){
    setDynCtrl((*controls)[i].get());
    // switch(i){
    //   case 0:
    //     setbrt((*controls)[i]->getVal().toInt());
    //     break;
    //   case 1:
    //     setspd((*controls)[i]->getVal().toInt());
    //     break;
    //   case 2:
    //     setscl((*controls)[i]->getVal().toInt());
    //     break;
    //   default:
    //     setDynCtrl((*controls)[i]);
    //     break;
    // }
  }
  active=true;
  load();
}

/*
 *  первоначальная загрузка эффекта, автозапускается из init()
 */
void EffectCalc::load(){}

bool EffectCalc::run(){
  return false;
}

/**
 * проверка на холостой вызов для эффектов с доп. задержкой
 */
bool EffectCalc::dryrun(float n, uint8_t delay){
  if((millis() - lastrun - delay) < (unsigned)((255 - speed) / n)) {
    active=false;
  } else {
    lastrun = millis();
    active=true;
  }

  return !active;
}

/**
 * status - статус воркера, если работает и загружен эффект, отдает true
 */
bool EffectCalc::status(){return active;}

/**
 * setDynCtrl - была смена динамического контрола, idx=3+
 * вызывается в UI, для реализации особого поведения (палитра и т.д.)...
 * https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/page-112#post-48848
 */
String EffectCalc::setDynCtrl(UIControl*_val){
  if(!_val)
    return String();
  String ret_val = _val->getVal();
  //LOG(printf_P, PSTR("ctrlVal=%s\n"), ret_val.c_str());
  if ( usepalettes && starts_with(_val->getName().c_str(), TINTF_084) ){ // Начинается с Палитра
    if(demoRndEffControls()){
      paletteIdx = random(_val->getMin().toInt(),_val->getMax().toInt()+1);
    } else
      paletteIdx = ret_val.toInt();
    palettemap(palettes, paletteIdx, _val->getMin().toInt(), _val->getMax().toInt());
    isCtrlPallete = true;
  }

  // имеет 7 id и начинается со строки "микрофон" с локализацией
  if( _val->getId()==7 && starts_with(_val->getName().c_str(), TINTF_020) ){
    // не будем заниматься ерундой и включать/выключать микрофон из конфига эффекта, для этого есть глобальный флажек
    // 
/*
    if(_lampstate){
      _lampstate->isMicOn = ret_val.toInt() && (_lampstate->mic_gpio != GPIO_NUM_NC);
      _lampstate->setMicAnalyseDivider(_lampstate->isMicOn);
    }
*/
  } else {
    if(demoRndEffControls()){ // для режима рандомного ДЕМО, если это не микрофон - то вернуть рандомное значение в пределах диапазона значений
      ret_val = String(random(_val->getMin().toInt(), _val->getMax().toInt()+1));
    }
  }

  switch(_val->getId()){
    case 1:
      speed = ret_val.toInt();
      LOGD(T_Effect, printf, "speed=%d, speedfactor=%2.2f\n", speed, speedFactor);
      break;
    case 2:
      scale = ret_val.toInt(); break;
    default:;
  }

  return ret_val;
}

// Load palletes into array
void EffectCalc::palettesload(){
  palettes.reserve(FASTLED_PALETTS_COUNT);
  palettes.push_back(&AuroraColors_p/*RainbowStripeColors_p*/);
  palettes.push_back(&ForestColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&LavaColors_p);
  palettes.push_back(&OceanColors_p);
  palettes.push_back(&PartyColors_p);
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&CloudColors_p);
  palettes.push_back(&EveningColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&WoodFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&AutumnColors_p);
  palettes.push_back(&AcidColors_p);
  palettes.push_back(&StepkosColors_p);
  palettes.push_back(&HolyLightsColors_p);
  palettes.push_back(&WaterfallColors_p);

  usepalettes = true; // активируем "авто-переключатель" палитр при изменении scale/R
  scale2pallete();    // выставляем текущую палитру
}

/**
 * palletemap - меняет указатель на текущую палитру из набора в соответствие с "ползунком"
 * @param _val - байт "ползунка"
 * @param _pals - набор с палитрами
 */
void EffectCalc::palettemap(std::vector<PGMPalette*> &_pals, const uint8_t _val, const uint8_t _min,  const uint8_t _max){
  if (!_pals.size() || _val>_max) {
    LOGD(T_Effect, println,"No palettes loaded or wrong value!");
    return;
  }
  ptPallete = (_max+0.1)/_pals.size();     // сколько пунктов приходится на одну палитру; 255.1 - диапазон ползунка, не включая 255, т.к. растягиваем только нужное :)
  palettepos = (uint8_t)(_max ? (float)_val/ptPallete : 0);
  curPalette = _pals.at(palettepos);
  palettescale = _val-ptPallete*(palettepos); // разбиваю на поддиапазоны внутри диапазона, будет уходить в 0 на крайней позиции поддиапазона, ну и хрен с ним :), хотя нужно помнить!
  
  LOGD(T_Effect, printf, "Mapping value to pallete: Psize=%d, POS=%d, ptPallete=%4.2f, palettescale=%d\n", _pals.size(), palettepos, ptPallete, palettescale);
}

/**
 * метод выбирает текущую палитру '*curPalette' из набора дотупных палитр 'palettes'
 * в соответствии со значением "бегунка" шкалы. В случае если задана паременная rval -
 * метод использует значение R,  иначе используется значение scale
 * (палитры меняются автоматом при изменении значения шкалы/дин. ползунка, метод оставлен для совместимости
 * и для первоначальной загрузки эффекта)
 */
void EffectCalc::scale2pallete(){
  if (!usepalettes)
    return;

  LOGD(T_Effect, println, "scale2pallete() Reset all controls, wtf???");
  // setbrt((*ctrls)[0]->getVal().toInt());
  // setspd((*ctrls)[1]->getVal().toInt());
  // setscl((*ctrls)[2]->getVal().toInt());

  for(unsigned i=0;i<ctrls->size();i++){
    setDynCtrl((*ctrls)[i].get());
  }
}

const String& EffectCalc::getCtrlVal(unsigned idx) {
    //return (idx<ctrls->size() && idx>=0) ? (*ctrls)[idx]->getVal() : dummy;

    // Добавлена поддержка вариантов следования индексов контролов вида 0,1,2,5,7 т.е. с пропусками
    dummy.clear();
    if(idx<ctrls->size() && idx<=2 && (*ctrls)[idx]->getId()==idx){
        return (*ctrls)[idx]->getVal();
    } else {
        for(unsigned i = 3; i<ctrls->size(); i++){
            if((*ctrls)[i]->getId()==idx){
                if(demoRndEffControls()){
                    dummy = random((*ctrls)[i]->getMin().toInt(),(*ctrls)[i]->getMax().toInt()+1);
                    return dummy;
                }
                else
                    return (*ctrls)[i]->getVal();
            }
        }
    }
    return dummy;
}

void UIControl::setVal(const String &_val) {
    switch(getType()&0x0F){
        case CONTROL_TYPE::RANGE:
        case CONTROL_TYPE::CHECKBOX:
            val=String(constrain(_val.toInt(),getMin().toInt(),getMax().toInt()));
            break;
        default:
            val=_val;
            break;
    }
}

UIControl& UIControl::operator =(const UIControl &rhs){
  id = rhs.id;
  ctype = rhs.ctype;
  control_name = rhs.control_name;
  val = rhs.val;
  min = rhs.min;
  max = rhs.max;
  step = rhs.step;
  return *this;
}

// Построение выпадающего списка эффектов для вебморды
void build_eff_names_list_file(EffectWorker &w, bool full){
  unsigned long s = millis();

  // delete existing file if any
  if(LittleFS.exists(full ? TCONST_eff_fulllist_json : TCONST_eff_list_json)){
    LittleFS.remove(full ? TCONST_eff_fulllist_json : TCONST_eff_list_json);
  }

  std::array<char, ARR_LIST_SIZE> *buff = new(std::nothrow) std::array<char, ARR_LIST_SIZE>;
  if (!buff) return;    // not enough mem

  fs::File hndlr = LittleFS.open(TCONST_eff_list_json_tmp, "w");
  
  size_t offset = 0;
  auto itr =  w.getEffectsList().cbegin();  // get const interator
  buff->at(offset++) = (char)0x5b;   // Open json with ASCII '['

  do {
    // skip effects that are excluded from selection list (main page)
    if (!full && !itr->flags.canBeSelected)
      continue;

    String effname;
    // if effect was renamed, than read it's name from json, otherwise from flash
    if (itr->flags.renamed)
      w.loadeffname(effname, itr->eff_nb);
    else
      effname = T_EFFNAMEID[(uint8_t)itr->eff_nb];
    

    // {"label":"50. Прыгуны","value":"50"}, => 30 bytes + NameLen (assume 35 to be safe)
    #define LIST_JSON_OVERHEAD  35
    //LOG(printf_P,PSTR("gen list: %d, %s\n"), itr->eff_nb, effname.c_str());

    if (ARR_LIST_SIZE - offset < effname.length() + LIST_JSON_OVERHEAD){
      // write to file and purge buffer
      hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), offset);
      offset = 0;
    }

    // create number prefix for effect name in list, i.e.  '8. '
    // for effect copies it will append clone number suffix, i.e. '75.0 '
    String name(EFF_NUMBER(itr->eff_nb));
    name += effname + MIC_SYMBOL(itr->eff_nb);    // add microphone symbol for effects that support it
    //name + (eff->eff_nb>255 ? String(" (") + String(eff->eff_nb&0xFF) + String(")") : String("")) + String(". ")

    offset += sprintf_P(buff->data()+offset, PSTR("{\"label\":\"%s\",\"value\":\"%d\"},"), name.c_str(), itr->eff_nb);
  } while (++itr != w.getEffectsList().cend());

  buff->at(--offset) = (char)0x5d;   // ASCII ']' implaced over last comma
  hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), ++offset);
  delete buff;
  hndlr.close();

  LittleFS.rename(TCONST_eff_list_json_tmp, full ? TCONST_eff_fulllist_json : TCONST_eff_list_json);
  LOGD(T_EffWrkr, printf, "\nGENERATE effects name json file for GUI(%s): %lums\n", full ? "brief" : "full", millis()-s);
}
