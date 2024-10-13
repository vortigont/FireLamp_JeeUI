/*
    This file is a part of FireLamp_JeeUI project
    https://github.com/vortigont/FireLamp_JeeUI

    Copyright © 2023-2024 Emil Muratov (vortigont)

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

#include "omnicron.hpp"
#include "embuifs.hpp"
#include "char_const.h"
#include <sstream>
#include "EmbUI.h"
#include "log.h"

// EmbUI handlers
static constexpr const char* A_get_mod_omnicron_task = "get_mod_omnicron_task";
static constexpr const char* A_set_mod_omnicron_task = "set_mod_omnicron_task";
static constexpr const char* A_set_mod_omnicron_task_rm = "set_mod_omnicron_task_rm";
static constexpr const char* T_mod_omnicron_bottom = "lampui.sections.mod_omnicron.bottom";
static constexpr const char* T_mod_omnicron_task = "lampui.pages.module.omnicron_task";

OmniCron::OmniCron() : GenericModule(T_omnicron, false){
  // add EmbUI's handler to get Cron's task config
  embui.action.add(A_get_mod_omnicron_task, [this](Interface *interf, JsonObjectConst data, const char* action){ _task_get(interf, data, action); } );

  embui.action.add(A_set_mod_omnicron_task, [this](Interface *interf, JsonObjectConst data, const char* action){ _task_set(interf, data, action); } );

  embui.action.add(A_set_mod_omnicron_task_rm, [this](Interface *interf, JsonObjectConst data, const char* action){ _task_remove(interf, data, action); } );

}

void OmniCron::start(){
  _cronos.start();
}

void OmniCron::stop(){
  _cronos.stop();
}

void OmniCron::load_cfg(JsonVariantConst cfg){
  // Crontab tasks
  JsonArrayConst tabs = cfg[T_event];

  _tasks.clear();
  _cronos.clear();

  LOGD(T_crontab, printf, "Loading %u crontab rules\n", tabs.size());

  for (JsonVariantConst e : tabs){
    omni_task_t t(
      {
        e[T_active].as<bool>(),
        0,
        e[T_descr].as<const char*>(),
        e[T_crontab].as<const char*>(),
        e[T_cmd].as<const char*>()
      }
    );

    if (t.active){
      t.tid = _cronos.addCallback(e[T_crontab], [this](cronos_tid id, void* arg){ _cron_callback(id, arg); });

      // parse commands and add it to actions container for the specified tid
      _parse_actions(t.tid, e[T_cmd].as<const char*>());
    }

    LOGV(T_crontab, printf, "add task:%u, tab:%s\n", t.tid, t.descr.data());
    _tasks.emplace_back(std::move(t));
  }
}

void OmniCron::_cron_callback(cronos_tid id, void* arg){
  LOGD(T_crontab, printf, "exec task:%u\n", id);
  for (auto& i : _actions){
    if (i.id != id)
      continue;

    LOGV(T_crontab, printf, "cmd:%d, arg:%d\n", i.cmd, i.param);
    // send command via event bus
    EVT_POST_DATA(LAMP_SET_EVENTS, i.cmd, &i.param, sizeof(i.param));
  }
}

void OmniCron::_parse_actions(cronos_tid id, const char* expr){
  // this will parse a simple strings like  '<num>=<num>,...', i.e. '10=42, 11=100, 15=42'
  std::istringstream iss;
  iss.str (expr);

  int cmd = 0, param = 0;
  char ch1, ch2;  // '=' and ',' sepparators

  /*
    WARN! This sstream pulls locales into firmware, it grows around 100k in size and 4k DRAM
    I need to rewrite this parser!
    wlocale-inst.o
    locale-inst.o
    cxx11-wlocale-inst.o
    cxx11-locale-inst.o
  */

  while(iss >> cmd >> ch1 >> param){
    LOGV(T_crontab, printf, "add id:%u cmd:%d\n", id, cmd);
    _actions.emplace_back( id, cmd, param );
    if(iss.tellg() == -1)
        break;
    else
        iss >> ch2;
  }
}

void OmniCron::_purge_actions(cronos_tid id){
  auto i = _actions.begin();
  for ( ; i != _actions.end();){
    if (i->id == id)
      i = _actions.erase(i);
    else
      ++i;
  }
}

void OmniCron::generate_cfg(JsonVariant cfg) const {
  JsonArray arr = cfg[T_event].to<JsonArray>();
  LOGV(T_crontab, printf, "serializing %u tasks\n", _tasks.size());
  for (const auto& i : _tasks){
    JsonObject item = arr.add<JsonObject>();
    item[T_active] = i.active;
    item[T_descr] = i.descr;
    item[T_crontab] = i.crontab;
    item[T_cmd] = i.cmd;
  }
}

void OmniCron::mkEmbUIpage(Interface *interf, JsonObjectConst data, const char* action){
  String key(T_ui_pages_module_prefix);
  key += label;
  // load Module's structure from a EmbUI's UI data
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( key.c_str() );
/*
  // Main frame MUST be flushed before sending other ui_data sections
  interf->json_frame_flush();
*/
  // prepare an object with cron rules, loaded via js from WebUI
  interf->json_frame_jscall("omnicron_tasks_load");
    JsonDocument doc;
    generate_cfg(doc.to<JsonObject>());
    interf->json_object_add(doc);
//  interf->json_frame_flush();

  // 'new' and 'exit' buttons
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( T_mod_omnicron_bottom );
  interf->json_frame_flush();
}

void OmniCron::_task_get(Interface *interf, JsonObjectConst data, const char* action){
  int idx = data[action];
  LOGD(T_crontab, printf, "Get crontab task:%d\n", idx);

  // first, build a page with task's setup
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( T_mod_omnicron_task );
  interf->json_frame_flush();

  // check if it's a real task id has been requested that I can post value data
  if (idx < 0 || idx >= _tasks.size())
    return;

  auto t = _tasks.at(idx);

  interf->json_frame_value();

  interf->value(T_active, t.active);
  interf->value(T_descr, t.descr);
  interf->value(T_crontab, t.crontab);
  interf->value(T_idx, idx);
  interf->value(T_cmd, t.cmd);

/*
  String cmd;
  cmd.reserve(64);

  for (const auto& c : _actions){
    if (c.id == t->tid){
      cmd += c.cmd;
      cmd += (char)0x3d;  // '='
      cmd += c.param;
      cmd += (char)0x2c;  // ','
    }
  }

  cmd.remove(cmd.length()-1, 1);
*/
  interf->json_frame_flush();
}

void OmniCron::_task_set(Interface *interf, JsonObjectConst data, const char* action){
  int idx = data[T_idx];

  if (idx >= static_cast<int>(_tasks.size())){
    LOGV(T_crontab, printf, "Wrong task idx:%d\n", idx);
    return;
  }
  LOGD(T_crontab, printf, "Set task with idx:%d\n", idx);

  if (idx == -1){
    // this is a NEW rule
    omni_task_t t({data[T_active].as<bool>(), 0, data[T_descr].as<const char*>(), data[T_crontab].as<const char*>(), data[T_cmd].as<const char*>()});
    if (t.active){
      // load task and it's actions to cronos
      t.tid = _cronos.addCallback(data[T_crontab], [this](cronos_tid id, void* arg){ _cron_callback(id, arg); });
      _parse_actions(t.tid, t.cmd.data());
    }

    _tasks.emplace_back(std::move(t));
  } else {
    // it's an update for existing rule
    omni_task_t& t = _tasks.at(idx);

    // first purge all tasks's rules
    _purge_actions(t.tid);
    t.descr = data[T_descr].as<const char*>();
    t.crontab = data[T_crontab].as<const char*>();
    t.cmd = data[T_cmd].as<const char*>();

    // if tasks's state has changed
    bool new_state = data[T_active].as<bool>();

    if (new_state){
      if (t.active){
        // task has been active already, need to update the expression only
        _cronos.setExpr(t.tid, t.crontab.data());
      } else {
        // need to create a new task
        t.tid = _cronos.addCallback(data[T_crontab], [this](cronos_tid id, void* arg){ _cron_callback(id, arg); });
      }
      _parse_actions(t.tid, t.cmd.data());
    } else {
      // new task's state if inactive, simply remove task from cronos if any
      _cronos.removeTask(t.tid);
      t.tid = 0;
    }

    // update new task's state
    t.active = new_state;
  }

  _cronos.reload();

  // load OmniCron's tasks list page
  mkEmbUIpage(interf, {}, NULL);

  // save tasks to file
  save();
}

void OmniCron::_task_remove(Interface *interf, JsonObjectConst data, const char* action){
  auto i = _tasks.begin() + data[action].as<int>();

  if (i == _tasks.end())
    return;

  _purge_actions(i->tid);
  _cronos.removeTask(i->tid);
  _tasks.erase(i);

  // load OmniCron's tasks list page
  mkEmbUIpage(interf, {}, NULL);
}