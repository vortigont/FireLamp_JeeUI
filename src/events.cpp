
#include "events.h"
#include <LittleFS.h>
#include "interface.h"
#include "actions.hpp"
#include "extra_tasks.h"

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

/**
 *  метод загружает и пробует десериализовать джейсон из файла в предоставленный документ,
 *  возвращает true если загрузка и десериализация прошла успешно
 *  @param doc - DynamicJsonDocument куда будет загружен джейсон
 *  @param jsonfile - файл, для загрузки
 */
bool EVENT_MANAGER::deserializeFile(DynamicJsonDocument& doc, const char* filepath){
  if (!filepath || !*filepath)
    return false;

  File jfile = LittleFS.open(filepath, "r");
  DeserializationError error;
  if (jfile){
    error = deserializeJson(doc, jfile);
    jfile.close();
  } else {
    return false;
  }

  if (error) {
    LOG(printf_P, PSTR("File: failed to load json file: %s, deserializeJson error: "), filepath);
    LOG(println, error.code());
    return false;
  }
  //LOG(printf_P,PSTR("File: %s deserialization took %d ms\n"), filepath, millis() - timest);
  return true;
}

void EVENT_MANAGER::loadConfig(const char *cfg)
{
    if(LittleFS.begin()){
        clear_events();
        DynamicJsonDocument doc(4096);
        String filename = cfg ? String(cfg) : String(F("/events_config.json"));
        if (!deserializeFile(doc, filename.c_str())){
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
    LOG(printf_P, PSTR("%s - %s\n"), ((DEV_EVENT *)event)->getName().c_str(), embui.timeProcessor.getFormattedShortTime().c_str());

    switch (event->getEvent()) {
    case EVENT_TYPE::ON : {
        run_action(ra::on);
        if (!event->getMessage().isEmpty()){
            // вывести текст на лампу через 3 секунды
            StringTask *t = new StringTask(event->getMessage().c_str(), 3 * TASK_SECOND, TASK_ONCE, nullptr, &ts, false, nullptr,  [](){
                StringTask *cur = (StringTask *)ts.getCurrentTask();
                remote_action(RA::RA_SEND_TEXT, cur->getData(), NULL);
            }, true);
            t->enableDelayed();
        }
        return;
    }
    case EVENT_TYPE::OFF: return run_action(ra::off);
    case EVENT_TYPE::DEMO: return run_action(ra::demo, event->getMessage()=="1");       // not sure what is the content of this String
    case EVENT_TYPE::ALARM: action = RA_ALARM; break;
    //case EVENT_TYPE::LAMP_CONFIG_LOAD: action = RA_LAMP_CONFIG; break;                // была какая-то загрузка стороннего конфига embui
#ifdef ESP_USE_BUTTON
    case EVENT_TYPE::BUTTONS_CONFIG_LOAD:  action = RA_BUTTONS_CONFIG; break;
#endif
    //case EVENT_TYPE::EFF_CONFIG_LOAD:  action = RA_EFF_CONFIG; break;                 // была какая-то мутная загрузка индекса эффектов из папки /backup/idx
    case EVENT_TYPE::EVENTS_CONFIG_LOAD: action = RA_EVENTS_CONFIG; break;
    case EVENT_TYPE::SEND_TEXT:  action = RA_SEND_TEXT; break;
    case EVENT_TYPE::SEND_TIME:  action = RA_SEND_TIME; break;
#ifdef AUX_PIN
    case EVENT_TYPE::AUX_ON: action = RA_AUX_ON; break;
    case EVENT_TYPE::AUX_OFF: action = RA_AUX_OFF; break;
    case EVENT_TYPE::AUX_TOGGLE: action = RA_AUX_TOGLE; break;
#endif
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
    case EVENT_TYPE::SET_GLOBAL_BRIGHT: action = RA_GLOBAL_BRIGHT; break;
    case EVENT_TYPE::SET_WHITE_HI: action = RA_WHITE_HI; break;
    case EVENT_TYPE::SET_WHITE_LO: action = RA_WHITE_LO; break;
    default:;
    }

    remote_action(action, event->getMessage().c_str(), NULL);
}
