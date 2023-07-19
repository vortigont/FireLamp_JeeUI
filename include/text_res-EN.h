/*
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

#ifndef __TEXT_RES_H
#define __TEXT_RES_H
#include <Arduino.h>

// Здесь строковые константы общего назначения, включая константы из интерфейса, что позволит локализировать при необходимости
// Ничего иного в данном файле не предполагается

/** набор строк с именами Effectов
 *  ссылки на эти строки собираются во флеш-массив в файле effect_types.h
 */
static const char* TEFF_000 = "";  // "пустышка"
static const char* TEFF_001 = "White lamp";
static const char* TEFF_002 = "Colors";
static const char* TEFF_003 = "Rainbow 2D";
static const char* TEFF_004 = "Confetti";
static const char* TEFF_005 = "M&M`s";
static const char* TEFF_006 = "Blizzard, Meteor Shower, Meteors";
static const char* TEFF_007 = "New Matrix";
static const char* TEFF_008 = "Aquarelle";
static const char* TEFF_009 = "Fireflies with a trace";
static const char* TEFF_010 = "Wandering cube";
static const char* TEFF_011 = "Pulse";
static const char* TEFF_012 = "Effectfall";
static const char* TEFF_013 = "Burning bush"; 
static const char* TEFF_014 = "Paintball";
static const char* TEFF_015 = "Noise 3D effects";
static const char* TEFF_016 = "Cells and other";
static const char* TEFF_017 = "Puzzles";
static const char* TEFF_018 = "Tixy.Land";
static const char* TEFF_019 = "N.Tesla";
static const char* TEFF_020 = "Oscillator";
static const char* TEFF_021 = "Storm";
static const char* TEFF_022 = "Fairy";
static const char* TEFF_023 = "Fount";
static const char* TEFF_024 = "Bouncing balls";
static const char* TEFF_025 = "Sinusoid";
static const char* TEFF_026 = "Metasphere";
static const char* TEFF_027 = "Spiral";
static const char* TEFF_028 = "Comets, Soaring Fires +";
static const char* TEFF_029 = "Bulbulator";
static const char* TEFF_030 = "Prismata";
static const char* TEFF_031 = "Flock and Predator";
static const char* TEFF_032 = "Whirlpool";
static const char* TEFF_033 = "Drift";
static const char* TEFF_034 = "Popcorn";
static const char* TEFF_035 = "Flicker";
static const char* TEFF_036 = "Radar";
static const char* TEFF_037 = "Waves";
static const char* TEFF_038 = "Fire 2012";
static const char* TEFF_039 = "Bengal Lights";
static const char* TEFF_040 = "Balls";
static const char* TEFF_041 = "DNA";
static const char* TEFF_042 = "Fire 2018";
static const char* TEFF_043 = "Combination lock";
static const char* TEFF_044 = "Cube 2D";
static const char* TEFF_045 = "Magma";
static const char* TEFF_046 = "Picasso";
static const char* TEFF_047 = "Starship Troopers";
static const char* TEFF_048 = "Flags";
static const char* TEFF_049 = "Jumpers";
static const char* TEFF_050 = "Vortexes";
static const char* TEFF_051 = "Stars";
static const char* TEFF_052 = "Glare on Water, Colors";
static const char* TEFF_053 = "Fireworks";
static const char* TEFF_054 = "Pacific";
static const char* TEFF_055 = "Munch";
static const char* TEFF_056 = "Color noise";
static const char* TEFF_057 = "Moths";
static const char* TEFF_058 = "Shadows";
static const char* TEFF_059 = "Patterns";
static const char* TEFF_060 = "Arrows";
static const char* TEFF_061 = "Wild balls";
static const char* TEFF_062 = "Attract";
static const char* TEFF_063 = "Snakes";
static const char* TEFF_064 = "Nexus";
static const char* TEFF_065 = "Maze";
static const char* TEFF_066 = "Curly Colors";
static const char* TEFF_067 = "Polar Lights";
static const char* TEFF_068 = "Flowering";
static const char* TEFF_069 = "Snakes Island";
static const char* TEFF_070 = "Children's Dreams";
static const char* TEFF_071 = "Metasphares";
static const char* TEFF_072 = "Lava lamp";
static const char* TEFF_073 = "Space Racer";
static const char* TEFF_074 = "Smoke" ;
static const char* TEFF_075 = "Fire Lamp";
static const char* TEFF_076 = "Mirage";
static const char* TEFF_250 = "Clock";
static const char* TEFF_254 = "Frequency Analyzer";
static const char* TEFF_255 = "Oscilloscope";

// -------------------------
#define DFTINTF_00A "Effect"
#define DFTINTF_020 "Microphone"
#define DFTINTF_084 "Palette"
#define DFTINTF_088 "Scale"
#define DFTINTF_0D0 "Color"
#define DFTINTF_0D1 "Palette/Scale"
#define DFTINTF_0D2 "Effect (0: cyclical)"
#define DFTINTF_0D3 "Color offset (0: cyclical)"
#define DFTINTF_0D4 "Subpixel"
#define DFTINTF_0D5 "Quantity"
#define DFTINTF_0D6 "Color generator"
#define DFTINTF_0D7 "Tint"
#define DFTINTF_0D8 "Foolery!"
#define DFTINTF_0D9 "Quantity/Color (night lamp)"
#define DFTINTF_0DA "Saturation"
#define DFTINTF_0DB "Gain"
#define DFTINTF_0DC "Sweep"
#define DFTINTF_0DD "Ripple"
#define DFTINTF_0DE "Blur"
#define DFTINTF_0DF "Flutter"
#define DFTINTF_0E0 "Night lamp"
#define DFTINTF_0E1 "Rings"
#define DFTINTF_0E2 "Glare (0: Off)"
#define DFTINTF_0E3 "Mode"
#define DFTINTF_0E4 "Linear Velocity"
#define DFTINTF_0E5 "Mode (1: random)"
#define DFTINTF_0E6 "Offset"
#define DFTINTF_0E7 "Quantity/Direction/Position"
#define DFTINTF_0E8 "Predator"
#define DFTINTF_0E9 "Mass"
#define DFTINTF_0EA "Speed ​​(Vertical)"
#define DFTINTF_0EB "Speed ​​(Horizontal)"
#define DFTINTF_0EC "Pattern (0: random)"
#define DFTINTF_0ED "Shading"
#define DFTINTF_0EE "Energy"
#define DFTINTF_0EF "Density"
#define DFTINTF_0F0 "Color/Rotation"
#define DFTINTF_0F1 "Size/Track (1-85-170-255)"
#define DFTINTF_0F2 "Start frequency"
#define DFTINTF_0F3 "Rotate"
#define DFTINTF_0F4 "Palette (0: generator)"
#define DFTINTF_0F5 "Smoothing"
#define DFTINTF_0F6 "Fill"
#define DFTINTF_0F7 "Thickness"
#define DFTINTF_0F9 "Behavior: 1/2"
#define DFTINTF_0FA "Color: 1/2"
#define DFTINTF_0FB "Palette (0: no color)"
#define DFTINTF_0FC "Color (1: random, 255: white)"
#define DFTINTF_0FD "Snowfall/Starfall/Meteors"
#define DFTINTF_0FE "Quantity"
#define DFTINTF_0FF "Type 1/2, Blur 3/4"
#define DFTINTF_100 "Palette (0: random color)"
#define DFTINTF_101 "Color 1 (0: cyclical)"
#define DFTINTF_102 "Cloud"
#define DFTINTF_103 "Blurring (0: no)"
#define DFTINTF_104 "Swipe"
#define DFTINTF_105 "Filter"
#define DFTINTF_106 "Physics"
#define DFTINTF_107 "Circles"
#define DFTINTF_108 "Option 1/2"
#define DFTINTF_109 "Flashes"
#define DFTINTF_10A "Glitch"
#define DFTINTF_10B "Storm"
#define DFTINTF_10C "Quantity of shells"
#define DFTINTF_10D "Size (width)"
#define DFTINTF_10E "Color shift (0: random)"
#define DFTINTF_10F "Size (height)"
#define DFTINTF_110 "Classic"
#define DFTINTF_111 "One color"
#define DFTINTF_112 "Inverse Colors"
#define DFTINTF_113 "Color 2 (0: cyclical)"
#define DFTINTF_114 "Quality/Speed"
#define DFTINTF_115 "Blur (1-4 with wind)"
#define DFTINTF_116 "Sin function moving"
#define DFTINTF_117 "Direction (0: cyclical)"
#define DFTINTF_118 "Signal threshold"
#define DFTINTF_11A "Color (1: cyclical)"

/** набор строк с текстовыми константами интерфейса
 */
static const char* TINTF_000 = "Effects";
static const char* TINTF_001 = "Text";
static const char* TINTF_002 = "Settings";
static const char* TINTF_in_sel_lst = "Included selection list";
static const char* TINTF_in_demo = "Included in Demo";
static const char* TINTF_005 = "Copy effect";
static const char* TINTF_006 = "Remove";
static const char* TINTF_007 = "Refresh effects-list";
static const char* TINTF_Save = "Save";
static const char* TINTF_009 = "Managing effects-list";
static const char* TINTF_00A = DFTINTF_00A;
static const char* TINTF_00B = "Exit";
static const char* TINTF_00C = "Global Brightness";
static const char* TINTF_00D = "Brightness";
static const char* TINTF_00E = "OFF/ON";
static const char* TINTF_00F = "Demo";
static const char* TINTF_010 = "Gl. Bright";
static const char* TINTF_011 = "Events";
static const char* TINTF_012 = "Mic";
static const char* TINTF_013 = "Button";
static const char* TINTF_014 = "More...";
static const char* TINTF_015 = "<<<";
static const char* TINTF_016 = ">>>";
static const char* TINTF_017 = "Waiting...";
static const char* TINTF_018 = "Configs";
static const char* TINTF_019 = "Download";
static const char* TINTF_01A = "Config";
static const char* TINTF_01B = "Create";
static const char* TINTF_01C = "Show on lamp";
static const char* TINTF_01D = "Text";
static const char* TINTF_01E = "Text color";
static const char* TINTF_01F = "Send";
static const char* TINTF_020 = DFTINTF_020;
static const char* TINTF_021 = "Microphone On";
static const char* TINTF_022 = "Zero correction";
static const char* TINTF_023 = "Noise level, units";
static const char* TINTF_024 = "Denoising";
static const char* TINTF_025 = "Microphone calibration";
static const char* TINTF_026 = "Turn microphone On";
static const char* TINTF_027 = "... in progress ...";
static const char* TINTF_028 = "WiFi";
static const char* TINTF_029 = "Client (STA)";
static const char* TINTF_02A = "WiFi client settings";
static const char* TINTF_02B = "Lamp name (mDNS Hostname/AP-SSID)";
static const char* TINTF_02C = "WiFi SSID";
static const char* TINTF_02D = "Password";
static const char* TINTF_02E = "Connect";
static const char* TINTF_02F = "Access poin (AP)";
static const char* TINTF_030 = "Display";
static const char* TINTF_031 = "WiFi settings";
static const char* TINTF_032 = "In AP mode, the lamp always works as an Access Point and will not connect to other WiFi networks.";
static const char* TINTF_033 = "WiFi mode";
static const char* TINTF_034 = "Protect AP with a password";
static const char* TINTF_035 = "MQTT";
static const char* TINTF_036 = "MQTT host";
static const char* TINTF_037 = "MQTT port";
static const char* TINTF_038 = "User";
static const char* TINTF_039 = "Interval mqtt sec.";
static const char* TINTF_03A = "Connect";
static const char* TINTF_03B = "Mirroring H";
static const char* TINTF_03C = "Mirroring V";
static const char* TINTF_03D = "Smooth effects switching";
static const char* TINTF_03E = "Randomize effect in Demo";
static const char* TINTF_03F = "Switch effect in Demo. (sec)";
static const char* TINTF_040 = "Sorting effects";
static const char* TINTF_041 = "Copies under original";
static const char* TINTF_042 = "Copies at the end of the list";
static const char* TINTF_043 = "Sort by index";
static const char* TINTF_044 = "Text scrolling speed";
static const char* TINTF_045 = "Offset of text output";
static const char* TINTF_046 = "Mixed (AP+STA)";
static const char* TINTF_047 = "Don`t show";
static const char* TINTF_048 = "Every hour";
static const char* TINTF_049 = "Every half hour";
static const char* TINTF_04A = "Every 15 minutes";
static const char* TINTF_04B = "Every 10 minutes";
static const char* TINTF_04C = "Every 5 minutes";
static const char* TINTF_04D = "Every minute";
static const char* TINTF_04E = "New Year greeting";
static const char* TINTF_04F = "Show period, minutes (0 - do not output)";
static const char* TINTF_050 = "New Year's Date/Time at format YYYY-MM-DDThh:mm";
static const char* TINTF_051 = "Date / Time / Time Zone";
static const char* TINTF_052 = "TimeZone shift/daylight saving rules applied automatically, no need to adjust manually.";
static const char* TINTF_053 = "Time zone";
static const char* TINTF_054 = "Spare NTP server (optional)";
static const char* TINTF_055 = "Date/Time, YYYY-MM-DDThh:mm:ss (without internet connection, if empty - from device)";
static const char* TINTF_056 = "Firmware update";
static const char* TINTF_057 = "OTA-PIO firmware update";
static const char* TINTF_058 = "Start";
static const char* TINTF_059 = "Downloading firmware";
static const char* TINTF_05A = "Upload";
static const char* TINTF_05B = "Event";
static const char* TINTF_05C = "Edit";
static const char* TINTF_05D = "Add";
static const char* TINTF_05E = "Actively";
static const char* TINTF_05F = "Event type";
static const char* TINTF_060 = "Lamp On";
static const char* TINTF_061 = "Lamp Off";
static const char* TINTF_062 = "Mode DEMO";
static const char* TINTF_063 = "Alarm";
static const char* TINTF_064 = "Upload lamp configuration";
static const char* TINTF_065 = "Loading effects configurations";
static const char* TINTF_066 = "Loading events configurations";
static const char* TINTF_067 = "Show the Text";
static const char* TINTF_068 = "Show the Time";
static const char* TINTF_069 = "State of Pin";
static const char* TINTF_06A = "Turn AUX On";
static const char* TINTF_06B = "Turn AUX Off";
static const char* TINTF_06C = "Toggle AUX";
static const char* TINTF_06D = "Event Date/Time";
static const char* TINTF_06E = "Repeat, min";
static const char* TINTF_06F = "Stop in, min";
static const char* TINTF_070 = "Parameter (text)";
static const char* TINTF_071 = "Replay";
static const char* TINTF_072 = "Monday";
static const char* TINTF_073 = "Tuesday";
static const char* TINTF_074 = "Wednesday";
static const char* TINTF_075 = "Thursday";
static const char* TINTF_076 = "Friday";
static const char* TINTF_077 = "Saturday";
static const char* TINTF_078 = "Sunday";
static const char* TINTF_079 = "Update";
static const char* TINTF_07A = "Action";
static const char* TINTF_07B = "Button activated";
static const char* TINTF_07C = "OFF/ON";
static const char* TINTF_07D = "Hold";
static const char* TINTF_07E = "Press";
static const char* TINTF_07F = "Singly";
static const char* TINTF_080 = "Fire Lamp";
static const char* TINTF_082 = "Other";
static const char* TINTF_083 = "Clean the lamp when switching effects";
static const char* TINTF_084 = DFTINTF_084;
static const char* TINTF_085 = "Sort by Alphabet (internal)";
static const char* TINTF_086 = "Events activated";
static const char* TINTF_087 = "Speed";
static const char* TINTF_088 = DFTINTF_088;
static const char* TINTF_effrename = "Rename effect";
static const char* TINTF_08A = "Sort by Alphabet (config)";
static const char* TINTF_08B = "Reset effect settings";
static const char* TINTF_08C = "Topic prefix";
static const char* TINTF_08D = "Sort by microphone";
static const char* TINTF_08E = "Debug";
static const char* TINTF_08F = "ESP settings";
static const char* TINTF_090 = "Effect list numbering";
static const char* TINTF_091 = "Mic. symbol in effect list";
static const char* TINTF_092 = "Setting up pins";
static const char* TINTF_093 = "Show the system menu";
static const char* TINTF_094 = "Button pin";
static const char* TINTF_095 = "Current limit (mA)";
static const char* TINTF_096 = "Reboot";
static const char* TINTF_097 = "Pin of the RX player";
static const char* TINTF_098 = "Pin of the TX player";
static const char* TINTF_099 = "DFPlayer";
static const char* TINTF_09A = "Show effect name in Demo";
static const char* TINTF_09B = "Volume";
static const char* TINTF_09C = "Time";
static const char* TINTF_09D = "Effect name";
static const char* TINTF_09E = "Effect (reset)";
static const char* TINTF_09F = "Disabled";
static const char* TINTF_0A0 = "First";
static const char* TINTF_0A1 = "Random";
static const char* TINTF_0A2 = "Random MP3";
static const char* TINTF_0A3 = "The sound of the alarm clock";
static const char* TINTF_0A4 = "Second";
static const char* TINTF_0A5 = "Third";
static const char* TINTF_0A6 = "Fourth";
static const char* TINTF_0A7 = "Fifth";
static const char* TINTF_0A8 = "Equalizer";
static const char* TINTF_0A9 = "Normal";
static const char* TINTF_0AA = "Pop";
static const char* TINTF_0AB = "Rock";
static const char* TINTF_0AC = "Jazz";
static const char* TINTF_0AD = "Classic";
static const char* TINTF_0AE = "Bass";
static const char* TINTF_0AF = "Mp3 player";
static const char* TINTF_0B0 = "Number of files in MP3 folder";
static const char* TINTF_0B1 = "Voice-over modes";
static const char* TINTF_0B2 = "Folder\\the number of the sound file (examples: MP3\\17, 5\\19)";
static const char* TINTF_0B3 = "Limit Alarm volume";
static const char* TINTF_0B4 = "Default";
static const char* TINTF_0B5 = "Remove from the list";
static const char* TINTF_0B6 = "Disabled";
static const char* TINTF_0B7 = "Option 1";
static const char* TINTF_0B8 = "Option 2";
static const char* TINTF_0B9 = "Parameter";
static const char* TINTF_0BA = "Alarm Clock Dawn";
static const char* TINTF_0BB = "Dawn period";
static const char* TINTF_0BC = "After down period";
static const char* TINTF_0BD = "<";
static const char* TINTF_0BE = ">";
static const char* TINTF_0BF = "<+5";
static const char* TINTF_0C0 = "+5>";
static const char* TINTF_0C1 = "seconds";
static const char* TINTF_0C2 = "minutes";
static const char* TINTF_0C3 = "hours";
static const char* TINTF_0C4 = "days";
static const char* TINTF_0C5 = "days";
static const char* TINTF_0C6 = "day";
static const char* TINTF_0C7 = "hours";
static const char* TINTF_0C8 = "hour";
static const char* TINTF_0C9 = "Rand.";
static const char* TINTF_0CA = "Darkening the background";
static const char* TINTF_0CB = "Warning";
static const char* TINTF_0CC = "minutes";
static const char* TINTF_0CD = "minute";
static const char* TINTF_0CE = "Drawing";
static const char* TINTF_0CF = "Fill";
static const char* TINTF_0D0 = DFTINTF_0D0;
static const char* TINTF_0D1 = "Sound from begin";
static const char* TINTF_0D2 = "Limit volume";
static const char* TINTF_0D3 = "Speed factor (1.0 as default)";
static const char* TINTF_0D4 = "Display settings";
static const char* TINTF_0D5 = "Brightness (0 - OFF)";
static const char* TINTF_0D6 = "Brightness with OFF lamp (0 - OFF)";
static const char* TINTF_0D7 = "24 hour format";
static const char* TINTF_0D8 = "Leading 0";
static const char* TINTF_0D9 = "Clear";
static const char* TINTF_0DA = "Scan";
static const char* TINTF_0DB = "FTP";
static const char* TINTF_0DC = "Control";
static const char* TINTF_0DD = "Gauge type";
static const char* TINTF_0DE = "Gauge color";
static const char* TINTF_0DF = "Text color";
static const char* TINTF_0E0 = "Show temperature";
static const char* TINTF_0E1 = "Sure?";
static const char* TINTF_0E2 = "Streaming";
static const char* TINTF_0E3 = "Mode";
static const char* TINTF_0E4 = "E1.31";
static const char* TINTF_0E5 = "SoulMate";
static const char* TINTF_0E6 = "Block Effect";
static const char* TINTF_0E7 = "Mapping";
static const char* TINTF_0E8 = "Starting Universe";
static const char* TINTF_0E9 = "Loading buttons configurations";
static const char* TINTF_0EA = "Night light";
static const char* TINTF_0EB = "Bright lamp";
static const char* TINTF_0EC = "Anyways";
static const char* TINTF_0ED = "Tell the time";
static const char* TINTF_0EE = "Disabled";
static const char* TINTF_0EF = "Vertical";
static const char* TINTF_0F0 = "Horizontal";

static const char* TINTF_gpiocfg = "GPIO Configuration";
static const char* TINTF_ledstrip = "LED strip layout";
static const char* TINTF_f_restore_state = "Restore state on power-on";

static const char* NY_MDG_STRING1 = "New Year in %d %s";
static const char* NY_MDG_STRING2 = "Happy New %d Year!";
//-------------------------

#endif
