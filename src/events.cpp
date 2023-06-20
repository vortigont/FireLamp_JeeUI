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

/*
    реализация планировщика событий

*/

#include "alarm.h"
#include "events.h"
#include "actions.hpp"

void EVENT_MANAGER::check_event(DEV_EVENT *event)
{
    if(!event->isEnabled || cb_func==nullptr) return;
    if(event->unixtime > *TimeProcessor::now() ) return;

    time_t _now = *(TimeProcessor::now());
    // точно попадает в период времени 1 минута, для однократных событий
    if(event->unixtime==_now)
    {
        cb_func(event); // сработало событие
        return;
    }

    struct tm t;
    tm *tme=&t;
    localtime_r(&(event->unixtime), tme);
    //LOG(printf_P, PSTR("%d %d\n"),current_time, eventtime);
    const tm* tm = localtime(&_now);

    // если сегодня + периодический
    if(event->repeat && event->unixtime <= _now && tm->tm_yday == tme->tm_yday){
        //LOG(printf_P, PSTR("%d %d\n"),hour(current_time)*60+minute(current_time), event->repeat);
        int tmdiff = ( tm->tm_hour * 60 + tm->tm_min )-( tme->tm_hour * 60 + tme->tm_min ); 
        if(tmdiff>=0 && !(tmdiff%event->repeat)){
            if(((tm->tm_hour * 60 + tm->tm_min)<=(tme->tm_hour * 60 + tme->tm_min + event->stopat)) || !event->stopat){ // еще не вышли за ограничения окончания события или его нет
                cb_func(event); // сработало событие
                return;
            }
        }
    }
    //uint8_t cur_day = tm->tm_wday-1; // 1 == Sunday
    uint8_t cur_day = tm->tm_wday; // 1 == Sunday
    if(!cur_day) cur_day = 7; // 7 = Sunday
    if((event->raw_data>>cur_day)&1) { // обрабатывать сегодня
        if(event->unixtime<=_now){ // время события было раньше/равно текущего
            //LOG(printf_P, PSTR("%d %d\n"),hour(current_time)*60+minute(current_time), event->repeat);
            if(tm->tm_hour==tme->tm_hour && tm->tm_min==tme->tm_min){ // точное совпадение
                cb_func(event); // сработало событие
                return;
            }
            if(event->repeat && tm->tm_hour>=tme->tm_hour){ // периодический в сегодняшний день
                int tmdiff = ( tm->tm_hour * 60 + tm->tm_min )-( tme->tm_hour * 60 + tme->tm_min ); 
                if(tmdiff>=0 && !(tmdiff%event->repeat)){
                    if(((tm->tm_hour * 60 + tm->tm_min)<=(tme->tm_hour * 60 + tme->tm_min+event->stopat)) || !event->stopat){ // еще не вышли за ограничения окончания события или его нет
                        cb_func(event); // сработало событие
                        return;
                    }
                }
            }
        }
    }
}

DEV_EVENT *EVENT_MANAGER::addEvent(const DEV_EVENT&event)
{
    DEV_EVENT *new_event = new DEV_EVENT(event);
    events->add(new_event);
    return new_event;
}

void EVENT_MANAGER::delEvent(const DEV_EVENT&event) {
    for(unsigned i=0;i<events->size();i++){
        if(*((*events)[i])==event){
            events->remove(i); // удаляет из списка и чистит память
        }
    }
}

bool EVENT_MANAGER::isEnumerated(const DEV_EVENT&event) {
    for(unsigned i=0;i<events->size();i++){
        if(*((*events)[i])==event){
            return true;
        }
    }
    return false;
}

void EVENT_MANAGER::events_handle()
{
    // пропускаем все ненулевые секунды
    if(!TimeProcessor::seconds00())
        return;

    for(unsigned i=0;i<events->size();i++){
        check_event((*events)[i]);
    }
}

void EVENT_MANAGER::clear_events()
{
    events->clear();
}

void EVENT_MANAGER::loadConfig(const char *cfg)
{
    if(LittleFS.begin()){
        clear_events();
        DynamicJsonDocument doc(4096);
        String filename = cfg ? String(cfg) : String(F("/events_config.json"));
        if (!embuifs::deserializeFile(doc, filename.c_str())){
            LOG(print, F("deserializeJson error: "));
            LOG(println, filename);
            return;
        }
        JsonArray arr = doc.as<JsonArray>();
        DEV_EVENT event;
        for (size_t i=0; i<arr.size(); i++) {
            JsonObject item = arr[i];
            event.raw_data = item[F("raw")].as<int>();
            event.unixtime = item[F("ut")].as<time_t>();
            event.event = (EVENT_TYPE)(item[F("ev")].as<int>());
            event.repeat = item[F("rp")].as<int>();
            event.stopat = item[F("sa")].as<int>();
            event.message = item[F("msg")].as<String>();
            DEV_EVENT *new_event = addEvent(event);
            if(new_event){
                LOG(printf_P, PSTR("[%u - %llu - %u - %u - %u - %s]\n"), new_event->raw_data, (unsigned long long)new_event->unixtime, new_event->event, new_event->repeat, new_event->stopat, new_event->message.c_str());
            }
        }

        LOG(println, F("Events config loaded"));
        doc.clear();
    }
}

void EVENT_MANAGER::saveConfig(const char *cfg)
{
    if(LittleFS.begin()){
        File configFile;
        if(cfg == nullptr)
            configFile = LittleFS.open(F("/events_config.json"), "w"); // PSTR("w") использовать нельзя, будет исключение!
        else
            configFile = LittleFS.open(cfg, "w"); // PSTR("w") использовать нельзя, будет исключение!
        configFile.print("[");
        
        LOG(println, F("Save events config"));
        bool firstLine=true;
        DEV_EVENT *next;
        for(unsigned i=0;i<events->size();i++){
            next = (*events)[i];
            LOG(printf_P, PSTR("%s{\"raw\":%u,\"ut\":%llu,\"ev\":%u,\"rp\":%u,\"sa\":%u,\"msg\":\"%s\"}"),
                (!firstLine?",":""), next->raw_data, (unsigned long long)next->unixtime, next->event, next->repeat, next->stopat, next->message.c_str());
            configFile.printf_P(PSTR("%s{\"raw\":%u,\"ut\":%llu,\"ev\":%u,\"rp\":%u,\"sa\":%u,\"msg\":\"%s\"}"),
                (!firstLine?",":""), next->raw_data, (unsigned long long)next->unixtime, next->event, next->repeat, next->stopat, next->message.c_str());
            firstLine=false;
        }
        configFile.print("]");
        configFile.flush();
        configFile.close();
        LOG(println, F("\nSave events config"));
    }
}

// обработка эвентов лампы
void event_worker(DEV_EVENT *event){
    RA action = RA_UNKNOWN;
    LOG(printf_P, PSTR("%s - %s\n"), ((DEV_EVENT *)event)->getName().c_str(), TimeProcessor::getInstance().getFormattedShortTime().c_str());

    switch (event->getEvent()) {
    case EVENT_TYPE::ON : {
        run_action(ra::on);
        if (!event->getMessage().isEmpty()){
            // вывести текст на лампу через 3 секунды
            StringTask *t = new StringTask(event->getMessage().c_str(), 3 * TASK_SECOND, TASK_ONCE, nullptr, &ts, false, nullptr,  [](){
                StringTask *cur = (StringTask *)ts.getCurrentTask();
                myLamp.sendString(cur->getData());
            }, true);
            t->enableDelayed();
        }
        return;
    }
    case EVENT_TYPE::OFF: return run_action(ra::off);
    case EVENT_TYPE::DEMO: return run_action(ra::demo, event->getMessage()=="1");       // not sure what is the content of this String
    case EVENT_TYPE::ALARM: return ALARMTASK::startAlarm(&myLamp, event->getMessage().c_str());               // взводим будильник
    //case EVENT_TYPE::LAMP_CONFIG_LOAD: action = RA_LAMP_CONFIG; break;                // была какая-то загрузка стороннего конфига embui
#ifdef ESP_USE_BUTTON
    // load button configuration from side file
    case EVENT_TYPE::BUTTONS_CONFIG_LOAD: return load_button_config(event->getMessage().c_str());
#endif
    //case EVENT_TYPE::EFF_CONFIG_LOAD:  action = RA_EFF_CONFIG; break;                 // была какая-то мутная загрузка индекса эффектов из папки /backup/idx
    case EVENT_TYPE::EVENTS_CONFIG_LOAD:  return load_events_config(event->getMessage().c_str());
    case EVENT_TYPE::SEND_TEXT: return myLamp.sendString(event->getMessage().c_str());
    case EVENT_TYPE::SEND_TIME: return myLamp.showTimeOnScreen(event->getMessage().c_str(), true);
    case EVENT_TYPE::AUX_ON:  return run_action(ra::aux, static_cast<bool>(event->getMessage().toInt()));
    case EVENT_TYPE::AUX_OFF: return run_action(ra::aux, static_cast<bool>(event->getMessage().toInt()));
    case EVENT_TYPE::AUX_TOGGLE:  return run_action(ra::aux_flip);
    case EVENT_TYPE::PIN_STATE: {
        if ((event->getMessage()).isEmpty()) break;

        String tmpS = event->getMessage();
        tmpS.replace(F("'"),F("\"")); // так делать не красиво, но шопаделаешь...
        StaticJsonDocument<256> doc;
        deserializeJson(doc, tmpS);
        JsonArray arr = doc.as<JsonArray>();
        for (size_t i = 0; i < arr.size(); i++) {
            JsonObject item = arr[i];
            uint8_t pin = item[FPSTR(TCONST_pin)].as<int>();
            String action = item[FPSTR(TCONST_act)].as<String>();
            pinMode(pin, OUTPUT);
            switch(action.c_str()[0]){
                case 'H':
                    digitalWrite(pin, HIGH); // LOW
                    break;
                case 'L':
                    digitalWrite(pin, LOW); // LOW
                    break;
                case 'T':
                    digitalWrite(pin, !digitalRead(pin)); // inverse
                    break;
                default:
                    break;
            }
        }
        break;
    }
    case EVENT_TYPE::SET_EFFECT: { run_action(ra::eff_switch, event->getMessage().toInt()); return; }       // switch effect
    case EVENT_TYPE::SET_WARNING: {
        /*
        WARN: this does not work reliably due to inability to properly deserialize nested json configuration
        from event file. msg that comes from the WebUI has unescaped quotes.
        TODO: rework ugly save routine
        */
        StaticJsonDocument<256> msg;
        Serial.println(event->getMessage());
        DeserializationError error = deserializeJson(msg, event->getMessage());
        if (!error) return;
        StaticJsonDocument<64> ev;
        ev[TCONST_event].shallowCopy(msg);
        JsonObject j = ev.as<JsonObject>();
        run_action(ra::warn, &j);
        return;
    }
    case EVENT_TYPE::SET_GLOBAL_BRIGHT: { run_action(ra::brt_global, static_cast<bool>(event->getMessage().toInt())); return; }
    case EVENT_TYPE::SET_WHITE_HI: action = RA_WHITE_HI; break;
    case EVENT_TYPE::SET_WHITE_LO: action = RA_WHITE_LO; break;
    default:;
    }

    remote_action(action, event->getMessage().c_str(), NULL);
}

String DEV_EVENT::getName() {
    String buffer;
    String day_buf = FPSTR(T_EVENT_DAYS);

    buffer.concat(isEnabled?F(" "):F("!"));
    
    TimeProcessor::getDateTimeString(buffer, unixtime);

    buffer.concat(F(","));
    switch (event)
    {
    case EVENT_TYPE::ON:
        buffer.concat(F("ON"));
        break;
    case EVENT_TYPE::OFF:
        buffer.concat(F("OFF"));
        break;
    case EVENT_TYPE::ALARM:
        buffer.concat(F("ALARM"));
        break;
    case EVENT_TYPE::DEMO:
        buffer.concat(F("DEMO"));
        break;
    case EVENT_TYPE::LAMP_CONFIG_LOAD:
        buffer.concat(F("LMP_GFG"));
        break;
    case EVENT_TYPE::EFF_CONFIG_LOAD:
        buffer.concat(F("EFF_GFG"));
        break;
#ifdef ESP_USE_BUTTON
    case EVENT_TYPE::BUTTONS_CONFIG_LOAD:
        buffer.concat(F("BUT_GFG"));
        break;
#endif
    case EVENT_TYPE::EVENTS_CONFIG_LOAD:
        buffer.concat(F("EVT_GFG"));
        break;
    case EVENT_TYPE::SEND_TEXT:
        buffer.concat(F("TEXT"));
        break;
    case EVENT_TYPE::SEND_TIME:
        buffer.concat(F("TIME"));
        break;
    case EVENT_TYPE::PIN_STATE:
        buffer.concat(F("PIN"));
        break; 
    case EVENT_TYPE::AUX_ON:
        buffer.concat(F("AUX ON"));
        break; 
    case EVENT_TYPE::AUX_OFF:
        buffer.concat(F("AUX OFF"));
        break; 
    case EVENT_TYPE::AUX_TOGGLE:
        buffer.concat(F("AUX TOGGLE"));
        break; 
    case EVENT_TYPE::SET_EFFECT:
        buffer.concat(F("EFFECT"));
        break;
    case EVENT_TYPE::SET_WARNING:
        buffer.concat(F("WARNING"));
        break;
    case EVENT_TYPE::SET_GLOBAL_BRIGHT:
        buffer.concat(F("GLOBAL BR"));
        break;
    case EVENT_TYPE::SET_WHITE_HI:
        buffer.concat(F("WHITE HI"));
        break;
    case EVENT_TYPE::SET_WHITE_LO:
        buffer.concat(F("WHITE LO"));
        break;
    default:
        break;
    }

    if(repeat) {buffer.concat(F(",")); buffer.concat(repeat);}
    if(repeat && stopat) { buffer.concat(F(",")); buffer.concat(stopat);}

    uint8_t t_raw_data = raw_data>>1;
    if(t_raw_data)
        buffer.concat(F(","));
    for(uint8_t i=1;i<8; i++){
        if(t_raw_data&1){
            //Serial.println, day_buf.substring((i-1)*2*2,i*2*2)); // по 2 байта на символ UTF16
            buffer.concat(day_buf.substring((i-1)*2*2,i*2*2)); // по 2 байта на символ UTF16
            if(t_raw_data >> 1)
                buffer.concat(F(",")); // кроме последнего
        }
        t_raw_data >>= 1;
    }

    if(message){
        buffer.concat(F(","));
        if(message.length()>5){
            buffer.concat(message.substring(0,4)+"...");
        } else {
            buffer.concat(message);
        }
    }
    return buffer;
}