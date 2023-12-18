#pragma once
#include "ui.h"

#define RESCHEDULE_DELAY    50         // async callback delay
// размер документа для конфигурации дисплеев
#define DISPLAY_JSIZE   1500


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
void getset_brightness(Interface *interf, const JsonObject *data, const char* action);

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

/**
 * @brief Set/get tm1637 configuration values
 * 
 */
void getset_tm1637(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief show page with buttons leading to confgigiration of additional devices attached
 * i.e. tm1637, mp3player, etc...
 * 
 * @param interf 
 * @param data 
 * @param action 
 */
void ui_page_setup_devices(Interface *interf, const JsonObject *data, const char* action);

/**
 * @brief a call-back handler that listens for status change events and publish it to EmbUI feeders
 * i.e. it's used to propagade IPC events into WebSocket/MQTT updates
 * 
 * @param handler_args 
 * @param base 
 * @param id 
 * @param event_data 
 */
void event_publisher(void* handler_args, esp_event_base_t base, int32_t id, void* event_data);


// ==========
#ifdef ESP_USE_BUTTON
void default_buttons();

/**
 * @brief подгрузить конфигурацию кнопки из стороннего файла
 * path should be relative to TCONST__backup_btn_
 */
void load_button_config(const char* path = NULL);
#endif


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
