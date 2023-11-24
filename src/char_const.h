#pragma once

/** набор служебных текстовых констант (не для локализации)
 */
static constexpr const char* T_display_type = "dtype";                          // LED Display engine type
static constexpr const char* T_drawing = "drawing";
static constexpr const char* T_effect_dynCtrl = "eff_dynCtrl";
static constexpr const char* T_gpio = "gpio";                                   // gpio key for display configuration
static constexpr const char* T_ws2812 = "ws2812";                               // ws2812 led stip type

// ws2812 config var names
static constexpr const char* T_mx_gpio = "mx_gpio";
static constexpr const char* T_CLmt = "CLmt";                                   // лимит тока
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



static constexpr const char* TCONST_act = "act";
static constexpr const char* TCONST_afS = "afS";
static constexpr const char* TCONST_alarmPT = "alarmPT";
static constexpr const char* TCONST_alarmP = "alarmP";
static constexpr const char* TCONST_alarmSound = "alarmSound";
static constexpr const char* TCONST_alarmT = "alarmT";
static constexpr const char* TCONST_Alarm = "Alarm";
static constexpr const char* TCONST_AUX = "AUX";
static constexpr const char* TCONST_aux_gpio = "aux_gpio";             // AUX gpio
static constexpr const char* TCONST_aux_ll = "aux_ll";                 // AUX logic level
static constexpr const char* TCONST_bactList = "bactList";
static constexpr const char* TCONST_blabel = "blabel";
static constexpr const char* TCONST_bparam = "bparam";
static constexpr const char* TCONST_bright = "bright";
static constexpr const char* TCONST_Btn = "Btn";
static constexpr const char* TCONST_buttList = "buttList";
static constexpr const char* TCONST_butt_conf = "butt_conf";
static constexpr const char* TCONST_clicks = "clicks";
static constexpr const char* TCONST_control = "control";
static constexpr const char* TCONST_copy = "copy";
static constexpr const char* TCONST_d1 = "d1";
static constexpr const char* TCONST_d2 = "d2";
static constexpr const char* TCONST_d3 = "d3";
static constexpr const char* TCONST_d4 = "d4";
static constexpr const char* TCONST_d5 = "d5";
static constexpr const char* TCONST_d6 = "d6";
static constexpr const char* TCONST_d7 = "d7";
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
static constexpr const char* TCONST_DTimer = "DTimer";
//static constexpr const char* TCONST_edit_lamp_config = "edit_lamp_config";
static constexpr const char* TCONST_edit_text_config = "edit_text_config";
static constexpr const char* TCONST_edit = "edit";
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
static constexpr const char* TCONST_enabled = "enabled";
static constexpr const char* TCONST_encoder = "encoder";
static constexpr const char* TCONST_encTxtCol = "encTxtCol";
static constexpr const char* TCONST_encTxtDel = "encTxtDel";
static constexpr const char* TCONST_EncVGCol = "EncVGCol";
static constexpr const char* TCONST_EncVG = "EncVG";
static constexpr const char* TCONST_eqSetings = "eqSetings";
static constexpr const char* TCONST_event = "event";
static constexpr const char* TCONST_eventList = "eventList";
static constexpr const char* TCONST_Events = "Events";
static constexpr const char* TCONST_event_conf = "event_conf";
static constexpr const char* TCONST_evList = "evList";
static constexpr const char* TCONST_f_restore_state = "f_rstt";          // Lamp flag "restore state"
static constexpr const char* TCONST_fcfg_gpio = "/gpio.json";
static constexpr const char* TCONST_fcfg_display = "/display.json";
static constexpr const char* TCONST_fileName2 = "fileName2";
static constexpr const char* TCONST_fileName = "fileName";
static constexpr const char* TCONST_fill = "fill";
static constexpr const char* TCONST_force = "force";
static constexpr const char* TCONST_hold = "hold";
static constexpr const char* TCONST_Host = "Host";
static constexpr const char* TCONST_Ip = "Ip";
static constexpr const char* TCONST_isClearing = "isClearing";
static constexpr const char* TCONST_isFaderON = "isFaderON";
static constexpr const char* TCONST_isOnMP3 = "isOnMP3";
static constexpr const char* TCONST_isPlayTime = "isPlayTime";
static constexpr const char* TCONST_isShowOff = "isShowOff";
static constexpr const char* TCONST_isStreamOn = "isStreamOn";
static constexpr const char* TCONST_lamptext = "lamptext";
static constexpr const char* TCONST_lamp_config = "lamp_config";
static constexpr const char* TCONST_limitAlarmVolume = "limitAlarmVolume";
static constexpr const char* TCONST_load = "load";
static constexpr const char* TCONST_lV = "lV";
static constexpr const char* TCONST_Mac = "Mac";
//static constexpr const char* TCONST_main = "main";
static constexpr const char* TCONST_makeidx = "makeidx";
static constexpr const char* TCONST_mapping = "mapping";
static constexpr const char* TCONST_Memory = "Memory";
static constexpr const char* TCONST_Mic = "Mic";
static constexpr const char* TCONST_mic_cal = "mic_cal";
static constexpr const char* TCONST_MIRR_H = "MIRR_H";
static constexpr const char* TCONST_MIRR_V = "MIRR_V";
static constexpr const char* TCONST_Mode = "Mode";
static constexpr const char* TCONST_mode = "mode";
static constexpr const char* TCONST_mosfet_gpio = "fet_gpio";             // MOSFET gpio
static constexpr const char* TCONST_mosfet_ll = "fet_ll";                // MOSFET logic level
static constexpr const char* TCONST_mp3count = "mp3count";
static constexpr const char* TCONST_mp3volume = "mp3volume";
static constexpr const char* TCONST_mp3rx = "mp3rx";
static constexpr const char* TCONST_mp3tx = "mp3tx";
static constexpr const char* TCONST_mp3_n5 = "mp3_n5";
static constexpr const char* TCONST_mp3_p5 = "mp3_p5";
static constexpr const char* TCONST_msg = "msg";
static constexpr const char* TCONST_nofade = "nofade";
static constexpr const char* TCONST_Normal = "Normal";
static constexpr const char* TCONST_numInList = "numInList";
static constexpr const char* TCONST_ny_period = "ny_period";
static constexpr const char* TCONST_ny_unix = "ny_unix";
static constexpr const char* TCONST_onetime = "onetime";
static constexpr const char* TCONST_on = "on";
static constexpr const char* TCONST_opt_pass = "opt_pass";
static constexpr const char* TCONST_OTA = "OTA";
static constexpr const char* TCONST_Other = "Other";
static constexpr const char* TCONST_pFS = "pFS";
static constexpr const char* TCONST_PINB = "PINB"; // пин кнопки
static constexpr const char* TCONST_pin = "pin";
static constexpr const char* TCONST_playEffect = "playEffect";
static constexpr const char* TCONST_playMP3 = "playMP3";
static constexpr const char* TCONST_playName = "playName";
static constexpr const char* TCONST_playTime = "playTime";
static constexpr const char* TCONST_pMem = "pMem";
static constexpr const char* TCONST_pRSSI = "pRSSI";
static constexpr const char* TCONST_pTemp = "pTemp";
static constexpr const char* TCONST_pTime = "pTime";
static constexpr const char* TCONST_pUptime = "pUptime";
static constexpr const char* TCONST_repeat = "repeat";
static constexpr const char* TCONST_RGB = "RGB";
static constexpr const char* TCONST_RSSI = "RSSI";
static constexpr const char* TCONST_save = "save";
static constexpr const char* TCONST_scale = "scale";
static constexpr const char* TCONST_settings_mic = "settings_mic";
static constexpr const char* TCONST_settings_mp3 = "settings_mp3";
static constexpr const char* TCONST_settings_wifi = "settings_wifi";
static constexpr const char* TCONST_showName = "showName";
static constexpr const char* TCONST_show_button = "show_button";
static constexpr const char* TCONST_show_butt = "show_butt";
static constexpr const char* TCONST_show_event = "show_event";
static constexpr const char* TCONST_show_mp3 = "show_mp3";
static constexpr const char* TCONST_soundfile = "soundfile";
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
static constexpr const char* TCONST_tm24 = "tm24";
static constexpr const char* TCONST_tm_clk = "tmclk";
static constexpr const char* TCONST_tm_dio = "tmdio";
static constexpr const char* TCONST_tmBrightOff = "tmBrightOff";
static constexpr const char* TCONST_tmBrightOn = "tmBrightOn";
static constexpr const char* TCONST_tmBright = "tmBright";
static constexpr const char* TCONST_tmEvent = "tmEvent";
static constexpr const char* TCONST_tmZero = "tmZero";
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
static constexpr const char* A_ui_page_effects = "ui_page_effects";             // Contstruct UI page - main Effects Control
static constexpr const char* A_ui_page_drawing = "ui_page_drawing";             // Contstruct UI page - drawing panel
static constexpr const char* A_ui_block_switches = "ui_block_switches";         // Contstruct UI block - show extended switches at effects page
static constexpr const char* A_effect_switch = "eff_sw_*";                      // Wildcard Effect switcher action
static constexpr const char* A_effect_switch_idx = "eff_sw_idx";                // Switch Effect to specified index
static constexpr const char* A_effect_switch_next = "eff_sw_next";              // Switch to next effect
static constexpr const char* A_effect_switch_prev = "eff_sw_prev";              // Switch to previous effect
static constexpr const char* A_effect_switch_rnd = "eff_sw_rnd";                // Switch to random effect
static constexpr const char* A_effect_ctrls = "eff_ctrls";                      // Generate and publish Effect controls (also it is an mqtt suffix for controls publish)
static constexpr const char* A_effect_dynCtrl = "eff_dynCtrl*";                 // Effect controls handler
static constexpr const char* A_display_hub75 = "display_hub75";                    // HUB75 display configuration
static constexpr const char* A_display_ws2812 = "display_ws2812";                // ws2812 display configuration


static constexpr const char* A_ui_page_effects_config = "effects_config";


static constexpr const char* TCONST_set_gpio = "s_gpio";                    // set gpio action
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

static constexpr const char* CMD_ON = "ON";                    // Без параметров - возвращает 1/0 в зависимости вкл/выкл лампа, принимает параметр - 1/0 (вкл/выкл лампу)
static constexpr const char* CMD_OFF = "OFF";                  // Без параметров - возвращает 1/0 в зависимости выкл/вкл лампа, принимает параметр - 1/0 (выкл/вкл лампу)
static constexpr const char* CMD_G_BRIGHT = "G_BRIGHT";        // Без параметров - возвращает 1/0 в зависимости вкл/выкл глобальная яркость, принимает параметр - 1/0 (вкл/выкл глобальная яркость)
static constexpr const char* CMD_G_BRTPCT = "G_BRTPCT";        // Без параметров - возвращает общую яркость в процентах, принимает параметр - 0-100 выставить общую яркость в процентах
static constexpr const char* CMD_DEMO = "DEMO";                // Без параметров - возвращает 1/0 в зависимости вкл/выкл глобальная яркость, принимает параметр - 1/0 (вкл/выкл глобальная яркость)
static constexpr const char* CMD_PLAYER = "PLAYER";            // Без параметров - возвращает 1/0 в зависимости вкл/выкл плеер, принимает параметр - 1/0 (вкл/выкл плеер)
static constexpr const char* CMD_MP3_SOUND = "MP3_SOUND";      // Без параметров - возвращает 1/0 в зависимости вкл/выкл проигрывание MP3, принимает параметр - 1/0 (вкл/выкл проигрывание MP3)
static constexpr const char* CMD_MP3_PREV = "MP3_PREV";        // Без параметров - переключает трек на 1 назад, принимает числовой параметр, на сколько треков вернуть назад
static constexpr const char* CMD_MP3_NEXT = "MP3_NEXT";        // Без параметров - переключает трек на 1 назад, принимает числовой параметр, на сколько треков вернуть назад
static constexpr const char* CMD_MP3_VOLUME = "MP3_VOL";       // принимает числовой параметр - установить громкость mp3 плеера
static constexpr const char* CMD_MIC = "MIC";                  // Без параметров - возвращает 1/0 в зависимости вкл/выкл микрофон, принимает параметр - 1/0 (вкл/выкл микрофон)
static constexpr const char* CMD_EFFECT = "EFFECT";            // Без параметров - возвращает номер текущего эффекта, принимает числовой параметр - включить заданный эффект
static constexpr const char* CMD_WARNING = "WARNING";          // Без параметров - мигает желтым 5 раз (1 раз в сек), принимает параметры /cmd?warning=[16777215,5000,500,1] - неблокирующий вывод мигалки поверх эффекта (выдача предупреждений), первое число - цвет, второе - полный период, третье - полупериод мигания, четвертое - тип уведомления (0 - цвет, 1-2 - цвет и счетчик, 3 - счетчик)
static constexpr const char* CMD_EFF_CONFIG = "EFF_CONFIG";    // Без параметров - возвращает ВСЕ настройки текущего эффекта, с параметрами - не реализовано пока
static constexpr const char* CMD_CONTROL = "CONTROL";          // Только с параметрами - принимает числовой параметр (номер контрола) и возвращает его текущие настройки, также принимает массив /cmd?control=[N,val] где - N - номер контрола, val - нужное значение
static constexpr const char* CMD_RGB = "RGB";                  // RGB = r,g,b т.е. к примеру для красного 255,0,0
static constexpr const char* CMD_LIST = "LIST";                // Без параметров - возвращает полный список эффектов (их номера)
static constexpr const char* CMD_SHOWLIST = "SHOWLIST";        // Без параметров - возвращает список отображаемых эффектов (их номера)
static constexpr const char* CMD_DEMOLIST = "DEMOLIST";        // Без параметров - возвращает список отображаемых эффектов (их номера) в DEMO режиме
static constexpr const char* CMD_EFF_NAME = "EFF_NAME";        // Без параметров - возвращает из конфига имя текущего эффекта, принимает числовой параметр (номер эффекта) и возвращает имя этого эффекта
static constexpr const char* CMD_EFF_ONAME = "EFF_ONAME";      // Без параметров - возвращает из внутренней памяти имя текущего эффекта, принимает числовой параметр (номер эффекта) и возвращает имя этого эффекта
static constexpr const char* CMD_MOVE_NEXT = "MOVE_NEXT";      // Без параметров - переключает на следующий эффект
static constexpr const char* CMD_MOVE_PREV = "MOVE_PREV";      // Без параметров - переключает на предыдущий эффект
static constexpr const char* CMD_MOVE_RND = "MOVE_RND";        // Без параметров - переключает на рандомный эффект
static constexpr const char* CMD_REBOOT = "REBOOT";            // Без параметров - перезагружает лампу
static constexpr const char* CMD_ALARM = "ALARM";              // Без параметров - возвращает 1/0 в зависимости включен ли сейчас будильник рассвет, принимает параметры "cmd?alarm={alarmP=N,alarmT=N,msg="Message",lV=N, afS=N, sT=N}", где alarmP длительность рассвета, alarmT - светить после рассвета, msg - сообщение, lV - уровень громкости будильника, afS - стандартная ли мелодия будильника, sT - мелодия будильника (от 0 до 7), все комманды указывать не обязательно
static constexpr const char* CMD_MATRIX = "MATRIX";            // Возвращает размер матрицы в формате [X, Y]
static constexpr const char* CMD_MSG = "MSG";                  // Только с параметрами - выводит сообщение на лампу
static constexpr const char* CMD_DRAW = "DRAW";                // Только с параметрами =[цвет,X,Y] - закрашивает пиксель указанным цветом (если включено рисование)
static constexpr const char* CMD_INC_CONTROL = "INC_CONTROL";  // Только с параметрами =[N,val] - увеличивает\уменьшает контрол N на значение val
static constexpr const char* CMD_FILL_MATRIX = "FILL_MATRIX";  // Только с параметрами - закрашивает матрицу указанным цветом (если включено рисование)
static constexpr const char* CMD_AUX_ON = "AUX_ON";            // Включает AUX пин
static constexpr const char* CMD_AUX_OFF = "AUX_OFF";          // Выключает AUX пин
static constexpr const char* CMD_AUX_TOGGLE = "AUX_TOGGLE";    // Переключает AUX пин
static constexpr const char* CMD_DRAWING = "DRAWING";          // Без параметров - возвращает 1/0 в зависимости вкл/выкл рисование, принимает параметр - 1/0 (вкл/выкл рисование)


