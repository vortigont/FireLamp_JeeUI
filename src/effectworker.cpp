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
#include "templates.hpp"
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
static constexpr const char* effects_cfg_fldr = "/eff/";
static constexpr const char* effects_controls_manifest_file = "/eff/controls.json";
// LOG tags
static constexpr const char* T_EffCtrl = "EffCtrl";


// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;


EffectControl::EffectControl(
        size_t idx,
        const char* name,
        int32_t val,
        int32_t min,
        int32_t max,
        int32_t scale_min,
        int32_t scale_max
        ) : 
        _idx(idx), _name(name), _val(val), _minv(min), _maxv(max), _scale_min(scale_min), _scale_max(scale_max){

  if (_name == nullptr){
    _name = T_ctrl;
    _name += _idx;
  }

  if (_minv == _maxv){
    _minv = 1;
    _maxv = 10;
  }

  if (_scale_min == _scale_max){
    _scale_min = _minv;
    _scale_max = _maxv;
  }

  if (_val < _minv || _val > _maxv)
    _val = (_scale_max - _scale_min + 1) / 2;
}

int32_t EffectControl::setVal(int32_t v){
  _val = clamp(v, _scale_min, _scale_max);
  return getScaledVal();
}

int32_t EffectControl::getScaledVal() const {
  LOGV(T_EffCtrl, printf, "getScaledV v:%d min:%d max:%d smn:%d smx:%d\n", _val, _minv, _maxv, _scale_min, _scale_max);
  return map(_val, _scale_min, _scale_max, _minv, _maxv);
}


const char* EffectsListItem_t::getLbl(effect_t eid){
  if (static_cast<size_t>(eid) >= fw_effects_nameindex.size())
    return fw_effects_nameindex.at(0);
  else
    return fw_effects_nameindex.at(static_cast<size_t>(eid));
};


EffConfiguration::EffConfiguration(effect_t effid) : _eid(effid), _locked(false) {
  loadEffconfig(effid);
};

EffConfiguration::~EffConfiguration(){
  // save config if any changes are pending
  if (tConfigSave){
    delete tConfigSave;
    _savecfg();
  }
}

/*
bool EffConfiguration::_eff_cfg_deserialize(JsonDocument &doc, const char *folder){
  LOGD(T_EffCfg, printf, "_eff_cfg_deserialize() eff:%u\n", num);
  String filename(fshlpr::getEffectCfgPath(e2int(num), folder));

  bool retry = true;
  READALLAGAIN:
  if (embuifs::deserializeFile(doc, filename.c_str() )){
    if ( e2int(num) > 255 || geteffcodeversion((uint8_t)num) == doc["ver"] ){ // только для базовых эффектов эта проверка
      return true;   // we are OK
    }
    LOGW(T_EffConfiguration, printf, "Wrong version in effect cfg file, reset to default (%d vs %d)\n", doc["ver"].as<uint8_t>(), geteffcodeversion((uint8_t)num));
  }
  // something is wrong with eff config file, recreate it to default
  create_eff_default_cfg_file(num, filename);   // пробуем перегенерировать поврежденный конфиг (todo: remove it and provide default from code)

  if (retry) {
    retry = false;
    goto READALLAGAIN;
  }

  LOGE(T_EffConfiguration, printf, "Failed to recreate eff config file: %s\n", filename.c_str());
  return false;
}
*/

void EffConfiguration::lock(){
  flushcfg();
  _locked = true;
}

bool EffConfiguration::loadEffconfig(effect_t effid){
  if (_locked) return false;   // won't load if config is not an empty one and locked

  lock();
  _eid = effid;
  _profile_idx = 0;

  if (effid == effect_t::empty){
    _controls.clear();
    unlock();
    return true;
  }

  _load_manifest();
  _load_preset();

  unlock();
  return true;
}

bool EffConfiguration::_load_manifest(){
  _controls.clear();

  // load controls schema from manifest config
  // make a filter document
  JsonDocument filter;
  filter[EffectsListItem_t::getLbl(_eid)] = true;
  JsonDocument doc;

  DeserializationError error = embuifs::deserializeFileWFilter(doc, effects_controls_manifest_file, filter);

  if (error){
    LOGW(T_EffCfg, printf, "can't load manifest for eff:%s, err:%s\n", EffectsListItem_t::getLbl(_eid), error.c_str());
    return false;
  }

  JsonArray arr = doc[EffectsListItem_t::getLbl(_eid)][T_ctrls];

  if (!arr.size()) return false;
  
  _controls.reserve(arr.size());

  // create control objects from manifest
  size_t idx{0};
  for (JsonObject o : arr){
    _controls.emplace_back(idx, o[P_label].as<const char*>(), 0, o[T_min] | 1, o[T_max] | 10, o[T_smin] | 1, o[T_smax] | 1);
    LOGV(T_EffCfg, printf, "New Ctrl:%d %d %d %d\n", o[T_min] | 1, o[T_max] | 10, o[T_smin] | 1, o[T_smax] | 1);
    ++idx;
  }

  LOGD(T_EffCfg, printf, "Loaded %u controls from manifest for eff:%s\n", arr.size(), EffectsListItem_t::getLbl(_eid));
  return true;
}

void EffConfiguration::_load_preset(int seq){
  String fname(effects_cfg_fldr);
  fname += EffectsListItem_t::getLbl(_eid);
  JsonDocument doc;
  DeserializationError error = embuifs::deserializeFile(doc, fname);

  if (error) return;

  if (seq < 0)
    _profile_idx = doc[T_last_profile];

  JsonObject o = doc[T_profiles][_profile_idx][T_ctrls];
  if (!o.size()){
    LOGD(T_EffCfg, println, "Profile values are missing!");
    return;
  }

  size_t idx{0};
  for (JsonPair kv : o){
    setValue(idx++, kv.value());
  }
}

int32_t EffConfiguration::setValue(size_t idx, int32_t v){
  LOGV("EffCfg", printf, "Control:%u/%u, setValue:%d\n", idx, _controls.size(), v);
  if (idx < _controls.size()){
    if (_locked){
      LOGW("EffCfg", println, "Locked! Skip setValue.");
      return _controls.at(idx).getVal();
    }
    autosave();
    return _controls.at(idx).setVal(v);
  }

  // for non-existing controls let's return -1
  return -1;
}

int32_t EffConfiguration::getValue(size_t idx) const {
  if (idx < _controls.size())
    return _controls.at(idx).getVal();

  return -1;
}


/*
void EffConfiguration::create_eff_default_cfg_file(effect_t nb, String &filename){

  const char* efname = T_EFFNAMEID[(uint8_t)nb]; // выдергиваем имя эффекта из таблицы
  LOGD(T_EffCfg, printf, "Make default config: %d %s\n", nb, efname);

  String  cfg(T_EFFUICFG[(uint8_t)nb]);    // извлекаем конфиг для UI-эффекта по-умолчанию из флеш-таблицы
  cfg.replace("@name@", efname);
  cfg.replace("@ver@", String(geteffcodeversion((uint8_t)nb)) );
  cfg.replace("@nb@", String(e2int( nb)));
  
  File configFile = LittleFS.open(filename, "w");
  if (configFile){
    configFile.print(cfg.c_str());
    configFile.close();
  }
}
*/
void EffConfiguration::_savecfg(){
  String fname(effects_cfg_fldr);
  fname += EffectsListItem_t::getLbl(_eid);
  fname += ".json";
  JsonDocument doc;
  DeserializationError error = embuifs::deserializeFile(doc, fname);

  doc[T_last_profile] = _profile_idx;

  if (!doc[T_profiles].as<JsonArray>().size())
    doc[T_profiles].to<JsonArray>();

  JsonObject a;

  if ( _profile_idx >= doc[T_profiles].size() )
    a = doc[T_profiles].add<JsonObject>()[T_ctrls].to<JsonObject>();
  else {
    a = doc[T_profiles][_profile_idx][T_ctrls];
    a.clear();
  }

  makeJson(a);
/*
  for (const auto& i: _controls){
    JsonObject kv = a.add<JsonObject>();
    kv[P_id] = i.getName();
    kv[P_value] = i.getVal();
  }
*/
  LOGD(T_EffCfg, printf, "_savecfg:%s\n", fname.c_str());
  embuifs::serialize2file(doc, fname);
}

void EffConfiguration::makeJson(JsonObject o){
  for (const auto& i: _controls){
    o[i.getName()] = i.getVal();
  }
}

void EffConfiguration::autosave(bool force) {
  if (force){
    if(tConfigSave)
      tConfigSave->cancel();
    LOGD(T_EffCfg, printf, "Force save eff cfg: %u\n", _eid);
    _savecfg();
    return;
  }

  if(!tConfigSave){ // task for delayed config autosave
    tConfigSave = new Task(CFG_AUTOSAVE_TIMEOUT, TASK_ONCE, [this](){
      _savecfg();
      //fsinforenew();
      LOGD(T_EffCfg, printf, "Autosave effect config: %u\n", _eid);
    }, &ts, false, nullptr, [this](){tConfigSave=nullptr;}, true);
    tConfigSave->enableDelayed();
  } else {
    tConfigSave->restartDelayed();
  }
}
/*
String EffConfiguration::getSerializedEffConfig(uint8_t replaceBright) const {
  JsonDocument doc;

  doc["nb"] = e2int(num);
  //doc["flags"] = flags.mask;
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
*/

//  ***** EffectWorker implementation *****

EffectWorker::EffectWorker() {

}

//EffectWorker::~EffectWorker() { clearEffectList(); }

/*
 * Создаем экземпляр класса калькулятора в зависимости от требуемого эффекта
 */
void EffectWorker::_spawn(effect_t eid){
  LOGD(T_EffWrkr, printf, "_spawn(%u)\n", eid);

  LedFB<CRGB> *canvas = display.getCanvas().get();
  if (!canvas) { LOGW(T_EffWrkr, println, "no canvas buffer!"); return; }

  // не создаем экземпляр калькулятора если воркер неактивен (лампа выключена и т.п.)
  if (!_status) {
    _switch_current_effect_item(eid);
    LOGI(T_EffWrkr, println, "worker is inactive");
    return;
  }

  // save and lock previous configs
  _effCfg.lock();

  // grab mutex
  std::unique_lock<std::mutex> lock(_mtx);

  // create a new instance of effect child
  switch (eid){

   case effect_t::magma :
    worker = std::make_unique<EffectMagma>(canvas);
    LOGD(T_EffWrkr, println, "Spawn magma");
    break;

/*
  case EFF_ENUM::EFF_COMET :
    worker = std::make_unique<EffectComet>(canvas);
    break;
  case EFF_ENUM::EFF_FLOCK :
    worker = std::make_unique<EffectFlock>(canvas);
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
  case EFF_ENUM::EFF_fireflies :
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
  case EFF_ENUM::EFF_OSCIL :
    worker = std::make_unique<EffectOscillator>(canvas);
    break;
  case EFF_ENUM::EFF_FAIRY : 
  case EFF_ENUM::EFF_FOUNT :
    worker = std::make_unique<EffectFairy>(canvas);
    break;
  case EFF_ENUM::EFF_CIRCLES :
    worker = std::make_unique<EffectCircles>(canvas);
    break;
  case EFF_ENUM::EFF_TWINKLES :
    worker = std::make_unique<EffectTwinkles>(canvas);
    break;
  case EFF_ENUM::EFF_WAVES :
    worker = std::make_unique<EffectWaves>(canvas);
    break;
  case EFF_ENUM::EFF_BALLS :
    worker = std::make_unique<EffectBalls>(canvas);
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
  case EFF_ENUM::EFF_liquidlamp :
    worker = std::make_unique<EffectLiquidLamp>(canvas);
    break;
  case EFF_ENUM::EFF_WHIRL :
    worker = std::make_unique<EffectWhirl>(canvas);
    break;
  case EFF_ENUM::EFF_STAR :
    worker = std::make_unique<EffectStar>(canvas);
    break;
  case effect_t::attractor :
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
   case EFF_ENUM::EFF_smokeballs :
    worker = std::make_unique<EffectSmokeballs>(canvas);
    break;
   case EFF_ENUM::EFF_FIRE2021 :
    worker = std::make_unique<EffectFire2021>(canvas);
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
  case EFF_ENUM::EFF_WATERCOLORS :
    worker = std::make_unique<EffectWcolor>(canvas);
    break;
  case EFF_ENUM::EFF_RADIALFIRE :
    worker = std::make_unique<EffectRadialFire>(canvas);
    break;
  case EFF_ENUM::EFF_SPBALS :
    worker = std::make_unique<EffectSplashBals>(canvas);
    break;
*/
  default:
    LOGW(T_EffWrkr, println, "Attempt to spawn nonexistent effect!");
    lock.unlock();  // release mutex
    _effCfg.unlock();
    return;
  }

  if (!worker){
    lock.unlock();
    _effCfg.unlock();
    // unable to create worker object somehow
    _switch_current_effect_item(effect_t::empty);
    return;
  }

  // initialize effect
  worker->load();

  _switch_current_effect_item(eid);
  // apply effect's controls
  applyControls();

  display.canvasProtect (worker->getCanvasProtect());         // set 'persistent' frambuffer flag if effect's manifest demands it

  // release mutex after effect init has complete
  lock.unlock();
  _effCfg.unlock();
  _start_runner();  // start calculator task IF we are marked as active

  // set newly loaded luma curve to the lamp
  run_action(ra::brt_lcurve, e2int(_effItem.curve));

  // send event    
  uint32_t n = e2int(eid);
  EVT_POST_DATA(LAMP_CHANGE_EVENTS, e2int(evt::lamp_t::effSwitchTo), &n, sizeof(uint32_t));
}

void EffectWorker::loadIndex(){
  // first generate a list from fw constants
  _load_default_fweff_list();


  if (!LittleFS.exists(F_effects_idx)){
    LOGD(T_EffWrkr, printf, "eff index file %s missing\n", F_effects_idx);
    makeIndexFileFromList();
    return;
  }

  // merge data from FS index file, it containes changed values for flags, etc...
  JsonDocument doc;
  embuifs::deserializeFile(doc, F_effects_idx);

  JsonArray arr = doc.as<JsonArray>();
  size_t len = arr.size();
  size_t eff_len = effects.size();

  for (JsonObject o : arr){
    effect_t eid = static_cast<effect_t>( o[P_id].as<unsigned>() );
    auto i = std::find_if(effects.begin(), effects.end(), [eid](const EffectsListItem_t &e){ return e.eid == eid; });
    if (i == effects.end())
      continue;

    i->flags.hidden = o[P_hidden];
    i->flags.disabledInDemo = o[T_demoDisabled];
    i->curve = static_cast<luma::curve>( o[T_luma].as<unsigned>() );
  }

  // if fw list and json lists have different size then save current list to file
  if (len != eff_len){
    LOGD(T_EffWrkr, println, "Update effects index file");
    makeIndexFileFromList();
  }
}

/*
void EffectWorker::removeConfig(const uint16_t nb, const char *folder){
  LOGD(T_EffWrkr, printf, "Remove from FS: %s\n", fshlpr::getEffectCfgPath(nb,folder).c_str());
  LittleFS.remove(fshlpr::getEffectCfgPath(nb,folder)); // удаляем файл
}
*/

/*
void EffectWorker::effectsReSort(SORT_TYPE _effSort)
{
  LOG(printf_P,PSTR("*Пересортировка эффектов*: %d\n"), _effSort);
  if(_effSort==255) _effSort=effSort; // Для дефолтного - берем с конфига

  switch(_effSort){
    case SORT_TYPE::ST_BASE :
      effects.sort([](EffectsListItem_t &a, EffectsListItem_t &b){ return (((a.eff_nb&0xFF) - (b.eff_nb&0xFF))<<8) + (((a.eff_nb&0xFF00) - (b.eff_nb&0xFF00))>>8);});
      break;
    case SORT_TYPE::ST_END :
      effects.sort([](EffectsListItem_t &a, EffectsListItem_t &b){ return a.eff_nb - b.eff_nb;}); // сортирую по eff_nb
      //effects.sort([](EffectsListItem_t *&a, EffectsListItem_t *&b){ return ((int32_t)(((a->eff_nb&0xFF)<<8) | ((a->eff_nb&0xFF00)>>8)) - (((b->eff_nb&0xFF)<<8) | ((b->eff_nb&0xFF00)>>8)));});
      //effects.sort([](EffectsListItem_t *&a, EffectsListItem_t *&b){ return (a->eff_nb&0xFF00) - (b->eff_nb&0xFF00) + (((a->eff_nb&0xFF) - (b->eff_nb&0xFF))<<8) + (((a->eff_nb&0xFF00) - (b->eff_nb&0xFF00))>>8);});
      break;
    case SORT_TYPE::ST_IDX :
      effects.sort([](EffectsListItem_t &a, EffectsListItem_t &b){ return (int)(a.getMS() - b.getMS());});
      break;
    case SORT_TYPE::ST_AB2 :
      // крайне медленный вариант, с побочными эффектами, пока отключаю и использую вместо него ST_AB
      //effects.sort([](EffectsListItem_t *&a, EffectsListItem_t *&b){ EffectWorker *tmp = new EffectWorker((uint16_t)0); String tmp1; tmp->loadeffname(tmp1, a->eff_nb); String tmp2; tmp->loadeffname(tmp2,b->eff_nb); delete tmp; return strcmp_P(tmp1.c_str(), tmp2.c_str());});
      //break;
    case SORT_TYPE::ST_AB :
      effects.sort([](EffectsListItem_t &a, EffectsListItem_t &b){ return std::string_view(T_EFFNAMEID[(uint8_t)a.eff_nb]).compare(T_EFFNAMEID[(uint8_t)b.eff_nb]); });
      break;
    case SORT_TYPE::ST_MIC :
      effects.sort([](EffectsListItem_t &a, EffectsListItem_t &b){ return ((int)(pgm_read_byte(T_EFFVER + (a.eff_nb&0xFF))&0x01) - (int)(pgm_read_byte(T_EFFVER + (b.eff_nb&0xFF))&0x01)); });
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
/*
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
*/

void EffectWorker::removeLists(){
  LittleFS.remove(TCONST_eff_list_json);
  LittleFS.remove(TCONST_eff_fulllist_json);
  LittleFS.remove(F_effects_idx);
}

void EffectWorker::makeIndexFileFromList(bool forceRemove){
  LOGD(T_EffWrkr, println, "writing effects index file" );
  if(forceRemove)
    removeLists();

  // need to save current element first
  auto idx = _effItem.eid;
  auto i = std::find_if(effects.begin(), effects.end(), [idx](const EffectsListItem_t &e){ return e.eid == idx; });
  if (i != effects.end()){
    (*i) = _effItem;
  }

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (const auto& i : effects){
    JsonObject o = arr.add<JsonObject>();
    o[T_idx] = e2int(i.eid);
    o[P_label] = EffectsListItem_t::getLbl(i.eid);
    o[P_hidden] = i.flags.hidden;
    o[T_demoDisabled] = i.flags.disabledInDemo;
    o[T_luma] = e2int( i.curve );
  }

  embuifs::serialize2file(doc, F_effects_idx);
}

void EffectWorker::_switch_current_effect_item(effect_t eid){
  //LOGV(T_EffWrkr, printf, "_switch_current_effect_item: %u\n", eid);
  if (eid == effect_t::empty){
    _effItem = effects.front();
  } else {
    // change current effect element to the one from a list
    //auto idx = _effItem.eid;
    auto i = std::find_if(effects.begin(), effects.end(), [eid](const EffectsListItem_t &e){ return e.eid == eid; });
    if (i == effects.end()){
      // something is crazy wrong, there is no such effect in a list with given eid, switch to default empty one
      _effItem = effects.front();
      LOGI(T_EffWrkr, printf, "_switch_current_effect_item: %u not found!\n", eid);
    } else {
      _effItem = *i;
      LOGV(T_EffWrkr, printf, "_switch_current_effect_item: %u:%s\n", i->eid, i->getLbl());
    }
  }
 
  // load effect configuration from a saved file
  _effCfg.loadEffconfig(_effItem.eid);
}

// удалить эффект
/*
void EffectWorker::deleteEffect(const EffectsListItem_t *eff, bool onlyCfgFile)
{
  // TODO rewrite it!
  for (auto i = effects.begin(); i != effects.end(); ++i){
    if ( (*i).eff_nb == eff->eff_nb){
          if(onlyCfgFile)
            removeConfig(e2int( eff->eff_nb ));    // only remove eff json cfg file
          else
            i->flags.hidden = true;
            //effects.erase(i);             // remove effect from drop-down selection list
            // TODO: need to update a list???
          return;
    }
  }
}
*/

// копирование эффекта
/*
void EffectWorker::copyEffect(const EffectsListItem_t *base)
{
  EffectsListItem_t copy(base); // создать копию переданного эффекта
  uint16_t maxfoundnb=base->eff_nb;
  for(unsigned i=0; i<effects.size();i++){
    if(effects[i].eff_nb>255 && ((effects[i].eff_nb&0x00FF)==(copy.eff_nb&0x00FF))){ // найдены копии
      if(maxfoundnb < effects[i].eff_nb) maxfoundnb=effects[i].eff_nb;
    }
  }

  int16_t newnum =(((((maxfoundnb & 0xFF00)>>8)+1) << 8 ) | (copy.eff_nb&0xFF)); // в старшем байте увеличиваем значение на число имеющихся копий
  copy.eff_nb = newnum;
  effects.push_back(std::move(copy));

  EffConfiguration copycfg(base->eff_nb);
  copycfg.num = newnum;
  // имя формируем с базового + индекс копии
  copycfg.effectName.concat("_");
  copycfg.effectName.concat(newnum>>8);

  // save new config file
  copycfg.autosave(true);
  removeLists();              // drop cached lists
  makeIndexFileFromList();    // rebuild index (it will be faster than wait to hit _rebuild_eff_list() )
}
*/
#ifdef DISABLED_CODE
// вернуть эффект на очереди из списка эффектов
EffectsListItem_t *EffectWorker::getSelectedListElement()
{
  for(unsigned i=0; i<effects.size(); i++){
    if(effects[i].eff_nb == curEff.num)
      return &effects[i];
  }
  return nullptr;
}

// вернуть текущий элемент списка
EffectsListItem_t *EffectWorker::getCurrentListElement()
{
  for(unsigned i=0; i<effects.size(); i++){
    if(effects[i].eff_nb==curEff.num)
      return &effects[i];
  }
  return nullptr;
}

// вернуть выбранный элемент списка
EffectsListItem_t *EffectWorker::getFirstEffect()
{
  if(effects.size()>0)
    return &effects[0];
  else
    return nullptr; // NONE
}

// вернуть выбранный элемент списка
EffectsListItem_t *EffectWorker::getEffect(effect_t select){
  for (unsigned i = 0; i < effects.size(); i++) {
      if (effects[i].eff_nb == select) {
          return &effects[i];
      }
  }
  LOGW(T_Effect, printf, "requested eff %u not found\n", select);
  return nullptr; // NONE
}

// вернуть следующий эффект, если передан nullptr, то возвращается первый
EffectsListItem_t *EffectWorker::getNextEffect(EffectsListItem_t *current){
    if(current == nullptr) return getFirstEffect();
    for (unsigned i = 0; i < effects.size(); i++) {
        if (effects[i].eff_nb == current->eff_nb) {
            return i+1<effects.size() ? &effects[i+1] : nullptr;
        }
    }
    return nullptr; // NONE
}
#endif

effect_t EffectWorker::getNextEffIndexForDemo(bool rnd){
  if (!effects.size()) return effect_t::empty;

  // look for random effect
  if (rnd){
    size_t attempt{20};     // limit number of iterations
    long n;
    do {
      n = random(1, effects.size()-1);
    } while ( ( effects.at(n).flags.disabledInDemo || effects.at(n).flags.hidden) && --attempt);

    return effects.at(n).eid;
  }

  // otherwise find current effect in a list
  auto idx = _effItem.eid;
  auto i = std::find_if(effects.begin(), effects.end(), [idx](const EffectsListItem_t &e){ return e.eid == idx; });

  // не нашли текущий эффект, возвращаем случайный
  if (i == effects.end())
    return getNextEffIndexForDemo(true);

  // ищем следующий доступный эффект для демо после текущего
  while ( ++i != effects.end()){
    if (i->flags.hidden || i->flags.disabledInDemo)
      continue;

    return i->eid;
  }

  // если не нашли, ищем с начала списка пропуская первую пустоту
  i = effects.begin();
  while ( ++i != effects.end()){
    if (i->flags.hidden || i->flags.disabledInDemo)
      continue;

    return i->eid;
  }

  // if nothing found, then return current effect
  return _effItem.eid;
}

// предыдущий эффект, кроме enabled==false
effect_t EffectWorker::getPrev(){
  if (!effects.size()) return effect_t::empty;

  // find current effect in a list
  auto idx = _effItem.eid;
  auto i = std::find_if(effects.begin(), effects.end(), [idx](const EffectsListItem_t &e){ return e.eid == idx; });

  // quite strange if there is no current effect found
  if (i == effects.end())
    return _effItem.eid;

  // look for as may times as there are elements
  size_t cnt = effects.size();
  do {
    // rollover if we are at begining
    if (i == effects.begin())
      i = effects.end();

    --i;

    if (!i->flags.hidden)
      return i->eid;

  } while (--cnt);

  // last resort
  return _effItem.eid;
}

// следующий эффект, кроме enabled==false
effect_t EffectWorker::getNext(){
  if (!effects.size()) return effect_t::empty;

  // find current effect in a list
  auto idx = _effItem.eid;
  auto i = std::find_if(effects.begin(), effects.end(), [idx](const EffectsListItem_t &e){ return e.eid == idx; });

  // quite strange if there is no current effect found
  if (i == effects.end())
    return _effItem.eid;

  // look for as may times as there are elements
  size_t cnt = effects.size();
  do {
    ++i;
    // rollover if we are at end
    if (i == effects.end()){
      i = effects.begin();
      ++i;    // skip empty
    }


    if (!i->flags.hidden)
      return i->eid;

  } while (--cnt);

  // last resort
  return _effItem.eid;
}

void EffectWorker::switchEffect(effect_t eid){
  // NOTE: if call has been made to the SAME effect number as the current one, than it MUST be force-switched anyway to recreate EffectCalc object
  // (it's required for a cases like new LedFB has been provided, etc)
  if (eid == _effItem.eid) return reset();

  LOGD(T_EffWrkr, printf, "switchEffect:%u\n", eid);
  _spawn(eid);
}

/*
void EffectWorker::setEffectName(const String &name, EffectsListItem_t*to){
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
  EffConfiguration cfg(to->eff_nb);
  cfg.effectName = name;
  cfg.autosave(true);
}
*/
#ifdef DISABLED_CODE
bool EffConfiguration::_eff_ctrls_load_from_jdoc(JsonDocument &EffConfiguration, std::vector<std::shared_ptr<UIControl>> &ctrls){
  LOGD(T_Effect, print, "_eff_ctrls_load_from_jdoc(), ");
  //LOG(printf_P, PSTR("Load MEM: %s - CFG: %s - DEF: %s\n"), effectName.c_str(), doc[T_name].as<String>().c_str(), worker->getName().c_str());
  // вычитываею список контроллов
  // повторные - скипаем, нехватающие - создаем
  // обязательные контролы 0, 1, 2 - яркость, скорость, масштаб, остальные пользовательские
  JsonArray arr = EffConfiguration[T_ctrls].as<JsonArray>();
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
          JsonVariant v = item[T_name];
          String name;
          if (v.is<const char*>())
            name = v.as<const char*>();
          else if (id == 1){
            name = TINTF_087;
          } else if (id == 2)
            name = TINTF_088;
          else {
            name = "Доп.";
            name += id;
          }

          v = item[T_val];
          String val( v.is<JsonVariant>() ? v.as<int>() : 128 );
          v = item[T_min];
          String min( v.is<JsonVariant>() && id>2 ? v.as<int>() : 1 );
          v = item[T_max];
          String max( v.is<JsonVariant>() && id>2 ? v.as<int>() : 255 );
          v = item[T_step];
          String step( v.is<JsonVariant>() && id>2 ?  v.as<int>() : 1);
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
#endif //DISABLED_CODE

void EffectWorker::_load_default_fweff_list(){
  effects.clear();
  effects.reserve(fw_effects_index.size());

  for (const auto& i : fw_effects_index){
    effects.emplace_back(i);
  }

  LOGD(T_EffWrkr, printf, "Loaded default list of effects, %u entries\n", effects.size());
}

/*
void EffectWorker::_load_eff_list_from_idx_file(){
  // if index file does not exist - load default list from firmware tables
  if (!LittleFS.exists(F_effects_idx)){
    LOGD(T_EffWrkr, printf, "eff index file %s missing, loading fw defaults\n", F_effects_idx);
    return _load_default_fweff_list();
  }

  JsonDocument doc;  // document for loading effects index from file

  if (!embuifs::deserializeFile(doc, F_effects_idx)){
    LittleFS.remove(F_effects_idx);    // remove corrupted index file
    return _load_default_fweff_list();
  }

  JsonArray arr = doc.as<JsonArray>();
  if(arr.size() == 0){
    LittleFS.remove(F_effects_idx);    // remove corrupted index file
    LOGW(T_EffWrkr, println, "eff index file corrupted, loading fw defaults");
    return _load_default_fweff_list();
  }

  effects.clear();
  for (JsonObject item : arr){
      eff_flags_t f;
      f.hidden = item[P_hidden];
      f.disabledInDemo = item[T_demoDisabled];
      effects.emplace_back( static_cast<effect_t>( item[T_idx].as<unsigned>() ), f);
      //LOG(printf_P,PSTR("%d : %d\n"),item["n"].as<uint16_t>(), item["f"].as<uint8_t>());
  }
  LOGD(T_EffWrkr, printf, "Loaded list of effects, %u entries\n", effects.size());
}
*/
/*
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
    EffectsListItem_t *eff = getEffect(nb);
    if(eff){  // such effect exist in list, apply flags
      flags = eff->flags.mask;
    } else {    // no such eff in list, must be an effect copy
      effects.emplace_back(nb, flags);
    }
    delay(1); // give other tasks some breathe
  }

  makeIndexFileFromList();
}
*/

void EffectWorker::reset(){
  if (worker) _spawn(getCurrentEffectNumber());
}

void EffectWorker::setLumaCurve(luma::curve c){
  if (c == _effItem.curve) return;  // quit if same value
  _effItem.curve = c;

  makeIndexFileFromList();
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
    if ( xTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(interframe_delay_ms) ) != pdTRUE ) {
    // if task has not been delayed, than we can't keep up with desired frame rate, let's give other tasks time to run anyway
      taskYIELD();
    }

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
  _spawn(getCurrentEffectNumber());      // spawn an instance of effect and run the task
}

void EffectWorker::stop(){
  _status = false;                  // task will self destruct on next iteration
  display.canvasProtect(false);     // force clear persistent flag for frambuffer (if any) 
}

void EffectWorker::applyControls(){
  if (!worker) return;
  for (const auto& i : _effCfg.getControls()){
    //LOGV(T_EffWrkr, printf, "apply ctrl %u\n", i.getIdx());
    worker->setControl(i.getIdx(), i.getScaledVal());
  }
}

void EffectWorker::setControlValue(size_t idx, int32_t v){
  if (idx >= _effCfg.getControls().size()){
    LOGW(T_EffWrkr, printf, "attempt to set non-exiting control:%u\n", idx);
    return;
  }
  
  if (worker)
    worker->setControl(idx, _effCfg.setValue(idx, v));
  autoSaveConfig();
};

////////////////////////////////////////////
/*  *** EffectCalc  implementation  ***   */


/**
 * проверка на холостой вызов для эффектов с доп. задержкой
 */
bool EffectCalc::dryrun(float n, uint8_t delay){
  if((millis() - lastrun - delay) < (unsigned)((255 - speed) / n)) {
    return false;
  }
  
  lastrun = millis();
  return true;
}

/**
 * status - статус воркера, если работает и загружен эффект, отдает true
 */
//bool EffectCalc::status(){return active;}

/**
 * setDynCtrl - была смена динамического контрола, idx=3+
 * вызывается в UI, для реализации особого поведения (палитра и т.д.)...
 * https://community.alexgyver.ru/threads/wifi-lampa-budilnik-proshivka-firelamp_jeeui-gpl.2739/page-112#post-48848
 */
/*
String EffectCalc::setDynCtrl(UIControl*_val){
  if(!_val)
    return String();
  String ret_val = _val->getVal();
  //LOG(printf_P, PSTR("ctrlVal=%s\n"), ret_val.c_str());
  if ( usepalettes && starts_with(_val->getName().c_str(), TINTF_084) ){ // Начинается с Палитра
    //if(demoRndEffControls()){
    //  paletteIdx = random(_val->getMin().toInt(),_val->getMax().toInt()+1);
    //} else
      paletteIdx = ret_val.toInt();
    palettemap(palettes, paletteIdx, _val->getMin().toInt(), _val->getMax().toInt());
    isCtrlPallete = true;
  }

  // имеет 7 id и начинается со строки "микрофон" с локализацией
  if( _val->getId()==7 && starts_with(_val->getName().c_str(), TINTF_020) ){
    // не будем заниматься ерундой и включать/выключать микрофон из конфига эффекта, для этого есть глобальный флажек
    // 
  } else {
    //if(demoRndEffControls()){ // для режима рандомного ДЕМО, если это не микрофон - то вернуть рандомное значение в пределах диапазона значений
    //  ret_val = String(random(_val->getMin().toInt(), _val->getMax().toInt()+1));
    //}
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

*/
// Load palletes into array
void EffectCalc::palettesload(){
  palettes.clear();
  palettes.reserve(FASTLED_PALETTS_COUNT);
  palettes.push_back(&AcidColors_p);
  palettes.push_back(&AlcoholFireColors_p);
  palettes.push_back(&AuroraColors_p);
  palettes.push_back(&AutumnColors_p);
  palettes.push_back(&CloudColors_p);
  palettes.push_back(&CopperFireColors_p);
  palettes.push_back(&EveningColors_p);
  palettes.push_back(&ForestColors_p);
  palettes.push_back(&HeatColors_p);
  palettes.push_back(&HolyLightsColors_p);
  palettes.push_back(&LavaColors_p);
  palettes.push_back(&LithiumFireColors_p);
  palettes.push_back(&NormalFire_p);
  palettes.push_back(&OceanColors_p);
  palettes.push_back(&PartyColors_p);
  palettes.push_back(&PotassiumFireColors_p);
  palettes.push_back(&RainbowColors_p);
  palettes.push_back(&RubidiumFireColors_p);
  palettes.push_back(&SodiumFireColors_p);
  palettes.push_back(&StepkosColors_p);
  palettes.push_back(&WaterfallColors_p);
  palettes.push_back(&WoodFireColors_p);
}


void EffectCalc::setControl(size_t idx, int32_t value){
  switch (idx){
    // speed control
    case 0:
      speed = value;
      LOGD(T_Effect, printf, "Eff speed:%d\n", value);
      break;
    // scale control
    case 1:
      scale = value;
      LOGD(T_Effect, printf, "Eff scale:%d\n", value);
      break;
    // pelette switch
    case 2:
      if (value >= palettes.size()){
        LOGV(T_Effect, printf, "palette idx out of bound:%d of %u\n", value, palettes.size());
        return;
      }
      curPalette = palettes.at(value);
      LOGD(T_Effect, printf, "Eff pallete:%d\n", value);
      break;

    default :;
  }
}

#ifdef DISABLED_CODE
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
                //if(demoRndEffControls()){
                //    dummy = random((*ctrls)[i]->getMin().toInt(),(*ctrls)[i]->getMax().toInt()+1);
                //    return dummy;
                //}
                //else
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
    if (!full && itr->flags.hidden)
      continue;

    // effect's 
    String effname = fw_effects_nameindex.at( e2int(itr->eff_nb) );
    // if effect was renamed, than read it's name from json, otherwise from flash
/*
    if (itr->flags.renamed)
      w.loadeffname(effname, itr->eff_nb);
    else
      effname = T_EFFNAMEID[(uint8_t)itr->eff_nb];
*/

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
    //String name(EFF_NUMBER(itr->eff_nb));
    String name(e2int(itr->eff_nb));

    offset += sprintf_P(buff->data()+offset, PSTR("{\"label\":\"%s\",\"value\":\"%d\"},"), name.c_str(), itr->eff_nb);
  } while (++itr != w.getEffectsList().cend());

  buff->at(--offset) = (char)0x5d;   // ASCII ']' implaced over last comma
  hndlr.write(reinterpret_cast<uint8_t*>(buff->data()), ++offset);
  delete buff;
  hndlr.close();

  LittleFS.rename(TCONST_eff_list_json_tmp, full ? TCONST_eff_fulllist_json : TCONST_eff_list_json);
  LOGD(T_EffWrkr, printf, "\nGENERATE effects name json file for GUI(%s): %lums\n", full ? "brief" : "full", millis()-s);
}

#endif
