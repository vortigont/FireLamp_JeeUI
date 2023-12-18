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
#include "effects.h"
#include "extra_tasks.h"
#include "templates.hpp"
#include LANG_FILE                  //"text_res.h"

#include "devices.h"
#ifdef ENCODER
    #include "enc.h"
#endif

#include "basicui.h"
#include "actions.hpp"
#include <type_traits>
#include "traits.hpp"               // embui traits
#include "evtloop.h"

/**
 * @brief Set device display brightness
 * 
 */
void getset_brightness(Interface *interf, const JsonObject *data, const char* action){
    if (data && (*data).size()){
        unsigned b = (*data)[A_dev_brightness];
        int evt = e2int( (*data)[TCONST_nofade] == true ? evt::lamp_t::brightness_nofade : evt::lamp_t::brightness);
        EVT_POST_DATA(LAMP_SET_EVENTS, evt, &b, sizeof(unsigned));
    }

    // publish only on empty data (i.e. GET req or from evt queue)
    if(interf && !data){
        interf->json_frame_value();
        interf->value(A_dev_brightness, myLamp.getBrightness());
        interf->value(V_dev_brtscale, myLamp.getBrightnessScale());
        interf->value(A_dev_lcurve, e2int(myLamp.effwrkr.getEffCfg().curve));
        interf->json_frame_flush();
    }
}

/**
 * @brief Set luma curve brightness adjustment value
 * 
 */
void set_lcurve(Interface *interf, const JsonObject *data, const char* action){
    if (data && (*data).size()){
        auto c = static_cast<luma::curve>((*data)[A_dev_lcurve].as<int>());
        myLamp.setLumaCurve(c);
        myLamp.effwrkr.setLumaCurve(c);
    }
    // publishing will be taken care by event listening fuction and getset_brightness
}

/**
 * Обработка вкл/выкл лампы
 */
void set_pwrswitch(Interface *interf, const JsonObject *data, const char* action){
    myLamp.power((*data)[action]);
    //EVT_POST(LAMP_SET_EVENTS, (*data)[A_dev_pwrswitch] ? e2int(evt::lamp_t::pwron) : e2int(evt::lamp_t::pwroff));

    #ifdef MP3PLAYER
        if(myLamp.getLampFlagsStuct().isOnMP3)
            mp3->setIsOn((*data)[A_dev_pwrswitch]);
    #endif
}

/**
 * @brief Switch to specific effect
 * could be triggered via WebUI's selector list or via ra::eff_switch
 * if switched successfully, than this function calls contorls publishing via MQTT
 */
void effect_switch(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    LOG(println, "effect_switch()");

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

    uint16_t num = (*data)[A_effect_switch_idx];
    EffectListElem *eff = myLamp.effwrkr.getEffect(num);
    if (!eff) return;                                       // some unknown effect requested, quit

    // сбросить флаг случайного демо
    //myLamp.setDRand(myLamp.getLampFlagsStuct().dRand);

    LOG(printf_P, PSTR("UI EFF switch to:%d, LampOn:%d, mode:%d\n"), eff->eff_nb, myLamp.isLampOn(), myLamp.getMode());
    myLamp.switcheffect(effswitch_t::num, eff->eff_nb);
}

void set_eff_prev(Interface *interf, const JsonObject *data, const char* action){
    run_action(ra::eff_prev);
}

void set_eff_next(Interface *interf, const JsonObject *data, const char* action){
    run_action(ra::eff_next);
}

void set_effects_dynCtrl(Interface *interf, const JsonObject *data, const char* action){

    LList<std::shared_ptr<UIControl>>&controls = myLamp.effwrkr.getControls();

    if (!data || !(*data).size()){
        if (!interf && !action) return;     // return if requred arguments are null

        std::string_view a(action);
        a.remove_prefix(std::string_view(T_effect_dynCtrl).length()); // chop off "dynCtrl"
        int idx = strtol(a.data(), NULL, 10);
        
        for(unsigned i=1; i<controls.size();i++){       // I skip first control here [0] as it's the old 'individual brightness'
            if (controls[i]->getId() == idx){
                // found requested control index, let's reply it's value!
                interf->json_frame_value();
                interf->value( action, controls[i]->getVal() );
                interf->json_frame_flush();
                return;
            }
        }
        return;
    }

    // else it's a "set" request to set a value
    String ctrlName;
    for(unsigned i=1; i<controls.size();i++){       // I skip first control here [0] as it's the old 'individual brightness'
        ctrlName = String(T_effect_dynCtrl) + controls[i]->getId();
        if((*data).containsKey(ctrlName)){
            if ((*data)[ctrlName].is<bool>() ){
                controls[i]->setVal((*data)[ctrlName] ? "1" : "0");     // больше стрингов во славу Богу стрингов!
            } else
                controls[i]->setVal((*data)[ctrlName]); // для всех остальных

            resetAutoTimers(true);
            myLamp.effwrkr.setDynCtrl(controls[i].get());
            break;
        }
    }
}

/*
    сохраняет настройки LED ленты
*/
void set_ledstrip(Interface *interf, const JsonObject *data, const char* action){
    {
        DynamicJsonDocument doc(DISPLAY_JSIZE);
        if (!embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !(*data).size()){
            if (interf){
                interf->json_frame_value(doc[T_ws2812], true);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc.containsKey(T_ws2812) ? doc[T_ws2812] : doc.createNestedObject(T_ws2812);

        for (JsonPair kvp : *data)
            dst[kvp.key()] = kvp.value();

        doc[T_display_type] = (*data)[T_display_type];   // move led type key to the root of the object
        dst.remove(T_display_type);

        // save new led strip config to file
        serializeJson(doc, Serial);
        embuifs::serialize2file(doc, TCONST_fcfg_display);
    }

    // if we are in hub75 mode, than need a reboot to load ws2812 engine
    if (display.get_engine_type() != engine_t::ws2812){
        run_action(ra::reboot);         // reboot in 5 sec
        if (interf) basicui::page_system_settings(interf, nullptr, NULL);
        return;
    }

    // установка максимального тока FastLED
    display.setCurrentLimit((*data)[T_CLmt]);

    display.updateStripeLayout(
        (*data)[T_width], (*data)[T_height],  // tile w,h
        (*data)[T_wcnt], (*data)[T_hcnt],     // tile count on w,h
        (*data)[T_snake],          // single tile configuration
        (*data)[T_vertical],
        (*data)[T_vflip],
        (*data)[T_hflip],
        (*data)[T_tsnake],         // canvas of tiles
        (*data)[T_tvertical],
        (*data)[T_tvflip],
        (*data)[T_thflip]
    );

    // go to "settings page"
    if (interf) basicui::page_system_settings(interf, nullptr, NULL);

    // Check if I need to reset FastLED gpio
    if (display.getGPIO() == (*data)[T_mx_gpio] || (*data)[T_mx_gpio] == static_cast<int>(GPIO_NUM_NC)) return;       /// gpio not changed or not set, just quit

    if (display.getGPIO() == GPIO_NUM_NC){
        // it's a cold start, so I can change GPIO on the fly
        display.setGPIO((*data)[T_mx_gpio]);
        display.start();
    } else {
        // otherwise new pin value could be set after reboot
        run_action(ra::reboot);         // reboot in 5 sec
    }

}


void set_hub75(Interface *interf, const JsonObject *data, const char* action){
    {
        DynamicJsonDocument doc(DISPLAY_JSIZE);
        if (!embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !(*data).size()){
            if (interf){
                interf->json_frame_value(doc[T_hub75], true);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc[T_hub75].isNull() ? doc.createNestedObject(T_hub75) : doc[T_hub75];

        // copy keys to a destination object
        for (JsonPair kvp : *data)
            dst[kvp.key()] = kvp.value();

        //doc[T_display_type] = e2int(engine_t::hub75);   // set engine to hub75
        doc[T_display_type] = (*data)[T_display_type];    // move display type key to the root of the object
        dst.remove(T_display_type);

        // save new led strip config to file
        embuifs::serialize2file(doc, TCONST_fcfg_display);
    }

    //if (display.get_engine_type() != engine_t::hub75){}
    if (interf) basicui::page_system_settings(interf, nullptr, NULL);
    run_action(ra::reboot);         // reboot in 5 sec
}

void getset_tm1637(Interface *interf, const JsonObject *data, const char* action){
    {
        DynamicJsonDocument doc(DISPLAY_CFG_JSIZE);
        if (!embuifs::deserializeFile(doc, TCONST_fcfg_display)) doc.clear();

        // if this is a request with no data, then just provide existing configuration and quit
        if (!data || !(*data).size()){
            if (interf && doc.containsKey(T_tm1637)){
                interf->json_frame_value(doc[T_tm1637], true);
                interf->json_frame_flush();
            }
            return;
        }

        JsonVariant dst = doc[T_tm1637].isNull() ? doc.createNestedObject(T_tm1637) : doc[T_tm1637];

        // copy keys to a destination object
        for (JsonPair kvp : *data)
            dst[kvp.key()] = kvp.value();

        embuifs::serialize2file(doc, TCONST_fcfg_display);

        JsonVariantConst cfg(dst);
        // reconfig the display
        tm1637_configure(cfg);
    }

    if (interf) ui_page_setup_devices(interf, nullptr, NULL);
}

// a call-back handler that listens for status change events and publish it to EmbUI feeders
void event_publisher(void* handler_args, esp_event_base_t base, int32_t id, void* event_data){
    // quit if there are no feeders to notify
    if (!embui.feeders.available()) return;

    // create an interface obj, since we will mostly send value frames, then no need to create large object
    Interface interf(&embui.feeders, 512);

    switch (static_cast<evt::lamp_t>(id)){
        // Lamp Power change state
        case evt::lamp_t::pwron :
        case evt::lamp_t::pwroff :
            interf.json_frame_value();
            interf.value(A_dev_pwrswitch, myLamp.isLampOn());
            break;

        // brightness related change notifications
        case evt::lamp_t::brightness :
        case evt::lamp_t::brightness_lcurve :
        case evt::lamp_t::brightness_scale :
            // call getset_brightness function with empty data, it will do feeders publishing
            getset_brightness(&interf, nullptr, NULL);
            break;

        default:;
    }

    interf.json_frame_flush();
}
