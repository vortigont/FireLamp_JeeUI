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
#include "embui_constants.h"        // EmbUI string literals
#include "templates.hpp"
#include "actions.hpp"
#include "evtloop.h"
#include "display.hpp"
#include "templates.hpp"
#include "log.h"

#ifndef MAX_FPS
#define MAX_FPS               (50U)                   // Максимальное число обсчитываемых и выводимых кадров в секунду
#endif

#define WRKR_TASK_CORE          CONFIG_ARDUINO_RUNNING_CORE    // task MUST be pinned to the second core to avoid LED glitches (if applicable)
#define WRKR_TASK_PRIO          tskIDLE_PRIORITY+1    // task priority
#ifdef LAMP_DEBUG_LEVEL
#define WRKR_TASK_STACK         2048                  // sprintf could take lot's of stack mem for debug messages
#else
#define WRKR_TASK_STACK         1536                  // effects code should mostly allocate mem on heap
#endif
#define WRKR_TASK_NAME          "EFF_WRKR"

#define MAX_NUM_OF_CTRL_PROFILES  10
#define CFG_AUTOSAVE_TIMEOUT       (60*1000U)         // таймаут сохранения конфигурации эффекта, по умолчанию - 60 секунд

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
    _val = (_maxv - _minv + 1) / 2;
}

int32_t EffectControl::setVal(int32_t v){
  _val = clamp(v, _minv, _maxv);
  return getScaledVal();
}

int32_t EffectControl::getScaledVal() const {
  LOGV(T_EffCtrl, printf, "getScaledV idx:%u v:%d scaled:%d min:%d max:%d smn:%d smx:%d\n", _idx, _val, map(_val,  _minv, _maxv, _scale_min, _scale_max), _minv, _maxv, _scale_min, _scale_max);
  return map(_val,  _minv, _maxv, _scale_min, _scale_max);
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

DeserializationError EffConfiguration::_load_cfg(JsonDocument& doc){
  String fname(effects_cfg_fldr);
  fname += EffectsListItem_t::getLbl(_eid);
  fname += T__json;

  DeserializationError error = embuifs::deserializeFile(doc, fname);

  if (error) {
    LOGD("EffCfg", printf, "can't load file:%s\n", fname.c_str());
  }
  return error;
}

void EffConfiguration::_lock(){
  if (_locked) return;
  _locked = true;
  flushcfg();
}

bool EffConfiguration::loadEffconfig(effect_t effid){
  if (_locked) return false;   // won't load if locked

  _lock();
  _eid = effid;
  _preset_idx = 0;

  if (effid == effect_t::empty){
    _controls.clear();
    _unlock();
    return true;
  }

  LOGI(T_EffCfg, printf, "loadEffconfig:%u:%s\n", _eid, EffectsListItem_t::getLbl(_eid));
  _load_manifest();
  // load controls values from an fs json file
  // switch to the last saved preset if any
  switchPreset();

  // no need to publish UI page if no one is listening
  if (embui.feeders.available()){
    auto interf = std::make_unique<Interface>(&embui.feeders);
    mkEmbUIpage(interf.get());
  }

  _unlock();
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

  if (!arr.size()){
    LOGE(T_EffCfg, printf, "manifest for eff %u:%s, err:%s is empty!\n", _eid, EffectsListItem_t::getLbl(_eid), error.c_str());
    return false;
  }

  _controls.reserve(arr.size());

  // create control objects from manifest
  size_t idx{0};
  for (JsonObject o : arr){
    _controls.emplace_back(idx, o[P_label].as<const char*>(), o[P_value], o[T_min] | 1, o[T_max] | 10, o[T_smin] | 1, o[T_smax] | 1);
    LOGV(T_EffCfg, printf, "_load_manifest Ctrl:%u v:%d %d %d %d %d\n", idx, o[P_value] | -1, o[T_min] | 1, o[T_max] | 10, o[T_smin] | 1, o[T_smax] | 1);
    ++idx;
  }

  LOGD(T_EffCfg, printf, "Loaded %u ctrls from manifest for eff: %u:%s\n", arr.size(), _eid, EffectsListItem_t::getLbl(_eid));
  return true;
}

void EffConfiguration::switchPreset(int32_t idx){
  JsonDocument doc;

  if (_load_cfg(doc)){  // if error
    return;
  }
  flushcfg();
  _switchPreset(idx, doc);
  
}

void EffConfiguration::_switchPreset(int32_t idx, JsonVariant doc){
  LOGI(T_EffCfg, printf, "_switchPreset to:%d\n", idx);

  // restore last used profile if specified one is wrong or < 0
  if (idx < 0 || idx >= MAX_NUM_OF_CTRL_PROFILES)
    _preset_idx = doc[T_last_profile] | 0;
  else
    _preset_idx = idx;

  // get total num of presets
  if (doc[T_profiles].size())
    _presets_total = doc[T_profiles].size();

  JsonObject o = doc[T_profiles][_preset_idx][T_ctrls];
  if (!o.size()){
    LOGD(T_EffCfg, printf, "preset is missing for idx:%d\n", idx);
    return;
  }

  // restore label
  JsonVariant lbl = o[P_label];
  if (lbl.is<const char*>()){
    _profile_lbl = lbl.as<const char*>();
  } else {
    LOGD(T_EffCfg, println, "preset's label is bad!");
    _profile_lbl = T_profile;
    _profile_lbl += _preset_idx;
  }

  size_t i{0};
  for (JsonPair kv : o){
    //LOGV("EffCfg", printf, "restore ctrl:%u value:%d\n", idx, kv.value().as<int>());
    if (i < _controls.size())
      _controls.at(i).setVal(kv.value());
    ++i;
  }

}

int32_t EffConfiguration::setValue(size_t idx, int32_t v){
  LOGD("EffCfg", printf, "Control:%u/%u, setValue:%d\n", idx, _controls.size(), v);
  if (idx < _controls.size()){
    if (_locked){
      LOGW("EffCfg", println, "Locked! Skip setValue.");
      return _controls.at(idx).getScaledVal();
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

void EffConfiguration::_savecfg(){
  JsonDocument doc;
  _load_cfg(doc);
  _savecfg(doc);
}

void EffConfiguration::_savecfg(JsonVariant doc){

  doc[T_last_profile] = _preset_idx;

  if (!doc[T_profiles].as<JsonArray>().size())
    doc[T_profiles].to<JsonArray>();

  JsonObject a;

  if ( _preset_idx >= doc[T_profiles].size() )
    a = doc[T_profiles].add<JsonObject>()[T_ctrls].to<JsonObject>();
  else {
    a = doc[T_profiles][_preset_idx][T_ctrls];
    a.clear();
  }

  doc[T_profiles][_preset_idx][P_label] = _profile_lbl.c_str();

  for (const auto& i: _controls){
    a[i.getName()] = i.getVal();
  }
/*
  for (const auto& i: _controls){
    JsonObject kv = a.add<JsonObject>();
    kv[P_id] = i.getName();
    kv[P_value] = i.getVal();
  }
*/
  String fname(effects_cfg_fldr);
  fname += EffectsListItem_t::getLbl(_eid);
  fname += T__json;
  LOGD(T_EffCfg, printf, "_savecfg:%s\n", fname.c_str());
  embuifs::serialize2file(doc, fname);
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
      LOGD(T_EffCfg, printf, "Autosave effect #%u\n", _eid);
    }, &ts, false, nullptr, [this](){tConfigSave=nullptr;}, true);
    tConfigSave->enableDelayed();
  } else {
    tConfigSave->restartDelayed();
  }
}

void EffConfiguration::mkEmbUIpage(Interface *interf){
  interf->json_frame_interface();
  // load effect's controls from uidata
  interf->json_section_uidata();
      String key( "lampui.effControls." );
      key += EffectsListItem_t::getLbl(_eid);
      interf->uidata_pick( key.c_str() );

  _jscall_preset_list_rebuild(interf);

  // publish control values
  embui_control_vals(interf);
}

void EffConfiguration::_jscall_preset_list_rebuild(Interface *interf){
  if (interf){
    JsonObject o = interf->json_frame_jscall(T_mk_eff_profile_list);
    o[T_Effect] = EffectsListItem_t::getLbl(_eid);
    o[P_idx] = _preset_idx;
    interf->json_frame_flush();
  }
}

void EffConfiguration::embui_control_vals(Interface *interf) const {
  interf->json_frame_value();
    // publish current effect index (for drop-down selector)
    interf->value(A_effect_switch_idx, e2int(_eid) );
    // publish control values
    size_t idx{0};
    for (const auto &i : _controls){
        String id(A_effect_control);
        id += idx++;
        interf->value(id, i.getVal());
    }
    // controls preset index
    interf->value(A_eff_preset, _preset_idx );
  interf->json_frame_flush();
}

void EffConfiguration::embui_preset_rename(Interface *interf, JsonObjectConst data, const char* action){
  JsonVariantConst lbl = data[P_label];
  if (!lbl.is<const char*>()) return;   // some bad data

  _profile_lbl = lbl.as<const char*>();
  _savecfg();
  _jscall_preset_list_rebuild(interf);
}

void EffConfiguration::embui_preset_clone(Interface *interf){
  if (_presets_total >= MAX_NUM_OF_CTRL_PROFILES) return;   // do not allow too many preset clones

  _preset_idx = _presets_total;
  ++_presets_total;
  _savecfg();
  _jscall_preset_list_rebuild(interf);
}

void EffConfiguration::embui_preset_delete(Interface *interf){
  if (_preset_idx == 0) return;   // won't remove last profile

  JsonDocument doc;
  if (_load_cfg(doc)){  // if error
    return;
  }

  doc[T_profiles].remove(_preset_idx);
  --_preset_idx;

  _switchPreset(_preset_idx, doc);

  _savecfg(doc);
  _jscall_preset_list_rebuild(interf);
}



///////////////////////////////////////////
//  ***** EffectWorker implementation *****

EffectWorker::EffectWorker() {
  // сформировать и опубликовать блок контролов текущего эффекта
  embui.action.add(A_effect_ctrls, [&](Interface *interf, JsonObjectConst data, const char* action){ _effCfg.embui_control_vals(interf); });
  // preset switcher
  embui.action.add(A_eff_preset, [&](Interface *interf, JsonObjectConst data, const char* action){ switchEffectPreset(data[A_eff_preset]); _effCfg.embui_control_vals(interf); });
  embui.action.add(A_eff_preset_lbl, [&](Interface *interf, JsonObjectConst data, const char* action){ _effCfg.embui_preset_rename(interf, data, action); });
  embui.action.add(A_eff_preset_new, [&](Interface *interf, JsonObjectConst data, const char* action){ _effCfg.embui_preset_clone(interf); });
  embui.action.add(A_eff_preset_remove, [&](Interface *interf, JsonObjectConst data, const char* action){ _effCfg.embui_preset_delete(interf); });

}

EffectWorker::~EffectWorker(){
  if(_runnerTask_h) vTaskDelete(_runnerTask_h);
  _runnerTask_h = nullptr;
  embui.action.remove(A_effect_ctrls);
  embui.action.remove(A_eff_preset_lbl);
  embui.action.remove(A_eff_preset_new);
  embui.action.remove(A_eff_preset_remove);
  embui.action.remove(A_eff_preset);
};

/*
 * Создаем экземпляр класса калькулятора в зависимости от требуемого эффекта
 */
void EffectWorker::_spawn(effect_t eid){
  LOGD(T_EffWrkr, printf, "_spawn %u:%s\n", eid, EffectsListItem_t::getLbl(eid));

  LedFB<CRGB> *canvas = display.getCanvas().get();
  if (!canvas) { LOGW(T_EffWrkr, println, "no canvas buffer!"); return; }

  // не создаем экземпляр калькулятора если воркер неактивен (лампа выключена и т.п.)
  if (!_status) {
    _switch_current_effect_item(eid);
    LOGI(T_EffWrkr, println, "worker is inactive");
    return;
  }

  // grab mutex
  std::unique_lock<std::mutex> lock(_mtx);

  // create a new instance of effect child
  switch (eid){

   case effect_t::empty :
    worker = std::make_unique<EffectNone>(canvas);
    break;

   case effect_t::bouncingballs :
    worker = std::make_unique<EffectBBalls>(canvas);
    break;

  case effect_t::fireveil :
    worker = std::make_unique<EffectFireVeil>(canvas);
    break;

   case effect_t::fire2012 :
    worker = std::make_unique<EffectFire2012>(canvas);
    break;

   case effect_t::liquidlamp :
    worker = std::make_unique<EffectLiquidLamp>(canvas);
    break;

   case effect_t::magma :
    worker = std::make_unique<EffectMagma>(canvas);
    break;

   case effect_t::metaballs :
    worker = std::make_unique<EffectMetaBalls>(canvas);
    break;

  case effect_t::nexus :
    worker = std::make_unique<EffectNexus>(canvas);
    break;

  case effect_t::picassoBalls :
    worker = std::make_unique<EffectPicassoMetaBalls>(canvas);
    break;

  case effect_t::picassoShapes :
    worker = std::make_unique<EffectPicassoShapes>(canvas);
    break;

  case effect_t::radialfire :
    worker = std::make_unique<EffectRadialFire>(canvas);
    break;
   case effect_t::smokeballs :
    worker = std::make_unique<EffectSmokeballs>(canvas);
    break;

/*
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
  case EFF_ENUM::EFF_MAZE :
    worker = std::make_unique<EffectMaze>(canvas);
    break;
  case EFF_ENUM::EFF_FRIZZLES :
    worker = std::make_unique<EffectFrizzles>(canvas);
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
  case EFF_ENUM::EFF_SPBALS :
    worker = std::make_unique<EffectSplashBals>(canvas);
    break;
*/
  default:
    LOGW(T_EffWrkr, println, "Attempt to spawn nonexistent effect!");
    lock.unlock();  // release mutex
    return;
  }

  if (!worker){
    lock.unlock();
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

void EffectWorker::makeIndexFileFromList(){
  LOGD(T_EffWrkr, println, "writing effects index file" );

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

void EffectWorker::switchEffectPreset(int32_t preset){
  _effCfg.switchPreset(preset);
  applyControls();
}

void EffectWorker::_load_default_fweff_list(){
  effects.clear();
  effects.reserve(fw_effects_index.size());

  for (const auto& i : fw_effects_index){
    effects.emplace_back(i);
  }

  LOGD(T_EffWrkr, printf, "Loaded default list of effects, %u entries\n", effects.size());
}

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
        LOGD(T_lamp, printf, "Eff:%u, FPS: %u, heap:%uk\n", getCurrentEffectNumber(), fps/10, ESP.getFreeHeap()/1024);
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
  LOGD(T_EffWrkr, println, "apply ctrls");
  for (const auto& i : _effCfg.getControls()){
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

void EffectWorker::embui_publish(Interface *interf) const {
  if (!interf){
    // no need to publish UI page if no one is listening
    if (!embui.feeders.available()) return;

    auto iface = std::make_unique<Interface>(&embui.feeders);
    _effCfg.embui_control_vals(iface.get());
    return;
  }

  _effCfg.embui_control_vals(interf);
}


////////////////////////////////////////////
/*  *** EffectCalc  implementation  ***   */

/**
 * проверка на холостой вызов для эффектов с доп. задержкой
 */
bool EffectCalc::dryrun(){
  // randomly skip frames proportional to 1/speed
  if (random8() > speed )
    return false;
  //if((millis() - lastrun - delay) < (unsigned)((255 - speed) / n)) {}
  
  //lastrun = millis();
  return true;
}

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
  palettes.push_back(&NeonColors_p);
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
      LOGV(T_Effect, printf, "Eff speed:%d\n", value);
      break;
    // scale control
    case 1:
      scale = value;
      LOGV(T_Effect, printf, "Eff scale:%d\n", value);
      break;
    // pelette switch
    case 2:
      if (value >= palettes.size()){
        LOGW(T_Effect, printf, "palette idx out of bound:%d of %u\n", value, palettes.size());
        return;
      }
      curPalette = palettes.at(value);
      LOGV(T_Effect, printf, "Eff pallete:%d\n", value);
      break;

    default :;
  }
}
