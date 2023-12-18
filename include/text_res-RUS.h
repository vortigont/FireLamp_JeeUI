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
static constexpr const char* TEFF_000 = "";  // "пустышка"
static constexpr const char* TEFF_001 = "Белая лампа";
static constexpr const char* TEFF_002 = "Цвета";
static constexpr const char* TEFF_003 = "Радуга 2D";
static constexpr const char* TEFF_004 = "Конфетти";
static constexpr const char* TEFF_005 = "Цветные драже";
static constexpr const char* TEFF_006 = "Метель, Звездопад, Метеоры";
static constexpr const char* TEFF_007 = "New Матрица";
static constexpr const char* TEFF_008 = "Акварель";
static constexpr const char* TEFF_009 = "Светлячки со шлейфом";
static constexpr const char* TEFF_010 = "Блуждающий кубик";
static constexpr const char* TEFF_011 = "Пульс";
static constexpr const char* TEFF_012 = "Эффектопад";
static constexpr const char* TEFF_013 = "Неопалимая купина"; 
static constexpr const char* TEFF_014 = "Пейнтбол";
static constexpr const char* TEFF_015 = "Эффекты 3D-шума";
static constexpr const char* TEFF_016 = "Ёлки-Палки";
static constexpr const char* TEFF_017 = "Пятнашки";
static constexpr const char* TEFF_018 = "Тикси Ленд";
static constexpr const char* TEFF_019 = "Н.Тесла";
static constexpr const char* TEFF_020 = "Осциллятор";
static constexpr const char* TEFF_021 = "Шторм";
static constexpr const char* TEFF_022 = "Фея";
static constexpr const char* TEFF_023 = "Источник";
static constexpr const char* TEFF_024 = "Прыгающие мячики";
static constexpr const char* TEFF_025 = "Синусоид";
static constexpr const char* TEFF_026 = "Метасферы";
static constexpr const char* TEFF_027 = "Спираль";
static constexpr const char* TEFF_028 = "Кометы, Парящие Огни +";
static constexpr const char* TEFF_029 = "Бульбулятор";
static constexpr const char* TEFF_030 = "Призмата";
static constexpr const char* TEFF_031 = "Стая и Хищник";
static constexpr const char* TEFF_032 = "Водоворот";
static constexpr const char* TEFF_033 = "Дрифт";
static constexpr const char* TEFF_034 = "Попкорн";
static constexpr const char* TEFF_035 = "Мерцание";
static constexpr const char* TEFF_036 = "Радар";
static constexpr const char* TEFF_037 = "Волны";
static constexpr const char* TEFF_038 = "Огонь 2012";
static constexpr const char* TEFF_039 = "Бенгальские Огни"; 
static constexpr const char* TEFF_040 = "Шары"; 
static constexpr const char* TEFF_041 = "ДНК";
static constexpr const char* TEFF_042 = "Огонь 2018";
static constexpr const char* TEFF_043 = "Кодовый замок";
static constexpr const char* TEFF_044 = "Куб 2D";
static constexpr const char* TEFF_045 = "Магма";
static constexpr const char* TEFF_046 = "Пикассо";
static constexpr const char* TEFF_047 = "Звездный Десант";
static constexpr const char* TEFF_048 = "Флаги";
static constexpr const char* TEFF_049 = "Прыгуны";
static constexpr const char* TEFF_050 = "Вихри";
static constexpr const char* TEFF_051 = "Звезды";
static constexpr const char* TEFF_052 = "Блики на воде, Цвета";
static constexpr const char* TEFF_053 = "Фейерверк";
static constexpr const char* TEFF_054 = "Тихий Океан";
static constexpr const char* TEFF_055 = "Вышиванка";
static constexpr const char* TEFF_056 = "Цветной шум";
static constexpr const char* TEFF_057 = "Мотыльки";
static constexpr const char* TEFF_058 = "Тени";
static constexpr const char* TEFF_059 = "Узоры";
static constexpr const char* TEFF_060 = "Стрелки";
static constexpr const char* TEFF_061 = "Дикие шарики";
static constexpr const char* TEFF_062 = "Притяжение";
static constexpr const char* TEFF_063 = "Змейки";
static constexpr const char* TEFF_064 = "Nexus"; 
static constexpr const char* TEFF_065 = "Лабиринт";
static constexpr const char* TEFF_066 = "Вьющийся Цвет";
static constexpr const char* TEFF_067 = "Северное Сияние";
static constexpr const char* TEFF_068 = "Цветение";
static constexpr const char* TEFF_069 = "Змеиный Остров"; 
static constexpr const char* TEFF_070 = "Детские Сны";
static constexpr const char* TEFF_071 = "Меташары";
static constexpr const char* TEFF_072 = "Лавовая лампа";
static constexpr const char* TEFF_073 = "Космо-Гонщик";
static constexpr const char* TEFF_074 = "Дым";
static constexpr const char* TEFF_075 = "Огненная Лампа";
static constexpr const char* TEFF_076 = "Мираж";
static constexpr const char* TEFF_250 = "Часы";
static constexpr const char* TEFF_254 = "Частотный анализатор";
static constexpr const char* TEFF_255 = "Осциллограф";

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
#define DFTINTF_24hrs "24 часа"

/** набор строк с текстовыми константами интерфейса
 */
static constexpr const char* TINTF_000 = "Эффекты";
static constexpr const char* TINTF_001 = "Вывод текста";
static constexpr const char* TINTF_002 = "Настройки";
static constexpr const char* TINTF_in_sel_lst = "Включен в список выбора";
static constexpr const char* TINTF_in_demo = "Включен в Демо";
static constexpr const char* TINTF_005 = "Копировать";
static constexpr const char* TINTF_006 = "Удалить";
static constexpr const char* TINTF_007 = "Обновить список эффектов";
static constexpr const char* TINTF_Save = "Сохранить";
static constexpr const char* TINTF_009 = "Управление списком эффектов";
static constexpr const char* TINTF_00A = DFTINTF_00A;
static constexpr const char* TINTF_exit = "Выход";
static constexpr const char* TINTF_00C = "Глоб. яркость";
static constexpr const char* TINTF_00D = "Яркость";
static constexpr const char* TINTF_00E = "Включить";
static constexpr const char* TINTF_00F = "Демо";
static constexpr const char* TINTF_010 = "Гл.Яркость";
static constexpr const char* TINTF_011 = "События";
static constexpr const char* TINTF_012 = "Микр.";
static constexpr const char* TINTF_013 = "Кнопка";
static constexpr const char* TINTF_014 = "Еще...";
static constexpr const char* TINTF_015 = "<<<";
static constexpr const char* TINTF_016 = ">>>";
static constexpr const char* TINTF_017 = "Ожидает...";
static constexpr const char* TINTF_018 = "Конфигурации";
static constexpr const char* TINTF_019 = "Загрузить";
static constexpr const char* TINTF_01A = "Конфигурация";
static constexpr const char* TINTF_01B = "Создать";
static constexpr const char* TINTF_01C = "Вывести на лампе";
static constexpr const char* TINTF_01D = "Текст";
static constexpr const char* TINTF_01E = "Цвет сообщения";
static constexpr const char* TINTF_01F = "Отправить";
static constexpr const char* TINTF_020 = DFTINTF_020;
static constexpr const char* TINTF_021 = "Микрофон активен";
static constexpr const char* TINTF_022 = "Коэф. коррекции нуля";
static constexpr const char* TINTF_023 = "Уровень шума, ед";
static constexpr const char* TINTF_024 = "Шумодав";
static constexpr const char* TINTF_025 = "Калибровка микрофона";
static constexpr const char* TINTF_026 = "Включите микрофон";
static constexpr const char* TINTF_027 = "... в процессе ...";
static constexpr const char* TINTF_028 = "WiFi";
static constexpr const char* TINTF_029 = "Клиент (STA)";
static constexpr const char* TINTF_02A = "Настройки WiFi-клиента";
static constexpr const char* TINTF_02B = "Имя лампы (mDNS Hostname/AP-SSID)";
static constexpr const char* TINTF_02C = "WiFi SSID";
static constexpr const char* TINTF_02D = "Password";
static constexpr const char* TINTF_02E = "Подключиться";
static constexpr const char* TINTF_02F = "Точка доступа (AP)";
static constexpr const char* TINTF_030 = "Отображение";
static constexpr const char* TINTF_031 = "Настройки WiFi и точки доступа";
static constexpr const char* TINTF_032 = "В режиме AP лампа всегда работает как точка доступа и не будет подключаться к другим WiFi-сетям, STA - режим клиента, AP+STA - смешанный";
static constexpr const char* TINTF_033 = "Режим WiFi";
static constexpr const char* TINTF_034 = "Защитить AP паролем";
static constexpr const char* TINTF_035 = "MQTT";
static constexpr const char* TINTF_036 = "MQTT host";
static constexpr const char* TINTF_037 = "MQTT port";
static constexpr const char* TINTF_038 = "User";
static constexpr const char* TINTF_039 = "Интервал mqtt, сек";
static constexpr const char* TINTF_03A = "Подключение";
static constexpr const char* TINTF_03B = "Отзеркаливание H";
static constexpr const char* TINTF_03C = "Отзеркаливание V";
static constexpr const char* TINTF_03D = "Плавное переключение эффектов";
static constexpr const char* TINTF_03E = "Случайный эффект в Демо";
static constexpr const char* TINTF_03F = "Смена эффекта в Демо, сек";
static constexpr const char* TINTF_040 = "Сортировка эффектов";
static constexpr const char* TINTF_041 = "Копии под оригинальным";
static constexpr const char* TINTF_042 = "Копии в конце";
static constexpr const char* TINTF_043 = "В порядке индекса";
static constexpr const char* TINTF_044 = "Скорость прокрутки текста";
static constexpr const char* TINTF_045 = "Смещение вывода текста";
static constexpr const char* TINTF_046 = "Смешанный (AP+STA)";
static constexpr const char* TINTF_047 = "Не выводить";
static constexpr const char* TINTF_048 = "Каждый час";
static constexpr const char* TINTF_049 = "Каждые полчаса";
static constexpr const char* TINTF_04A = "Каждые 15 минут";
static constexpr const char* TINTF_04B = "Каждые 10 минут";
static constexpr const char* TINTF_04C = "Каждые 5 минут";
static constexpr const char* TINTF_04D = "Каждую минуту";
static constexpr const char* TINTF_04E = "Новогоднее поздравление";
static constexpr const char* TINTF_04F = "Период вывода в минутах (0 - не выводить)";
static constexpr const char* TINTF_050 = "Дата/время нового года в формате YYYY-MM-DDThh:mm";
static constexpr const char* TINTF_051 = "Дата / Время / Часовая зона";
static constexpr const char* TINTF_052 = "Установки часовой зоны. Правила смены поясного/сезонного времени применяются автоматически, ручной коррекции не требуется. Если в вашей зоны нет в списке, можно выбрать общую зону сдвига от Гринвича";
static constexpr const char* TINTF_053 = "Часовая зона";
static constexpr const char* TINTF_054 = "Резервный NTP-сервер (не обязательно)";
static constexpr const char* TINTF_055 = "Дата/время в формате YYYY-MM-DDThh:mm:ss (без интернета, если пусто - время с устройства)";
static constexpr const char* TINTF_056 = "Обновление";
static constexpr const char* TINTF_058 = "Начать";
static constexpr const char* TINTF_059 = "Загрузка прошивки";
static constexpr const char* TINTF_05A = "Upload";
static constexpr const char* TINTF_05B = "Событие";
static constexpr const char* TINTF_05C = "Редактировать";
static constexpr const char* TINTF_05D = "Добавить";
static constexpr const char* TINTF_05E = "Активно";
static constexpr const char* TINTF_05F = "Тип события";
static constexpr const char* TINTF_060 = "Включить лампу";
static constexpr const char* TINTF_061 = "Выключить лампу";
static constexpr const char* TINTF_062 = "Режим DEMO";
static constexpr const char* TINTF_063 = "Будильник";
static constexpr const char* TINTF_064 = "Загрузка конф. лампы";
static constexpr const char* TINTF_065 = "Загрузка конф. эффектов";
static constexpr const char* TINTF_066 = "Загрузка конф. событий";
static constexpr const char* TINTF_067 = "Вывести текст";
static constexpr const char* TINTF_068 = "Вывести время";
static constexpr const char* TINTF_069 = "Состояние пина";
static constexpr const char* TINTF_06A = "Включить AUX";
static constexpr const char* TINTF_06B = "Выключить AUX";
static constexpr const char* TINTF_06C = "Переключить AUX";
static constexpr const char* TINTF_06D = "Дата/время события";
static constexpr const char* TINTF_06E = "Повтор, мин";
static constexpr const char* TINTF_06F = "Останов через, мин";
static constexpr const char* TINTF_070 = "Параметр (текст)";
static constexpr const char* TINTF_071 = "Повтор, дни недели";
static constexpr const char* TINTF_072 = "Понедельник";
static constexpr const char* TINTF_073 = "Вторник";
static constexpr const char* TINTF_074 = "Среда";
static constexpr const char* TINTF_075 = "Четверг";
static constexpr const char* TINTF_076 = "Пятница";
static constexpr const char* TINTF_077 = "Суббота";
static constexpr const char* TINTF_078 = "Воскресенье";
static constexpr const char* TINTF_079 = "Обновить";
static constexpr const char* TINTF_07A = "Действие";
static constexpr const char* TINTF_07B = "Кнопка активна";
static constexpr const char* TINTF_07C = "OFF/ON";
static constexpr const char* TINTF_07D = "Удержание";
static constexpr const char* TINTF_07E = "Нажатия";
static constexpr const char* TINTF_07F = "Однократно";
static constexpr const char* TINTF_080 = "Огненная лампа";
static constexpr const char* TINTF_082 = "Другие";
static constexpr const char* TINTF_083 = "Очищать лампу при смене эффектов";
static constexpr const char* TINTF_084 = DFTINTF_084;
static constexpr const char* TINTF_085 = "В алфавитном порядке (внутренние)";
static constexpr const char* TINTF_086 = "События активны";
static constexpr const char* TINTF_087 = "Скорость";
static constexpr const char* TINTF_088 = DFTINTF_088;
static constexpr const char* TINTF_effrename = "Переименовать эффект";
static constexpr const char* TINTF_08A = "В алфавитном порядке (конфигурация)";
static constexpr const char* TINTF_08B = "Сброс установок эффекта";
static constexpr const char* TINTF_08C = "Префикс топиков";
static constexpr const char* TINTF_08D = "Сортировка по микрофону";
static constexpr const char* TINTF_08E = "Отладка";
static constexpr const char* TINTF_08F = "Настройки ESP";
static constexpr const char* TINTF_090 = "Нумерация в списке эффектов";
static constexpr const char* TINTF_091 = "Символ микрофона в списке";
static constexpr const char* TINTF_092 = "Настройка пинов";
static constexpr const char* TINTF_093 = "Показывать системное меню";
static constexpr const char* TINTF_094 = "Пин кнопки";
//static constexpr const char* TINTF_095 = "Лимит по току (mA)";
static constexpr const char* TINTF_096 = "Перезагрузка";
static constexpr const char* TINTF_097 = "Пин RX плеера";
static constexpr const char* TINTF_098 = "Пин TX плеера";
static constexpr const char* TINTF_099 = "DFPlayer";
static constexpr const char* TINTF_09A = "Название эффекта в Демо";
static constexpr const char* TINTF_09B = "Громкость";
static constexpr const char* TINTF_09C = "Время";
static constexpr const char* TINTF_09D = "Имя эффекта";
static constexpr const char* TINTF_09E = "Эффект(сброс)";
static constexpr const char* TINTF_09F = "Отключен";
static constexpr const char* TINTF_0A0 = "Первый";
static constexpr const char* TINTF_0A1 = "Случайный";
static constexpr const char* TINTF_0A2 = "Случайный MP3";
static constexpr const char* TINTF_0A3 = "Звук будильника";
static constexpr const char* TINTF_0A4 = "Второй";
static constexpr const char* TINTF_0A5 = "Третий";
static constexpr const char* TINTF_0A6 = "Четвертый";
static constexpr const char* TINTF_0A7 = "Пятый";
static constexpr const char* TINTF_0A8 = "Эквалайзер";
static constexpr const char* TINTF_0A9 = "Номальный";
static constexpr const char* TINTF_0AA = "Поп";
static constexpr const char* TINTF_0AB = "Рок";
static constexpr const char* TINTF_0AC = "Джаз";
static constexpr const char* TINTF_0AD = "Классика";
static constexpr const char* TINTF_0AE = "Бас";
static constexpr const char* TINTF_0AF = "MP3 плеер";
static constexpr const char* TINTF_0B0 = "Кол-во файлов в папке MP3";
static constexpr const char* TINTF_0B1 = "Режимы озвучивания";
static constexpr const char* TINTF_0B2 = "Папка\\номер звукового файла (примеры: MP3\\17, 5\\19)";
static constexpr const char* TINTF_0B3 = "Ограничить громкость будильника";
static constexpr const char* TINTF_0B4 = "По умолчанию";
static constexpr const char* TINTF_0B5 = "Удалить из списка";
static constexpr const char* TINTF_0B6 = "Отключено";
static constexpr const char* TINTF_0B7 = "Вариант 1";
static constexpr const char* TINTF_0B8 = "Вариант 2";
static constexpr const char* TINTF_0B9 = "Параметр";
static constexpr const char* TINTF_0BA = "Будильник Рассвет";
static constexpr const char* TINTF_0BB = "Длительность рассвета, мин";
static constexpr const char* TINTF_0BC = "Светить после рассвета, мин";
static constexpr const char* TINTF_0BD = "<";
static constexpr const char* TINTF_0BE = ">";
static constexpr const char* TINTF_0BF = "<+5";
static constexpr const char* TINTF_0C0 = "+5>";
static constexpr const char* TINTF_0C1 = "секунд";
static constexpr const char* TINTF_0C2 = "минут";
static constexpr const char* TINTF_0C3 = "часов";
static constexpr const char* TINTF_0C4 = "дней";
static constexpr const char* TINTF_0C5 = "дня";
static constexpr const char* TINTF_0C6 = "день";
static constexpr const char* TINTF_0C7 = "часа";
static constexpr const char* TINTF_0C8 = "час";
static constexpr const char* TINTF_0C9 = "Случ.";
static constexpr const char* TINTF_0CA = "Затемнение фона";
static constexpr const char* TINTF_0CB = "Предупреждение";
static constexpr const char* TINTF_0CC = "минуты";
static constexpr const char* TINTF_0CD = "минута";
static constexpr const char* TINTF_0CE = "Рисование";
static constexpr const char* TINTF_0CF = "Заливка";
static constexpr const char* TINTF_0D0 = DFTINTF_0D0;
static constexpr const char* TINTF_0D1 = "Звук вначале";
static constexpr const char* TINTF_0D2 = "Ограничить громкость";
static constexpr const char* TINTF_0D3 = "Коэф. скорости (1.0 по умолчанию)";
static constexpr const char* TINTF_0D4 = "Настройки дисплея";
//static constexpr const char* TINTF_0D5 = "Яркость (0 - выключить)";
//static constexpr const char* TINTF_0D6 = "Яркость с выкл. лампой (0 - выключить)";
//static constexpr const char* TINTF_0D7 = "24х часовой формат";
//static constexpr const char* TINTF_0D8 = "Ведущий 0";
static constexpr const char* TINTF_0D9 = "Очистить";
static constexpr const char* TINTF_0DA = "Сканировать";
static constexpr const char* TINTF_0DB = "FTP";
static constexpr const char* TINTF_0DC = "Управление";
static constexpr const char* TINTF_0DD = "Шкала индикатора";
static constexpr const char* TINTF_0DE = "Цвет шкалы";
static constexpr const char* TINTF_0DF = "Цвет текста";
static constexpr const char* TINTF_0E0 = "Отображение температуры";
static constexpr const char* TINTF_0E1 = "Уверены?";
static constexpr const char* TINTF_0E2 = "Трансляция";
static constexpr const char* TINTF_0E3 = "Режим";
static constexpr const char* TINTF_0E4 = "E1.31";
static constexpr const char* TINTF_0E5 = "SoulMate";
static constexpr const char* TINTF_0E6 = "Блок.Эффект";
static constexpr const char* TINTF_0E7 = "Разметка";
static constexpr const char* TINTF_0E8 = "Начальный Universe";
static constexpr const char* TINTF_0E9 = "Загрузка конф. кнопок";
static constexpr const char* TINTF_0EA = "Ночник";
static constexpr const char* TINTF_0EB = "Светильник";
static constexpr const char* TINTF_0EC = "В любом состоянии";
static constexpr const char* TINTF_0ED = "Озвучивать время";
static constexpr const char* TINTF_0EE = "Отключена";
static constexpr const char* TINTF_0EF = "Вертикальная";
static constexpr const char* TINTF_0F0 = "Горизонтальная";

static constexpr const char* TINTF_display_setup = "Настройка LED-дисплея";
static constexpr const char* TINTF_display_type = "Тип LED-матрицы";
static constexpr const char* TINTF_f_restore_state = "Восстанавливать состояние при включении";
static constexpr const char* TINTF_cfg_hub75 = "Конфигурация HUB75 панелей";
static constexpr const char* TINTF_gpiocfg = "Настройка GPIO";
static constexpr const char* TINTF_ledstrip = "Топология LED ленты";
static constexpr const char* TINTF_setup_tm1637 = "Дисплей TM1637";

static constexpr const char* NY_MDG_STRING1 = "До нового года осталось %d %s";
static constexpr const char* NY_MDG_STRING2 = "C новым %d годом!";
//-------------------------

#endif