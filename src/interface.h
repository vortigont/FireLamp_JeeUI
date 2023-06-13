#pragma once
#include "EmbUI.h"
#include "log.h"

typedef enum _remote_action {
    RA_UNKNOWN,
//    RA_ON,
//    RA_OFF,
//    RA_DEMO,
//    RA_DEMO_NEXT,       // trigger effect change in Demo mode
//    RA_ALARM,
//    RA_ALARM_OFF,
//    RA_LAMP_CONFIG,     // load another config for embui
//    RA_EFF_CONFIG,
#ifdef ESP_USE_BUTTON
    RA_BUTTONS_CONFIG,
#endif
    RA_EVENTS_CONFIG,
/*
#ifdef AUX_PIN
    RA_AUX_ON,
    RA_AUX_OFF,
    RA_AUX_TOGLE,
#endif
*/
//    RA_REBOOT,
//    RA_EFF_NEXT,
//    RA_EFF_PREV,
//    RA_EFF_RAND,
    RA_BRIGHT_NF,
    RA_CONTROL,
//#ifdef MP3PLAYER
//    RA_MP3_PREV,
//    RA_MP3_NEXT,
//    RA_MP3_SOUND,
//    RA_PLAYERONOFF,
//    RA_MP3_VOL,
//#endif
#ifdef MIC_EFFECTS
//    RA_MIC,
//    RA_MICONOFF,
#endif
//    RA_EFFECT,          // called on effect change events
    RA_SEND_TEXT,
    RA_SEND_TIME,
    RA_SEND_IP,
    RA_WHITE_HI,
    RA_WHITE_LO,
    RA_WIFI_REC,
    RA_GLOBAL_BRIGHT,
    RA_BRIGHT_PCT,      // get/set brightness in percents
//    RA_WARNING,
//    RA_DRAW,
//    RA_FILLMATRIX,
//    RA_RGB
} RA;

void remote_action(RA action, ...);

String httpCallback(const String &param, const String &value, bool isset);
#ifdef ESP_USE_BUTTON
void default_buttons();
#endif

// ---------------------

void section_main_frame(Interface *interf, JsonObject *data);
void section_effects_frame(Interface *interf, JsonObject *data);
void section_text_frame(Interface *interf, JsonObject *data);
// реализация настроек тут своя, отличная от фреймворка
void section_settings_frame(Interface *interf, JsonObject *data);
void pubCallback(Interface *interf);
void set_onflag(Interface *interf, JsonObject *data);
void save_lamp_flags();

uint8_t uploadProgress(size_t len, size_t total);

void delayedcall_show_effects();
void show_effects_config_param(Interface *interf, JsonObject *data);

/**
 * блок формирования страницы с контролами для настроек параметров эффектов
 * здесь выводится ПОЛНЫЙ сипсок эффектов
 */
void block_effects_config(Interface *interf, JsonObject *data);

#ifdef MIC_EFFECTS
void show_settings_mic(Interface *interf, JsonObject *data);
#endif
