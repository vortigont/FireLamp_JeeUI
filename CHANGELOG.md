# Change Log

## 3.4.0 (2023-12-01)
- Update EmbUI resources
- Update Builder scripts
- refactoring MQTT/HTTP API
   - make API for MQTT/HTTP compatible with WebSocket data messages format.
   - Integrate message exchange between APIs.
   - Removed obsolete code, i.e. sync_parameters(), http callback actions etc...
- refactoring code related to EmbUI API changes
   - deprecate old-style weak functions, replaced with assignable action handlers
   - sting literals refactoring to match EmbUI api change
   - major refactoring for interface function names, const literals, etc...
- avoid reboot on led stripe reconfig, when saving matrix config if gpio has not changed then avoid useless reboot
- external devices
   - removed TM1637_CLOCK define from config files, tm1637 suport is built-in by default, module is enabled if it's pins are defined through webui config
- WebUI
   - implement WebUI configuration page for HUB75 parameters setup
   - add tiled configuration setup for ws2812 matrixes
   - run-time switch between ws2812 and HUB75 panel output
   - hide obsolete functionality "Text", "Events"
   - removed "Кол-во файлов в папке MP3" setting from UI
+ HUB75 panels output
   - implement HUB75 backend engine for HUB75 panels via HUB75-I2S-DMA lib
   - hub75 configuration via API
   - add NO_CIE1931 option for HUB75 I2S lib
+ LedFB libarary
   -  implement LEDDisplay class
      output device access is now hidden behind LEDDisplay class,
      which provides methods to access canvas/overlay as memory-backed buffers.
      Output device specific implementation and overlay mixing is of no dependency on other components.
   - OverlayEngine class that manages canvas buffer binding to end device and overlay operations such as allocation, blending, back-buffering
   - introduce 'persistence' flag for effect indicating that it uses canvas buffer as pixel data storage and it should persist on ovelay operations (using back-buffer for this)
   - add support for LedTiles transformations.
      - Led matrixes or stripes of same topology could be combined into tiles
      - same kind of topology transformations supported for tiles as for Led stripe
- reworkied "Drawing" feature using OverlayEngine, simplified packet data structures for pixel data
- segregate effect calculation into sepparate Taks
   - a Task is created on-demand to run effect calculations and render result via OverlayEngine
- Effects fixes
   - Noise3dMap::fillNoise was using wrong x,y mapping
   - refactoring for EffectRadialFire, make it dimension agnostic
   - removed old "Clock Effect"
   - EffectPacific removed static vars, fixed color application
   - EffectSinusoid3 clear buffer on load
   - EffectBBalls limit num of balls to panel width
   - 3DNoise effect running on non-square panels
   - EffectComet runnning on non sq pane
   - EffectRingsLock - fix crashing when running on panel, some internal restructuring
   - EffectNoise crash on non rectangular canvas
   - EffectMaze was crashing on non-rect canvas
   - EffectBBalls changing number of balls could crash some times
   - refine EffectSmokeballs
     - максимальное число огней оганиченно ширина/4
     - огни бегают по всей ширине широкоформатной матрицы
     - добавлен контрол высоты огней
     - мелкие оптимизации
   - EffectLiquidLamp fix sporadic crashes on filter change
- platformio.ini - update libs definitions, set SemVers for known external libs


## 3.3.0 (2023-08-24)
- removed lamp configurations editor
- adjust UI related code to match EmbUI BAPI changes
- removed 8266 legacy - F() macro, FPSTR macro, PROGMEM
- fader changes
  - adjust lower fader brightness with FADE_LOWBRTFRACT parameter,
  - fixed brightness for fast fading
- use run-time tunable for save/restore on/off/demo state on power-up, Closes #24
- fix fire2018 dependence on 'brightness' variable, Closes #28
- reworked brightness control, add luma curves lib
   - removed Global/Individual brightness for effects, using only Global brightness from now on
   - ignore first "special" control for brightness in the code
   - introduce selectable luma curve correction for brightness, applies globally, could be overriden per each effect config
   - adjusted API/actions to work with global brightness only
   - brightness could be scaled to any range, i.e. 0%-100%, or 1-20 (default)
- includes alignment
- tm1637 fix for blinking dots
- add thread safety WA for effect switching via '<<','>>' UI buttons
- fix race condition in EffectShadows
- DFPlayer - fix crash on start with undefined pins
- LAMP power ON/OFF consistency
  - EffectWorker will reset active effect on switch-on
  - LAMP object will force-wipe the matrix when shutting OFF
    - when led fader is done
    - when switch-off without fader
    - when LAMP::frameShow() has been called but lamp is no state for drawing anything (it fixes issues with leftover artifacts when not using fader)
    - when LAMP::effectsTick() ended up in a no rerun state for effectsTimer scheduler
- fix potential out-of-bound access crash in GradientPaletteList::[] operator
- implement EffectWorker::switchEffect
    - regression when switching effects without fader
    - controls settings were not saved on switch
    - sometimes lamp crashed on switch
    - removed EffectCalc::pre_init()
    - replaced several similar methods for delayed effect switching with a single EffectWorker::switchEffect()
- all in a run-time matrix configuration
    - настройка вывода подключения матрицы из WebUI
    - настройка размеров матрицы из WebUI
    - настройка ориентации/топологии матрицы из WebUI
- LedFB - implement run-time topology transformation
- other fixes
    * fixed crash when accessin non-existing speedfactor control on empty controls list
    * fixed wrong loading of controls for non-fading effect switch
    * fix potential issue with dirty sleds buffer
    * fix argument reodreding Noise3dMap::lxy()
- rework drawing feature
    - adopted drawing functionality for my framework (js and mcu code)
    - drastically simplified backend side code, removed strings deserialization, parsing and processing
    - removed extra actions for drawing, using one callback for all data
    - adjusted front-end code to do most string conversion and serialize formatted data for backend
- implement additional 404 handler for WLED that does not break Captive Portal detection
- bye bye 8266!


## 3.2.0 (2023-07-18)
last release branch with esp8266 support

- disable WLED announce for 8266 (it crashes in sys context if adding MDNS announce for wled), won't fix
- annouce manifest for WLED mobile app
- removed most of _remote_action::RA* calls based on variadics and pointers, it's UB in C++
   reimplemented remote_action features with templated functions and EmbUI callback injection,
   making data/events flow in proper way reusing registered actions for WebUI
- rework AUX pin control actions, made it run-time ping configurable
- fix LOG macro warnings
- removed ace_editor from FS
- remove code related to WHITE_LAMP_MODE
- fix tm display dots blink


## v3.1.1 (2023-05-29)
 - исправлена сборка с TM1637 display
 - для энкодера упрощены вызовы из обработчика прерываний, длинные операции вынесены в loop()
 - fix issue with incorrect brightness on effect save
 - fix FIre2018 - last column is always off
 - fix in EffectMaze
 - EffectPuzzles random fix
 - fixed an issue buffer switch if effect tries to clear canvas on initialization
 - EffectBBalls - aligh balls
 - fix boolean controls

## v3.1.0 (2023-05-16)
 - исправлены проблемы сборки под обновленный фреймворк есп32 и платформу есп32 v6.1.7
 - введен класс кадрового CRGB буфера абстрагирующий топологию и тип устройства вывода
   - буфер допускает привязку/отвязку от FastLED, копирование, атомарный обмен и общие операции над RGB массивом
   - методы обращения к буферу и транляции под топологию матрицы из EffectMath упразднены в пользу работы через LedFB
   - буфера лампы под слои текста/рисовалки переведены на использование экземпляров LedFB, часть операций копирования заменена атомарной заменой
   - убраны флаги зеркалирования из объекта лампы, топология определяеся конфигурацией буфера
 - изменения в коде эффектов
   - множество эффектов переработанно с использованием структур сущностей и стандартных контейнеров
   - оптимизация использования памяти для эффектов
      - для части эффектов убраны излишние буфера и промежуточное копирование
      - эффекты с переменным числом объектов, вместе с изменением числа сущностей растет/уменьшается их контейнер вместо использования фиксированных массивов
   - промежуточные двумерные массивы для эффектов унифицированы в абстрактный шаблон Vector2D
   - переработаны эффекты на основе Noise3D Map
     - Noise3D Map выделен в отдельный класс с собственным буфером
     - унифицирован метод регенерации шума
   - переработан класс Boid и зависимые от него эффекты, убраны зависимость от статических макросов WIDTH/HEIGHT, добавлен контейнер для объектов стаи
   - все эффекты переведены на работу через фреймбуфер
      - убраны зависимости от макросов WIDTH/HEIGHT
      - убраны зависимости от статических констант/макросов производных WIDTH/HEIGHT
      - убраны все прямые обращения к глобальному экземпляру класса FastLED
   - весь код кроме объекта LedFB избавлен от зависимости на статические макросы WIDTH/HEIGHT и их производные

## v3.0.0 (2023-04-07) первый выпуск проекта после форка
 - добавлена http/mqtt команда для регулировки яркости лампы в процентах 0-100
 - добавлена http/mqtt команда для регулировки громкости мп3 плеера
 - добавлен класс Effcfg для управления конфигурацией эффектов, упразднены операции с создание тяжелых экземпляров класса EffectWorker при копировании конфигураций эффектов, работе затухания и пр.
 - раскрыто системное меню по-умолчанию, упразднена опция сборки 'SHOWSYSCONFIG'
 - при сборке с плеером настройки DFPlayer'а всегда показываются в WebUI, независимо от того подключен ли плеер
 - значики микрофона и нумерация эффектов в списках включена по-умолчанию, убрана опция, поозволяющая отключать эти символы
 - реализована буферизованная запись при генерации кеш-файлов и индекса на ФС
 - значительное ускорение процедур генерации кеш-файлов и индекса на ФС
 - упраздены кривые схемы и разнобой с медленной/быстрой/отложенной генерацией файлов списков эффектов, используется один унифицированный генератор
 - переработана схема подрузки конфигурации и переключения эффектов, устранены дублирующие вызовы, оптимизированна работа с файлами
 - реализовано корректное кеширование списков эффектов на стороне браузера, устранены излишние подгрузки в случае если списки не изменялись
 - переработан класс реализующий плавное изменение яркости
 - оптимизирована mqtt публикация, в случае если нет подключения с серверу, попытки публикации отменяются
 - оптимизированы классы GradientPaletteList, GradientPalette
 - удалена поддержка протокола ArduinoOTA, успользуется обновление через http
 - влит код из репозитория andy040670 примерно от 2021 года, устранены ошибки сборки
 - переработаны списки контролов UIControl
    - указатели заменены на smartpointers, реализованы конструкторы копирования, глубокие копии списков, устранение утечек памяти, ошибки работы с памятью, висячие указатели
 - к репозиторию настроена CI сборка
 - переработаны скрипты установки/сборки для windows (спасибо andy040670)
    - добавлены переменные для путей к репозиториям и зависимостям
    - убрана зависимость от внешней утилиты wget.exe
 - реструктуризация кода, файлов, сторонних бибилиотек
    - некоторые сторонние библиотеки убраны из репозитория и заменены внешними зависимостями
    - переименованы констатны TCONST_00XX, CMD_* в человекочитаемый вид
    - переименованы множество имен переменных, классов, макросов в соответсвии с общепринятым стилем
    - переработаны заголовочные файлы, перенесены определения и декларации в соответсвии с логической принадлежностью файлов, код методов перенесен в соответствующие cpp файлы
    - переработаны инклюды заголовочных файлов в соответсвии с минимально необходимыми зависимостями
    - из репозитория удалены посторонние бинарные файлы
 - исправлены множество ошибкок в коде работы с эффектами - вылеты за границы массивов, деление на ноль, утечки и излишний расход памяти, оптимизация циклов и т.п.
    - EffectSplashBals
    - EffectOsc
    - EffectLeapers
    - EffectPicasso
    - EffectMagma
    - EffectNexus
    - EffectLiquidLamp
    - EffectPicasso
    - EffectSmokeballs
    - EffectBBalls
    - EffectMaze
    - EffectWrain
    - Puzzle
    - EffectSmokeballs
    - EffectPolarL
    - EffectPacific
    - EffectStarShips
    - EffectFire2021
    - EffectCube2d
    - EffectPopcorn
    - EffectPuzzles
    - EffectRadialFire
    - EffectWcolor
    - new Effect - Tetris Clock