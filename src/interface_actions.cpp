/*
Copyright © 2023 Emil Muratov (vortigont)
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

#include "interface.h"
#include "lamp.h"
#include "display.hpp"
#include "devices.h"
#include "basicui.h"
#include "actions.hpp"
#include "traits.hpp"               // embui traits
#include "evtloop.h"
#include "log.h"

/**
 * @brief Set device display brightness
 * 
 */
void getset_brightness(Interface *interf, JsonObjectConst data, const char* action){
    if (!data.isNull()){
        unsigned b = data[A_dev_brightness];
        LOGV(T_WebUI, printf, "getset_brightness(%u)\n", b);
        int evt = e2int( data[TCONST_nofade] == true ? evt::lamp_t::brightness_nofade : evt::lamp_t::brightness);
        EVT_POST_DATA(LAMP_SET_EVENTS, evt, &b, sizeof(unsigned));
    }

    // publish only on empty data (i.e. GET req or from evt queue)
    if(interf && !data){
        interf->json_frame_value();
        interf->value(A_dev_brightness, myLamp.getBrightness());
        interf->value(V_dev_brtscale, myLamp.getBrightnessScale());
        //interf->value(A_dev_lcurve, e2int(myLamp.effwrkr.getEffCfg().curve));
        interf->json_frame_flush();
    }
}

/**
 * @brief Set luma curve brightness adjustment value
 * 
 */
void set_lcurve(Interface *interf, JsonObjectConst data, const char* action){
    if (!data.isNull()){
        auto c = static_cast<luma::curve>(data[A_dev_lcurve].as<int>());
        myLamp.setLumaCurve(c);
        myLamp.effwrkr.setLumaCurve(c);
    }
    // publishing will be taken care by event listening fuction and getset_brightness
}

/**
 * Обработка вкл/выкл лампы
 */
void set_pwrswitch(Interface *interf, JsonObjectConst data, const char* action){
    myLamp.power(data[action]);
}

/**
 * @brief Switch to specific effect
 * could be triggered via WebUI's selector list or via ra::eff_switch
 * if switched successfully, than this function calls contorls publishing via MQTT
 */
void effect_switch(Interface *interf, JsonObjectConst data, const char* action){
    if (data.isNull()) return;

    /*
     if fader is in progress now, than we just skip switching,
     on one hand it prevents cyclic fast effect switching
     but not sure if this a good idea or bad, let's see
    */
    if (LEDFader::getInstance()->running()) return;

    std::string_view action_view(action);

    // Switch to next effect
    if (action_view.compare(A_effect_switch_next) == 0)
        return run_action(ra::eff_next);

    // Switch to prev effect
    if (action_view.compare(A_effect_switch_prev) == 0)
        return run_action(ra::eff_prev);

    size_t num = data[A_effect_switch_idx];

    LOGD(T_WebUI, printf, "switch to:%u, LampPWR:%u\n", num, myLamp.getPwr());
    myLamp.switcheffect(effswitch_t::num, static_cast<effect_t>(num) );
}

/**
 * @brief контролы эффектов
 * 
 * @param interf 
 * @param data 
 * @param action 
 */
void set_effect_control(Interface *interf, JsonObjectConst data, const char* action){

    std::string_view a(action);
    a.remove_prefix(std::string_view(A_effect_control).length()); // chop off "eff_control_"
    int idx{std::atoi(a.data())};
    LOGI(T_WebUI, printf, "set_effect_control:%s, value:%d\n", action ? action : T_empty, data[action].as<int>());

    myLamp.effwrkr.setControlValue(idx, data[action]);
}

/*
    сохраняет настройки LED ленты
*/
void set_ledstrip(Interface *interf, JsonObjectConst data, const char* action){
    {
        JsonDocument doc;
        if (embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !data.size()){
            if (interf){
                interf->json_frame_value(doc[T_ws2812]);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc[T_ws2812].is<JsonObject>() ? doc[T_ws2812] : doc[T_ws2812].to<JsonObject>();

        for (JsonPairConst kvp : data)
            dst[kvp.key()] = kvp.value();

        doc[T_display_type] = data[T_display_type];   // move led type key to the root of the object
        dst.remove(T_display_type);

        // save new led strip config to file
        serializeJson(doc, Serial);
        embuifs::serialize2file(doc, TCONST_fcfg_display);
    }

    // if we are in hub75 mode, than need a reboot to load ws2812 engine
    if (display.get_engine_type() != engine_t::ws2812){
        run_action(ra::reboot);         // reboot in 5 sec
        if (interf) basicui::page_system_settings(interf, {}, NULL);
        return;
    }

    // установка максимального тока FastLED
    display.setCurrentLimit(data[T_CLmt]);

    display.updateStripeLayout(
        data[T_width], data[T_height],  // tile w,h
        data[T_wcnt], data[T_hcnt],     // tile count on w,h
        data[T_snake],          // single tile configuration
        data[T_vertical],
        data[T_vflip],
        data[T_hflip],
        data[T_tsnake],         // canvas of tiles
        data[T_tvertical],
        data[T_tvflip],
        data[T_thflip]
    );


    int colorder = data[T_col_order];

    // go to "settings page"
    if (interf) basicui::page_system_settings(interf, {}, NULL);

    // Check if I need to reset FastLED gpio or change color order
    if ( colorder == display.getColorOrder() && ( display.getGPIO() == data[T_mx_gpio] || data[T_mx_gpio] == static_cast<int>(GPIO_NUM_NC) ) ) return;       /// gpio not changed or not set, just quit

    if (display.getGPIO() == GPIO_NUM_NC){
        // it's a cold start, so I can change GPIO on the fly
        display.setColorOrder(colorder);
        display.setGPIO(data[T_mx_gpio]);
        display.start();
    } else {
        // otherwise new pin value could be set after reboot
        run_action(ra::reboot);         // reboot in 5 sec
    }

}


void set_hub75(Interface *interf, JsonObjectConst data, const char* action){
    {
        JsonDocument doc;
        if (embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !data.size()){
            if (interf){
                interf->json_frame_value(doc[T_hub75]);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc[T_hub75].isNull() ? doc[T_hub75].to<JsonObject>() : doc[T_hub75];

        // copy keys to a destination object
        for (JsonPairConst kvp : data)
            dst[kvp.key()] = kvp.value();

        //doc[T_display_type] = e2int(engine_t::hub75);   // set engine to hub75
        doc[T_display_type] = data[T_display_type];    // move display type key to the root of the object
        dst.remove(T_display_type);

        // save new led strip config to file
        embuifs::serialize2file(doc, TCONST_fcfg_display);
    }

    //if (display.get_engine_type() != engine_t::hub75){}
    if (interf) basicui::page_system_settings(interf, {}, NULL);
    run_action(ra::reboot);         // reboot in 5 sec
}

void getset_tm1637(Interface *interf, JsonObjectConst data, const char* action){
    {
        JsonDocument doc;
        if (embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !data.size()){
            if (interf && doc[T_tm1637].is<JsonObject>()){
                interf->json_frame_value(doc[T_tm1637]);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc[T_tm1637].isNull() ? doc[T_tm1637].to<JsonObject>() : doc[T_tm1637];

        // copy keys to a destination object
        for (JsonPairConst kvp : data)
            dst[kvp.key()] = kvp.value();

        embuifs::serialize2file(doc, TCONST_fcfg_display);

        JsonVariantConst cfg(dst);
        // reconfig the display
        tm1637_configure(cfg);
    }

    if (interf) ui_page_setup_devices(interf, {}, NULL);
}

void getset_settings_other(Interface *interf, JsonObjectConst data, const char* action){

    // if this is a request with no data, then just provide existing configuration and quit
    if (data.isNull()){
        if (!interf) return;
        interf->json_frame_value();
            interf->value(T_swFade, myLamp.getFaderFlag());
            interf->value(T_swWipeScreen, myLamp.getClearingFlag());
            interf->value(T_demoRndOrder, myLamp.getLampFlagsStuct().demoRndOrderSwitching);
            interf->value(T_demoRndCtrls, myLamp.getLampFlagsStuct().demoRndEffControls);
            interf->value(T_restoreState, myLamp.getLampFlagsStuct().restoreState);
            interf->value(T_DemoTime, myLamp.getDemoTime());
            interf->value(V_dev_brtscale, myLamp.getBrightnessScale());
            // speedfactor is not saved/restored
            //interf->value(T_effSpeedFactor, myLamp.());
        interf->json_frame_flush();
        return;
    }

    // otherwise set supplied values

    myLamp.setFaderFlag(data[T_swFade]);
    myLamp.setClearingFlag(data[T_swWipeScreen]);
    myLamp.setRestoreState(data[T_restoreState]);
    myLamp.setBrightnessScale( data[V_dev_brtscale] );

    myLamp.setDemoRndSwitch(data[T_demoRndOrder]);
    myLamp.setDemoRndEffControls(data[T_demoRndCtrls]);
    myLamp.setDemoTime(data[T_DemoTime]);

    // speedfactor is not saved/restored
    //float sf = data[T_effSpeedFactor];
    //SETPARAM(T_effSpeedFactor);
    //myLamp.setSpeedFactor(sf);

    myLamp.save_flags();

    if(interf)
        basicui::page_system_settings(interf, data, NULL);
}

// a call-back handler that listens for status CHANGE events and publish it to EmbUI feeders
void event_publisher(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    // quit if there are no feeders to notify
    if (!embui.feeders.available()) return;

    // create an interface obj, since we will mostly send value frames, then no need to create large object
    Interface interf(&embui.feeders);

    switch (static_cast<evt::lamp_t>(id)){
        // Lamp Power change state
        case evt::lamp_t::pwron :
            interf.json_frame_value();
            interf.value(A_dev_pwrswitch, true);
            break;
        case evt::lamp_t::pwroff :
            interf.json_frame_value();
            interf.value(A_dev_pwrswitch, false);
            break;

        // brightness related change notifications
        case evt::lamp_t::brightness :
        case evt::lamp_t::brightness_lcurve :
        case evt::lamp_t::brightness_scale :
            // call getset_brightness function with empty data, it will do feeders publishing
            getset_brightness(&interf, {}, NULL);
            break;

        default:;
    }

    interf.json_frame_flush();
}
