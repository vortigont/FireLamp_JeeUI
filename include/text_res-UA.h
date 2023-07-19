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

/** набор строк с именами эффектов
 *  ссылки на эти строки собираются во флеш-массив в файле effect_types.h
 */
static const char* TEFF_000 = "";  // "пустышка"
static const char* TEFF_001 = "Біла лампа";
static const char* TEFF_002 = "Кольори";
static const char* TEFF_003 = "Веселка 2D";
static const char* TEFF_004 = "Конфеті";
static const char* TEFF_005 = "Кольорові драже";
static const char* TEFF_006 = "Хурделиця, Зіркопад, Метеори";
static const char* TEFF_007 = "Матриця";
static const char* TEFF_008 = "Акварель";
static const char* TEFF_009 = "Світлячки зі шлейфом";
static const char* TEFF_010 = "Блукаючий кубик";
static const char* TEFF_011 = "Пульс";
static const char* TEFF_012 = "Ефектопад";
static const char* TEFF_013 = "Неопаліма купина"; 
static const char* TEFF_014 = "Пейнтбол";
static const char* TEFF_015 = "Шум 3D";
static const char* TEFF_016 = "Клітинки-Ялинки";
static const char* TEFF_017 = "П'ятнашки";
static const char* TEFF_018 = "Тіксі Ленд";
static const char* TEFF_019 = "Н.Тесла";
static const char* TEFF_020 = "Осцилятор";
static const char* TEFF_021 = "Шторм";
static const char* TEFF_022 = "Фея";
static const char* TEFF_023 = "Джерело";
static const char* TEFF_024 = "Стрибаючі м'ячики";
static const char* TEFF_025 = "Синусоїд";
static const char* TEFF_026 = "Метасфери";
static const char* TEFF_027 = "Спіраль";
static const char* TEFF_028 = "Комети, Витаючі Вогні +";
static const char* TEFF_029 = "Бульбулятор";
static const char* TEFF_030 = "Призмата";
static const char* TEFF_031 = "Стадо та Хижак";
static const char* TEFF_032 = "Водоверть";
static const char* TEFF_033 = "Дрифт";
static const char* TEFF_034 = "Попкорн";
static const char* TEFF_035 = "Мерехтіння";
static const char* TEFF_036 = "Радар";
static const char* TEFF_037 = "Хвилі";
static const char* TEFF_038 = "Вогонь 2012";
static const char* TEFF_039 = "Бенгальські Вогні";
static const char* TEFF_040 = "Кульки";
static const char* TEFF_041 = "ДНК";
static const char* TEFF_042 = "Вогонь 2018";
static const char* TEFF_043 = "Кодовий замок";
static const char* TEFF_044 = "Куб 2D";
static const char* TEFF_045 = "Магма";
static const char* TEFF_046 = "Пікассо";
static const char* TEFF_047 = "Зоряний Десант";
static const char* TEFF_048 = "Прапори";
static const char* TEFF_049 = "Стрибуни";
static const char* TEFF_050 = "Вихори";
static const char* TEFF_051 = "Зірки";
static const char* TEFF_052 = "Відблиски води, Кольори";
static const char* TEFF_053 = "Феєрверк";
static const char* TEFF_054 = "Тихий Океан";
static const char* TEFF_055 = "Вишиванка";
static const char* TEFF_056 = "Кольоровий шум";
static const char* TEFF_057 = "Метелики";
static const char* TEFF_058 = "Тіні";
static const char* TEFF_059 = "Візерунки";
static const char* TEFF_060 = "Стрілки";
static const char* TEFF_061 = "Дикі кульки";
static const char* TEFF_062 = "Притягування";
static const char* TEFF_063 = "Змійки";
static const char* TEFF_064 = "Nexus";
static const char* TEFF_065 = "Лабіринт (Тест)";
static const char* TEFF_066 = "В'юнкі кольори";
static const char* TEFF_067 = "Північне Сяйво";
static const char* TEFF_068 = "Цвітіння";
static const char* TEFF_069 = "Зміїний Острів";
static const char* TEFF_070 = "Дитячі сни";
static const char* TEFF_071 = "Метабульки";
static const char* TEFF_072 = "Лавова Лампа";
static const char* TEFF_073 = "Космо-Рейсер";
static const char* TEFF_074 = "Дим";
static const char* TEFF_075 = "Вогняна Лампа";
static const char* TEFF_076 = "Міраж";
static const char* TEFF_250 = "Годинник";
static const char* TEFF_254 = "Частотний аналізатор";
static const char* TEFF_255 = "Осцилограф";

#define DFTINTF_00A "Ефект"
#define DFTINTF_020 "Мікрофон"
#define DFTINTF_084 "Палітра"
#define DFTINTF_088 "Масштаб"
#define DFTINTF_0D0 "Колір"
#define DFTINTF_0D1 "Палітра/Масштаб"
#define DFTINTF_0D2 "Ефект (0: циклічно)"
#define DFTINTF_0D3 "Зміщення кольору (0: циклічно)"
#define DFTINTF_0D4 "Cубпіксель"
#define DFTINTF_0D5 "Кількість"
#define DFTINTF_0D6 "Генератор кольору"
#define DFTINTF_0D7 "Відтінок"
#define DFTINTF_0D8 "Запал!"
#define DFTINTF_0D9 "Кількість/Колір (нічник)"
#define DFTINTF_0DA "Насиченість"
#define DFTINTF_0DB "Посилення"
#define DFTINTF_0DC "Розгортка"
#define DFTINTF_0DD "Пульсація"
#define DFTINTF_0DE "Розмиття"
#define DFTINTF_0DF "Порхання"
#define DFTINTF_0E0 "Нічник"
#define DFTINTF_0E1 "Кільця"
#define DFTINTF_0E2 "Відблиски (0: Вимкнути)"
#define DFTINTF_0E3 "Режим"
#define DFTINTF_0E4 "Лінійна швидкість"
#define DFTINTF_0E5 "Режим (1: випадковий)"
#define DFTINTF_0E6 "Зсув"
#define DFTINTF_0E7 "Кількість/Напрям/Положення"
#define DFTINTF_0E8 "Хижак"
#define DFTINTF_0E9 "Маса"
#define DFTINTF_0EA "Швидкість (Верт.)"
#define DFTINTF_0EB "Швидкість (Гориз.)"
#define DFTINTF_0EC "Візерунок (0: випадковий)"
#define DFTINTF_0ED "Затемнення"
#define DFTINTF_0EE "Енергія"
#define DFTINTF_0EF "Щільність"
#define DFTINTF_0F0 "Колір/Обертання"
#define DFTINTF_0F1 "Розмір/Шлейф (1-85-170-255)"
#define DFTINTF_0F2 "Частота запусків"
#define DFTINTF_0F3 "Поворот"
#define DFTINTF_0F4 "Палітра (0: генератор)"
#define DFTINTF_0F5 "Згладжування"
#define DFTINTF_0F6 "Заповнення"
#define DFTINTF_0F7 "Товщина"
#define DFTINTF_0F9 "Поведінка: 1/2"
#define DFTINTF_0FA "Колір: 1/2"
#define DFTINTF_0FB "Палітра (0: без кольору)"
#define DFTINTF_0FC "Колір (1: випадковий, 255: білий)"
#define DFTINTF_0FD "Снігопад/Зорепад/Метеори"
#define DFTINTF_0FE "Кількість"
#define DFTINTF_0FF "Тип 1/2, Розмиття 3/4"
#define DFTINTF_100 "Палітра (0: випадковий колір)"
#define DFTINTF_101 "Колір 1 (0: циклічно)"
#define DFTINTF_102 "Хмарка"
#define DFTINTF_103 "Розмиття (0: немає)"
#define DFTINTF_104 "Розмах"
#define DFTINTF_105 "Фільтр"
#define DFTINTF_106 "Фізика"
#define DFTINTF_107 "Кола"
#define DFTINTF_108 "Варіант 1/2"
#define DFTINTF_109 "Спалахи"
#define DFTINTF_10A "Глюк"
#define DFTINTF_10B "Шторм"
#define DFTINTF_10C "Кількість снарядів"
#define DFTINTF_10D "Розмір (ширина)"
#define DFTINTF_10E "Зміщення кольору (0: випадковий)"
#define DFTINTF_10F "Розмір (висота)"
#define DFTINTF_110 "Класика"
#define DFTINTF_111 "Один колір"
#define DFTINTF_112 "Зворотні Кольори"
#define DFTINTF_113 "Колір 2 (0: циклічно)"
#define DFTINTF_114 "Якість / Швидкість"
#define DFTINTF_115 "Розмиття (1-4 з вітром)"
#define DFTINTF_116 "Переміщення по синусу"
#define DFTINTF_117 "Напрямок (0: циклічно)"
#define DFTINTF_118 "Поріг сигналу"
#define DFTINTF_11A "Колір (1 = циклічно)"


/** набор строк с текстовыми константами интерфейса
 */
static const char* TINTF_000 = "Ефекти";
static const char* TINTF_001 = "Вивід тексту";
static const char* TINTF_002 = "Налаштування";
static const char* TINTF_in_sel_lst = "В загальному переліку";
static const char* TINTF_in_demo = "В переліку Демо'";
static const char* TINTF_005 = "Копіювати";
static const char* TINTF_006 = "Видалити";
static const char* TINTF_007 = "Оновити перелік ефектів";
static const char* TINTF_Save = "Зберегти";
static const char* TINTF_009 = "Керування переліком ефектів";
static const char* TINTF_00A = DFTINTF_00A;
static const char* TINTF_00B = "Вихід";
static const char* TINTF_00C = "Глоб.яскравість";
static const char* TINTF_00D = "Яскравість";
static const char* TINTF_00E = "Увімкнути";
static const char* TINTF_00F = "Демо";
static const char* TINTF_010 = "Гл.Яскрав";
static const char* TINTF_011 = "Події";
static const char* TINTF_012 = "Мікр.";
static const char* TINTF_013 = "Кнопка";
static const char* TINTF_014 = "Ще...";
static const char* TINTF_015 = "<<<";
static const char* TINTF_016 = ">>>";
static const char* TINTF_017 = "Очікуйте...";
static const char* TINTF_018 = "Конфігурації";
static const char* TINTF_019 = "Завантажити";
static const char* TINTF_01A = "Конфігурація";
static const char* TINTF_01B = "Створити";
static const char* TINTF_01C = "Вивести на лампу";
static const char* TINTF_01D = "Текст";
static const char* TINTF_01E = "Колір повідомлення";
static const char* TINTF_01F = "Надіслати";
static const char* TINTF_020 = DFTINTF_020;
static const char* TINTF_021 = "Мікрофон активний";
static const char* TINTF_022 = "Коеф. корекції нуля";
static const char* TINTF_023 = "Рівень шуму, од";
static const char* TINTF_024 = "Подавлення шуму";
static const char* TINTF_025 = "Калібровка мікрофона";
static const char* TINTF_026 = "Включіть мікрофон";
static const char* TINTF_027 = "... в процесі ...";
static const char* TINTF_028 = "WiFi";
static const char* TINTF_029 = "Клієнт (STA)";
static const char* TINTF_02A = "Налаштування клієнта WiFi";
static const char* TINTF_02B = "Ім'я лампи (mDNS Hostname/AP-SSID)";
static const char* TINTF_02C = "WiFi SSID";
static const char* TINTF_02D = "Password";
static const char* TINTF_02E = "Підключитися";
static const char* TINTF_02F = "Точка доступу (AP)";
static const char* TINTF_030 = "Відображення";
static const char* TINTF_031 = "Налаштування WiFi та точки доступу";
static const char* TINTF_032 = "В режимі AP лампа завжди працює як точка доступу, і не буде підключатися до інших WiFi-мереж";
static const char* TINTF_033 = "Режим WiFi";
static const char* TINTF_034 = "Захистити AP паролем";
static const char* TINTF_035 = "MQTT";
static const char* TINTF_036 = "MQTT host";
static const char* TINTF_037 = "MQTT port";
static const char* TINTF_038 = "Користувач";
static const char* TINTF_039 = "Інтервал MQTT сек.";
static const char* TINTF_03A = "Підключити";
static const char* TINTF_03B = "Відзеркалення H";
static const char* TINTF_03C = "Відзеркалення V";
static const char* TINTF_03D = "Плавна зміна ефектів";
static const char* TINTF_03E = "Випадковий ефект в Демо";
static const char* TINTF_03F = "Зміна ефекта в Демо. (сек)";
static const char* TINTF_040 = "Сортуваня ефектів";
static const char* TINTF_041 = "Копії під оригіналом";
static const char* TINTF_042 = "Копії в кінці переліку'";
static const char* TINTF_043 = "Відповідно номеру";
static const char* TINTF_044 = "Швидкість прокрутки тексту";
static const char* TINTF_045 = "Зміщення виводу текста";
static const char* TINTF_046 = "Змішаний (AP+STA)";
static const char* TINTF_047 = "Не показувати";
static const char* TINTF_048 = "Кожну годину";
static const char* TINTF_049 = "Кожні півгодинни";
static const char* TINTF_04A = "Кожні 15 хвилин";
static const char* TINTF_04B = "Кожні 10 хвилин";
static const char* TINTF_04C = "Кожні 5 хвилин";
static const char* TINTF_04D = "Кожну хвилину";
static const char* TINTF_04E = "Новорічне привітання";
static const char* TINTF_04F = "Період відображення, хвилин (0 - не відображати)";
static const char* TINTF_050 = "Дата/час Нового Року у форматі YYYY-MM-DDThh:mm";
static const char* TINTF_051 = "Дата / Час / Часова зона";
static const char* TINTF_052 = "Налаштування часової зони. Правила поясного / сезонного часу застосовуються автоматично, ручна корекція не вимагається. Якщо вашої зони немає у списку, ви можете обрати загальну зону зсуву від Грінвича";
static const char* TINTF_053 = "Часова зона";
static const char* TINTF_054 = "резервний NTP-сервер (не обов'язково)";
static const char* TINTF_055 = "Дата/час у форматі YYYY-MM-DDThh:mm:ss (якщо нема інтернета, якщо пусто - взяти з пристрою)";
static const char* TINTF_056 = "Оновлення";
static const char* TINTF_057 = "Оновлення ОТА-PIO";
static const char* TINTF_058 = "Почати";
static const char* TINTF_059 = "Завантаження прошивки";
static const char* TINTF_05A = "Upload";
static const char* TINTF_05B = "Подія";
static const char* TINTF_05C = "Редагувати";
static const char* TINTF_05D = "Додати";
static const char* TINTF_05E = "Активно";
static const char* TINTF_05F = "Тип події";
static const char* TINTF_060 = "Увімкнути лампу";
static const char* TINTF_061 = "Вимкнути лампу";
static const char* TINTF_062 = "Режим DEMO";
static const char* TINTF_063 = "Будильник";
static const char* TINTF_064 = "Завантажити конфіг лампи";
static const char* TINTF_065 = "Завантажити конфіг ефектів";
static const char* TINTF_066 = "Завантажити конфіг подій";
static const char* TINTF_067 = "Вивести текст";
static const char* TINTF_068 = "Вивести час";
static const char* TINTF_069 = "Стан піна";
static const char* TINTF_06A = "Увімкнути AUX";
static const char* TINTF_06B = "Вимкнути AUX";
static const char* TINTF_06C = "Переключити стан AUX";
static const char* TINTF_06D = "Дата/час події";
static const char* TINTF_06E = "Повтор, хв";
static const char* TINTF_06F = "Зупинити через, хв";
static const char* TINTF_070 = "Параметр (текст)";
static const char* TINTF_071 = "Повтор";
static const char* TINTF_072 = "Понеділок";
static const char* TINTF_073 = "Вівторок";
static const char* TINTF_074 = "Середа";
static const char* TINTF_075 = "Четверг";
static const char* TINTF_076 = "П'ятница";
static const char* TINTF_077 = "Субота";
static const char* TINTF_078 = "Неділя";
static const char* TINTF_079 = "Оновити";
static const char* TINTF_07A = "Дія";
static const char* TINTF_07B = "Кнопка активна";
static const char* TINTF_07C = "OFF/ON";
static const char* TINTF_07D = "Утримання";
static const char* TINTF_07E = "Натискання";
static const char* TINTF_07F = "Одноразово";
static const char* TINTF_080 = "Вогняна Лампа";
#if defined(EMBUI_USE_FTP) && defined(EMBUI_USE_MQTT)
static const char* TINTF_081 = "WiFi, MQTT & FTP";
#elif defined(EMBUI_USE_MQTT)
static const char* TINTF_081 = "WiFi & MQTT";
#elif defined(EMBUI_USE_FTP)
static const char* TINTF_081 = "WiFi & FTP";
#else
static const char* TINTF_081 = "WiFi";
#endif
static const char* TINTF_082 = "Інші";
static const char* TINTF_083 = "Очищувати лампу при зміні ефектів";
static const char* TINTF_084 = DFTINTF_084;
static const char* TINTF_085 = "В алфавітному порядку (внутрішнє)";
static const char* TINTF_086 = "Події активні";
static const char* TINTF_087 = "Швидкість";
static const char* TINTF_088 = DFTINTF_088;
static const char* TINTF_effrename = "Назва ефекту";
static const char* TINTF_08A = "В алфавітному порядку (конфігурація)";
static const char* TINTF_08B = "Скинути налаштування ефекта";
static const char* TINTF_08C = "Префікс топіків";
static const char* TINTF_08D = "Сортуваня по мікрофону";
static const char* TINTF_08E = "Налаштування";
static const char* TINTF_08F = "Налаштування ESP";
static const char* TINTF_090 = "Нумерація в переліку ефектів";
static const char* TINTF_091 = "Символ мікрофона в переліку";
static const char* TINTF_092 = "Налаштування пінів";
static const char* TINTF_093 = "Показати налаштування ESP";
static const char* TINTF_094 = "Пін кнопки";
static const char* TINTF_095 = "Обмеження струму (mA)";
static const char* TINTF_096 = "Перезавантаження";
static const char* TINTF_097 = "Пін RX DFPlayer";
static const char* TINTF_098 = "Пін TX DFPlayer";
static const char* TINTF_099 = "DFPlayer";
static const char* TINTF_09A = "Назва ефекту в Демо";
static const char* TINTF_09B = "Гучність";
static const char* TINTF_09C = "Час";
static const char* TINTF_09D = "Назва ефекту";
static const char* TINTF_09E = "Ефект(ресет)";
static const char* TINTF_09F = "Відключений";
static const char* TINTF_0A0 = "Перший";
static const char* TINTF_0A1 = "Випадковий";
static const char* TINTF_0A2 = "Випадковий MP3";
static const char* TINTF_0A3 = "Звук будильника";
static const char* TINTF_0A4 = "Другий";
static const char* TINTF_0A5 = "Третій";
static const char* TINTF_0A6 = "Четвертий";
static const char* TINTF_0A7 = "Пятий";
static const char* TINTF_0A8 = "Еквалайзер";
static const char* TINTF_0A9 = "Номальний";
static const char* TINTF_0AA = "Поп";
static const char* TINTF_0AB = "Рок";
static const char* TINTF_0AC = "Джаз";
static const char* TINTF_0AD = "Класика";
static const char* TINTF_0AE = "Бас";
static const char* TINTF_0AF = "MP3 плеєр";
static const char* TINTF_0B0 = "Кількість файлів в теці MP3";
static const char* TINTF_0B1 = "Режими озвучування";
static const char* TINTF_0B2 = "Тека\\номер звукового файла (приклади: MP3\\17, 5\\19)";
static const char* TINTF_0B3 = "Обмежити гучність будильника";
static const char* TINTF_0B4 = "За замовчуванням";
static const char* TINTF_0B5 = "Видалити зі списку";
static const char* TINTF_0B6 = "Відключено";
static const char* TINTF_0B7 = "Варіант 1";
static const char* TINTF_0B8 = "Варіант 2";
static const char* TINTF_0B9 = "Параметр";
static const char* TINTF_0BA = "Будильник Світанок";
static const char* TINTF_0BB = "Тривалість світанку";
static const char* TINTF_0BC = "Світіння після світанку";
static const char* TINTF_0BD = "<";
static const char* TINTF_0BE = ">";
static const char* TINTF_0BF = "<+5";
static const char* TINTF_0C0 = "+5>";
static const char* TINTF_0C1 = "секунд";
static const char* TINTF_0C2 = "хвилин";
static const char* TINTF_0C3 = "годин";
static const char* TINTF_0C4 = "днів";
static const char* TINTF_0C5 = "дні";
static const char* TINTF_0C6 = "день";
static const char* TINTF_0C7 = "годин";
static const char* TINTF_0C8 = "година";
static const char* TINTF_0C9 = "Випадк.";
static const char* TINTF_0CA = "Затемнення фону";
static const char* TINTF_0CB = "Попередження";
static const char* TINTF_0CC = "хвилини";
static const char* TINTF_0CD = "хвилина";
static const char* TINTF_0CE = "Малювання";
static const char* TINTF_0CF = "Заливка";
static const char* TINTF_0D0 = DFTINTF_0D0;
static const char* TINTF_0D1 = "Звук спочатку";
static const char* TINTF_0D2 = "Обмежити гучність";
static const char* TINTF_0D3 = "Коеф. швидкості";
static const char* TINTF_0D4 = "Налаштування дисплея";
static const char* TINTF_0D5 = "Яскравість (0 - виключити)";
static const char* TINTF_0D6 = "Яскравість з викл. лампою (0 - виключити)";
static const char* TINTF_0D7 = "24х годинний формат";
static const char* TINTF_0D8 = "Ведучий 0";
static const char* TINTF_0D9 = "Очистити";
static const char* TINTF_0DA = "Сканувати";
static const char* TINTF_0DB = "FTP";
static const char* TINTF_0DC = "Управління";
static const char* TINTF_0DD = "Тип шкали";
static const char* TINTF_0DE = "Колір шкали";
static const char* TINTF_0DF = "Колір тексту";
static const char* TINTF_0E0 = "Показувати температуру";
static const char* TINTF_0E1 = "Впевнені?";
static const char* TINTF_0E2 = "Трансляція";
static const char* TINTF_0E3 = "Режим";
static const char* TINTF_0E4 = "E1.31";
static const char* TINTF_0E5 = "SoulMate";
static const char* TINTF_0E6 = "Блок.Ефект";
static const char* TINTF_0E7 = "Розмітка";
static const char* TINTF_0E8 = "Початковий Universe";
static const char* TINTF_0E9 = "Завантажити конфіг кнопок";
static const char* TINTF_0EA = "Нічник";
static const char* TINTF_0EB = "Світильник";
static const char* TINTF_0EC = "У будь-якому випадку";
static const char* TINTF_0ED = "Промовити час";
static const char* TINTF_0EE = "Відключено";
static const char* TINTF_0EF = "Вертикальна";
static const char* TINTF_0F0 = "Горизонтальна";

static const char* TINTF_gpiocfg = "GPIO Configuration";
static const char* TINTF_ledstrip = "LED strip layout";
static const char* TINTF_f_restore_state = "Restore state on power-on";

static const char* NY_MDG_STRING1 = "До нового року залишилось %d %s";
static const char* NY_MDG_STRING2 = "З новим %d роком!";
//-------------------------

#endif