#pragma once
#include "EmbUI.h"
#include "log.h"

#define RESCHEDULE_DELAY    50         // async callback delay

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
//#ifdef ESP_USE_BUTTON
//    RA_BUTTONS_CONFIG,
//#endif
//    RA_EVENTS_CONFIG,
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
//    RA_BRIGHT_NF,
//    RA_CONTROL,
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
//    RA_SEND_TEXT,
//    RA_SEND_TIME,
    RA_SEND_IP,
    RA_WHITE_HI,
    RA_WHITE_LO,
    RA_WIFI_REC,
//    RA_GLOBAL_BRIGHT,
//    RA_BRIGHT_PCT,      // get/set brightness in percents
//    RA_WARNING,
//    RA_DRAW,
//    RA_FILLMATRIX,
//    RA_RGB
} RA;

/**
 * @brief обработчик, для поддержки приложения WLED APP
 * https://play.google.com/store/apps/details?id=com.aircoookie.WLED
 * обслуживает '/win'
 * https://kno.wled.ge/interfaces/http-api/
 * @param request AsyncWebServerRequest
 */
void wled_handle(AsyncWebServerRequest *request);

void ws_action_handle(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

//String httpCallback(const String &param, const String &value, bool isset);

// ---------------------

// register embui action handlers and parameters
void embui_actions_register();

// сброс таймера демо и автосохранение контролов эффекта
void resetAutoTimers(bool isEffects);

/**
 * @brief Lamp's PowerSwitch
 * handles powerswitch event
 */
void set_pwrswitch(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief Set device display brightness
 * 
 */
void set_brightness(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief Set luma curve brightness adjustment value
 * 
 */
void set_lcurve(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief Switch to specific effect
 * could be triggered via WebUI's selector list or via ra::eff_switch
 * if switched successfully, than this function calls contorls publishing via MQTT
 */
void effect_switch(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief this function is a wrapper for block_effect_controls() to publish current effect controls to various feeders
 * it either can use a provided Interface object, or it will create a new one if called
 * from an internal fuctions not a post callbacks
 * 
 */
void publish_effect_controls(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief effects controls handler
 * it picks "dynCtrl*" actions and adjusts current effect parameters
 * 
 */
void set_effects_dynCtrl(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief Set/get ledstrip configuration values
 * 
 */
void set_ledstrip(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief Set/get HUB75 configuration values
 * 
 */
void set_hub75(Interface *interf, const JsonObject *data, const char* action);


// ==========
#ifdef ESP_USE_BUTTON
void default_buttons();

/**
 * @brief подгрузить конфигурацию кнопки из стороннего файла
 * path should be relative to TCONST__backup_btn_
 */
void load_button_config(const char* path = NULL);
#endif


// устаревшая дергалка активностей
void remote_action(RA action, ...);

void section_effects_frame(Interface *interf, const JsonObject *data, const char* action);
void section_text_frame(Interface *interf, const JsonObject *data, const char* action);
// реализация настроек тут своя, отличная от фреймворка
void section_settings_frame(Interface *interf, const JsonObject *data, const char* action);
void pubCallback(Interface *interf);
//void save_lamp_flags();

// disabled as not handled by external lib
//uint8_t uploadProgress(size_t len, size_t total);

void show_effects_config_param(Interface *interf, const JsonObject *data, const char* action);

/**
 * блок формирования страницы с контролами для настроек параметров эффектов
 * здесь выводится ПОЛНЫЙ сипсок эффектов
 */
void block_effects_config(Interface *interf, const JsonObject *data, const char* action);

#ifdef MIC_EFFECTS
void show_settings_mic(Interface *interf, const JsonObject *data, const char* action);
#endif

/**
 * @brief load events configuarion from file
 * 
 * @param path 
 */
void load_events_config(const char* path = NULL);
