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
#include "log.h"

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
    omni_task_t t({e[T_active].as<bool>(), 0, e[T_descr].as<const char*>()});
    if (t.active){
      t.tid = _cronos.addCallback(e[T_crontab], [this](cronos_tid id, void* arg){ _cron_callback(id, arg); });
    }

    LOGV(T_crontab, printf, "add task:%u, tab:%s\n", t.tid, e[T_descr].as<const char*>());
    // parse commands and add it to actions container for the specified tid
    _parse_actions(t.tid, e[T_cmd].as<const char*>());
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

  for (const auto& i : _tasks){
    JsonObject item = arr.add<JsonObject>();
    item[T_active] = i.active;
    item[T_descr] = i.descr;
    //items[T_crontab] = i.
  }

}

void OmniCron::mkEmbUIpage(Interface *interf, const JsonObject *data, const char* action){
  String key(T_ui_pages_module_prefix);
  key += label;
  // load Module's structure from a EmbUI's UI data
  interf->json_frame_interface();
  interf->json_section_uidata();
  interf->uidata_pick( key.c_str() );
  // Main frame MUST be flushed before sending other ui_data sections
  interf->json_frame_flush();

  interf->json_frame_interface();
  interf->json_section_uidata();

  for (int i = 0; i !=_tasks.size(); ++i){
    String idx(i);
    interf->uidata_pick( "lampui.sections.mod_omnicron.brief", NULL, idx.c_str() );
  }
  interf->json_frame_flush();
  // prepare an object with alarms setups, loaded via js from WebUI
  interf->json_frame_jscall("alarm_items_load");

  JsonDocument doc;
  getConfig(doc.to<JsonObject>());
  interf->json_frame_add(doc);

  // otherwise just show a message that no config could be set w/o activating the widget
  //interf->uidata_pick("lampui.sections.mod_alarm.msg_inactive");

  interf->json_frame_flush();

}
