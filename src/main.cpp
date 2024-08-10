/*
Copyright © 2023-2024 Emil Muratov (Vortigont)
Copyright © 2020 Dmytro Korniienko (kDn)

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
#include "lamp.h"
#include "interface.h"
#include "display.hpp"
#include "actions.hpp"
#include "evtloop.h"
#include "devices.h"
#include "components.hpp"
#include "log.h"

#ifdef EMBUI_USE_MQTT
#include "AsyncMqttClient/Callbacks.hpp"
String ha_autodiscovery();
#endif




// restores LED fb config from file
void led_fb_setup();

// mDNS announce for WLED app
void wled_announce();
// 404 handler
bool http_notfound(AsyncWebServerRequest *request);
// MQTT callback
void mqttOnMessageCallback(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);



// Arduino setup
void setup() {
    // debug console
#if ARDUINO_USB_CDC_ON_BOOT == 1
    Serial.begin();
    // let USB-serial connect
    #if EMBUI_DEBUG_LEVEL > 3 || LAMP_DEBUG_LEVEL > 3
        delay(4000);
    #endif
#else
    Serial.begin(115200);
#endif

    LOGI(T_Main, printf, "Setup: free heap: %uk, PSRAM:%uk\n\n", ESP.getFreeHeap()/1024, ESP.getFreePsram()/1024);

    // cap ADC resolution to 10 bit
    analogReadResolution(10);

    // Start event loop task
    evt::start();
    // event bus sniffer
    //evt::debug();

    // add WLED mobile app handler
    embui.server.on("/win", HTTP_ANY, [](AsyncWebServerRequest *request){ wled_handle(request); } );
    // special 404 handler to workaround WLED bug 
    embui.on_notfound( [](AsyncWebServerRequest *r){ return http_notfound(r);} );

    //  *** EmbUI ***
    // Инициализируем EmbUI фреймворк - загружаем конфиг, запускаем WiFi и все зависимые от него службы
    embui.begin();
    // change periodic WebUI publish interval
    embui.setPubInterval(30);

    // OmniCron Scheduler - activate on time sync
    //TimeProcessor::getInstance().attach_callback( [](){ omnicron.start(); } );

    // Add mDNS CB handler for WLED app
    embui.wifi->mdns_cb = wled_announce;

    // register EmbUI action callbacks
    embui_actions_register();

    // Load display configuration from file and create a proper LED buffer
    display.start();
    // Load tm1637 if enabled
    tm1637_setup();
    // Load button support if enabled
    button_cfg_load();
    // Load DFPlayer support if enabled
    dfplayer_cfg_load();

    // Lamp object initialization must be done AFTER display.start(), so that display object could create pixel buffer first
    // TODO: this is ugly
    //myLamp.effwrkr.setEffSortType((SORT_TYPE)embui.paramVariant(V_effSort).as<int>()); // сортировка должна быть определена до заполнения
    myLamp.effwrkr.initDefault();
    myLamp.lamp_init();

    // Hookup IPC event publisher callback
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(evt::get_hndlr(), LAMP_CHANGE_EVENTS, ESP_EVENT_ANY_ID, event_publisher, NULL, NULL));

    // spawn Modules instances from saved configurations, this must be done AFTER display initialisation
    zookeeper.start();
    zookeeper.setHandlers();

    LOGI(T_Main, printf, "Setup complete: free heap: %uk, PSRAM:%uk\n\n", ESP.getFreeHeap()/1024, ESP.getFreePsram()/1024);
}   // End setup()


void loop() {
    embui.handle(); // цикл, необходимый фреймворку
    myLamp.handle(); // цикл, обработка лампы
    // do not have anything usefull there except mike and ftp
    vTaskDelay(1);
}

//------------------------------------------

#ifdef EMBUI_USE_MQTT
// реализация autodiscovery
String ha_autodiscovery()
{
    LOG(println,"MQTT: Autodiscovery");
    DynamicJsonDocument hass_discover(1024);

    hass_discover["~"] = embui.mqttPrefix();     // embui.param(P_m_pref) + "/embui/")
    hass_discover["name"] = embui.hostname();        // name
    hass_discover["uniq_id"] = embui.macid();        // String(ESP.getChipId(), HEX); // unique_id

    hass_discover["avty_t"] = "~pub/online";  // availability_topic
    hass_discover["pl_avail"] = "1";          // payload_available
    hass_discover["pl_not_avail"] = "0";      // payload_not_available

    hass_discover["cmd_t"] = "~set/on";       // command_topic
    hass_discover["stat_t"] = "~pub/on";      // state_topic
    hass_discover["pl_on"] = "1";             // payload_on
    hass_discover["pl_off"] = "0";            // payload_off

    hass_discover["json_attr_t"] = "~pub/state"; // json_attributes_topic

    hass_discover["rgb_cmd_t"] = "~set/rgb";        // rgb_command_topic
    hass_discover["rgb_stat_t"] = "~pub/rgb";       // rgb_state_topic

    hass_discover["bri_cmd_t"] = "~set/g_bright";     // brightness_command_topic
    hass_discover["bri_stat_t"] = "~pub/dynCtrl0";    // brightness_state_topic
    hass_discover["bri_scl"] = 255;

    JsonArray data = hass_discover.createNestedArray("effect_list");
    data.add(TCONST_Normal);
    data.add(TCONST_Alarm);
    data.add(K_demo);
    data.add(TCONST_RGB);
    data.add(TCONST_White);
    data.add(TCONST_Other);

    //---------------------

    hass_discover["fx_cmd_t"] = "~set/mode";                                 // effect_command_topic
    hass_discover["fx_stat_t"] = "~pub/state";                               // effect_state_topic
    hass_discover["fx_tpl"] = "{{ value_json.Mode }}";                       // effect_template

    hass_discover["clr_temp_cmd_t"] = "~set/speed";     // speed as color temperature
    hass_discover["clr_temp_stat_t"] = "~pub/speed";    // speed as color temperature
    hass_discover["min_mireds"] = 1;
    hass_discover["max_mireds"] = 255;

    hass_discover["whit_val_cmd_t"] = "~set/scale";     // scale as white level (Яркость белого)
    hass_discover["whit_val_stat_t"] = "~pub/scale";    // scale as white level
    hass_discover["whit_val_scl"] = 255;

    // hass_discover["xy_cmd_t"] = "~set/speed";     // scale as white level (Яркость белого)
    // hass_discover["xy_stat_t"] = "~pub/speed";    // scale as white level
    //hass_discover["whit_val_scl"] = 255; // 'xy_val_tpl':          'xy_value_template',

    String hass_discover_str;
    serializeJson(hass_discover, hass_discover_str);
    hass_discover.clear();

    embui.mqttClient->publish((String("homeassistant/light/") + embui.hostname() + "/config").c_str(), 0, 1, hass_discover_str.c_str());
    return hass_discover_str;
}


 // функция вызывается, когда приходят данные MQTT
void mqttOnMessageCallback(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
  LOG(printf_P, PSTR("MQTT Message: [%s - %s]\n"), topic, payload);
    // todo: rewrite it with string_view
/*
  if(topic.startsWith(TCONST_embui_get_)){
    String sendtopic=topic;
    sendtopic.replace(TCONST_embui_get_, "");
    if(sendtopic==TCONST_eff_config){
        sendtopic=String(TCONST_embui_pub_)+sendtopic;
        String effcfg;
        if (fshlpr::getfseffconfig(myLamp.effects.getCurrent(), effcfg))
            embui.publish(sendtopic, effcfg, true); // отправляем обратно в MQTT в топик embui/pub/
    } else if(sendtopic==TCONST_state){
        sendData();
    }
  }
*/
}

// Periodic MQTT publishing
void sendData(){
    // Здесь отсылаем текущий статус лампы и признак, что она живая (keepalive)
    DynamicJsonDocument obj(512);
    //JsonObject obj = doc.to<JsonObject>();
    switch (myLamp.getMode())
    {
        case LAMPMODE::MODE_NORMAL :
            obj[TCONST_Mode] = TCONST_Normal;
            break;
        case LAMPMODE::MODE_ALARMCLOCK :
            obj[TCONST_Mode] = TCONST_Alarm;
            break;
        case LAMPMODE::MODE_DEMO :
            obj[TCONST_Mode] = K_demo;
            break;
        case LAMPMODE::MODE_RGBLAMP :
            obj[TCONST_Mode] = TCONST_RGB;
            break;
        //case LAMPMODE::MODE_WHITELAMP :
        //    obj[TCONST_Mode] = TCONST_White;
        //    break;
        default:
            obj[TCONST_Mode] = TCONST_Other;
            break;
    }
    //obj[TCONST_Time] = TimeProcessor::getInstance().getFormattedShortTime();
    //obj[TCONST_Memory] = myLamp.getLampState().freeHeap;
    //obj[TCONST_Uptime] = String(embui.getUptime());
    obj[TCONST_RSSI] = String(myLamp.getLampState().rssi);
    obj[TCONST_Ip] = WiFi.localIP().toString();
    obj[TCONST_Mac] = WiFi.macAddress();
    obj[TCONST_Host] = String("http://")+embui.hostname();
    //obj[TCONST_Version] = embui.getEmbUIver();
    obj[P_MQTTTopic] = embui.mqttPrefix();     // embui.param(P_m_pref) + "/embui/")
    //String sendtopic=TCONST_embui_pub_;
    //sendtopic+=TCONST_state;
    String out;
    serializeJson(obj, out);
    //LOG(println, "send MQTT Data :");
    //LOG(println, out);
    embui.publish(TCONST_state, out.c_str(), true); // отправляем обратно в MQTT в топик embui/pub/
}
#endif

void wled_announce(){
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", (const char*)embui.macid());
}

// rewriter for buggy WLED app
// https://github.com/Aircoookie/WLED-App/issues/37
bool http_notfound(AsyncWebServerRequest *request){
    if (request->url().indexOf("win&") != -1){
        String req(request->url());
        req.replace("win&", "win?");
        request->redirect(req);
        return true;
    }
    // not our case, no action was made
    return false;
}
