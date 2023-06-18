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
#include "main.h"
#include "filehelpers.hpp"
#include <SPIFFSEditor.h>

#ifdef DS18B20
#include "DS18B20.h"
#endif

// Led matrix frame buffer
LedFB mx(WIDTH, HEIGHT);
// FastLED controller
CLEDController *cled;

// объект лампы
LAMP myLamp(mx);

#ifdef ESP_USE_BUTTON
Buttons *myButtons;
#endif

#ifdef MP3PLAYER
MP3PlayerDevice *mp3 = nullptr;
#endif

#ifdef TM1637_CLOCK
// TM1637 display
// https://github.com/AKJ7/TM1637/
TMCLOCK *tm1637 = nullptr;
#endif

// forward declarations

/**
 * @brief restore gpio configurtion and initialise attached devices
 * 
 */
void gpio_setup();

// mDNS announce for WLED app
void wled_announce();


void setup() {
    Serial.begin(115200);

    LOG(printf_P, PSTR("\n\nsetup: free heap  : %d\n"), ESP.getFreeHeap());

#ifdef ESP32
    LOG(printf_P, PSTR("setup: free PSRAM  : %d\n"), ESP.getFreePsram()); // 4194252
#endif

    // setup LED matrix
    cled = &FastLED.addLeds<WS2812B, LAMP_PIN, COLOR_ORDER>(mx.data(), mx.size());
    // hook framebuffer to contoller
    mx.bind(cled);

#ifdef EMBUI_USE_UDP
    embui.udp(); // Ответ на UDP запрс. в качестве аргумента - переменная, содержащая macid (по умолчанию)
#endif

    // Add mDNS handler for WLED app
#ifndef ESP8266
    embui.set_callback(CallBack::attach, CallBack::STAGotIP, wled_announce);
#endif

    // EmbUI
    embui.begin(); // Инициализируем EmbUI фреймворк - загружаем конфиг, запускаем WiFi и все зависимые от него службы

#ifdef EMBUI_USE_MQTT
    //embui.mqtt(embui.param(F("m_pref")), embui.param(F("m_host")), embui.param(F("m_port")).toInt(), embui.param(F("m_user")), embui.param(F("m_pass")), mqttCallback, true); // false - никакой автоподписки!!!
    //embui.mqtt(mqttCallback, true);
    embui.mqtt(mqttCallback, mqttConnect, true);
#endif

    myLamp.effects.setEffSortType((SORT_TYPE)embui.paramVariant(FPSTR(TCONST_effSort)).as<int>()); // сортировка должна быть определена до заполнения
    myLamp.effects.initDefault(); // если вызывать из конструктора, то не забыть о том, что нужно инициализировать Serial.begin(115200); иначе ничего не увидеть!
    myLamp.events.loadConfig();

#ifdef RTC
    rtc.init();
#endif

#ifdef DS18B20
    ds_setup();
#endif

    // restore matrix current limit from config
    myLamp.lamp_init(embui.paramVariant(FPSTR(TCONST_CLmt)));

#ifdef ESP_USE_BUTTON
    myLamp.setbPin(embui.param(FPSTR(TCONST_PINB)).toInt());
    myButtons = new Buttons(myLamp.getbPin(), PULL_MODE, NORM_OPEN);
    if (!myButtons->loadConfig()) {
      default_buttons();
      myButtons->saveConfig();
    }
#endif

    myLamp.events.setEventCallback(event_worker);

    // configure and init attached devices
    gpio_setup();

#ifdef ESP8266
  embui.server.addHandler(new SPIFFSEditor(F("esp8266"),F("esp8266"), LittleFS));
#else
  embui.server.addHandler(new SPIFFSEditor(LittleFS, F("esp32"), F("esp32")));
#endif

  sync_parameters();

  embui.setPubInterval(10);   // change periodic WebUI publish interval from EMBUI_PUB_PERIOD to 10 secs

#ifdef ENCODER
  enc_setup();
#endif

    LOG(println, F("setup() done"));
}   // End setup()


void loop() {
    embui.handle(); // цикл, необходимый фреймворку

    myLamp.handle(); // цикл, обработка лампы
#ifdef ENCODER
    encLoop(); // цикл обработки событий энкодера. Эта функция будет отправлять в УИ изменения, только тогда, когда подошло время ее loop
#endif

#ifdef RTC
    rtc.updateRtcTime();
#endif

#ifdef TM1637_CLOCK
    EVERY_N_SECONDS(1) {
        if (tm1637) tm1637->tm_loop();
    }
#endif
#ifdef DS18B20
    EVERY_N_MILLIS(1000*DS18B_READ_DELAY + 25) {
        ds_loop();
    }
#endif
#ifdef USE_STREAMING
    if (ledStream)
        ledStream->handle();
#endif
}

//------------------------------------------

#ifdef EMBUI_USE_MQTT
// реализация autodiscovery
String ha_autodiscovery()
{
    LOG(println,F("MQTT: Autodiscovery"));
    DynamicJsonDocument hass_discover(1024);
    String name = embui.param(FPSTR(P_hostname));
    String unique_id = embui.mc;

    hass_discover[F("~")] = embui.id(FPSTR(TCONST_embui_));     // embui.param(FPSTR(P_m_pref)) + F("/embui/")
    hass_discover[F("name")] = name;                // name
    hass_discover[F("uniq_id")] = unique_id;        // String(ESP.getChipId(), HEX); // unique_id

    hass_discover[F("avty_t")] = F("~pub/online");  // availability_topic
    hass_discover[F("pl_avail")] = F("1");          // payload_available
    hass_discover[F("pl_not_avail")] = F("0");      // payload_not_available

    hass_discover[F("cmd_t")] = F("~set/on");       // command_topic
    hass_discover[F("stat_t")] = F("~pub/on");      // state_topic
    hass_discover[F("pl_on")] = F("1");             // payload_on
    hass_discover[F("pl_off")] = F("0");            // payload_off

    hass_discover[F("json_attr_t")] = F("~pub/state"); // json_attributes_topic

    hass_discover[F("rgb_cmd_t")] = "~set/rgb";        // rgb_command_topic
    hass_discover[F("rgb_stat_t")] = "~pub/rgb";       // rgb_state_topic

    hass_discover[F("bri_cmd_t")] = F("~set/g_bright");     // brightness_command_topic
    hass_discover[F("bri_stat_t")] = F("~pub/dynCtrl0");    // brightness_state_topic
    hass_discover[F("bri_scl")] = 255;

    JsonArray data = hass_discover.createNestedArray(F("effect_list"));
    data.add(FPSTR(TCONST_Normal));
    data.add(FPSTR(TCONST_Alarm));
    data.add(FPSTR(TCONST_Demo));
    data.add(FPSTR(TCONST_RGB));
    data.add(FPSTR(TCONST_White));
    data.add(FPSTR(TCONST_Other));

    //---------------------

    hass_discover[F("fx_cmd_t")] = F("~set/mode");                                 // effect_command_topic
    hass_discover[F("fx_stat_t")] = F("~pub/state");                               // effect_state_topic
    hass_discover[F("fx_tpl")] = F("{{ value_json.Mode }}");                       // effect_template

    hass_discover[F("clr_temp_cmd_t")] = F("~set/speed");     // speed as color temperature
    hass_discover[F("clr_temp_stat_t")] = F("~pub/speed");    // speed as color temperature
    hass_discover[F("min_mireds")] = 1;
    hass_discover[F("max_mireds")] = 255;

    hass_discover[F("whit_val_cmd_t")] = F("~set/scale");     // scale as white level (Яркость белого)
    hass_discover[F("whit_val_stat_t")] = F("~pub/scale");    // scale as white level
    hass_discover[F("whit_val_scl")] = 255;

    // hass_discover[F("xy_cmd_t")] = F("~set/speed");     // scale as white level (Яркость белого)
    // hass_discover[F("xy_stat_t")] = F("~pub/speed");    // scale as white level
    //hass_discover[F("whit_val_scl")] = 255; // 'xy_val_tpl':          'xy_value_template',

    String hass_discover_str;
    serializeJson(hass_discover, hass_discover_str);
    hass_discover.clear();

    embui.publishto(String(F("homeassistant/light/")) + name + F("/config"), hass_discover_str, true);
    return hass_discover_str;
}

extern void mqtt_dummy_connect();
void mqttConnect(){ 
    mqtt_dummy_connect();
    ha_autodiscovery();
}

ICACHE_FLASH_ATTR void mqttCallback(const String &topic, const String &payload){ // функция вызывается, когда приходят данные MQTT
  LOG(printf_P, PSTR("Message [%s - %s]\n"), topic.c_str() , payload.c_str());
  if(topic.startsWith(FPSTR(TCONST_embui_get_))){
    String sendtopic=topic;
    sendtopic.replace(FPSTR(TCONST_embui_get_), "");
    if(sendtopic==FPSTR(TCONST_eff_config)){
        sendtopic=String(FPSTR(TCONST_embui_pub_))+sendtopic;
        String effcfg;
        if (fshlpr::getfseffconfig(myLamp.effects.getCurrent(), effcfg)) embui.publish(sendtopic, effcfg, true); // отправляем обратно в MQTT в топик embui/pub/
    } else if(sendtopic==FPSTR(TCONST_state)){
        sendData();
    }
  }
}

// Periodic MQTT publishing
void sendData(){
    // Здесь отсылаем текущий статус лампы и признак, что она живая (keepalive)
    DynamicJsonDocument obj(512);
    //JsonObject obj = doc.to<JsonObject>();
    switch (myLamp.getMode())
    {
        case LAMPMODE::MODE_NORMAL :
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_Normal);
            break;
        case LAMPMODE::MODE_ALARMCLOCK :
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_Alarm);
            break;
        case LAMPMODE::MODE_DEMO :
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_Demo);
            break;
        case LAMPMODE::MODE_RGBLAMP :
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_RGB);
            break;
        case LAMPMODE::MODE_WHITELAMP :
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_White);
            break;
        default:
            obj[FPSTR(TCONST_Mode)] = FPSTR(TCONST_Other);
            break;
    }
    obj[FPSTR(TCONST_Time)] = String(embui.timeProcessor.getFormattedShortTime());
    obj[FPSTR(TCONST_Memory)] = String(myLamp.getLampState().freeHeap);
    obj[FPSTR(TCONST_Uptime)] = String(embui.getUptime());
    obj[FPSTR(TCONST_RSSI)] = String(myLamp.getLampState().rssi);
    obj[FPSTR(TCONST_Ip)] = WiFi.localIP().toString();
    obj[FPSTR(TCONST_Mac)] = WiFi.macAddress();
    obj[FPSTR(TCONST_Host)] = String(F("http://"))+WiFi.localIP().toString();
    obj[FPSTR(TCONST_Version)] = embui.getEmbUIver();
    obj[FPSTR(TCONST_MQTTTopic)] = embui.id(FPSTR(TCONST_embui_));     // embui.param(FPSTR(P_m_pref)) + F("/embui/")
    String sendtopic=FPSTR(TCONST_embui_pub_);
    sendtopic+=FPSTR(TCONST_state);
    String out;
    serializeJson(obj, out);
    LOG(println, F("send MQTT Data :"));
    LOG(println, out);
    embui.publish(sendtopic, out, true); // отправляем обратно в MQTT в топик embui/pub/
}
#endif

void gpio_setup(){
    DynamicJsonDocument doc(512);
    embuifs::deserializeFile(doc, FPSTR(TCONST_fcfg_gpio));
    int rxpin, txpin;
#ifdef MP3PLAYER
    // spawn an instance of mp3player
    rxpin = doc[FPSTR(TCONST_mp3rx)] | -1;
    txpin = doc[FPSTR(TCONST_mp3tx)] | -1;
    LOG(printf_P, PSTR("DFPlayer: rx:%d tx:%d\n"), rxpin, txpin);
    mp3 = new MP3PlayerDevice(rxpin, txpin, embui.paramVariant(FPSTR(TCONST_mp3volume)) | DFPLAYER_DEFAULT_VOL );
#endif

#ifdef TM1637_CLOCK
    rxpin = doc[FPSTR(TCONST_tm_clk)] | -1;
    txpin = doc[FPSTR(TCONST_tm_dio)] | -1;
    if (rxpin != -1 && txpin != -1){
        tm1637 = new TMCLOCK(rxpin, txpin);
        tm1637->tm_setup();
    }
#endif 
}

void wled_announce(){
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", (const char*)embui.mc);
}
