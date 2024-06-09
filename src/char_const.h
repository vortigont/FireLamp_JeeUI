#pragma once

/** набор служебных текстовых констант (не для локализации)
 */

// LOG tags
static constexpr const char* T_Effect = "Effect";
static constexpr const char* T_EffCfg = "EffCfg";
static constexpr const char* T_EffWrkr = "EffWrkr";
static constexpr const char* T_Fade = "Fade";
static constexpr const char* T_Widget = "Widget";
static constexpr const char* T_WebUI = "WebUI";
static constexpr const char* T_WdgtMGR = "WdgtMGR";

// NVS ns
static constexpr const char* T_lamp = "lamp";

static constexpr const char* T_bright = "bright";
static constexpr const char* T_brt_scale = "brt_scale";
static constexpr const char* T_DemoTime = "DemoTime";
static constexpr const char* T_drawing = "drawing";
static constexpr const char* T_effect_dynCtrl = "eff_dynCtrl";
static constexpr const char* T_encoder = "encoder";
static constexpr const char* T_enctype = "enctype";
static constexpr const char* T_gpio = "gpio";                                   // gpio key for display configuration
static constexpr const char* T_logicL= "logicL";                                // logic level for button
static constexpr const char* T_pull = "pull";


// Display
static constexpr const char* TCONST_fcfg_display = "/display.json";
static constexpr const char* T_Display = "Display";                             // LOG macro
static constexpr const char* T_display_type = "dtype";                          // LED Display engine type

// ws2812 config var names
static constexpr const char* T_ws2812 = "ws2812";                               // ws2812 led stip type
static constexpr const char* T_mx_gpio = "mx_gpio";
static constexpr const char* T_CLmt = "CLmt";                                   // лимит тока
static constexpr const char* T_col_order = "col_order";                         // RGB color order
static constexpr const char* T_hcnt = "hcnt";
static constexpr const char* T_height = "height";
static constexpr const char* T_hflip = "hflip";
static constexpr const char* T_snake = "snake";
static constexpr const char* T_thflip = "thflip";
static constexpr const char* T_tsnake = "tsnake";
static constexpr const char* T_tvertical = "tvertical";
static constexpr const char* T_tvflip = "tvflip";
static constexpr const char* T_vertical = "vertical";
static constexpr const char* T_vflip = "vflip";
static constexpr const char* T_wcnt = "wcnt";
static constexpr const char* T_width = "width";

// HUB75 related
static constexpr const char* T_hub75 = "hub75";                                 // HUB75 panel
static constexpr const char* T_R1 = "R1";
static constexpr const char* T_R2 = "R2";
static constexpr const char* T_G1 = "G1";
static constexpr const char* T_G2 = "G2";
static constexpr const char* T_B1 = "B1";
static constexpr const char* T_B2 = "B2";
static constexpr const char* T_A = "A";
static constexpr const char* T_B = "B";
static constexpr const char* T_C = "C";
static constexpr const char* T_D = "D";
static constexpr const char* T_E = "E";
static constexpr const char* T_CLK = "CLK";
static constexpr const char* T_OE = "OE";
static constexpr const char* T_LAT = "LAT";
static constexpr const char* T_shift_drv = "driver";
static constexpr const char* T_clk_rate = "clkrate";
static constexpr const char* T_lat_blank = "latblank";
static constexpr const char* T_clk_phase = "clkphase";
static constexpr const char* T_min_refresh = "minrr";
static constexpr const char* T_color_depth = "colordpth";

// TM1637 Display
static constexpr const char* T_On = "-On-";
static constexpr const char* T_Off = "Off";
static constexpr const char* T_tm1637 = "tm1637";
static constexpr const char* T_tm_12h = "clk12h";
static constexpr const char* T_tm_lzero = "lzero";
static constexpr const char* T_tm_brt_on = "brtOn";
static constexpr const char* T_tm_brt_off = "brtOff";

// Button events
static constexpr const char* T_benc_cfg = "/benc.json";
static constexpr const char* T_btn_cfg = "btn_cfg";
static constexpr const char* T_btn_event = "btn_event";
static constexpr const char* T_btn_events = "btn_events";
static constexpr const char* T_debounce = "debounce";
static constexpr const char* T_lamp_event = "lamp_event";
static constexpr const char* T_lamppwr = "lamppwr";
static constexpr const char* T_lpresst = "lpresst";
static constexpr const char* T_mclickt = "mclickt";

// DFPlayer
static constexpr const char* T_DFPlayer = "DFPlayer";
static constexpr const char* T_dfplayer_cfg = "/dfplayer.json";
static constexpr const char* T_mp3vol = "mp3vol";
static constexpr const char* T_eff_tracks = "eff_tracks";
static constexpr const char* T_eff_tracks_loop = "eff_tracks_loop";
static constexpr const char* T_mp3mute = "mp3mute";


// Widgets
static constexpr const char* T_widgets_cfg = "/widgets.json";
static constexpr const char* T_clock = "clock";                     // LABEL Clock widget
static constexpr const char* T_alrmclock = "alrmclock";             // LABEL Alarm Clock / Cockoo
static constexpr const char* T_widgets = "widgets";                 // NVS namespace

static constexpr const char* T_color1 = "color1";
static constexpr const char* T_color2 = "color2";
static constexpr const char* T_color3 = "color3";
static constexpr const char* T_color4 = "color4";

static constexpr const char* T_alpha_t = "alpha_t";                 // alpha transparency
static constexpr const char* T_alpha_b = "alpha_b";
static constexpr const char* T_alpha_b2 = "alpha_b2";
// static constexpr const char* T_color3 = "color3";
static constexpr const char* T_font1 = "font1";
static constexpr const char* T_font2 = "font2";
static constexpr const char* T_font3 = "font3";
static constexpr const char* T_x1offset = "x1off";
static constexpr const char* T_y1offset = "y1off";
static constexpr const char* T_x2offset = "x2off";
static constexpr const char* T_y2offset = "y2off";
// Widget Alarm clock
static constexpr const char* T_seconds = "seconds";
static constexpr const char* T_hr = "hr";                           // alarm hour
static constexpr const char* T_cockoo_hr = "cockoo_hr";             // hourly cockoo
static constexpr const char* T_cockoo_hhr = "cockoo_hhr";           // half hour cockoo
static constexpr const char* T_quarter = "quarter";                 // quarter hour cockoo
static constexpr const char* T_snd = "snd";                         // track number to play as alarm melody
static constexpr const char* T_riseOn = "riseOn";
static constexpr const char* T_startBr = "startBr";
static constexpr const char* T_endBr = "endBr";
static constexpr const char* T_offset = "offset";
static constexpr const char* T_duration = "duration";
static constexpr const char* T_duskOn = "duskOn";
static constexpr const char* T_duskPrwOff = "duskPrwOff";
static constexpr const char* T_duskStartBr = "duskStartBr";
static constexpr const char* T_duskEndBr = "duskEndBr";
static constexpr const char* T_duskDuration = "duskDuration";
static constexpr const char* T_duskEff = "duskEff";

//static constexpr const char* T_quarter = "alarms";
//static constexpr const char* TCONST_Alarm = "Alarm";

// Events
static constexpr const char* T_event = "event";


// Other
static constexpr const char* T_Active = "Active";
static constexpr const char* T_active = "active";
static constexpr const char* T_arg = "arg";
static constexpr const char* T_clicks = "clicks";
static constexpr const char* T_ctrls = "ctrls";
static constexpr const char* T_device = "device";
static constexpr const char* T_edit = "edit";
static constexpr const char* T_Enable = "Enable";
static constexpr const char* T_enabled = "enabled";
static constexpr const char* T_id = "id";
static constexpr const char* T_idx = "idx";
static constexpr const char* T_invert = "invert";
static constexpr const char* T_Main = "Main";                           // used as LOG tag
static constexpr const char* T_max = "max";
static constexpr const char* T_mic = "mic";                             // microphone key (gpio)
static constexpr const char* T_min = "min";
static constexpr const char* T_name = "name";
static constexpr const char* T_Notification = "Notification";
static constexpr const char* T_on = "on";
static constexpr const char* T_off = "off";
static constexpr const char* T_opt = "opt";
static constexpr const char* T_pwr = "pwr";
static constexpr const char* T_rx = "rx";
static constexpr const char* T_step = "step";
static constexpr const char* T_tx = "tx";
static constexpr const char* T_timeout = "timeout";
static constexpr const char* T_type = "type";
static constexpr const char* T_val = "val";


static constexpr const char* TCONST_act = "act";
static constexpr const char* TCONST_afS = "afS";
static constexpr const char* TCONST_AUX = "AUX";
static constexpr const char* TCONST_aux_gpio = "aux_gpio";             // AUX gpio
static constexpr const char* TCONST_aux_ll = "aux_ll";                 // AUX logic level
static constexpr const char* TCONST_bactList = "bactList";
static constexpr const char* TCONST_blabel = "blabel";
static constexpr const char* TCONST_bparam = "bparam";
static constexpr const char* TCONST_Btn = "Btn";
static constexpr const char* TCONST_buttList = "buttList";
static constexpr const char* TCONST_butt_conf = "butt_conf";
static constexpr const char* TCONST_control = "control";
static constexpr const char* TCONST_copy = "copy";
static constexpr const char* TCONST_debug = "debug";
static constexpr const char* TCONST_delall = "delall";
static constexpr const char* TCONST_delCfg = "delCfg";
static constexpr const char* TCONST_delete = "delete";
static constexpr const char* TCONST_delfromlist = "delfromlist";
static constexpr const char* TCONST_del_ = "del*";
static constexpr const char* TCONST_direct = "direct";
static constexpr const char* TCONST_DRand = "DRand";
static constexpr const char* TCONST_drawbuff = "drawbuff";
static constexpr const char* TCONST_drawClear = "drawClear";
static constexpr const char* TCONST_draw_dat = "draw_dat";
static constexpr const char* TCONST_ds18b20 = "ds18b20";
//static constexpr const char* TCONST_edit_lamp_config = "edit_lamp_config";
static constexpr const char* TCONST_edit_text_config = "edit_text_config";
static constexpr const char* TCONST_eff_config = "eff_config";
static constexpr const char* TCONST_eff_fav = "eff_fav";
static constexpr const char* TCONST_eff_fulllist_json = "/eff_fulllist.json"; // a json serialized full list of effects and it's names for WebUI drop-down list on effects management page
static constexpr const char* TCONST_eff_index = "/eff_index.json";
static constexpr const char* TCONST_eff_list_json_tmp = "/eff_list.json.tmp"; // a json serialized list of effects and it's names for WebUI drop-down list
static constexpr const char* TCONST_eff_list_json = "/eff_list.json";         // a json serialized list of effects and it's names for WebUI drop-down list on main page
static constexpr const char* TCONST_eff_sel = "eff_sel";
static constexpr const char* TCONST_effHasMic = "effHasMic";
static constexpr const char* TCONST_effListConf = "effListConf";
static constexpr const char* TCONST_effname = "effname";
static constexpr const char* TCONST_eqSetings = "eqSetings";
static constexpr const char* TCONST_eventList = "eventList";
static constexpr const char* TCONST_Events = "Events";
static constexpr const char* TCONST_event_conf = "event_conf";
static constexpr const char* TCONST_evList = "evList";
static constexpr const char* TCONST_f_restore_state = "f_rstt";          // Lamp flag "restore state"
static constexpr const char* TCONST_fcfg_gpio = "/gpio.json";
static constexpr const char* TCONST_fileName2 = "fileName2";
static constexpr const char* TCONST_fileName = "fileName";
static constexpr const char* TCONST_fill = "fill";
static constexpr const char* TCONST_force = "force";
static constexpr const char* TCONST_hold = "hold";
static constexpr const char* TCONST_Host = "Host";
static constexpr const char* TCONST_Ip = "Ip";
static constexpr const char* TCONST_isClearing = "isClearing";
static constexpr const char* TCONST_isFaderON = "isFaderON";
static constexpr const char* TCONST_isPlayTime = "isPlayTime";
static constexpr const char* TCONST_isShowOff = "isShowOff";
static constexpr const char* TCONST_isStreamOn = "isStreamOn";
static constexpr const char* TCONST_lamptext = "lamptext";
static constexpr const char* TCONST_lamp_config = "lamp_config";
static constexpr const char* TCONST_limitAlarmVolume = "limitAlarmVolume";
static constexpr const char* TCONST_load = "load";
//static constexpr const char* TCONST_Mac = "Mac";
static constexpr const char* TCONST_makeidx = "makeidx";
static constexpr const char* TCONST_mapping = "mapping";
static constexpr const char* TCONST_Memory = "Memory";
static constexpr const char* TCONST_mic_cal = "mic_cal";
static constexpr const char* TCONST_MIRR_H = "MIRR_H";
static constexpr const char* TCONST_MIRR_V = "MIRR_V";
static constexpr const char* TCONST_Mode = "Mode";
static constexpr const char* TCONST_mode = "mode";
static constexpr const char* TCONST_mosfet_gpio = "fet_gpio";             // MOSFET gpio
static constexpr const char* TCONST_mosfet_ll = "fet_ll";                // MOSFET logic level
static constexpr const char* TCONST_msg = "msg";
static constexpr const char* TCONST_nofade = "nofade";
static constexpr const char* TCONST_Normal = "Normal";
static constexpr const char* TCONST_numInList = "numInList";
static constexpr const char* TCONST_onetime = "onetime";
static constexpr const char* TCONST_Other = "Other";
static constexpr const char* TCONST_pFS = "pFS";
static constexpr const char* TCONST_PINB = "PINB"; // пин кнопки
static constexpr const char* TCONST_pin = "pin";
static constexpr const char* TCONST_RGB = "RGB";
static constexpr const char* TCONST_settings_mic = "settings_mic";
static constexpr const char* TCONST_settings_mp3 = "settings_mp3";
static constexpr const char* TCONST_settings_wifi = "settings_wifi";
static constexpr const char* TCONST_showName = "showName";
static constexpr const char* TCONST_show_button = "show_button";
static constexpr const char* TCONST_show_butt = "show_butt";
static constexpr const char* TCONST_show_event = "show_event";
static constexpr const char* TCONST_spdcf = "spdcf";
static constexpr const char* TCONST_speed = "speed";
static constexpr const char* TCONST_state = "state";
static constexpr const char* TCONST_STA = "STA";
static constexpr const char* TCONST_stopat = "stopat";
static constexpr const char* TCONST_streaming = "streaming";
static constexpr const char* TCONST_stream_type = "stream_type";
static constexpr const char* TCONST_sT = "sT";
static constexpr const char* TCONST_sysSettings = "sysSettings";
static constexpr const char* TCONST_textsend = "textsend";
static constexpr const char* TCONST_text_config = "text_config";
static constexpr const char* TCONST_Time = "Time";
static constexpr const char* TCONST_txtBfade = "txtBfade";
static constexpr const char* TCONST_txtColor = "txtColor";
static constexpr const char* TCONST_txtOf = "txtOf";
static constexpr const char* TCONST_txtSpeed = "txtSpeed";
static constexpr const char* TCONST_Universe = "Universe";
static constexpr const char* TCONST_Uptime = "Uptime";
static constexpr const char* TCONST_value = "value";
static constexpr const char* TCONST_Version = "Version";
static constexpr const char* TCONST_White = "White";
static constexpr const char* TCONST__5f9ea0 = "#5f9ea0";
static constexpr const char* TCONST__708090 = "#708090";
static constexpr const char* TCONST__backup_btn_ = "/backup/btn/";
static constexpr const char* TCONST__backup_evn_ = "/backup/evn/";
static constexpr const char* TCONST__backup_glb_ = "/backup/glb/";
static constexpr const char* TCONST__backup_idx = "/backup/idx";
static constexpr const char* TCONST__backup_idx_ = "/backup/idx/";
static constexpr const char* TCONST__ffffff = "#ffffff";
static constexpr const char* TCONST__tmplist_tmp = "/tmplist.tmp";
static constexpr const char* TCONST__tmpqlist_tmp = "/tmpqlist.tmp";
// текст элементов интерфейса
//static constexpr const char* I_zmeika = "змейка";
//static constexpr const char* I_vert = "вертикальная";
//static constexpr const char* I_vflip = "зеркальные столбцы";
//static constexpr const char* I_hflip = "зеркальные строки";

/* ***  conf variable names *** */

// Lamp class flags packed into uint32_t
static constexpr const char* V_lampFlags = "lampFlags";
// brightness scale value
static constexpr const char* V_dev_brtscale = "dev_brtscale";
// Saved last running effect index
static constexpr const char* V_effect_idx = "eff_idx";
// Effect list sorting
static constexpr const char* V_effSort = "effSort";
// Microphone scale level
static constexpr const char* V_micScale = "micScale";
// Microphone noise level
static constexpr const char* V_micNoise = "micNoise";
// Microphone Rdc level
static constexpr const char* V_micRdcLvl = "micnRdcLvl";



// имена ключей конфигурации / акшены
static constexpr const char* K_demo = "demo";
static constexpr const char* A_dev_brightness = "dev_brightness";               // Scaled brightness value based on scale
static constexpr const char* A_dev_lcurve = "dev_lcurve";                       // Luma curve
static constexpr const char* A_dev_pwrswitch = "dev_pwrswitch";                 // Lamp on/off switch
static constexpr const char* A_dev_btnlock = "dev_btnlock";                     // Button lock
static constexpr const char* A_dev_mike = "dev_mike";                           // Enable/disable microphone
static constexpr const char* A_ui_page_effects = "ui_page_effects";             // Contstruct UI page - main Effects Control
static constexpr const char* A_ui_page_drawing = "ui_page_drawing";             // Contstruct UI page - drawing panel
static constexpr const char* A_ui_block_switches = "ui_block_switches";         // Contstruct UI block - show extended switches at effects page
static constexpr const char* A_ui_page_setupdevs = "ui_page_setupdevs";         // periferal devices setup menu page
static constexpr const char* A_ui_page_widgets = "ui_page_widgets";             // Widgets page
static constexpr const char* A_effect_switch = "eff_sw_*";                      // Wildcard Effect switcher action
static constexpr const char* A_effect_switch_idx = "eff_sw_idx";                // Switch Effect to specified index
static constexpr const char* A_effect_switch_next = "eff_sw_next";              // Switch to next effect
static constexpr const char* A_effect_switch_prev = "eff_sw_prev";              // Switch to previous effect
static constexpr const char* A_effect_switch_rnd = "eff_sw_rnd";                // Switch to random effect
static constexpr const char* A_effect_ctrls = "eff_ctrls";                      // Generate and publish Effect controls (also it is an mqtt suffix for controls publish)
static constexpr const char* A_effect_dynCtrl = "eff_dynCtrl*";                 // Effect controls handler
static constexpr const char* A_display_hub75 = "display_hub75";                 // HUB75 display configuration
static constexpr const char* A_display_ws2812 = "display_ws2812";               // ws2812 display configuration
// TM1637
static constexpr const char* A_display_tm1637 = "*et_display_tm1637";           // get/set tm1637 display configuration
// button / encoder
static constexpr const char* A_button_gpio = "*et_button_gpio";                 // get/set button gpio configuration
static constexpr const char* A_button_evt_edit = "button_evt_edit";             // edit button action form
static constexpr const char* A_button_evt_save = "button_evt_save";             // save/apply button action form
static constexpr const char* A_encoder_gpio = "*et_encoder_gpio";               // get/set encoder gpio configuration
// DFPlayer
static constexpr const char* A_dfplayer_dev = "*et_dfplayer_device";            // get/set DFPlayer device configuration
static constexpr const char* A_dfplayer_opt = "*et_dfplayer_opt";               // get/set DFPlayer functions
// Widgets
static constexpr const char* A_set_widget = "set_wdgt_*";                       // set Widget's configuration
static constexpr const char* A_set_widget_onoff = "set_wdgtena_*";              // enable/disable Widget by name
static constexpr const char* A_set_wcfg_alrm = "set_wcfg_alrm";                 // set alarm widget item configuration

static constexpr const char* A_set_gpio = "*et_gpio";                           // ge/set gpios action


static constexpr const char* A_ui_page_effects_config = "effects_config";

static constexpr const char* TCONST_set_butt = "set_butt";                  // set button
static constexpr const char* TCONST_set_effect = "set_effect";
static constexpr const char* TCONST_set_enc = "set_enc";
static constexpr const char* TCONST_set_event = "set_event";
static constexpr const char* TCONST_set_mic = "set_mic";
static constexpr const char* TCONST_set_mp3 = "set_mp3";
static constexpr const char* TCONST_set_other = "set_other";

// прочие именованные секции
static constexpr const char* T_switches = "switches";


/** набор служебных текстовых констант (HTTP/MQTT запросы) */
static constexpr const char* MQT_effect_controls = "effect/controls/";      // topic suffix
static constexpr const char* MQT_lamp = "lamp/";

//static constexpr const char* CMD_MP3_PREV = "MP3_PREV";        // Без параметров - переключает трек на 1 назад, принимает числовой параметр, на сколько треков вернуть назад
//static constexpr const char* CMD_MP3_NEXT = "MP3_NEXT";        // Без параметров - переключает трек на 1 назад, принимает числовой параметр, на сколько треков вернуть назад
