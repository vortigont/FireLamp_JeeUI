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
#include "filehelpers.hpp"
#include "embuifs.hpp"
#include "templates.hpp"
#include "actions.hpp"
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
#ifdef LAMP_DEBUG
#define WRKR_TASK_STACK         2048                  // sprintf could take lot's of stack mem for debug messages
#else
#define WRKR_TASK_STACK         1536                  // effects code should mostly allocate mem on heap
#endif
#define WRKR_TASK_NAME          "EFF_WRKR"

constexpr int target_fps{MAX_FPS};                     // desired FPS rate for effect runner
constexpr int interframe_delay_ms = 1000 / target_fps;


// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

static constexpr const char c_snd[] = "snd";

/*
// true deep-copy of UIControl ponters
void clone_controls_list(const LList<UIControl*> &src, LList<UIControl*> &dst){
  while (dst.size()){
    UIControl* c = dst.shift();
    delete c;
    c = nullptr;
  }
  LList<UIControl*>::ConstIterator i(src.cbegin());
  while(i != src.cend()){
    dst.add(new UIControl(**i));
    LOG(printf_P,PSTR("Clone ctrl: %s\n"), (*i)->getName().c_str());
    ++i;
  }
}
*/

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

bool Effcfg::_eff_cfg_deserialize(DynamicJsonDocument &doc, const char *folder){
  LOG(printf_P, PSTR("_eff_cfg_deserialize() eff:%u\n"), num);
  String filename(fshlpr::getEffectCfgPath(num,folder));

  bool retry = true;
  READALLAGAIN:
  if (embuifs::deserializeFile(doc, filename.c_str() )){
    if ( num>255 || geteffcodeversion((uint8_t)num) == doc["ver"] ){ // только для базовых эффектов эта проверка
      return true;   // we are OK
    }
    LOG(printf_P, PSTR("Wrong version in effect cfg file, reset cfg to default (%d vs %d)\n"), doc["ver"].as<uint8_t>(), geteffcodeversion((uint8_t)num));
  }
  // something is wrong with eff config file, recreate it to default
  create_eff_default_cfg_file(num, filename);   // пробуем перегенерировать поврежденный конфиг (todo: remove it and provide default from code)

  if (retry) {
    retry = false;
    goto READALLAGAIN;
  }

  LOG(printf_P, PSTR("Failed to recreate eff config file: %s\n"), filename.c_str());
  return false;
}

bool Effcfg::loadeffconfig(uint16_t nb, const char *folder){
  num = nb;
  DynamicJsonDocument doc(DYNJSON_SIZE_EFF_CFG);
  if (!_eff_cfg_deserialize(doc, folder)) return false;   // error loading file

  version = doc["ver"];
  effectName = doc["name"] ? doc["name"].as<const char*>() : T_EFFNAMEID[(uint8_t)nb];
  if (doc[c_snd])
    soundfile = doc[c_snd].as<const char*>();
  else
    soundfile.clear();

  brt = doc["brt"];
  curve = doc[A_dev_lcurve] ? static_cast<luma::curve>(doc[A_dev_lcurve].as<int>()) : luma::curve::cie1931;


  return _eff_ctrls_load_from_jdoc(doc, controls);
}

void Effcfg::create_eff_default_cfg_file(uint16_t nb, String &filename){

  const char* efname = T_EFFNAMEID[(uint8_t)nb]; // выдергиваем имя эффекта из таблицы
  LOG(printf_P,PSTR("Make default config: %d %s\n"), nb, efname);

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
  LOG(printf_P,PSTR("Writing eff #%d cfg: %s\n"), num, filename.c_str());
  configFile = LittleFS.open(filename, "w"); // PSTR("w") использовать нельзя, будет исключение!
  //configFile.w
  configFile.print(getSerializedEffConfig());
  configFile.close();
}

void Effcfg::autosave(bool force) {
  if (force){
    if(tConfigSave)
      tConfigSave->cancel();
    LOG(printf_P,PSTR("Force save eff cfg: %d\n"), num);
    _savecfg();
    //fsinforenew();
    return;
  }

  if(!tConfigSave){ // task for delayed config autosave
    tConfigSave = new Task(CFG_AUTOSAVE_TIMEOUT, TASK_ONCE, [this](){
      _savecfg();
      //fsinforenew();
      LOG(printf_P,PSTR("Autosave effect config: %d\n"), num);
    }, &ts, false, nullptr, [this](){tConfigSave=nullptr;}, true);
    tConfigSave->enableDelayed();
  } else {
    tConfigSave->restartDelayed();
  }
}

String Effcfg::getSerializedEffConfig(uint8_t replaceBright) const {
  DynamicJsonDocument doc(DYNJSON_SIZE_EFF_CFG);

  doc["nb"] = num;
  doc["flags"] = flags.mask;
  doc["name"] = effectName;
  doc["ver"] = version;
  if (brt) doc["brt"] = brt;
  if (curve != luma::curve::cie1931) doc[A_dev_lcurve] = e2int(curve);
  doc["snd"] = soundfile;
  JsonArray arr = doc.createNestedArray("ctrls");
  for (auto c = controls.cbegin(); c != controls.cend(); ++c){
    auto ctrl = c->get();
    JsonObject var = arr.createNestedObject();
    var[P_id]=ctrl->getId();
    var[P_type]=ctrl->getType();
    var["name"] = ctrl->getName();
    var["val"]  = ctrl->getVal();
    var[P_min]=ctrl->getMin();
    var[P_max]=ctrl->getMax();
    var[P_step]=ctrl->getStep();
  }

  String cfg_str;
  serializeJson(doc, cfg_str);

  return cfg_str;
}


//  ***** EffectWorker implementation *****

EffectWorker::EffectWorker(LAMPSTATE *_lampstate) : lampstate(_lampstate) {
  // create 3 'faivored' superusefull controls for 'brightness', 'speed', 'scale'
  for(int8_t id=0;id<3;id++){
    auto c = std::make_shared<UIControl>(
      id,                                     // id
      CONTROL_TYPE::RANGE,                    // type
      id==0 ? String(TINTF_00D) : id==1 ? String(TINTF_087) : String(TINTF_088)           // name
    );
    curEff.controls.add(c);
  }
  //pendingCtrls = controls;
}

//EffectWorker::~EffectWorker() { clearEffectList(); }

/*
 * Создаем экземпляр класса калькулятора в зависимости от требуемого эффекта
 */
void EffectWorker::workerset(uint16_t effect){
  LOG(printf_P,PSTR("Wrkr set: %u\n"), effect);

  LedFB<CRGB> *canvas = display.getCanvas().get();
  if (!canvas) { LOG(println, "E: no canvas buffer!"); return; }

  // load effect configuration from a saved file
  curEff.loadeffconfig(effect);

  // не создаем экземпляр калькулятора если воркер неактивен (лампа выключена и т.п.)
  if (!_status) { LOG(println, "W: worker is inactive"); return; }

  // grab mutex
  std::unique_lock<std::mutex> lock(_mtx);
  switch (static_cast<EFF_ENUM>(effect%256)) // номер может быть больше чем ENUM из-за копирований, находим эффект по модулю
  {
  case EFF_ENUM::EFF_TIME :
    worker = std::make_unique<TetrisClock>(display.getCanvas());
    break;
  case EFF_ENUM::EFF_SWIRL :
    worker = std::unique_ptr<EffectSwirl>(new EffectSwirl(canvas));
    break;
  case EFF_ENUM::EFF_COMET :
    worker = std::unique_ptr<EffectComet>(new EffectComet(canvas));
    break;
  case EFF_ENUM::EFF_FLOCK :
    worker = std::unique_ptr<EffectFlock>(new EffectFlock(canvas));
    break;
  case EFF_ENUM::EFF_PRIZMATA :
    worker = std::unique_ptr<EffectPrismata>(new EffectPrismata(canvas));
    break;
  case EFF_ENUM::EFF_SPIRO :
    worker = std::unique_ptr<EffectSpiro>(new EffectSpiro(canvas));
    break;
  case EFF_ENUM::EFF_METABALLS :
    worker = std::unique_ptr<EffectMetaBalls>(new EffectMetaBalls(canvas));
    break;
  case EFF_ENUM::EFF_SINUSOID3 :
    worker = std::unique_ptr<EffectSinusoid3>(new EffectSinusoid3(canvas));
    break;
  case EFF_ENUM::EFF_BBALS :
    worker = std::unique_ptr<EffectBBalls>(new EffectBBalls(canvas));
    break;
  case EFF_ENUM::EFF_PAINTBALL :
    worker = std::unique_ptr<EffectLightBalls>(new EffectLightBalls(canvas));
    break;
  case EFF_ENUM::EFF_PULSE :
    worker = std::unique_ptr<EffectPulse>(new EffectPulse(canvas));
    break;
  case EFF_ENUM::EFF_CUBE :
    worker = std::unique_ptr<EffectBall>(new EffectBall(canvas));
    break;
  case EFF_ENUM::EFF_LIGHTER_TRACES :
    worker = std::unique_ptr<EffectLighterTracers>(new EffectLighterTracers(canvas));
    break;
  case EFF_ENUM::EFF_RAINBOW_2D :
    worker = std::unique_ptr<EffectRainbow>(new EffectRainbow(canvas));
    break;
  case EFF_ENUM::EFF_COLORS :
    worker = std::unique_ptr<EffectColors>(new EffectColors(canvas));
    break;
  case EFF_ENUM::EFF_WHITE_COLOR :
    worker = std::unique_ptr<EffectWhiteColorStripe>(new EffectWhiteColorStripe(canvas));
    break;
  case EFF_ENUM::EFF_MATRIX :
    worker = std::unique_ptr<EffectMatrix>(new EffectMatrix(canvas));
    break;
  case EFF_ENUM::EFF_SPARKLES :
    worker = std::unique_ptr<EffectSparcles>(new EffectSparcles(canvas));
    break;
  case EFF_ENUM::EFF_EVERYTHINGFALL :
    worker = std::unique_ptr<EffectMira>(new EffectMira(canvas));
    break;
  case EFF_ENUM::EFF_FIRE2012 :
    worker = std::unique_ptr<EffectFire2012>(new EffectFire2012(canvas));
    break;
  case EFF_ENUM::EFF_SNOWSTORMSTARFALL :
    worker = std::unique_ptr<EffectStarFall>(new EffectStarFall(canvas));
    break;
  case EFF_ENUM::EFF_3DNOISE :
    worker = std::unique_ptr<Effect3DNoise>(new Effect3DNoise(canvas));
    break;
  case EFF_ENUM::EFF_CELL :
    worker = std::unique_ptr<EffectCell>(new EffectCell(canvas));
    break;
  case EFF_ENUM::EFF_T_LEND :
    worker = std::unique_ptr<EffectTLand>(new EffectTLand(canvas));
    break;
  case EFF_ENUM::EFF_OSCIL :
    worker = std::unique_ptr<EffectOscillator>(new EffectOscillator(canvas));
    break;
  case EFF_ENUM::EFF_WRAIN : 
    worker = std::unique_ptr<EffectWrain>(new EffectWrain(canvas));
    break;
  case EFF_ENUM::EFF_FAIRY : 
  case EFF_ENUM::EFF_FOUNT :
    worker = std::unique_ptr<EffectFairy>(new EffectFairy(canvas));
    break;
  case EFF_ENUM::EFF_CIRCLES :
    worker = std::unique_ptr<EffectCircles>(new EffectCircles(canvas));
    break;
  case EFF_ENUM::EFF_DRIFT :
    worker = std::unique_ptr<EffectDrift>(new EffectDrift(canvas));
    break;
  case EFF_ENUM::EFF_POPCORN :
    worker = std::unique_ptr<EffectPopcorn>(new EffectPopcorn(canvas));
    break;
  case EFF_ENUM::EFF_TWINKLES :
    worker = std::unique_ptr<EffectTwinkles>(new EffectTwinkles(canvas));
    break;
  case EFF_ENUM::EFF_WAVES :
    worker = std::unique_ptr<EffectWaves>(new EffectWaves(canvas));
    break;
  case EFF_ENUM::EFF_RADAR :
    worker = std::unique_ptr<EffectRadar>(new EffectRadar(canvas));
    break;
  case EFF_ENUM::EFF_BENGALL :
    worker = std::unique_ptr<EffectBengalL>(new EffectBengalL(canvas));
    break;
  case EFF_ENUM::EFF_BALLS :
    worker = std::unique_ptr<EffectBalls>(new EffectBalls(canvas));
    break;
  case EFF_ENUM::EFF_FIRE2018 :
    worker = std::unique_ptr<EffectFire2018>(new EffectFire2018(canvas));
    break;
  case EFF_ENUM::EFF_RINGS :
    worker = std::unique_ptr<EffectRingsLock>(new EffectRingsLock(canvas));
    break;
  case EFF_ENUM::EFF_CUBE2 :
    worker = std::unique_ptr<EffectCube2d>(new EffectCube2d(canvas));
    break;
  case EFF_ENUM::EFF_PICASSO :
  case EFF_ENUM::EFF_PICASSO4 :
    worker = std::unique_ptr<EffectPicasso>(new EffectPicasso(canvas));
    break;
  case EFF_ENUM::EFF_STARSHIPS :
    worker = std::unique_ptr<EffectStarShips>(new EffectStarShips(canvas));
    break;
  case EFF_ENUM::EFF_FLAGS :
    worker = std::unique_ptr<EffectFlags>(new EffectFlags(canvas));
    break;
  case EFF_ENUM::EFF_LEAPERS :
    worker = std::unique_ptr<EffectLeapers>(new EffectLeapers(canvas));
    break;
  case EFF_ENUM::EFF_LIQUIDLAMP :
    worker = std::unique_ptr<EffectLiquidLamp>(new EffectLiquidLamp(canvas));
    break;
  case EFF_ENUM::EFF_AQUARIUM :
    worker = std::unique_ptr<EffectAquarium>(new EffectAquarium(canvas));
    break;
  case EFF_ENUM::EFF_WHIRL :
    worker = std::unique_ptr<EffectWhirl>(new EffectWhirl(canvas));
    break;
  case EFF_ENUM::EFF_STAR :
    worker = std::unique_ptr<EffectStar>(new EffectStar(canvas));
    break;
  case EFF_ENUM::EFF_FIREWORKS :
    worker = std::unique_ptr<EffectFireworks>(new EffectFireworks(canvas));
    break;
  case EFF_ENUM::EFF_PACIFIC :
    worker = std::unique_ptr<EffectPacific>(new EffectPacific(canvas));
    break;
  case EFF_ENUM::EFF_MUNCH :
    worker = std::unique_ptr<EffectMunch>(new EffectMunch(canvas));
    break;
  case EFF_ENUM::EFF_NOISE :
    worker = std::unique_ptr<EffectNoise>(new EffectNoise(canvas));
    break;
  case EFF_ENUM::EFF_BUTTERFLY :
    worker = std::unique_ptr<EffectButterfly>(new EffectButterfly(canvas));
    break;
  case EFF_ENUM::EFF_SHADOWS :
    worker = std::unique_ptr<EffectShadows>(new EffectShadows(canvas));
    break;
  case EFF_ENUM::EFF_PATTERNS :
    worker = std::unique_ptr<EffectPatterns>(new EffectPatterns(canvas));
    break;
  case EFF_ENUM::EFF_ARROWS :
    worker = std::unique_ptr<EffectArrows>(new EffectArrows(canvas));
    break;
  case EFF_ENUM::EFF_NBALLS :
    worker = std::unique_ptr<EffectNBals>(new EffectNBals(canvas));
    break;
  case EFF_ENUM::EFF_ATTRACT :
    worker = std::unique_ptr<EffectAttract>(new EffectAttract(canvas));
    break;
  case EFF_ENUM::EFF_SNAKE :
    worker = std::unique_ptr<EffectSnake>(new EffectSnake(canvas));
    break;
  case EFF_ENUM::EFF_NEXUS :
    worker = std::unique_ptr<EffectNexus>(new EffectNexus(canvas));
    break;
  case EFF_ENUM::EFF_MAZE :
    worker = std::unique_ptr<EffectMaze>(new EffectMaze(canvas));
    break;
  case EFF_ENUM::EFF_FRIZZLES :
    worker = std::unique_ptr<EffectFrizzles>(new EffectFrizzles(canvas));
    break;
  case EFF_ENUM::EFF_POLARL :
    worker = std::unique_ptr<EffectPolarL>(new EffectPolarL(canvas));
    break;
  case EFF_ENUM::EFF_SNAKEISLAND :
    worker = std::unique_ptr<EffectSnakeIsland>(new EffectSnakeIsland(canvas));
    break;
   case EFF_ENUM::EFF_SMOKEBALLS :
    worker = std::unique_ptr<EffectSmokeballs>(new EffectSmokeballs(canvas));
    break;
   case EFF_ENUM::EFF_RACER :
    worker = std::unique_ptr<EffectRacer>(new EffectRacer(canvas));
    break;
   case EFF_ENUM::EFF_MAGMA :
    worker = std::unique_ptr<EffectMagma>(new EffectMagma(canvas));
    break;
   case EFF_ENUM::EFF_FIRE2021 :
    worker = std::unique_ptr<EffectFire2021>(new EffectFire2021(canvas));
    break;
   case EFF_ENUM::EFF_PUZZLES :
    worker = std::unique_ptr<EffectPuzzles>(new EffectPuzzles(canvas));
    break;
   case EFF_ENUM::EFF_PILE :
    worker = std::unique_ptr<EffectPile>(new EffectPile(canvas));
    break;
   case EFF_ENUM::EFF_DNA :
    worker = std::unique_ptr<EffectDNA>(new EffectDNA(canvas));
    break;
   case EFF_ENUM::EFF_SMOKER :
    worker = std::unique_ptr<EffectSmoker>(new EffectSmoker(canvas));
    break;
  case EFF_ENUM::EFF_MIRAGE :
    worker = std::unique_ptr<EffectMirage>(new EffectMirage(canvas));
    break;
  case EFF_ENUM::EFF_WATERCOLORS :
    worker = std::unique_ptr<EffectWcolor>(new EffectWcolor(canvas));
    break;
  case EFF_ENUM::EFF_RADIALFIRE :
    worker = std::unique_ptr<EffectRadialFire>(new EffectRadialFire(canvas));
    break;
  case EFF_ENUM::EFF_SPBALS :
    worker = std::unique_ptr<EffectSplashBals>(new EffectSplashBals(canvas));
    break;
  case EFF_ENUM::EFF_FLOWER :
    worker = std::unique_ptr<EffectFlower>(new EffectFlower(canvas));
    break;
#ifdef MIC_EFFECTS
  case EFF_ENUM::EFF_VU :
    worker = std::unique_ptr<EffectVU>(new EffectVU(canvas));
    break;
  case EFF_ENUM::EFF_OSC :
    worker = std::unique_ptr<EffectOsc>(new EffectOsc(canvas));
    break;

#endif

  default:
    worker = std::unique_ptr<EffectNone>(new EffectNone(canvas)); // std::unique_ptr<EffectCalc>(new EffectCalc());
  }

  // release mutex
  lock.unlock();

  if(worker){
    // окончательная инициализация эффекта тут
    worker->init(static_cast<EFF_ENUM>(effect%256), &curEff.controls, lampstate);

    // set newly loaded luma curve to the lamp
    run_action(ra::brt_lcurve, e2int(curEff.curve));
    display.canvasProtect(eff_persistent_buff[effect%256]);     // set 'persistent' frambuffer flag if effect's manifest demands it
    _start_runner();  // start calculator task IF we are marked as active
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
  String filename = fshlpr::getEffectCfgPath(nb,folder);
  LOG(printf_P,PSTR("Remove from FS: %s\n"), filename.c_str());
  LittleFS.remove(filename); // удаляем файл
}

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
#ifdef MIC_EFFECTS
    case SORT_TYPE::ST_MIC :
      effects.sort([](EffectListElem &a, EffectListElem &b){ return ((int)(pgm_read_byte(T_EFFVER + (a.eff_nb&0xFF))&0x01) - (int)(pgm_read_byte(T_EFFVER + (b.eff_nb&0xFF))&0x01)); });
      break;
#endif
    default:
      break;
  }
}

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
  DynamicJsonDocument doc(DYNJSON_SIZE_EFF_CFG);
  bool ok = embuifs::deserializeFile(doc, filename.c_str());
  if (ok && doc["name"]){
    _effectName = doc["name"].as<const char*>(); // перенакрываем именем из конфига, если есть
  } else {
    _effectName = T_EFFNAMEID[(uint8_t)nb];   // выбираем имя по-умолчанию из флеша если конфиг поврежден
  }
}

/**
* вычитать только имя\путь звука из конфиг-файла и записать в предоставленную строку
* в случае отсутствия/повреждения возвращает пустую строку
* @param effectName - String куда записать результат
* @param nb  - айди эффекта
* @param folder - какой-то префикс для каталога
*/
void EffectWorker::loadsoundfile(String& _soundfile, const uint16_t nb, const char *folder)
{
  String filename = fshlpr::getEffectCfgPath(nb,folder);
  DynamicJsonDocument doc(2048);
  bool ok = embuifs::deserializeFile(doc, filename.c_str());
  LOG(printf_P,PSTR("snd: %s\n"),doc["snd"].as<String>().c_str());
  if (ok && doc["snd"]){
    _soundfile = doc["snd"].as<String>(); // перенакрываем именем из конфига, если есть
  } else if(!ok) {
    _soundfile.clear();
  }
}


void EffectWorker::removeLists(){
  LittleFS.remove(TCONST_eff_list_json);
  LittleFS.remove(TCONST_eff_fulllist_json);
  LittleFS.remove(TCONST_eff_index);
}

void EffectWorker::makeIndexFileFromList(const char *folder, bool forceRemove)
{
  unsigned long s = millis();
  if(forceRemove)
    removeLists();

  std::array<char, ARR_LIST_SIZE> *buff = new(std::nothrow) std::array<char, ARR_LIST_SIZE>;
  if (!buff) return;    // not enough mem

  File hndlr;
  fshlpr::openIndexFile(hndlr, folder);
  effectsReSort(SORT_TYPE::ST_IDX); // сброс сортировки перед записью

  size_t offset = 0;
  auto itr =  effects.cbegin();  // get const interator
  buff->at(offset++) = (char)0x5b;   // Open json with ASCII '['

  do {
    // {"n":%d,"f":%d},   => 32 bytes is more than enough
    if (ARR_LIST_SIZE - offset < 32){
      // write to file and purge buffer
      //LOG(println,"Dumping buff...");
      hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), offset);
      offset = 0;
    }

    offset += sprintf_P(buff->data()+offset, PSTR("{\"n\":%d,\"f\":%d},"), itr->eff_nb, itr->flags.mask);
  } while (++itr != effects.cend());

  buff->at(--offset) = (char)0x5d;   // ASCII ']' implaced over last comma
  hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), ++offset);
  hndlr.close();
  delete buff;

  LOG(printf_P, PSTR("Индекс эффектов обновлен, %lums\n"), millis()-s );
  effectsReSort(); // восстанавливаем сортировку
}

// удалить эффект
void EffectWorker::deleteEffect(const EffectListElem *eff, bool isCfgRemove)
{
  for(unsigned i=0; i<effects.size(); i++){
      if(effects[i].eff_nb == eff->eff_nb){
          if(isCfgRemove)
            removeConfig(eff->eff_nb);
          effects.unlink(i);
          break;
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
  effects.add(copy);

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
    if(effects[i].eff_nb==pendingEff.num)
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
      //LOG(println,effects[i]->eff_nb);
      if (effects[i].eff_nb == select) {
          return &effects[i];
      }
  }
  LOG(printf_P, PSTR("requested eff %u not found\n"), select);
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

// получить номер эффекта смещенного на количество шагов, к ближайшему большему при превышении (для DEMO)
uint16_t EffectWorker::getByCnt(byte cnt)
{
  uint16_t firstfound = curEff.num;
  bool found = false;
  for(unsigned i=0; i<effects.size(); i++){
      if(curEff.num == effects[i].eff_nb){
          found = true;
          continue;
      }
      if(effects[i].isFavorite() && firstfound == curEff.num){
          firstfound = effects[i].eff_nb;
      }
      if(effects[i].isFavorite() && found  && effects[i].eff_nb != curEff.num){
            --cnt;
            if(!cnt){
                firstfound = effects[i].eff_nb;
                break;
            }
      }
  }
  if(cnt){ // список кончился, но до сих пор не нашли... начинаем сначала
      for(unsigned i=0; i<effects.size(); i++){
          if(effects[i].isFavorite() && effects[i].eff_nb!=curEff.num){
              --cnt;
              if(!cnt){
                  firstfound = effects[i].eff_nb;
                  break;
              }
          }
      }
  }
  return firstfound;
}

// предыдущий эффект, кроме canBeSelected==false
uint16_t EffectWorker::getPrev()
{
  if(isEffSwPending()) return pendingEff.num; // если эффект в процессе смены, то возвращаем pendingEffNum

  // все индексы списка и их синхронизация - фигня ИМХО, исходим только от curEff
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
uint16_t EffectWorker::getNext()
{
  if(isEffSwPending()) return pendingEff.num; // если эффект в процессе смены, то возвращаем pendingEffNum

  // все индексы списка и их синхронизация - фигня ИМХО, исходим только от curEff
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

void EffectWorker::switchEffect(uint16_t effnb, bool twostage){
  LOG(println, "EffectWorker::switchEffect() ");
  // NOTE: if call has been made to the SAME effect number as the current one, than it MUST be force-switched anyway to recreate EffectCalc object
  // (it's required for a cases like new LedFB has been provided, etc)
  if (effnb == curEff.num) return reset();

  // if it's a first call for two-stage switch, than we just preload coontrols and quit
  if (twostage && effnb != pendingEff.num){
    LOG(printf_P,PSTR("preloading controls for eff: %u, current eff:%u\n"), effnb, curEff.num);
    pendingEff.loadeffconfig(effnb);
    return;
  }

  curEff.flushcfg();  // сохраняем конфигурацию предыдущего эффекта если были несохраненные изменения

  // if it's a second of a two-stage call, than switch to pending
  if (twostage && isEffSwPending()){
    LOG(printf_P,PSTR("to pending %d\n"), pendingEff.num);
    workerset(pendingEff.num);      // first we change the effect
  } else {
    // other way, consider it as a direct switch to specified effect
    LOG(printf_P,PSTR("direct switch EffWorker to %d\n"), effnb);
    pendingEff.num = effnb;
    workerset(effnb);
  }

  pendingEff.controls.clear();        // no longer needed
}

void EffectWorker::fsinforenew(){
#ifdef ESP8266
    FSInfo fs_info;
    LittleFS.info(fs_info);
    if(lampstate)
    lampstate->fsfreespace = fs_info.totalBytes-fs_info.usedBytes;
#endif
#ifdef ESP32
    if(lampstate)
    lampstate->fsfreespace = LittleFS.totalBytes() - LittleFS.usedBytes();
#endif
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

void EffectWorker::setSoundfile(const String &_soundfile, EffectListElem*to){
  if(to->eff_nb==curEff.num){
    curEff.soundfile=_soundfile;
    curEff.autosave();
    return;
  }

  // load specific configuration
  Effcfg cfg(to->eff_nb);
  cfg.soundfile=_soundfile;
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

bool Effcfg::_eff_ctrls_load_from_jdoc(DynamicJsonDocument &effcfg, LList<std::shared_ptr<UIControl>> &ctrls){
  LOG(print, PSTR("_eff_ctrls_load_from_jdoc(), "));
  //LOG(printf_P, PSTR("Load MEM: %s - CFG: %s - DEF: %s\n"), effectName.c_str(), doc["name"].as<String>().c_str(), worker->getName().c_str());
  // вычитываею список контроллов
  // повторные - скипаем, нехватающие - создаем
  // обязательные контролы 0, 1, 2 - яркость, скорость, масштаб, остальные пользовательские
  JsonArray arr = effcfg["ctrls"].as<JsonArray>();
  if (!arr) return false;
  LOG(printf_P, PSTR("got arr of %u controls\n"), arr.size());

  ctrls.clear();
  uint8_t id_tst = 0x0; // пустой
  for (JsonObject item : arr) {
      uint8_t id = item["id"].as<uint8_t>();
      if(!(id_tst&(1<<id))){ // проверка на существование контрола
          id_tst |= 1<<item["id"].as<uint8_t>(); // закладываемся не более чем на 8 контролов, этого хватит более чем :)
          String name = item.containsKey("name") ?
              item["name"].as<String>()
              : id == 0 ? String(TINTF_00D)
              : id == 1 ? String(TINTF_087)
              : id == 2 ? String(TINTF_088)
              : String("Доп.")+String(id);
          String val = item.containsKey("val") ? item["val"].as<String>() : String(1);
          String min = item.containsKey("min") && id>2 ? item["min"].as<String>() : String(1);
          String max = item.containsKey("max") && id>2 ? item["max"].as<String>() : String(255);
          String step = item.containsKey("step") && id>2 ?  item["step"].as<String>() : String(1);
          CONTROL_TYPE type = item["type"].as<CONTROL_TYPE>();
          type = ((type & 0x0F)!=CONTROL_TYPE::RANGE) && id<3 ? CONTROL_TYPE::RANGE : type;
          min = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "0" : min;
          max = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "1" : max;
          step = ((type & 0x0F)==CONTROL_TYPE::CHECKBOX) ? "1" : step;
          auto c = std::make_shared<UIControl>( id, type, name, val, min, max, step );
          ctrls.add(c);
          //LOG(printf_P,PSTR("%d %d %s %s %s %s %s\n"), id, type, name.c_str(), val.c_str(), min.c_str(), max.c_str(), step.c_str());
      }
  }

  // тест стандартных контроллов
  for(int8_t id=0;id<3;id++){
      if(!((id_tst>>id)&1)){ // не найден контрол, нужно создать
        auto c = std::make_shared<UIControl>(
              id,                                     // id
              CONTROL_TYPE::RANGE,                    // type
              id==0 ? TINTF_00D : id==1 ? TINTF_087 : TINTF_088,           // name
              "127",                            // value
              "1",                              // min
              "255",                            // max
              "1"                               // step
        );
        ctrls.add(c);
      }
  }

  ctrls.sort([](std::shared_ptr<UIControl> &a, std::shared_ptr<UIControl> &b){ return (*a).getId() - (*b).getId();}); // сортирую по id
  return true;
}

void EffectWorker::_load_default_fweff_list(){
  effects.clear();

  for (uint16_t i = 0; i != 256U; i++){
    if (!strlen(T_EFFNAMEID[i]) && i)   // пропускаем индексы-"пустышки" без названия, кроме 0 "EFF_NONE"
      continue;

#ifndef MIC_EFFECTS
//    if(i>EFF_ENUM::EFF_TIME) continue;    // пропускаем эффекты для микрофона, если отключен микрофон
#endif

    EffectListElem el(i, SET_ALL_EFFFLAGS);
    effects.add(el);
  }
  LOG(printf_P, PSTR("Loaded default list of effects, %u entries\n"), effects.size());
}

void EffectWorker::_load_eff_list_from_idx_file(const char *folder){
  // todo: check if supplied alternative path starts with '/'
  String filename(folder ? folder : "");
  filename += TCONST_eff_index; // append 'eff_index.json' filename

  // if index file does not exist - load default list from firmware tables
  if (!LittleFS.exists(filename)){
    LOG(printf_P, PSTR("eff index file %s missing, loading fw defaults\n"), filename.c_str());
    return _rebuild_eff_list();
  }

  DynamicJsonDocument doc(4096);  // document for loading effects index from file

  if (!embuifs::deserializeFile(doc, filename.c_str())){
    LittleFS.remove(filename);    // remove corrupted index file
    return _rebuild_eff_list();
  }

  JsonArray arr = doc.as<JsonArray>();
  if(arr.isNull() || arr.size()==0){
    LittleFS.remove(filename);    // remove corrupted index file
    LOG(println, "eff index file corrupted, loading fw defaults");
    return _rebuild_eff_list();
  }

  effects.clear();
  for (JsonObject item : arr){
      if(item.containsKey("n")){
        EffectListElem el(item["n"].as<uint16_t>(), item["f"].as<uint8_t>());
        effects.add(el);
      }
      //LOG(printf_P,PSTR("%d : %d\n"),item["n"].as<uint16_t>(), item["f"].as<uint8_t>());
  }

  effects.sort([](EffectListElem &a, EffectListElem &b){ return a.eff_nb - b.eff_nb;}); // сортирую по eff_nb

  int32_t chk = -1; // удаляю дубликаты
  for(unsigned i=0; i<effects.size(); i++){
    if((int32_t)effects[i].eff_nb==chk){
      effects.unlink(i);
      continue;
    }
    chk = effects[i].eff_nb;
  }
  effectsReSort();
  LOG(printf_P, PSTR("Loaded list of effects, %u entries\n"), effects.size());
}

void EffectWorker::_rebuild_eff_list(const char *folder){
  LOG(println, "_rebuild_eff_list()");
  // load default fw list first
  _load_default_fweff_list();

  String sourcedir;

  if (folder) {
      sourcedir.concat("/");
      sourcedir.concat(folder);
  }
  sourcedir.concat("/eff");

#ifdef ESP8266
  Dir dir = LittleFS.openDir(sourcedir);
#endif

#ifdef ESP32
  File dir = LittleFS.open(sourcedir);
  if (!dir || !dir.isDirectory()){
    LOG(print, "Can't open dir: "); LOG(println, sourcedir);
    return;
  }
#endif

  String fn;

  DynamicJsonDocument doc(2048);

#ifdef ESP8266
  while (dir.next())
#else
  File _f;
  while(_f = dir.openNextFile())
#endif
  {   // keep this bracket, otherwise VSCode cant fold a region
#ifdef ESP8266
      fn = sourcedir + "/" + dir.fileName();
#else
      fn = sourcedir + "/" + _f.name();
#endif

    if (!embuifs::deserializeFile(doc, fn.c_str())) {
      //#ifdef ESP32
      //_f.close();
      //#endif
      LittleFS.remove(fn);                // delete corrupted config
      continue;
    }

    uint16_t nb = doc["nb"].as<uint16_t>();
    uint8_t flags = doc["flags"].as<uint8_t>();
    EffectListElem *eff = getEffect(nb);
    if(eff){  // such effect exist in list, apply flags
      flags = eff->flags.mask;
    } else {    // no such eff in list, must be an effect copy
      EffectListElem el(nb, flags);
      effects.add(el);
    }
    delay(1); // give other tasks some breathe
  }

  makeIndexFileFromList();
}

void EffectWorker::reset(){
  if (worker) workerset(getCurrent());
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
                          WRKR_TASK_CORE) == pdPASS;
}

void EffectWorker::_runnerHndlr(){
  TickType_t xLastWakeTime = xTaskGetTickCount ();

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

    // aquire mutex
    std::unique_lock<std::mutex> lock(_mtx);
    if (worker->run()){
      // effect has rendered a data in buffer, need to call the engine draw it
      display.show();
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
  workerset(getCurrent());      // spawn an instance of effect and run the task
}

void EffectWorker::stop(){
  std::unique_lock<std::mutex> lock(_mtx);
  _status = false;                  // task will self destruct on next iteration
  worker.reset();
  display.clear();
  display.canvasProtect(false);     // force clear persistent flag for frambuffer (if any) 
}


/*  *** EffectCalc  implementation  ***   */
void EffectCalc::init(EFF_ENUM eff, LList<std::shared_ptr<UIControl>> *controls, LAMPSTATE* state){
  effect = eff;
  ctrls = controls;
  _lampstate = state;

  isMicActive = isMicOnState();
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
  //if((millis() - lastrun - EFFECTS_RUN_TIMER) < (unsigned)(255-speed)/n){
  if((millis() - lastrun - delay) < (unsigned)(float(255 - speed) / n)) {
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
  if (usepalettes && _val->getName().startsWith(TINTF_084)==1){ // Начинается с Палитра
    if(isRandDemo()){
      paletteIdx = random(_val->getMin().toInt(),_val->getMax().toInt()+1);
    } else
      paletteIdx = ret_val.toInt();
    palettemap(palettes, paletteIdx, _val->getMin().toInt(), _val->getMax().toInt());
    isCtrlPallete = true;
  }

  if(_val->getId()==7 && _val->getName().startsWith(TINTF_020)==1){ // Начинается с микрофон и имеет 7 id
    isMicActive = (ret_val.toInt() && isMicOnState()) ? true : false;
#ifdef MIC_EFFECTS
    if(_lampstate)
      _lampstate->setMicAnalyseDivider(isMicActive);
#endif
  } else {
    if(isRandDemo()){ // для режима рандомного ДЕМО, если это не микрофон - то вернуть рандомное значение в пределах диапазона значений
      ret_val = String(random(_val->getMin().toInt(), _val->getMax().toInt()+1));
    }
  }

  switch(_val->getId()){
    //case 0: brightness = getBrightness(); break; // яркость всегда как есть, без рандома, но с учетом глобальности :) //LOG(printf_P,PSTR("brightness=%d\n"), brightness);
    case 1: speed = ret_val.toInt(); speedfactor = getSpeedFactor()*SPEED_ADJ; break; // LOG(printf_P,PSTR("speed=%d, speedfactor=%2.2f\n"), speed, speedfactor);
    case 2: scale = ret_val.toInt(); break;
    default: break;
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
    LOG(println,"No palettes loaded or wrong value!");
    return;
  }
  ptPallete = (_max+0.1)/_pals.size();     // сколько пунктов приходится на одну палитру; 255.1 - диапазон ползунка, не включая 255, т.к. растягиваем только нужное :)
  palettepos = (uint8_t)(_max ? (float)_val/ptPallete : 0);
  curPalette = _pals.at(palettepos);
  palettescale = _val-ptPallete*(palettepos); // разбиваю на поддиапазоны внутри диапазона, будет уходить в 0 на крайней позиции поддиапазона, ну и хрен с ним :), хотя нужно помнить!
  
  LOG(printf_P,PSTR("Mapping value to pallete: Psize=%d, POS=%d, ptPallete=%4.2f, palettescale=%d\n"), _pals.size(), palettepos, ptPallete, palettescale);
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

  LOG(println, "scale2pallete() Reset all controls, wtf???");
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
                if(isRandDemo()){
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
  LOG(printf_P, PSTR("\nGENERATE effects name json file for GUI(%s): %lums\n"), full ? "brief" : "full", millis()-s);
}
