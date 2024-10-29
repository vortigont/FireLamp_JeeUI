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

#include "config.h"
#include "interface.h"
#include "lamp.h"
#include "devices.h"
#include "effects.h"
#include "templates.hpp"
#include "basicui.h"
#include "actions.hpp"
#include <type_traits>
#include "evtloop.h"
#include "devices.h"
#include "components.hpp"
#include "log.h"
#include LANG_FILE                  //"text_res.h"

// modules


// версия ресурсов в стороннем джейсон файле
#define UIDATA_VERSION      23

#define DEMO_MIN_PERIOD     10
#define DEMO_MAX_PERIOD     900
#define DEMO_PERIOD_STEP    10

#define SPEEDFACTOR_DEFAULT 1.0


/**
 * @brief numeric indexes for pages
 * it MUST not overlap with basicui::page index
 */
enum class page : uint16_t {
    main = 50,
    eff_config,
    mike,
    setup_display,
    setup_dfplayer,
    setup_bttn,
    setup_encdr,
    setup_other,
    setup_tm1637,
    setup_devices,      // page with configuration links to external devices

    modules = 101,      // available widgets page
    setup_gpio = 105
};

// enumerator for gpio setup form
enum class gpio_device:uint8_t {
    ledstrip,
    dfplayer,
    mosfet,
    aux,
    tmdisplay
};


/**
 * @brief enumerator with a files of effect lists for webui 
 * i.e. cached json files with effect names for drop down lists 
 */
enum class lstfile_t {
    selected,
    full,
    all
};

// *** forward declarations ***

// CallBack - Create main index page
void ui_page_main(Interface *interf, JsonObjectConst data, const char* action);
// CallBack - append Lamps's settings elements to system's "Settings" page
void block_user_settings(Interface *interf, JsonObjectConst data, const char* action);

void ui_page_effects(Interface *interf, JsonObjectConst data, const char* action);
void ui_page_setup_devices(Interface *interf, JsonObjectConst data, const char* action);
void ui_section_effects_list_configuration(Interface *interf, JsonObjectConst data, const char* action);
// build side-menu
void ui_section_menu(Interface *interf, JsonObjectConst data, const char* action);
//void show_effects_config(Interface *interf, JsonObjectConst data, const char* action);
// construct a page with Display setup
void page_display_setup(Interface *interf, JsonObjectConst data, const char* action);
// construct a page with TM1637 setup
void ui_page_tm1637_setup(Interface *interf, JsonObjectConst data, const char* action);
// send/set element values to the page with gpio setup
void getset_gpios(Interface *interf, JsonObjectConst data, const char* action);
void page_settings_other(Interface *interf, JsonObjectConst data, const char* action);
void section_sys_settings_frame(Interface *interf, JsonObjectConst data, const char* action);


/**
 * @brief function renders display configuration pages to the WebUI
 * 
 * @param engine_t e - an engine type to show controls for
 * 
 */
void block_display_setup(Interface *interf, engine_t e);




/* *** WebUI generators *** */

/**
 * @brief index page for WebUI,
 * it loads in each new WebSOcket connection
 * 
 */
void ui_page_main(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;

    interf->json_frame_interface(); //TINTF_080);
    interf->json_section_manifest(TINTF_080, embui.macid(), 0, LAMPFW_VERSION_STRING);       // app name/version manifest
    interf->json_section_end();

    // build side menu
    ui_section_menu(interf, data, action);
    interf->json_frame_flush();     // close frame

    // generate effect's list
    interf->json_frame_jscall("make_effect_list");

    if(WiFi.getMode() & WIFI_MODE_STA){
        ui_page_effects(interf, data, action);
    } else {
        // открываем страницу с настройками WiFi если контроллер не подключен к внешней AP
        basicui::page_settings_netw(interf, {}, NULL);
    }
}

/**
 * @brief loads UI page from uidata storage and triggers value generation for UI elements
 * 
 * @param interf 
 * @param data 
 * @param action 
 */
void uidata_page_selector(Interface *interf, JsonObjectConst data, const char* action, page idx){
    interf->json_frame_interface();
    interf->json_section_uidata();

    switch (idx){
        // настрока gpio
        case page::setup_gpio :
            interf->uidata_pick( "lampui.pages.gpiosetup" );
            interf->json_frame_flush();
            getset_gpios(interf, {}, NULL);
            break;

        default:;                   // by default do nothing
    }

  interf->json_frame_flush();
}

/**
 * @brief when action is called to display a specific page
 * this selector picks and calls correspoding method
 * using common selector simplifes and reduces a number of registered actions required 
 * 
 */
void ui_page_selector(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf || !data || data[A_ui_page].isNull()) return;  // quit if no section specified

    // get a page index
    page idx = static_cast<page>(data[A_ui_page].as<int>());

    switch (idx){
        //case page::eff_config :   // страница "Управление списком эффектов"
        //    show_effects_config(interf, {}, NULL);
        //    return;
        case page::setup_bttn :    // страница настроек кнопки
            return page_button_setup(interf, {}, NULL);
        case page::setup_other :    // страница "настройки"-"другие"
            return page_settings_other(interf, {}, NULL);
        case page::setup_display :  // led display setup (strip/hub75)
            return page_display_setup(interf, {}, NULL);
        case page::setup_devices :  // periferal devices setup selector page
            return ui_page_setup_devices(interf, {}, NULL);
        case page::setup_tm1637 :   // tm1637 display setup
            return ui_page_tm1637_setup(interf, {}, NULL);
        case page::setup_dfplayer :
            return page_dfplayer_setup(interf, {}, NULL);

        default:;                   // by default do nothing
    }

    if (e2int(idx) > 100)
        uidata_page_selector(interf, data, action, idx);
}

void ui_section_menu(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();

    interf->option(A_ui_page_effects, TINTF_000);           //  Эффекты
    //interf->option(A_ui_page_drawing, TINTF_0CE);         //  Рисование (оключено, т.к. используется старая схема с глобальным оверлеем)
    interf->option(A_ui_page_modules, "🏗 Модули");         //  Modules
    basicui::menuitem_settings(interf);                     //  настройки

    interf->json_section_end();
}

#ifdef DISABLED_CODE
/**
 * UI блок с настройками параметров эффекта
 * выводится на странице "Управление списком эффектов"
 */
void ui_section_effects_list_configuration(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;

    interf->json_section_begin(TCONST_set_effect);

    interf->text(TCONST_effname, "", TINTF_effrename);       // поле под новое имя оставляем пустым

    interf->json_section_line();
        //interf->checkbox(TCONST_eff_sel, !confEff->flags.hidden, TINTF_in_sel_lst);         // доступен для выбора в выпадающем списке на главной странице
        //interf->checkbox(TCONST_eff_fav, !confEff->flags.disabledInDemo, TINTF_in_demo);    // доступен в демо-режиме
    interf->json_section_end();

    interf->spacer();

    interf->button(button_t::submit, TCONST_set_effect, TINTF_Save, P_GRAY);            // Save btn
    interf->button_value(button_t::submit, TCONST_set_effect, TCONST_copy, TINTF_005);  // Copy button

    interf->json_section_line();
        interf->button_value(button_t::submit, TCONST_set_effect, TCONST_delfromlist, TINTF_0B5, P_RED);    // удалить эффект из списка
        interf->button_value(button_t::submit, TCONST_set_effect, TCONST_delall, TINTF_0B4, P_GREEN);       // сбросить настройки эффекта по-умолчанию
    interf->json_section_end();

    interf->button_value(button_t::submit, TCONST_set_effect, TCONST_makeidx, TINTF_007, P_BLACK);

    interf->json_section_end(); // json_section_begin(TCONST_set_effect);
}

/**
 * страница "Управление списком эффектов"
 * здесь выводится ПОЛНЫЙ список эффектов в выпадающем списке
 */
void show_effects_config(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;

    interf->json_frame_interface();
    interf->json_section_main(A_ui_page_effects_config, TINTF_009);
    //confEff = myLamp.effwrkr.getSelectedListElement();

    if(LittleFS.exists(TCONST_eff_fulllist_json)){
        // формируем и отправляем кадр с запросом подгрузки внешнего ресурса
        interf->json_frame(P_xload);

        interf->json_section_content();
/*
        interf->select_xload(TCONST_effListConf, static_cast<unsigned>( myLamp.effwrkr.getCurrentEffectItem().eid ), TINTF_00A,
                        true,   // direct
                        TCONST_eff_fulllist_json
                );
*/
        interf->json_section_end();

        // generate block with effect settings controls
        ui_section_effects_list_configuration(interf, {}, NULL);
        interf->spacer();
        interf->button(button_t::generic, A_ui_page_effects, TINTF_exit);
        interf->json_frame_flush();
        return;
    }

    interf->constant("Rebuilding effects list, pls retry in a second...");
    interf->json_frame_flush();
    //rebuild_effect_list_files(lstfile_t::full);
}
#endif  // DISABLED_CODE


/**
 * @brief page with buttons leading to configuration of various external devices
 * 
 */
void ui_page_setup_devices(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_main(A_ui_page_setupdevs, "Конфигурация периферийных устройств");

    // display setup
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_display), TINTF_display_setup);

    // Button configuration
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_bttn), TINTF_ButtonEncoder);

    // tm1637
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_tm1637), TINTF_setup_tm1637);

    // MP3 player
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_dfplayer), TINTF_099);

    interf->json_frame_flush();
}

/**
 * @brief build a page with tm1637 configuration
 * it contains a set of controls and options
 */
void ui_page_tm1637_setup(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.tm1637" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_tm1637(interf, {}, NULL);
}

// this will trigger widgets list page opening
void ui_page_modules(Interface *interf, JsonObjectConst data, const char* action){
  uidata_page_selector(interf, data, action, page::modules);
}

/**
 * @brief build a page with Button / Encoder configuration
 * it contains a set of controls and options
 */
void page_button_setup(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.button" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_button_gpio(interf, {}, NULL);
    getset_encoder_gpio(interf, {}, NULL);

    JsonDocument doc;
    if (embuifs::deserializeFile(doc, T_benc_cfg)) return;      // config is missing, bad
    JsonArray bevents( doc[T_btn_events] );

    interf->json_frame_interface();
    interf->json_section_begin("button_events_list");

    int cnt = 0;
    for (JsonVariant value : bevents) {
        JsonObject obj = value.as<JsonObject>();
        interf->json_section_begin(String("sec") + cnt, (const char*)0, false, false, true );
        interf->checkbox(P_EMPTY, obj[T_enabled], "Active");
        interf->checkbox(P_EMPTY, obj[T_pwr], "Pwr On/Off");

        String s;
        switch (obj[T_btn_event].as<int>()){
            case 2:
                s = "Click";
                break;
            case 3:
                s = "Long Press";
                break;
            case 5:
                s = "Hold repeat";
                break;
            case 6:
                s = "MultiClick:";
                s += obj[T_clicks].as<int>();
                break;
            default:
                s = "Unknown";
        }

        interf->constant(s);

        switch (obj[T_lamp_event].as<int>()){
            case 11:
                s = "PwrOn";
                break;
            case 12:
                s = "PwrOff";
                break;
            case 13:
                s = "Pwr Toggle";
                break;
            case 24:
                s = "Яркость:";
                s += obj[T_arg].as<int>();
                break;
            case 30:
                s = "Эффект номер:";
                s += obj[T_arg].as<int>();
                break;
            case 31:
                s = "Следующий эффект";
                break;
            case 32:
                s = "Предыдущий эффект";
                break;
            case 33:
                s = "Случайный эффект";
                break;
            default:
                s = "Unknown";
        }

        interf->constant(s);
        interf->button_value(button_t::generic, A_button_evt_edit, cnt , T_edit);

        interf->json_section_end();
        ++cnt;
    }

    interf->json_frame_flush();
}

void page_button_evtedit(Interface *interf, JsonObjectConst data, const char* action){
    JsonDocument doc;
    if (embuifs::deserializeFile(doc, T_benc_cfg)) return;
    JsonArray bevents( doc[T_btn_events] );
    int idx = data[A_button_evt_edit];
    JsonObject obj = bevents[idx];

    interf->json_frame_interface();
    interf->json_section_begin("button_events_edit", "Button Event Editor", true);
        // side-load button configuration form
        interf->json_section_uidata();
            interf->uidata_pick( "lampui.sections.button_event" );
        interf->json_section_end();
    interf->json_frame_flush();

    // fill the form with values
    interf->json_frame_value(obj);
    interf->value(T_idx, idx);
    interf->json_frame_flush();
}

void page_button_evt_save(Interface *interf, JsonObjectConst data, const char* action){
    JsonDocument doc;
    if (embuifs::deserializeFile(doc, T_benc_cfg)) doc.clear();
    JsonArray bevents( doc[T_btn_events] );
    int idx = data[T_idx];
    JsonObject obj = idx < bevents.size() ? bevents[idx] : bevents.add<JsonObject>();

    // copy keys from post'ed object
    for (JsonPairConst kvp : data)
        obj[kvp.key()] = kvp.value();

    embuifs::serialize2file(doc, T_benc_cfg);

    button_configure_events(doc[T_btn_events]);

    if (interf) page_button_setup(interf, {}, NULL);
}

// DFPlayer related pages
void page_dfplayer_setup(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.dfplayer" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_dfplayer_device(interf, {}, NULL);
    getset_dfplayer_opt(interf, {}, NULL);
}

#ifdef DISABLED_CODE
/**
 * @brief переключение эффекта в выпадающем списке на странице "управление списком эффектов"
 * т.к. страница остается таже, нужно только обновить значения нескольких полей значениями для нового эффекта
 */
void set_effects_config_list(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf || !data) return;

    // получаем номер выбраного эффекта 
    uint16_t num = data[TCONST_effListConf].as<uint16_t>();
/*
    if(confEff){ // если переключаемся, то сохраняем предыдущие признаки в эффект до переключения
        LOG(printf_P, PSTR("eff_sel: %d eff_fav : %d, new eff:%d\n"), data[TCONST_eff_sel].as<bool>(),data[TCONST_eff_fav].as<bool>(), num);
    }
*/
    //confEff = myLamp.effwrkr.getEffect(static_cast<effect_t>(num));

    // обновляем поля
    interf->json_frame_value();

    //interf->value(TCONST_eff_sel, !confEff->flags.hidden, false);          // доступен для выбора в выпадающем списке на главной странице
    //interf->value(TCONST_eff_fav, !confEff->flags.disabledInDemo, false);             // доступен в демо-режиме

    interf->json_frame_flush();
}
#endif // DISABLED_CODE


/**
 * Формирование и вывод секции с дополнительными переключателями на основной странице
 * вкл/выкл, демо, и пр. что скрывается за кнопкой "Ещё..."
 * формируется не основная страница а секция, заменяющая собой одноименную секцию на основной странице
 */
void ui_block_mainpage_switches(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface(P_content);    // replace sections on existing main page
    // load uidata objects for the lamp
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.sections.main_switches");
    interf->json_section_end();
    interf->json_frame_flush();

    interf->json_frame_value();
        // lamp pwr switch
        interf->value(A_dev_pwrswitch, myLamp.getPwr());
        // demo status
        interf->value(K_demo, myLamp.getDemoMode());
        // button lock
        getset_btn_lock(interf, {}, NULL);
        // current effect's luma curve
        interf->value(A_dev_lcurve, e2int(myLamp.effwrkr.getCurrentEffectItem().curve));
    interf->json_frame_flush();

    // request state publishing from MP3Player
    EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::mp3state));
}

/*  
    Страница "Эффекты" (заглавная страница)
    здесь выводится список эффектов который не содержит "скрытые" элементы
*/
void ui_page_effects(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();

    interf->json_section_uidata();
        interf->uidata_pick( "lampui.pages.effTitle" );

    interf->json_frame_value();
        interf->value(A_dev_pwrswitch, myLamp.isLampOn());
        interf->value(A_dev_brightness, static_cast<int>(myLamp.getBrightness()));

    // build effect controls
    myLamp.effwrkr.mkEmbUIpage(interf);
}

/**
 * @brief handle Demo flag change from WebUI
 * 
 */
void set_demoflag(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;
    bool newdemo = data[K_demo];
    myLamp.demoMode(newdemo);
}

void set_auxflag(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;
    int pin = embui.getConfig()[TCONST_aux_gpio];
    if ( pin == -1) return;
    bool state = ( digitalRead(pin) == embui.getConfig()[TCONST_aux_ll] );

    if ((data[TCONST_AUX]) != state) {
        digitalWrite(pin, !state);
    }
}

/**
 * @brief WebUI страница "Настройки" - "другие"
 * 
 */
void page_settings_other(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.otheropts" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_settings_other(interf, {}, NULL);
}

/*
void set_overlay_drawing(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;
    myLamp.enableDrawing(data[TCONST_drawbuff]);
}
*/
void set_mp3mute(Interface *interf, JsonObjectConst data, const char* action){
    if (!data) return;

    bool v = data[T_mp3mute];
    EVT_POST(LAMP_SET_EVENTS, e2int(v ? evt::lamp_t::mp3mute : evt::lamp_t::mp3unmute ));
}

void set_mp3volume(Interface *interf, JsonObjectConst data, const char* action){
    int32_t volume = data[T_mp3vol];
    EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::mp3vol), &volume, sizeof(volume));
}

/*
    сохраняет настройки GPIO и перегружает контроллер
 */
void getset_gpios(Interface *interf, JsonObjectConst data, const char* action){

    if (!data || !data.size()){
        JsonDocument doc;
        if (embuifs::deserializeFile(doc, TCONST_fcfg_gpio)) doc.clear();     // reset if cfg is broken or missing

        // it's a request, send current configuration
        interf->json_frame_value(doc);
        interf->json_frame_flush();
        return;
    }

    // save posted config to file
    embuifs::serialize2file(data, TCONST_fcfg_gpio);

    run_action(ra::reboot);         // reboot in 5 sec
    basicui::page_system_settings(interf, {}, NULL);
}

/* Страница "Рисование"
void ui_page_drawing(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;
    interf->json_frame_interface();  //TINTF_080);
    interf->json_section_main(A_ui_page_drawing, TINTF_0CE);

    JsonDocument doc;
    JsonObject param = doc.to<JsonObject>();

    param[T_width] = display.getLayout().canvas_w();
    param[T_height] = display.getLayout().canvas_h();
    param[TCONST_blabel] = TINTF_0CF;
    param[TCONST_drawClear] = TINTF_0D9;

    interf->checkbox(TCONST_drawbuff, myLamp.isDrawOn(), TINTF_0CE, true);
    interf->div(T_drawing, T_drawing, embui.param(TCONST_txtColor), TINTF_0D0, P_EMPTY, param);

    interf->json_frame_flush();
}
*/

/**
 * @brief additional elements on system settings page
 * 
 */
void block_user_settings(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_section_begin("cfg_buttons");
    // periferal devices
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_devices), "Внешние устройства");
    // mike
    interf->button_value(button_t::generic, A_ui_page, e2int(page::mike), TINTF_020);

    // other
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_other), TINTF_082);

    // show gpio setup page button
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_gpio), TINTF_gpiocfg);
    interf->json_frame_flush();
}

// обработчик, для поддержки приложения WLED APP
void wled_handle(AsyncWebServerRequest *request){
    if(request->hasParam("T")){
        int pwr = request->getParam("T")->value().toInt();
        if (pwr == 2)
            EVT_POST(LAMP_SET_EVENTS, e2int(evt::lamp_t::pwrtoggle));   // '2' is for toggle
        else
            EVT_POST(LAMP_SET_EVENTS, pwr ? e2int(evt::lamp_t::pwron) : e2int(evt::lamp_t::pwroff));
    }
    uint8_t bright = myLamp.isLampOn() ? myLamp.getBrightness() : 0;

    if (request->hasParam("A")){
        bright = request->getParam("A")->value().toInt();
        run_action(ra::brt_nofade, bright);
    }

    String result = "<?xml version=\"1.0\" ?><vs><ac>";
    result.concat(myLamp.isLampOn()?bright:0);
    result.concat("</ac><ds>");
    result.concat(embui.hostname());
    result.concat("</ds></vs>");

    request->send(200, PGmimexml, result);
}

void show_progress(Interface *interf, JsonObjectConst data, const char* action){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_hidden(T_DO_OTAUPD, String(TINTF_056) + " : " + data[TINTF_05A].as<String>()+ "%");
    interf->json_section_end();
    interf->json_frame_flush();
}

/**
 * @brief build a page with LED Display setup
 * 
 */
void page_display_setup(Interface *interf, JsonObjectConst data, const char* action){
    interf->json_frame_interface();
    interf->json_section_main(P_EMPTY, TINTF_display_setup);

    // determine which value we should set drop-down list to
    int select_val = data && data[T_display_type].is<int>() ? data[T_display_type] : e2int(display.get_engine_type());

    interf->select(T_display_type, select_val, TINTF_display_type, true);
        interf->option(0, "ws2812b LED stripe");
        interf->option(1, "HUB75 RGB Panel");
    interf->json_section_end();

    interf->spacer();

    // if parameter for the specific page has been given
    if (data && data[T_display_type].is<int>()){
        if (data[T_display_type] == e2int(engine_t::hub75))
            block_display_setup(interf, engine_t::hub75);
        else
            block_display_setup(interf, engine_t::ws2812);
    } else { // check running engine type
        if ( display.get_engine_type() == engine_t::hub75)
            // load page block with HUB75 setup
            block_display_setup(interf, engine_t::hub75);
        else
            // load page block with ledstrip setup
            block_display_setup(interf, engine_t::ws2812);
    }

    // previous blocks MUST flush the interface frame!
}

/**
 * @brief build a section with LED-strip setup
 * it contains a set of controls to setup LedStrip topology
 */
void block_display_setup(Interface *interf, engine_t e){
    interf->json_section_uidata();
        interf->uidata_pick( e == engine_t::hub75 ? "lampui.settings.hub75" : "lampui.settings.ws2812");
    interf->json_frame_flush();

    JsonDocument doc;
    // if config can't be loaded, then just quit
    auto key = e == engine_t::hub75 ? T_hub75 : T_ws2812;
    if (embuifs::deserializeFile(doc, TCONST_fcfg_display) || doc[key] == nullptr ) return;

    interf->json_frame_value(doc[key]);
    interf->json_frame_flush();

/*
    // this code is obsolete, left for reference only

    // open a section
    interf->json_section_begin(A_display_ws2812, TINTF_ledstrip);

    interf->hidden(T_display_type, e2int(engine_t::ws2812));        // set hidden value for led type to ws2812

    interf->comment("Параметры матрицы (смена gpio требует перезагрузки)");

    interf->json_section_line(); // расположить в одной линии
        // gpio для подключения LED матрицы
        interf->number_constrained(T_mx_gpio, display.getGPIO(), "LED Matrix gpio", 1, -1, NUM_OUPUT_PINS);
        interf->number_constrained(T_CLmt, static_cast<int>(display.getCurrentLimit()), TINTF_095, 100, 1000, 16000);    // FastLED current limit
    interf->json_section_end();
    interf->json_section_line(); // расположить в одной линии
        interf->number_constrained(T_width,  (int)display.getLayout().tile_w(), "ширина", 1, 1, 256);
        interf->number_constrained(T_height, (int)display.getLayout().tile_h(), "высота", 1, 1, 256);
    interf->json_section_end();

    interf->json_section_line(); // расположить в одной линии
        interf->checkbox(T_snake, display.getLayout().snake(), I_zmeika, false);
        interf->checkbox(T_vflip, display.getLayout().vmirror(), I_vflip, false);
    interf->json_section_end();
    interf->json_section_line(); // расположить в одной линии
        interf->checkbox(T_vertical, display.getLayout().vertical(), I_vert, false);
        interf->checkbox(T_hflip, display.getLayout().hmirror(), I_hflip, false);
    interf->json_section_end();

    interf->spacer();

    interf->comment("Параметры каскада матриц");
    interf->json_section_line(); // расположить в одной линии
        interf->number_constrained(T_wcnt,   (int)display.getLayout().tile_wcnt(), "плиток по X", 1, 1, 32);
        interf->number_constrained(T_hcnt,   (int)display.getLayout().tile_hcnt(), "плиток по Y", 1, 1, 32);
    interf->json_section_end();
    interf->json_section_line(); // расположить в одной линии
        interf->checkbox(T_tsnake, display.getLayout().tileLayout.snake(), I_zmeika);
        interf->checkbox(T_tvflip, display.getLayout().tileLayout.vmirror(), I_vflip);
    interf->json_section_end();
    interf->json_section_line(); // расположить в одной линии
        interf->checkbox(T_tvertical, display.getLayout().tileLayout.vertical(), I_vert);
        interf->checkbox(T_thflip, display.getLayout().tileLayout.hmirror(), I_hflip);
    interf->json_section_end();

    interf->button(button_t::submit, A_display_ws2812, TINTF_Save);  // Save
    interf->button(button_t::generic, A_ui_page_settings, TINTF_exit);           // Exit

    interf->json_frame_flush();     // close "K_set_ledstrip" section and flush frame
*/
}

/**
 * Набор конфигурационных переменных и callback-обработчиков EmbUI
 */
void embui_actions_register(){
    // создаем конфигурационные параметры и регистрируем обработчики активностей

    // create defailt values for some vars

    // регистрируем обработчики активностей
    embui.action.set_mainpage_cb(ui_page_main);                             // index page callback
    embui.action.set_settings_cb(block_user_settings);                      // "settings" page options callback

    embui.action.add(A_ui_page, ui_page_selector);                          // ui page switcher, same as in basicui::
    embui.action.add(A_ui_page_effects, ui_page_effects);                   // меню: переход на страницу "Эффекты"
    //embui.action.add(A_ui_page_drawing, ui_page_drawing);                   // меню: переход на страницу "Рисование"
    embui.action.add(A_ui_page_modules, ui_page_modules);                   // меню: переход на страницу "Модули"
    embui.action.add(A_ui_block_switches, ui_block_mainpage_switches);      // нажатие кнопки "еще..." на странице "Эффекты"

    // device controls
    embui.action.add(A_dev_pwrswitch, set_pwrswitch);                       // lamp's powerswitch action
    embui.action.add(A_dev_brightness, getset_brightness);                  // Lamp brightness
    embui.action.add(A_dev_lcurve, set_lcurve);                             // luma curve control

    // Effects control
    embui.action.add(A_effect_switch, effect_switch);                       // effect switcher action
    embui.action.add(A_effect_control_mask, set_effect_control);            // Effect controls handler

    // display configurations
    embui.action.add(A_display_ws2812, set_ledstrip);                       // Set LED strip layout setup
    embui.action.add(A_display_hub75, set_hub75);                           // Set options for HUB75 panel
    embui.action.add(A_display_tm1637, getset_tm1637);                      // get/set tm1637 display configuration

    // button configurations
    embui.action.add(A_dev_btnlock, getset_btn_lock);                       // button locking
    embui.action.add(A_button_gpio, getset_button_gpio);                    // button setup
    embui.action.add(A_button_evt_edit, page_button_evtedit);               // button event edit form
    embui.action.add(A_button_evt_save, page_button_evt_save);              // button save/apply event
    embui.action.add(A_encoder_gpio, getset_encoder_gpio);                  // encoder gpio

    // DFPlayer
    embui.action.add(A_dfplayer_dev, getset_dfplayer_device);               // DFPlayer device setup
    embui.action.add(A_dfplayer_opt, getset_dfplayer_opt);                  // DFPlayer options setup
    embui.action.add(T_mp3vol, set_mp3volume);
    embui.action.add(T_mp3mute, set_mp3mute);

    embui.action.add(A_set_gpio, getset_gpios);                             // Get/Set gpios
    embui.action.add(A_getset_other, getset_settings_other);                   // get/set settings "other" page handler


    // to be refactored
    embui.action.add(K_demo, set_demoflag);
    embui.action.add(TCONST_AUX, set_auxflag);

    // disable old overlay buffer
    //embui.action.add(TCONST_draw_dat, set_drawing);
    //embui.action.add(TCONST_drawbuff, set_overlay_drawing);

    embui.action.add(T_display_type, page_display_setup);                // load display setup page depending on selected disp type (action for drop down list)

}
