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
#include "modules/clock/mod_clock.hpp"


// версия ресурсов в стороннем джейсон файле
#define UIDATA_VERSION      22

#define DEMO_MIN_PERIOD     10
#define DEMO_MAX_PERIOD     900
#define DEMO_PERIOD_STEP    10

// placeholder for effect list rebuilder task
Task *delayedOptionTask = nullptr;
// эффект, который сейчас конфигурируется на странице "Управление списком эффектов"
EffectListElem *confEff = nullptr;

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
void ui_page_main(Interface *interf, const JsonObject *data, const char* action);
// CallBack - append Lamps's settings elements to system's "Settings" page
void user_settings_frame(Interface *interf, const JsonObject *data, const char* action);

void ui_page_effects(Interface *interf, const JsonObject *data, const char* action);
void ui_page_setup_devices(Interface *interf, const JsonObject *data, const char* action);
void ui_section_effects_list_configuration(Interface *interf, const JsonObject *data, const char* action);
void show_effects_config(Interface *interf, const JsonObject *data, const char* action);
//void show_settings_enc(Interface *interf, const JsonObject *data, const char* action);

// construct a page with Display setup
void page_display_setup(Interface *interf, const JsonObject *data, const char* action);
// construct a page with TM1637 setup
void ui_page_tm1637_setup(Interface *interf, const JsonObject *data, const char* action);
// send/set element values to the page with gpio setup
void getset_gpios(Interface *interf, const JsonObject *data, const char* action);
void page_settings_other(Interface *interf, const JsonObject *data, const char* action);
void section_sys_settings_frame(Interface *interf, const JsonObject *data, const char* action);
//void show_settings_butt(Interface *interf, const JsonObject *data, const char* action);


/**
 * @brief function renders display configuration pages to the WebUI
 * 
 * @param engine_t e - an engine type to show controls for
 * 
 */
void block_display_setup(Interface *interf, engine_t e);

/**
 * @brief rebuild cached json file with effects names list
 * i.e. used for sideloading in WebUI
 * @param full - rebuild full list or brief, excluding hidden effs
 * todo: implement an event queue
 */
void rebuild_effect_list_files(lstfile_t lst);




/* *** auxilary functions *** */

// сброс таймера демо и настройка автосохранений
void resetAutoTimers(bool isEffects=false){
    myLamp.demoReset();
    if(isEffects)
        myLamp.effwrkr.autoSaveConfig();
}



/* *** WebUI generators *** */

/**
 * @brief loads UI page from uidata storage and triggers value generation for UI elements
 * 
 * @param interf 
 * @param data 
 * @param action 
 */
void uidata_page_selector(Interface *interf, const JsonObject *data, const char* action, page idx){
    interf->json_frame_interface();
    interf->json_section_uidata();

    switch (idx){
        // настрока gpio
        case page::setup_gpio :
            interf->uidata_pick( "lampui.pages.gpiosetup" );
            interf->json_frame_flush();
            getset_gpios(interf,  nullptr, NULL);
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
void ui_page_selector(Interface *interf, const JsonObject *data, const char* action){
    if (!interf || !data || (*data)[A_ui_page].isNull()) return;  // quit if no section specified

    // get a page index
    page idx = static_cast<page>((*data)[A_ui_page].as<int>());

    switch (idx){
        case page::eff_config :   // страница "Управление списком эффектов"
            show_effects_config(interf, nullptr, NULL);
            return;
        case page::mike :         // страница настроек микрофона
            show_settings_mic(interf, nullptr, NULL);
            return;
        case page::setup_bttn :    // страница настроек кнопки
            return page_button_setup(interf, nullptr, NULL);
        case page::setup_other :    // страница "настройки"-"другие"
            return page_settings_other(interf, nullptr, NULL);
        case page::setup_display :  // led display setup (strip/hub75)
            return page_display_setup(interf, nullptr, NULL);
        case page::setup_devices :  // periferal devices setup selector page
            return ui_page_setup_devices(interf, nullptr, NULL);
        case page::setup_tm1637 :   // tm1637 display setup
            return ui_page_tm1637_setup(interf, nullptr, NULL);
        case page::setup_dfplayer :
            return page_dfplayer_setup(interf, nullptr, NULL);

        default:;                   // by default do nothing
    }

    if (e2int(idx) > 100)
        uidata_page_selector(interf, data, action, idx);
}

void ui_section_menu(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;
    // создаем меню
    interf->json_section_menu();

    interf->option(A_ui_page_effects, TINTF_000);           //  Эффекты
    //interf->option(A_ui_page_drawing, TINTF_0CE);        //  Рисование (оключено, т.к. используется старая схема с глобальным оверлеем)
    interf->option(A_ui_page_modules, "Modules");           //  Modules
    basicui::menuitem_settings(interf);                     //  настройки

    interf->json_section_end();
}

/**
 * UI блок с настройками параметров эффекта
 * выводится на странице "Управление списком эффектов"
 */
void ui_section_effects_list_configuration(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;

    interf->json_section_begin(TCONST_set_effect);

    interf->text(TCONST_effname, "", TINTF_effrename);       // поле под новое имя оставляем пустым

    interf->json_section_line();
        interf->checkbox(TCONST_eff_sel, confEff->canBeSelected(), TINTF_in_sel_lst);      // доступен для выбора в выпадающем списке на главной странице
        interf->checkbox(TCONST_eff_fav, confEff->enabledInDemo(), TINTF_in_demo);         // доступен в демо-режиме
    interf->json_section_end();

    interf->spacer();

    // sorting option
/*
    interf->select(V_effSort, TINTF_040);
        interf->option(SORT_TYPE::ST_BASE, TINTF_041);
        interf->option(SORT_TYPE::ST_END, TINTF_042);
        interf->option(SORT_TYPE::ST_IDX, TINTF_043);
        interf->option(SORT_TYPE::ST_AB, TINTF_085);
        interf->option(SORT_TYPE::ST_AB2, TINTF_08A);
        interf->option(SORT_TYPE::ST_MIC, TINTF_08D);  // эффекты с микрофоном
    interf->json_section_end();
*/

    interf->button(button_t::submit, TCONST_set_effect, TINTF_Save, P_GRAY);            // Save btn
    interf->button_value(button_t::submit, TCONST_set_effect, TCONST_copy, TINTF_005);  // Copy button
    //if (confEff->eff_nb&0xFF00) { // пока удаление только для копий, но в теории можно удалять что угодно
        // interf->button_value(button_t::submit, TCONST_set_effect, TCONST_del_, TINTF_006, P_RED);
    //}

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
void show_effects_config(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;

    interf->json_frame_interface();
    interf->json_section_main(A_ui_page_effects_config, TINTF_009);
    confEff = myLamp.effwrkr.getSelectedListElement();

    if(LittleFS.exists(TCONST_eff_fulllist_json)){
        // формируем и отправляем кадр с запросом подгрузки внешнего ресурса
        interf->json_frame(P_xload);

        interf->json_section_content();
        interf->select(TCONST_effListConf, (int)confEff->eff_nb, TINTF_00A,
                        true,   // direct
                        TCONST_eff_fulllist_json
                );
        interf->json_section_end();

        // generate block with effect settings controls
        ui_section_effects_list_configuration(interf, nullptr, NULL);
        interf->spacer();
        interf->button(button_t::generic, A_ui_page_effects, TINTF_exit);
        interf->json_frame_flush();
        return;
    }

    interf->constant("Rebuilding effects list, pls retry in a second...");
    interf->json_frame_flush();
    rebuild_effect_list_files(lstfile_t::full);
}


/**
 * @brief индексная страница WebUI
 * 
 */
void ui_page_main(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;

    interf->json_frame_interface(); //TINTF_080);
    interf->json_section_manifest(TINTF_080, embui.macid(), 0, LAMPFW_VERSION_STRING);       // app name/version manifest
    interf->json_section_end();

    // load uidata objects for the lamp
    interf->json_section_uidata();
        interf->uidata_xload("lampui", "js/ui_lamp.json", false, UIDATA_VERSION);
    interf->json_section_end();

    ui_section_menu(interf, data, action);
    interf->json_frame_flush();     // close frame

    if(WiFi.getMode() & WIFI_MODE_STA){
        ui_page_effects(interf, data, action);
    } else {
        // открываем страницу с настройками WiFi если контроллер не подключен к внешней AP
        basicui::page_settings_netw(interf, nullptr, NULL);
    }
}

/**
 * @brief page with buttons leading to configuration of various external devices
 * 
 */
void ui_page_setup_devices(Interface *interf, const JsonObject *data, const char* action){
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
void ui_page_tm1637_setup(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.tm1637" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_tm1637(interf, nullptr, NULL);
}

// this will trigger widgets list page opening
void ui_page_modules(Interface *interf, const JsonObject *data, const char* action){
  uidata_page_selector(interf, data, action, page::modules);
}

/**
 * @brief build a page with Button / Encoder configuration
 * it contains a set of controls and options
 */
void page_button_setup(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.button" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_button_gpio(interf, nullptr, NULL);
    getset_encoder_gpio(interf, nullptr, NULL);

    JsonDocument doc;
    if (!embuifs::deserializeFile(doc, T_benc_cfg)) return;      // config is missing, bad
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

void page_button_evtedit(Interface *interf, const JsonObject *data, const char* action){
    JsonDocument doc;
    if (!embuifs::deserializeFile(doc, T_benc_cfg)) return;
    JsonArray bevents( doc[T_btn_events] );
    int idx = (*data)[A_button_evt_edit];
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

void page_button_evt_save(Interface *interf, const JsonObject *data, const char* action){
    JsonDocument doc;
    if (!embuifs::deserializeFile(doc, T_benc_cfg)) doc.clear();
    JsonArray bevents( doc[T_btn_events] );
    int idx = (*data)[T_idx];
    JsonObject obj = idx < bevents.size() ? bevents[idx] : bevents.add<JsonObject>();

    // copy keys from post'ed object
    for (JsonPair kvp : *data)
        obj[kvp.key()] = kvp.value();

    embuifs::serialize2file(doc, T_benc_cfg);

    button_configure_events(doc[T_btn_events]);

    if (interf) page_button_setup(interf, nullptr, NULL);
}

// DFPlayer related pages
void page_dfplayer_setup(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.dfplayer" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_dfplayer_device(interf, nullptr, NULL);
    getset_dfplayer_opt(interf, nullptr, NULL);
}


/**
 * обработчик установок эффекта
 */
void set_effects_config_param(Interface *interf, const JsonObject *data, const char* action){
    if (!confEff || !data) return;
    EffectListElem *effect = confEff;
    
    //bool isNumInList =  (*data)[TCONST_numInList] == "1";

    bool isEffHasMic = (*data)[TCONST_effHasMic];
    myLamp.setEffHasMic(isEffHasMic);

    // sorting is removed
    //SORT_TYPE st = (*data)[V_effSort].as<SORT_TYPE>();
    //embui.var(V_effSort, (*data)[V_effSort]); 
    //myLamp.effwrkr.setEffSortType(st);
    myLamp.save_flags();
    
    String act = (*data)[TCONST_set_effect];
    // action is to "copy" effect
    if (act == TCONST_copy) {
        myLamp.effwrkr.copyEffect(effect); // копируем текущий, это вызовет перестроение индекса
        LOG(println, "Effect copy, rebuild list");
        rebuild_effect_list_files(lstfile_t::all);
        return;
    }
    
    // action is to "delete" effect
    if (act == TCONST_delfromlist || act == TCONST_delall) {
        uint16_t tmpEffnb = effect->eff_nb;
        LOG(printf, "delete effect->eff_nb=%d\n", tmpEffnb);
        bool isCfgRemove = (act == TCONST_delall);

        if(tmpEffnb==myLamp.effwrkr.getCurrentEffectNumber()){
            myLamp.effwrkr.switchEffect(EFF_ENUM::EFF_NONE);
            run_action(ra::eff_next);
        }

        confEff = myLamp.effwrkr.getEffect(EFF_ENUM::EFF_NONE);
        if(isCfgRemove){
            myLamp.effwrkr.deleteEffect(effect, true);  // удаляем только конфиг эффекта с ФС
            myLamp.effwrkr.makeIndexFileFromList();     // создаем индекс по текущему списку и на выход
            //myLamp.effwrkr.makeIndexFileFromFS();       // создаем индекс по файлам ФС и на выход
            rebuild_effect_list_files(lstfile_t::all);
        } else {
            myLamp.effwrkr.deleteEffect(effect, false); // удаляем эффект только из активного списка
            myLamp.effwrkr.makeIndexFileFromList();     // создаем индекс по текущему списку и на выход
            rebuild_effect_list_files(lstfile_t::selected);
        }
        return;
    }

    // action is "rebuild effects index"
    if (act == TCONST_makeidx) {
        myLamp.effwrkr.removeLists();
        myLamp.effwrkr.initDefault();
        LOG(println, PSTR("Force rebuild index"));
        rebuild_effect_list_files(lstfile_t::all);
        return;
    }
    
    // if selectivity changed, than need to rebuild json eff list for main page
    if ( (*data)[TCONST_eff_sel] != effect->canBeSelected() ){
        effect->canBeSelected((*data)[TCONST_eff_sel]);
        LittleFS.remove(TCONST_eff_list_json);
    }

    // could be used in demo
    effect->enabledInDemo((*data)[TCONST_eff_fav]);

    // check if effect has been renamed
    if (!(*data)[TCONST_effname].isNull()){
        LOG(println, PSTR("Effect rename, rebuild list"));
        myLamp.effwrkr.setEffectName((*data)[TCONST_effname], effect);
        // effect has been renamed, need to update BOTH dropdown list jsons
        myLamp.effwrkr.makeIndexFileFromList(NULL, true);
        return show_effects_config(interf, nullptr, NULL);       // force reload setup page
    }

    resetAutoTimers();
    myLamp.effwrkr.makeIndexFileFromList(); // обновить индексный файл после возможных изменений
    //ui_page_main(interf, nullptr, NULL);
}

/**
 * @brief переключение эффекта в выпадающем списке на странице "управление списком эффектов"
 * т.к. страница остается таже, нужно только обновить значения нескольких полей значениями для нового эффекта
 */
void set_effects_config_list(Interface *interf, const JsonObject *data, const char* action){
    if (!interf || !data) return;

    // получаем номер выбраного эффекта 
    uint16_t num = (*data)[TCONST_effListConf].as<uint16_t>();

    if(confEff){ // если переключаемся, то сохраняем предыдущие признаки в эффект до переключения
        LOG(printf_P, PSTR("eff_sel: %d eff_fav : %d, new eff:%d\n"), (*data)[TCONST_eff_sel].as<bool>(),(*data)[TCONST_eff_fav].as<bool>(), num);
    }

    confEff = myLamp.effwrkr.getEffect(num);

    //resetAutoTimers();

    // обновляем поля
    interf->json_frame_value();

    interf->value(TCONST_eff_sel, confEff->canBeSelected(), false);          // доступен для выбора в выпадающем списке на главной странице
    interf->value(TCONST_eff_fav, confEff->enabledInDemo(), false);             // доступен в демо-режиме

    interf->json_frame_flush();
}

#ifdef EMBUI_USE_MQTT
void mqtt_publish_selected_effect_config_json(){
  if (!embui.mqttAvailable()) return;
  embui.publish("effect/jsconfig", myLamp.effwrkr.getEffCfg().getSerializedEffConfig().c_str(), true);
}
#endif

/**
 * @brief UI block with current effect's controls
 * 
 */
void block_effect_controls(Interface *interf, const JsonObject *data, const char* action){

    JsonArrayConst sect = interf->json_section_begin(A_effect_ctrls, P_EMPTY, false, false, false, true);   // do not append section to main
    std::vector<std::shared_ptr<UIControl>> &controls = myLamp.effwrkr.getControls();
    uint8_t ctrlCaseType; // тип контрола, старшие 4 бита соответствуют CONTROL_CASE, младшие 4 - CONTROL_TYPE

    bool isMicOn = myLamp.getLampFlagsStuct().isMicOn;
    LOGD(T_WebUI, printf, "Make UI for %d controls\n", controls.size());
    for(unsigned i=0; i<controls.size();i++)
        if(controls[i]->getId()==7 && controls[i]->getName().startsWith(TINTF_020))
            isMicOn = isMicOn && controls[i]->getVal().toInt();

    LOGD(T_WebUI, printf, "block_effect_controls() got %u ctrls\n", controls.size());
    for (const auto &ctrl : controls){
        if (!ctrl->getId()) continue;       // skip old "brightness control"

        ctrlCaseType = ctrl->getType();
        switch(ctrlCaseType>>4){
            case CONTROL_CASE::HIDE :
                continue;
                break;
            case CONTROL_CASE::ISMICON :
                if(!isMicOn && (!myLamp.getLampFlagsStuct().isMicOn || !(ctrl->getId()==7 && ctrl->getName().startsWith(TINTF_020)==1) )) continue;
                break;
            case CONTROL_CASE::ISMICOFF :
                if(isMicOn && (myLamp.getLampFlagsStuct().isMicOn || !(ctrl->getId()==7 && ctrl->getName().startsWith(TINTF_020)==1) )) continue;
                break;
            default: break;
        }

        // если сейчас активен демо-режим и включена опция "случайные контролы в демо", добавлять префикс с имени контрола
        bool rnd_prefix = myLamp.getLampFlagsStuct().demoRndEffControls && myLamp.getLampFlagsStuct().demoMode;
        String ctrlId(T_effect_dynCtrl);
        ctrlId += ctrl->getId();
        String ctrlName = ctrl->getId() ? ctrl->getName() : TINTF_00D;

        switch(ctrlCaseType&0x0F){
            case CONTROL_TYPE::RANGE :
                {
                    if(rnd_prefix && ctrl->getId()>0 && !(ctrl->getId()==7 && ctrl->getName().startsWith(TINTF_020)==1))
                        ctrlName=String(TINTF_Rand)+ctrlName;
                    int value = ctrl->getId() ? ctrl->getVal().toInt() : myLamp.getBrightness();
                    if(interf) interf->range( ctrlId, (long)value, ctrl->getMin().toInt(), ctrl->getMax().toInt(), ctrl->getStep().toInt(), ctrlName, true);
                }
                break;
            case CONTROL_TYPE::EDIT :
                {
                    String ctrlName = ctrl->getName();
                    if(rnd_prefix && ctrl->getId()>0 && !(ctrl->getId()==7 && ctrl->getName().startsWith(TINTF_020)==1))
                        ctrlName=String(TINTF_Rand)+ctrlName;
                    
                    if(interf) interf->text(ctrlId
                    , ctrl->getVal()
                    , ctrlName
                    );
                    break;
                }
            case CONTROL_TYPE::CHECKBOX :
                {
                    String ctrlName = ctrl->getName();
                    if(rnd_prefix && ctrl->getId()>0 && !(ctrl->getId()==7 && ctrl->getName().startsWith(TINTF_020)==1))
                        ctrlName=String(TINTF_Rand)+ctrlName;

                    if(interf) interf->checkbox(ctrlId
                    , ctrl->getVal() == "1" ? true : false
                    , ctrlName
                    , true
                    );
                    break;
                }
            default:
                break;
        }
    }

    if(interf) interf->json_section_end();

    String topic(C_pub);
    topic += A_effect_ctrls;
    embui.publish(topic.c_str(), sect, true);

    LOGD(T_WebUI, println, "eof block_effect_controls()");
}

/**
 * @brief this function is a wrapper for block_effect_controls() to publish current effect controls to various feeders
 * it either can use a provided Interface object, or it will create a new one if called
 * from an internal fuctions not a post callbacks
 * 
 */
void publish_effect_controls(Interface *interf, const JsonObject *data, const char* action){
    LOG(println, "publish_effect_controls()");

    bool remove_iface = false;
    if (!interf){
        // no need to publish if no one is listening
        if (!embui.feeders.available()) return;
        interf = new Interface(&embui.feeders);
        remove_iface = true;
    }
    interf->json_frame_interface();
    block_effect_controls(interf, data, action);
    interf->json_frame_flush();

    // publish also current effect index (for drop-down selector)
    interf->json_frame_value();
    interf->value(A_effect_switch_idx, myLamp.effwrkr.getCurrentEffectNumber());
    interf->json_frame_flush();
    if (remove_iface) delete interf;
}

/**
 * Формирование и вывод секции с дополнительными переключателями на основной странице
 * вкл/выкл, демо, и пр. что скрывается за кнопкой "Ещё..."
 * формируется не основная страница а секция, заменяющая собой одноименную секцию на основной странице
 */
void ui_block_mainpage_switches(Interface *interf, const JsonObject *data, const char* action){
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
        getset_btn_lock(interf, nullptr, NULL);
        // Mike
        interf->value(A_dev_mike, myLamp.getMicState());
        // current effect's luma curve
        interf->value(A_dev_lcurve, e2int(myLamp.effwrkr.getEffCfg().curve));
    interf->json_frame_flush();

    // request state publishing from MP3Player
    EVT_POST(LAMP_GET_EVENTS, e2int(evt::lamp_t::mp3state));


#ifdef DISABLED_CODE
    interf->json_frame_interface("content");    // replace sections on existing main page

    interf->json_section_begin(T_switches);   // section to replace
    interf->json_section_line();
    interf->checkbox(A_dev_pwrswitch, myLamp.isLampOn(), TINTF_00E, true);
    interf->checkbox(K_demo, myLamp.getMode() == LAMPMODE::MODE_DEMO, TINTF_00F, true);
    interf->checkbox(TCONST_drawbuff, myLamp.isDrawOn(), TINTF_0CE, true);
    interf->checkbox(TCONST_Mic, myLamp.isMicOnOff(), TINTF_012, true);
    interf->checkbox(TCONST_AUX, embui.paramVariant(TCONST_AUX), TCONST_AUX, true);
    interf->checkbox(T_mp3mute, myLamp.isMP3mute(), "MP3 Mute" /*TINTF_099*/, true);

#ifdef LAMP_DEBUG
    interf->checkbox(TCONST_debug, myLamp.isDebugOn(), TINTF_08E, true);
#endif
    // curve selector
    interf->select(A_dev_lcurve, e2int(myLamp.effwrkr.getEffCfg().curve), "Luma curve", true);  // luma curve selector
        interf->option(0, "binary");
        interf->option(1, "linear");
        interf->option(2, "cie1931");
        interf->option(3, "exponent");
        interf->option(4, "sine");
        interf->option(5, "square");
    interf->json_section_end();     // select

    interf->json_section_end();     // json_section_line()
/*
    if(mp3->isMP3Mode()){
        interf->json_section_line("line124"); // спец. имя - разбирается внутри html
        interf->button(button_t::generic, CMD_MP3_PREV, TINTF_0BD, P_GRAY);
        interf->button(button_t::generic, CMD_MP3_NEXT, TINTF_0BE, P_GRAY);
        interf->button(button_t::generic, TCONST_mp3_p5, TINTF_0BF, P_GRAY);
        interf->button(button_t::generic, TCONST_mp3_n5, TINTF_0C0, P_GRAY);
        interf->json_section_end(); // line
    }
*/
    // регулятор громкости mp3 плеера
    interf->range(T_mp3vol, embui.paramVariant(T_mp3vol).as<int>(), 1, 30, 1, TINTF_09B, true);

    interf->button(button_t::generic, A_ui_page_effects, TINTF_exit);
    interf->json_frame_flush();
#endif  // DISABLED_CODE

}

/*  Страница "Эффекты" (заглавная страница)
    здесь выводится список эффектов который не содержит "скрытые" элементы
*/
void ui_page_effects(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;

    // start a new xload frame (need an xload for effects list)
    interf->json_frame_interface();
    interf->json_section_main(A_ui_page_effects, TINTF_000);

    // open a new section for flags, it could be replaced later with verbose switch box
    interf->json_section_begin(T_switches);
        interf->json_section_line();
            interf->checkbox(A_dev_pwrswitch, myLamp.isLampOn(), TINTF_00E, true);
            interf->button(button_t::generic, A_ui_block_switches, TINTF_014);
        interf->json_section_end(); // line
    interf->json_section_end(); // flags section

    if(LittleFS.exists(TCONST_eff_list_json)){
        // формируем и отправляем кадр с запросом подгрузки внешнего ресурса

        interf->json_section_xload();
            // side load drop-down list from /eff_list.json file
            interf->select(A_effect_switch_idx, myLamp.effwrkr.getCurrentEffectNumber(), TINTF_00A, true, TCONST_eff_list_json);
        interf->json_section_end(); // close xload section

        // 'next', 'prev' effect buttons << >>
        interf->json_section_line();
            interf->button(button_t::generic, A_effect_switch_prev, TINTF_015, TCONST__708090);
            interf->button(button_t::generic, A_effect_switch_next, TINTF_016, TCONST__5f9ea0);
        interf->json_section_end();

        interf->range(A_dev_brightness, static_cast<int>(myLamp.getBrightness()), 0, static_cast<int>(myLamp.getBrightnessScale()), 1, TINTF_00D, true);

        // build a block of controls for current effect
        block_effect_controls(interf, data, NULL);

        interf->button_value(button_t::generic, A_ui_page, e2int(page::eff_config), TINTF_009);
    } else {
        interf->constant("Rebuilding effects list, pls retry in a sec...");
        rebuild_effect_list_files(lstfile_t::selected);
    }

    interf->json_section_end();     // close main section
    interf->json_frame_flush();
}

/**
 * @brief handle Demo flag change from WebUI
 * 
 */
void set_demoflag(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    bool newdemo = (*data)[K_demo];
    myLamp.demoMode(newdemo);
}

void set_auxflag(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    int pin = embui.paramVariant(TCONST_aux_gpio);
    if ( pin == -1) return;
    bool state = ( digitalRead(pin) == embui.paramVariant(TCONST_aux_ll) );

    if (((*data)[TCONST_AUX]) != state) {
        digitalWrite(pin, !state);
    }
}

/**
 * @brief UI Draw on screen function
 * 
 */
/*
void set_drawing(Interface *interf, const JsonObject *data, const char* action){
    // draw pixel
    if ((*data)[P_color]){
        CRGB c = strtol((*data)[P_color].as<const char*>(), NULL, 0);
        myLamp.writeDrawBuf(c, (*data)["col"], (*data)["row"]);
        return;
    }
    // screen solid fill
    if ((*data)[TCONST_fill]){
        CRGB val = strtol((*data)[TCONST_fill].as<const char*>(), NULL, 0);
        myLamp.fillDrawBuf(val);
    }
}

// clear draw buffer to solid balck
void set_clear(Interface *interf, const JsonObject *data, const char* action){
        CRGB color=CRGB::Black;
        myLamp.fillDrawBuf(color);
}
*/
void show_settings_mic(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_main(TCONST_settings_mic, TINTF_020);

    interf->checkbox(A_dev_mike, myLamp.getLampFlagsStuct().isMicOn, TINTF_012, true);

    interf->json_section_begin(TCONST_set_mic);
        interf->number_constrained(V_micScale, round(myLamp.getLampState().getMicScale() * 10) / 10, TINTF_022, 0.1f, 0.1f, 4.0f);
        interf->number_constrained(V_micNoise, round(myLamp.getLampState().getMicNoise() * 10) / 10, TINTF_023, 0.1f, 0.0f, 32.0f);
        interf->range (V_micRdcLvl, (int)myLamp.getLampState().getMicNoiseRdcLevel(), 0, 4, 1, TINTF_024, false);
        interf->button(button_t::submit, TCONST_set_mic, TINTF_Save, P_GRAY);
    interf->json_section_end();

    interf->spacer();
    interf->button(button_t::generic, A_ui_page_settings, TINTF_exit);

    interf->json_frame_flush();
}

void set_settings_mic(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    float scale = (*data)[V_micScale];
    float noise = (*data)[V_micNoise];
    mic_noise_reduce_level_t rdl = static_cast<mic_noise_reduce_level_t>((*data)[V_micRdcLvl].as<unsigned>());

    LOGI(T_WebUI, printf, "Set mike: scale=%2.3f noise=%2.3f rdl=%u\n", scale, noise, (uint32_t)rdl);

    embui.var(V_micScale, scale);
    embui.var_dropnulls(V_micNoise, noise);
    embui.var_dropnulls(V_micRdcLvl, (*data)[V_micRdcLvl].as<unsigned>());

    // apply to running configuration
    myLamp.getLampState().setMicScale(scale);
    myLamp.getLampState().setMicNoise(noise);
    myLamp.getLampState().setMicNoiseRdcLevel(rdl);
    // mike has onChange control and does not work here
    //myLamp.setMicOnOff((*data)[T_mic]);

    basicui::page_system_settings(interf, data, NULL);
}

void set_micflag(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    myLamp.setMicOnOff((*data)[A_dev_mike]);
    LOGD(T_WebUI, printf, "Set mike: %u/%u\n", (*data)[T_mic].as<bool>(), myLamp.getLampFlagsStuct().isMicOn);
}

/**
 * @brief WebUI страница "Настройки" - "другие"
 * 
 */
void page_settings_other(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_uidata();
        interf->uidata_pick( "lampui.settings.otheropts" );
    interf->json_frame_flush();

    // call setter with no data, it will publish existing config values if any
    getset_settings_other(interf, nullptr, NULL);
}

#ifdef DISABLED_CODE
/**
 * @brief WebUI страница "Настройки" - "другие" (устаревший код)
 * 
 */
void page_settings_other(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_main(A_getset_other, TINTF_002);
    
    interf->spacer(TINTF_030);

    interf->checkbox(T_restoreState, myLamp.getLampFlagsStuct().restoreState, TINTF_f_restore_state, false);
    interf->checkbox(T_swFade, myLamp.getLampFlagsStuct().fadeEffects , TINTF_03D, false);
    interf->checkbox(T_swWipeScreen, myLamp.getLampFlagsStuct().wipeOnEffChange , TINTF_083, false);
    interf->json_section_line();
        interf->checkbox(T_demoRndOrder, myLamp.getLampFlagsStuct().demoRandom , TINTF_03E, false);
    interf->json_section_end(); // line

    interf->number_constrained(V_dev_brtscale, static_cast<int>(myLamp.getBrightnessScale()), "Brightness Scale", 1, 5, static_cast<int>(MAX_BRIGHTNESS));

    interf->json_section_line();
        interf->range(T_DemoTime, static_cast<int>(myLamp.getDemoTime()), DEMO_MIN_PERIOD, DEMO_MAX_PERIOD, DEMO_PERIOD_STEP, TINTF_03F);
        float sf = embui.paramVariant(T_effSpeedFactor);
        interf->range(T_effSpeedFactor, sf, 0.25f, 4.0f, 0.25f, TINTF_0D3, false);
    interf->json_section_end(); // line

    interf->button(button_t::submit, A_getset_other, TINTF_Save, P_GRAY);

    interf->spacer();
    interf->button(button_t::generic, A_ui_page_settings, TINTF_exit);

    interf->json_frame_flush();
}
#endif // DISABLED_CODE

void set_debugflag(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    myLamp.setDebug((*data)[TCONST_debug]);
    myLamp.save_flags();
}

// enable/disable overlay drawing
/*
void set_overlay_drawing(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    myLamp.enableDrawing((*data)[TCONST_drawbuff]);
}
*/
void set_mp3mute(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;

    bool v = (*data)[T_mp3mute];
    EVT_POST(LAMP_SET_EVENTS, e2int(v ? evt::lamp_t::mp3mute : evt::lamp_t::mp3unmute ));
}

void set_mp3volume(Interface *interf, const JsonObject *data, const char* action){
    if (!data) return;
    int32_t volume = (*data)[T_mp3vol];
    EVT_POST_DATA(LAMP_SET_EVENTS, e2int(evt::lamp_t::mp3vol), &volume, sizeof(volume));
}

/*
    сохраняет настройки GPIO и перегружает контроллер
 */
void getset_gpios(Interface *interf, const JsonObject *data, const char* action){

    if (!data || !(*data).size()){
        JsonDocument doc;
        if (!embuifs::deserializeFile(doc, TCONST_fcfg_gpio)) doc.clear();     // reset if cfg is broken or missing

        // it's a request, send current configuration
        interf->json_frame_value(doc);
        interf->json_frame_flush();
        return;
    }

    // save posted config to file
    embuifs::serialize2file(*data, TCONST_fcfg_gpio);

    run_action(ra::reboot);         // reboot in 5 sec
    basicui::page_system_settings(interf, nullptr, NULL);
}

//Страница "Рисование"
void ui_page_drawing(Interface *interf, const JsonObject *data, const char* action){
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

// Create Additional buttons on "Settings" page
void user_settings_frame(Interface *interf, const JsonObject *data, const char* action){
    // periferal devices
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_devices), "Внешние устройства");
    // mike
    interf->button_value(button_t::generic, A_ui_page, e2int(page::mike), TINTF_020);

    // other
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_other), TINTF_082);

    // show gpio setup page button
    interf->button_value(button_t::generic, A_ui_page, e2int(page::setup_gpio), TINTF_gpiocfg);
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

void show_progress(Interface *interf, const JsonObject *data, const char* action){
    if (!interf) return;
    interf->json_frame_interface();
    interf->json_section_hidden(T_DO_OTAUPD, String(TINTF_056) + " : " + (*data)[TINTF_05A].as<String>()+ "%");
    interf->json_section_end();
    interf->json_frame_flush();
}
/*
uint8_t uploadProgress(size_t len, size_t total){
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    static int prev = 0; // используется чтобы не выводить повторно предыдущее значение, хрен с ней, пусть живет
    float part = total / 50.0;
    int curr = len / part;
    uint8_t progress = 100*len/total;
    if (curr != prev) {
        prev = curr;
        for (int i = 0; i < curr; i++) Serial.print("=");
        Serial.print("\n");
        obj[TINTF_05A] = progress;
        CALL_INTF_OBJ(show_progress);
    }
    if (myLamp.getGaugeType()!=GAUGETYPE::GT_NONE){
        GAUGE::GaugeShow(len, total, 100);
    }
    return progress;
}
*/

/**
 * @brief build a page with LED Display setup
 * 
 */
void page_display_setup(Interface *interf, const JsonObject *data, const char* action){
    interf->json_frame_interface();
    interf->json_section_main(P_EMPTY, TINTF_display_setup);

    // determine which value we should set drop-down list to
    int select_val = data && (*data).containsKey(T_display_type) ? (*data)[T_display_type] : e2int(display.get_engine_type());

    interf->select(T_display_type, select_val, TINTF_display_type, true);
        interf->option(0, "ws2812b LED stripe");
        interf->option(1, "HUB75 RGB Panel");
    interf->json_section_end();

    interf->spacer();

    // if parameter for the specific page has been given
    if (data && (*data).containsKey(T_display_type)){
        if ((*data)[T_display_type] == e2int(engine_t::hub75))
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
    if (!embuifs::deserializeFile(doc, TCONST_fcfg_display) || !doc.containsKey( e == engine_t::hub75 ? T_hub75 : T_ws2812)) return;

    interf->json_frame_value(doc[e == engine_t::hub75 ? T_hub75 : T_ws2812]);
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
 * @brief rebuild cached json file with effects names list
 * i.e. used for sideloading in WebUI
 * @param full - rebuild full list or brief, excluding hidden effs
 * todo: implement an event queue
 */
void rebuild_effect_list_files(lstfile_t lst){
    if (delayedOptionTask)      // task is already running, skip
        return;
    // schedule a task to rebuild effects names list files
    delayedOptionTask = new Task(500, TASK_ONCE,
        [lst](){
            switch (lst){
                case lstfile_t::full :
                    build_eff_names_list_file(myLamp.effwrkr, true);
                    if (embui.feeders.available()){  // refresh UI page with a regenerated list
                        Interface interf(&embui.feeders);
                        show_effects_config(&interf, nullptr, NULL);
                    }
                    break;
                case lstfile_t::all :
                    build_eff_names_list_file(myLamp.effwrkr, true);
                    // intentionally fall-trough this to default
                    [[fallthrough]];
                default :
                    build_eff_names_list_file(myLamp.effwrkr);
                    if (embui.feeders.available()){  // refresh UI page with a regenerated list
                        Interface interf(&embui.feeders);
                        ui_page_main(&interf, nullptr, NULL);
                    }
            }
        },
        &ts, true, nullptr, [](){ delayedOptionTask=nullptr; }, true
    );
}


/**
 * Набор конфигурационных переменных и callback-обработчиков EmbUI
 */
void embui_actions_register(){
    // создаем конфигурационные параметры и регистрируем обработчики активностей

    embui.var_create(V_micScale, 1.28);

    embui.var_create(T_effSpeedFactor, 1.0);

    embui.var_create(T_mp3vol, 25); // громкость
    //embui.var_create(TCONST_mp3count, 255); // кол-во файлов в папке mp3 (установка убрана, используется значение по-умолчанию равное максимальному числу эффектов)

    // регистрируем обработчики активностей
    embui.action.set_mainpage_cb(ui_page_main);                             // index page callback
    embui.action.set_settings_cb(user_settings_frame);                      // "settings" page options callback

    embui.action.add(A_ui_page, ui_page_selector);                          // ui page switcher, same as in basicui::
    embui.action.add(A_ui_page_effects, ui_page_effects);                   // меню: переход на страницу "Эффекты"
    embui.action.add(A_ui_page_drawing, ui_page_drawing);                   // меню: переход на страницу "Рисование"
    embui.action.add(A_ui_page_modules, ui_page_modules);                   // меню: переход на страницу "Модули"
    embui.action.add(A_ui_block_switches, ui_block_mainpage_switches);      // нажатие кнопки "еще..." на странице "Эффекты"

    // device controls
    embui.action.add(A_dev_pwrswitch, set_pwrswitch);                       // lamp's powerswitch action
    embui.action.add(A_dev_brightness, getset_brightness);                  // Lamp brightness
    embui.action.add(A_dev_lcurve, set_lcurve);                             // luma curve control

    // Effects control
    embui.action.add(A_effect_switch, effect_switch);                       // effect switcher action
    embui.action.add(A_effect_ctrls, publish_effect_controls);              // сформировать и опубликовать блок контролов текущего эффекта
    embui.action.add(A_effect_dynCtrl, set_effects_dynCtrl);                // Effect controls handler

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

    embui.action.add(TCONST_effListConf, set_effects_config_list);
    embui.action.add(TCONST_set_effect, set_effects_config_param);
    embui.action.add(K_demo, set_demoflag);

    embui.action.add(TCONST_AUX, set_auxflag);

    // disable old overlay buffer
    //embui.action.add(TCONST_draw_dat, set_drawing);
    //embui.action.add(TCONST_drawbuff, set_overlay_drawing);

    embui.action.add(T_display_type, page_display_setup);                // load display setup page depending on selected disp type (action for drop down list)

    embui.action.add(TCONST_set_mic, set_settings_mic);
    embui.action.add(A_dev_mike, set_micflag);

}
