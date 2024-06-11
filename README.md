__[CHANGELOG](/CHANGELOG.md)__ | [![PlatformIO CI](https://github.com/vortigont/FireLamp_JeeUI/actions/workflows/pio_build.yml/badge.svg)](https://github.com/vortigont/FireLamp_JeeUI/actions/workflows/pio_build.yml)

## Огненная лампа FireLamp_EmbUI
Лампа, гирлянда, информер с поддержкой светодиодных матриц/лент ws2812 и HUB75 RGB панелей.

Деполитизированный форк проекта "огненной" лампы [FireLamp_EmbUI](https://github.com/DmytroKorniienko/FireLamp_EmbUI).

<details>
  <summary>Project Manifest</summary>

Проект был пересобран из старых форков и архивов репозитория пользователей и участников разработки на момент примерно 2021 года. В [исходном](https://github.com/DmytroKorniienko/FireLamp_EmbUI) проекте был полностью вырезан русский язык, задним числом переписана история разработки в Git и удалена соотвествующая тема [форума](https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proekta.1411/). Данный форк это не срез исходного репозитория, хотя сохранил часть общей истории до определенного момента.

</details>

Канал в Telegram для общения пользователей прошивки - [LampDevs](https://t.me/LampDevs)

Группа [ВКонтакте](https://vk.com/club226065124)

Тема на [форуме Гайвера](https://community.alexgyver.ru/threads/wifi-lampa-budilnik-obsuzhdenie-proshivki-firelamp_embui.7257/)



### Галерея

[![ESP32 FireLamp RGB HUB75 Informer panel Demo](https://img.youtube.com/vi/ZrAhoebgl2Q/0.jpg)](https://www.youtube.com/watch?v=ZrAhoebgl2Q)


## Содержание
- [Огненная лампа FireLamp\_EmbUI](#огненная-лампа-firelamp_embui)
  - [Галерея](#галерея)
- [Содержание](#содержание)
  - [WiKi проекта](#wiki-проекта)
  - [Концепт](#концепт)
  - [Библиотеки, используемые в проекте](#библиотеки-используемые-в-проекте)
  - [ESP8266 vs ESP32](#esp8266-vs-esp32)

### WiKi проекта
Актуальную документацию по проекту можно найти в [WiKi](https://github.com/vortigont/EmbUI/wiki)

### Прошивка / сборка проекта
Подробно опции прошивки сборки описаны в [WiKi](https://github.com/vortigont/FireLamp_JeeUI/wiki/%D0%A1%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D0%BF%D1%80%D0%BE%D1%88%D0%B8%D0%B2%D0%BA%D0%B0-%D0%BF%D1%80%D0%BE%D0%B5%D0%BA%D1%82%D0%B0)

### Концепт
Исходный проект был полностью реструктурирован и переписан. Из "огненой лампы" проект трансформировался в универсальный движёк для работы как с адресными лентами так и RGB HUB75 панелями. Проект академический, используется как платформа для разработки отдельных библиотек под платфому ESP32. Целью разработок - создание качественных асинхронных библиотек, классов и компонентов.

В разработке основное внимание уделяется структуре кода, декомпозиции и объектоориентированности в ущерб набору "фич" и количества функций.
Основная идея развития - проект должен быть лекго конфигурируем и полностью настраиваться без необходимости создания дремучих конфиг файлов и пересборок уникальных прошивок. Вся периферия и функционал должен быть максимально гибко настраиваем уже во время работы кода без необходимости пересборки/перепрошивки проекта.
Проект поставляется в виде [бинарных сборок](https://github.com/vortigont/FireLamp_JeeUI/wiki/%D0%A1%D0%B1%D0%BE%D1%80%D0%BA%D0%B0-%D0%B8-%D0%BF%D1%80%D0%BE%D1%88%D0%B8%D0%B2%D0%BA%D0%B0-%D0%BF%D1%80%D0%BE%D0%B5%D0%BA%D1%82%D0%B0), может быть прошит и использован с любым шильдом/платой под HUB75 информеры/ленты. назначение выводов под периферию выполняется целиком из WebUI.

### Библиотеки, используемые в проекте
В процессе работы на проектом были разработаны с нуля или адаптированы следующие библиотеки:
 - [EmbUI](https://github.com/vortigont/EmbUI) - Embedded WebUI framefork. Building interactive UI based on websockets for your Arduino projects
 - [ESPAsyncButton](https://github.com/vortigont/ESPAsyncButton) - Event-based asynchronous button library for ESP32 family chips. It generate events for various button press patterns, like short/long press, clicks, autorepeat, multiple clicks
 - [LedFB](https://github.com/vortigont/LedFB) - template-based LED framebuffer library for FastLED / AdafruitGFX API
 - [DFMiniMp3](https://github.com/vortigont/DFMiniMp3) - библиотека DFPlayer'а от [Makuna](https://github.com/Makuna/DFMiniMp3), адаптированная для run-time конфигурации
 - [esp32-flashz](https://github.com/vortigont/esp32-flashz) - zlib compressed OTA firmware update for ESP32. Implements on-the-fly OTA images decompression on upload/flashing
 - [TM1637](https://github.com/vortigont/TM1637) - адаптированная библиотека от [AKJ7](https://github.com/AKJ7/TM1637) переработанная для дополнительных эффектов анимации
 - [ESP32-DNSServerAsync](https://github.com/vortigont/ESP32-DNSServerAsync) - Async version of ESP32's DNSServer based on AsyncUDP lib 

### ESP8266 vs ESP32
Разработка ведется под контроллеры семейства **esp32**. ESP8266 морально устарел, было принято решение не тратить время на поддержку совместимости и уделить больше времени и ресурсов актуальной платформе.
Последний стабильно работающий на контроллерах 8266 выпуск проекта - версия [v3.1.1](../../releases/tag/v3.1.1), прочий код в ветке [legacy](../../tree/legacy).


