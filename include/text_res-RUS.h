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
static const char* TEFF_001 = "Белая лампа";
static const char* TEFF_002 = "Цвета";
static const char* TEFF_003 = "Радуга 2D";
static const char* TEFF_004 = "Конфетти";
static const char* TEFF_005 = "Цветные драже";
static const char* TEFF_006 = "Метель, Звездопад, Метеоры";
static const char* TEFF_007 = "New Матрица";
static const char* TEFF_008 = "Акварель";
static const char* TEFF_009 = "Светлячки со шлейфом";
static const char* TEFF_010 = "Блуждающий кубик";
static const char* TEFF_011 = "Пульс";
static const char* TEFF_012 = "Эффектопад";
static const char* TEFF_013 = "Неопалимая купина"; 
static const char* TEFF_014 = "Пейнтбол";
static const char* TEFF_015 = "Эффекты 3D-шума";
static const char* TEFF_016 = "Ёлки-Палки";
static const char* TEFF_017 = "Пятнашки";
static const char* TEFF_018 = "Тикси Ленд";
static const char* TEFF_019 = "Н.Тесла";
static const char* TEFF_020 = "Осцилятор";
static const char* TEFF_021 = "Шторм";
static const char* TEFF_022 = "Фея";
static const char* TEFF_023 = "Источник";
static const char* TEFF_024 = "Прыгающие мячики";
static const char* TEFF_025 = "Синусоид";
static const char* TEFF_026 = "Метасферы";
static const char* TEFF_027 = "Спираль";
static const char* TEFF_028 = "Кометы, Парящие Огни +";
static const char* TEFF_029 = "Бульбулятор";
static const char* TEFF_030 = "Призмата";
static const char* TEFF_031 = "Стая и Хищник";
static const char* TEFF_032 = "Водоворот";
static const char* TEFF_033 = "Дрифт";
static const char* TEFF_034 = "Попкорн";
static const char* TEFF_035 = "Мерцание";
static const char* TEFF_036 = "Радар";
static const char* TEFF_037 = "Волны";
static const char* TEFF_038 = "Огонь 2012";
static const char* TEFF_039 = "Бенгальские Огни"; 
static const char* TEFF_040 = "Шары"; 
static const char* TEFF_041 = "ДНК";
static const char* TEFF_042 = "Огонь 2018";
static const char* TEFF_043 = "Кодовый замок";
static const char* TEFF_044 = "Куб 2D";
static const char* TEFF_045 = "Магма";
static const char* TEFF_046 = "Пикассо";
static const char* TEFF_047 = "Звездный Десант";
static const char* TEFF_048 = "Флаги";
static const char* TEFF_049 = "Прыгуны";
static const char* TEFF_050 = "Вихри";
static const char* TEFF_051 = "Звезды";
static const char* TEFF_052 = "Блики на воде, Цвета";
static const char* TEFF_053 = "Фейерверк";
static const char* TEFF_054 = "Тихий Океан";
static const char* TEFF_055 = "Вышиванка";
static const char* TEFF_056 = "Цветной шум";
static const char* TEFF_057 = "Мотыльки";
static const char* TEFF_058 = "Тени";
static const char* TEFF_059 = "Узоры";
static const char* TEFF_060 = "Стрелки";
static const char* TEFF_061 = "Дикие шарики";
static const char* TEFF_062 = "Притяжение";
static const char* TEFF_063 = "Змейки";
static const char* TEFF_064 = "Nexus"; 
static const char* TEFF_065 = "Лабиринт";
static const char* TEFF_066 = "Вьющийся Цвет";
static const char* TEFF_067 = "Северное Сияние";
static const char* TEFF_068 = "Цветение";
static const char* TEFF_069 = "Змеиный Остров"; 
static const char* TEFF_070 = "Детские Сны";
static const char* TEFF_071 = "Меташары";
static const char* TEFF_072 = "Лавовая лампа";
static const char* TEFF_073 = "Космо-Гонщик";
static const char* TEFF_074 = "Дым";
static const char* TEFF_075 = "Огненная Лампа";
static const char* TEFF_076 = "Мираж";
static const char* TEFF_250 = "Часы";
static const char* TEFF_254 = "Частотный анализатор";
static const char* TEFF_255 = "Осциллограф";

//-------------------------
#define DFTINTF_00A "Эффект"
#define DFTINTF_020 "Микрофон"
#define DFTINTF_084 "Палитра"
#define DFTINTF_088 "Масштаб"
#define DFTINTF_0D0 "Цвет"
#define DFTINTF_0D1 "Палитра/Масштаб"
#define DFTINTF_0D2 "Эффект (0 = циклично)"
#define DFTINTF_0D3 "Смещение цвета (0 = циклично)"
#define DFTINTF_0D4 "Cубпиксель"
#define DFTINTF_0D5 "Количество"
#define DFTINTF_0D6 "Генератор цвета"
#define DFTINTF_0D7 "Оттенок"
#define DFTINTF_0D8 "Жогово!"
#define DFTINTF_0D9 "Количество / Цвет(ночнник)"
#define DFTINTF_0DA "Насыщенность"
#define DFTINTF_0DB "Усиление"
#define DFTINTF_0DC "Развертка"
#define DFTINTF_0DD "Пульсация"
#define DFTINTF_0DE "Размытие"
#define DFTINTF_0DF "Порхание"
#define DFTINTF_0E0 "Ночник"
#define DFTINTF_0E1 "Кольца"
#define DFTINTF_0E2 "Блики (0:Откл.)"
#define DFTINTF_0E3 "Режим"
#define DFTINTF_0E4 "Линейная скорость"
#define DFTINTF_0E5 "Режим (1 = случайный)"
#define DFTINTF_0E6 "Смещение"
#define DFTINTF_0E7 "Количество/Направление/Положение"
#define DFTINTF_0E8 "Хищник"
#define DFTINTF_0E9 "Масса"
#define DFTINTF_0EA "Скорость (Верт.)"
#define DFTINTF_0EB "Скорость (Гориз.)"
#define DFTINTF_0EC "Узор (0 = случайный)"
#define DFTINTF_0ED "Затенение"
#define DFTINTF_0EE "Энергия"
#define DFTINTF_0EF "Плотность"
#define DFTINTF_0F0 "Цвет/Вращение"
#define DFTINTF_0F1 "Размер/Шлейф(1-85-170-255)"
#define DFTINTF_0F2 "Частота запусков"
#define DFTINTF_0F3 "Поворот"
#define DFTINTF_0F4 "Палитра (0 = генератор)"
#define DFTINTF_0F5 "Сглаживание"
#define DFTINTF_0F6 "Заполнение"
#define DFTINTF_0F7 "Толщина"
#define DFTINTF_0F9 "Поведение: 1/2"
#define DFTINTF_0FA "Цвет: 1/2"
#define DFTINTF_0FB "Палитра (0 = без цвета)"
#define DFTINTF_0FC "Цвет (1 = случайный, 255 = белый)"
#define DFTINTF_0FD "Снегопад/Звездопад/Метеоры"
#define DFTINTF_0FE "Кол-во"
#define DFTINTF_0FF "Тип 1/2, Размытие 3/4"
#define DFTINTF_100 "Палитра (0 = случайный цвет)"
#define DFTINTF_101 "Цвет 1 (0 = циклично)"
#define DFTINTF_102 "Тучка"
#define DFTINTF_103 "Размытие (0 = нет)"
#define DFTINTF_104 "Размах"
#define DFTINTF_105 "Фильтр"
#define DFTINTF_106 "Физика"
#define DFTINTF_107 "Круги"
#define DFTINTF_108 "Вариант 1/2"
#define DFTINTF_109 "Вспышки"
#define DFTINTF_10A "Глюк"
#define DFTINTF_10B "Шторм"
#define DFTINTF_10C "Количество снарядов"
#define DFTINTF_10D "Размер (ширина)"
#define DFTINTF_10E "Сдвиг цвета (0 = случайный)"
#define DFTINTF_10F "Размер (высота)"
#define DFTINTF_110 "Классика"
#define DFTINTF_111 "Один цвет"
#define DFTINTF_112 "Обратные Цвета"
#define DFTINTF_113 "Цвет 2 (0 = циклично)"
#define DFTINTF_114 "Качество/Скорость"
#define DFTINTF_115 "Размытие (1-4 с ветром)"
#define DFTINTF_116 "Перемещение по синусу"
#define DFTINTF_117 "Направление (0 = циклично)"
#define DFTINTF_118 "Порог сигнала"
#define DFTINTF_11A "Цвет (1 = циклично)"

/** набор строк с текстовыми константами интерфейса
 */
static const char* TINTF_000 = "Эффекты";
static const char* TINTF_001 = "Вывод текста";
static const char* TINTF_002 = "Настройки";
static const char* TINTF_in_sel_lst = "Включен в список выбора";
static const char* TINTF_in_demo = "Включен в Демо";
static const char* TINTF_005 = "Копировать";
static const char* TINTF_006 = "Удалить";
static const char* TINTF_007 = "Обновить список эффектов";
static const char* TINTF_Save = "Сохранить";
static const char* TINTF_009 = "Управление списком эффектов";
static const char* TINTF_00A = DFTINTF_00A;
static const char* TINTF_00B = "Выход";
static const char* TINTF_00C = "Глоб. яркость";
static const char* TINTF_00D = "Яркость";
static const char* TINTF_00E = "Включить";
static const char* TINTF_00F = "Демо";
static const char* TINTF_010 = "Гл.Яркость";
static const char* TINTF_011 = "События";
static const char* TINTF_012 = "Микр.";
static const char* TINTF_013 = "Кнопка";
static const char* TINTF_014 = "Еще...";
static const char* TINTF_015 = "<<<";
static const char* TINTF_016 = ">>>";
static const char* TINTF_017 = "Ожидает...";
static const char* TINTF_018 = "Конфигурации";
static const char* TINTF_019 = "Загрузить";
static const char* TINTF_01A = "Конфигурация";
static const char* TINTF_01B = "Создать";
static const char* TINTF_01C = "Вывести на лампе";
static const char* TINTF_01D = "Текст";
static const char* TINTF_01E = "Цвет сообщения";
static const char* TINTF_01F = "Отправить";
static const char* TINTF_020 = DFTINTF_020;
static const char* TINTF_021 = "Микрофон активен";
static const char* TINTF_022 = "Коэф. коррекции нуля";
static const char* TINTF_023 = "Уровень шума, ед";
static const char* TINTF_024 = "Шумодав";
static const char* TINTF_025 = "Калибровка микрофона";
static const char* TINTF_026 = "Включите микрофон";
static const char* TINTF_027 = "... в процессе ...";
static const char* TINTF_028 = "WiFi";
static const char* TINTF_029 = "Клиент (STA)";
static const char* TINTF_02A = "Настройки WiFi-клиента";
static const char* TINTF_02B = "Имя лампы (mDNS Hostname/AP-SSID)";
static const char* TINTF_02C = "WiFi SSID";
static const char* TINTF_02D = "Password";
static const char* TINTF_02E = "Подключиться";
static const char* TINTF_02F = "Точка доступа (AP)";
static const char* TINTF_030 = "Отображение";
static const char* TINTF_031 = "Настройки WiFi и точки доступа";
static const char* TINTF_032 = "В режиме AP лампа всегда работает как точка доступа и не будет подключаться к другим WiFi-сетям, STA - режим клиента, AP+STA - смешанный";
static const char* TINTF_033 = "Режим WiFi";
static const char* TINTF_034 = "Защитить AP паролем";
static const char* TINTF_035 = "MQTT";
static const char* TINTF_036 = "MQTT host";
static const char* TINTF_037 = "MQTT port";
static const char* TINTF_038 = "User";
static const char* TINTF_039 = "Интервал mqtt, сек";
static const char* TINTF_03A = "Подключение";
static const char* TINTF_03B = "Отзеркаливание H";
static const char* TINTF_03C = "Отзеркаливание V";
static const char* TINTF_03D = "Плавное переключение эффектов";
static const char* TINTF_03E = "Случайный эффект в Демо";
static const char* TINTF_03F = "Смена эффекта в Демо, сек";
static const char* TINTF_040 = "Сортировка эффектов";
static const char* TINTF_041 = "Копии под оригинальным";
static const char* TINTF_042 = "Копии в конце";
static const char* TINTF_043 = "В порядке индекса";
static const char* TINTF_044 = "Скорость прокрутки текста";
static const char* TINTF_045 = "Смещение вывода текста";
static const char* TINTF_046 = "Смешанный (AP+STA)";
static const char* TINTF_047 = "Не выводить";
static const char* TINTF_048 = "Каждый час";
static const char* TINTF_049 = "Каждые полчаса";
static const char* TINTF_04A = "Каждые 15 минут";
static const char* TINTF_04B = "Каждые 10 минут";
static const char* TINTF_04C = "Каждые 5 минут";
static const char* TINTF_04D = "Каждую минуту";
static const char* TINTF_04E = "Новогоднее поздравление";
static const char* TINTF_04F = "Период вывода в минутах (0 - не выводить)";
static const char* TINTF_050 = "Дата/время нового года в формате YYYY-MM-DDThh:mm";
static const char* TINTF_051 = "Дата / Время / Часовая зона";
static const char* TINTF_052 = "Установки часовой зоны. Правила смены поясного/сезонного времени применяются автоматически, ручной коррекции не требуется. Если в вашей зоны нет в списке, можно выбрать общую зону сдвига от Гринвича";
static const char* TINTF_053 = "Часовая зона";
static const char* TINTF_054 = "Резервный NTP-сервер (не обязательно)";
static const char* TINTF_055 = "Дата/время в формате YYYY-MM-DDThh:mm:ss (без интернета, если пусто - время с устройства)";
static const char* TINTF_056 = "Обновление";
static const char* TINTF_057 = "Обновление по ОТА-PIO";
static const char* TINTF_058 = "Начать";
static const char* TINTF_059 = "Загрузка прошивки";
static const char* TINTF_05A = "Upload";
static const char* TINTF_05B = "Событие";
static const char* TINTF_05C = "Редактировать";
static const char* TINTF_05D = "Добавить";
static const char* TINTF_05E = "Активно";
static const char* TINTF_05F = "Тип события";
static const char* TINTF_060 = "Включить лампу";
static const char* TINTF_061 = "Выключить лампу";
static const char* TINTF_062 = "Режим DEMO";
static const char* TINTF_063 = "Будильник";
static const char* TINTF_064 = "Загрузка конф. лампы";
static const char* TINTF_065 = "Загрузка конф. эффектов";
static const char* TINTF_066 = "Загрузка конф. событий";
static const char* TINTF_067 = "Вывести текст";
static const char* TINTF_068 = "Вывести время";
static const char* TINTF_069 = "Состояние пина";
static const char* TINTF_06A = "Включить AUX";
static const char* TINTF_06B = "Выключить AUX";
static const char* TINTF_06C = "Переключить AUX";
static const char* TINTF_06D = "Дата/время события";
static const char* TINTF_06E = "Повтор, мин";
static const char* TINTF_06F = "Останов через, мин";
static const char* TINTF_070 = "Параметр (текст)";
static const char* TINTF_071 = "Повтор, дни недели";
static const char* TINTF_072 = "Понедельник";
static const char* TINTF_073 = "Вторник";
static const char* TINTF_074 = "Среда";
static const char* TINTF_075 = "Четверг";
static const char* TINTF_076 = "Пятница";
static const char* TINTF_077 = "Суббота";
static const char* TINTF_078 = "Воскресенье";
static const char* TINTF_079 = "Обновить";
static const char* TINTF_07A = "Действие";
static const char* TINTF_07B = "Кнопка активна";
static const char* TINTF_07C = "OFF/ON";
static const char* TINTF_07D = "Удержание";
static const char* TINTF_07E = "Нажатия";
static const char* TINTF_07F = "Однократно";
static const char* TINTF_080 = "Огненная лампа";
#if defined(EMBUI_USE_FTP) && defined(EMBUI_USE_MQTT)
static const char* TINTF_081 = "WiFi, MQTT & FTP";
#elif defined(EMBUI_USE_MQTT)
static const char* TINTF_081 = "WiFi & MQTT";
#elif defined(EMBUI_USE_FTP)
static const char* TINTF_081 = "WiFi & FTP";
#else
static const char* TINTF_081 = "WiFi";
#endif
static const char* TINTF_082 = "Другие";
static const char* TINTF_083 = "Очищать лампу при смене эффектов";
static const char* TINTF_084 = DFTINTF_084;
static const char* TINTF_085 = "В алфавитном порядке (внутренние)";
static const char* TINTF_086 = "События активны";
static const char* TINTF_087 = "Скорость";
static const char* TINTF_088 = DFTINTF_088;
static const char* TINTF_effrename = "Переименовать эффект";
static const char* TINTF_08A = "В алфавитном порядке (конфигурация)";
static const char* TINTF_08B = "Сброс установок эффекта";
static const char* TINTF_08C = "Префикс топиков";
static const char* TINTF_08D = "Сортировка по микрофону";
static const char* TINTF_08E = "Отладка";
static const char* TINTF_08F = "Настройки ESP";
static const char* TINTF_090 = "Нумерация в списке эффектов";
static const char* TINTF_091 = "Символ микрофона в списке";
static const char* TINTF_092 = "Настройка пинов";
static const char* TINTF_093 = "Показывать системное меню";
static const char* TINTF_094 = "Пин кнопки";
static const char* TINTF_095 = "Лимит по току (mA)";
static const char* TINTF_096 = "Перезагрузка";
static const char* TINTF_097 = "Пин RX плеера";
static const char* TINTF_098 = "Пин TX плеера";
static const char* TINTF_099 = "DFPlayer";
static const char* TINTF_09A = "Название эффекта в Демо";
static const char* TINTF_09B = "Громкость";
static const char* TINTF_09C = "Время";
static const char* TINTF_09D = "Имя эффекта";
static const char* TINTF_09E = "Эффект(сброс)";
static const char* TINTF_09F = "Отключен";
static const char* TINTF_0A0 = "Первый";
static const char* TINTF_0A1 = "Случайный";
static const char* TINTF_0A2 = "Случайный MP3";
static const char* TINTF_0A3 = "Звук будильника";
static const char* TINTF_0A4 = "Второй";
static const char* TINTF_0A5 = "Третий";
static const char* TINTF_0A6 = "Четвертый";
static const char* TINTF_0A7 = "Пятый";
static const char* TINTF_0A8 = "Эквалайзер";
static const char* TINTF_0A9 = "Номальный";
static const char* TINTF_0AA = "Поп";
static const char* TINTF_0AB = "Рок";
static const char* TINTF_0AC = "Джаз";
static const char* TINTF_0AD = "Классика";
static const char* TINTF_0AE = "Бас";
static const char* TINTF_0AF = "MP3 плеер";
static const char* TINTF_0B0 = "Кол-во файлов в папке MP3";
static const char* TINTF_0B1 = "Режимы озвучивания";
static const char* TINTF_0B2 = "Папка\\номер звукового файла (примеры: MP3\\17, 5\\19)";
static const char* TINTF_0B3 = "Ограничить громкость будильника";
static const char* TINTF_0B4 = "По умолчанию";
static const char* TINTF_0B5 = "Удалить из списка";
static const char* TINTF_0B6 = "Отключено";
static const char* TINTF_0B7 = "Вариант 1";
static const char* TINTF_0B8 = "Вариант 2";
static const char* TINTF_0B9 = "Параметр";
static const char* TINTF_0BA = "Будильник Рассвет";
static const char* TINTF_0BB = "Длительность рассвета, мин";
static const char* TINTF_0BC = "Светить после рассвета, мин";
static const char* TINTF_0BD = "<";
static const char* TINTF_0BE = ">";
static const char* TINTF_0BF = "<+5";
static const char* TINTF_0C0 = "+5>";
static const char* TINTF_0C1 = "секунд";
static const char* TINTF_0C2 = "минут";
static const char* TINTF_0C3 = "часов";
static const char* TINTF_0C4 = "дней";
static const char* TINTF_0C5 = "дня";
static const char* TINTF_0C6 = "день";
static const char* TINTF_0C7 = "часа";
static const char* TINTF_0C8 = "час";
static const char* TINTF_0C9 = "Случ.";
static const char* TINTF_0CA = "Затемнение фона";
static const char* TINTF_0CB = "Предупреждение";
static const char* TINTF_0CC = "минуты";
static const char* TINTF_0CD = "минута";
static const char* TINTF_0CE = "Рисование";
static const char* TINTF_0CF = "Заливка";
static const char* TINTF_0D0 = DFTINTF_0D0;
static const char* TINTF_0D1 = "Звук вначале";
static const char* TINTF_0D2 = "Ограничить громкость";
static const char* TINTF_0D3 = "Коэф. скорости (1.0 по умолчанию)";
static const char* TINTF_0D4 = "Настройки дисплея";
static const char* TINTF_0D5 = "Яркость (0 - выключить)";
static const char* TINTF_0D6 = "Яркость с выкл. лампой (0 - выключить)";
static const char* TINTF_0D7 = "24х часовой формат";
static const char* TINTF_0D8 = "Ведущий 0";
static const char* TINTF_0D9 = "Очистить";
static const char* TINTF_0DA = "Сканировать";
static const char* TINTF_0DB = "FTP";
static const char* TINTF_0DC = "Управление";
static const char* TINTF_0DD = "Шкала индикатора";
static const char* TINTF_0DE = "Цвет шкалы";
static const char* TINTF_0DF = "Цвет текста";
static const char* TINTF_0E0 = "Отображение температуры";
static const char* TINTF_0E1 = "Уверены?";
static const char* TINTF_0E2 = "Трансляция";
static const char* TINTF_0E3 = "Режим";
static const char* TINTF_0E4 = "E1.31";
static const char* TINTF_0E5 = "SoulMate";
static const char* TINTF_0E6 = "Блок.Эффект";
static const char* TINTF_0E7 = "Разметка";
static const char* TINTF_0E8 = "Начальный Universe";
static const char* TINTF_0E9 = "Загрузка конф. кнопок";
static const char* TINTF_0EA = "Ночник";
static const char* TINTF_0EB = "Светильник";
static const char* TINTF_0EC = "В любом состоянии";
static const char* TINTF_0ED = "Озвучивать время";
static const char* TINTF_0EE = "Отключена";
static const char* TINTF_0EF = "Вертикальная";
static const char* TINTF_0F0 = "Горизонтальная";

static const char* TINTF_gpiocfg = "Конфигурация GPIO";
static const char* TINTF_ledstrip = "Топология LED ленты";
static const char* TINTF_f_restore_state = "Восстанавливать состояние при включении";

static const char* NY_MDG_STRING1 = "До нового года осталось %d %s";
static const char* NY_MDG_STRING2 = "C новым %d годом!";
//-------------------------

#endif