color 0A
setlocal
set workdir=%~dp0
echo off
if exist "%USERPROFILE%\Python\python.exe" (
	set PYTHONHOME=%USERPROFILE%\Python
	set PYTHONPATH=%USERPROFILE%\Python
)
PATH=%PATH%;%workdir%;%USERPROFILE%\.platformio\penv\Scripts;%PYTHONPATH%;
REM set the code page to UTF8
chcp 65001 >nul
rem @chcp 1251>nul
mode con: cols=88
cls
Echo.
if not exist "%workdir%\out_bin" (mkdir %workdir%\out_bin)
for /F "tokens=8" %%i in ('"%ProgramFiles%\Git\bin\git" checkout') do set "branch=%%i"
set "branch=%branch:~0,-1%"
echo   [33mЗагруженная версия:[m [92m[31m%workdir% %branch%[m [92m
:m1
Echo  ╔══════════════════════════════════════════╦══════════════════════════════════════════╗
Echo  ║  (English)      COMMANDS                 ║  (Russian)      КОМАНДЫ                  ║
Echo  ╠══════════════════════════════════════════╩══════════════════════════════════════════╣
Echo  ║             ♦ GIT commands ♦             │             ♦ Работа с GIT ♦             ║
Echo  ║  Switch/update - MASTER branch      ► 1  │  Переключить/обновить - ветка MASTER     ║
Echo  ║  Switch/update - Last release  v3.4 ► 2  │  Переключить/обновить - выпуск v 3.4     ║
:Echo  ║  Switch/update - FEAT branch        ► 2b │  Переключить/обновить - ветка FEAT       ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║  Reset changes in local repo!       ► 3  │  Сбросить изменения в локальном репо!    ║
Echo  ║  [41m WARNING! [m [92mThis will revert all changes!│  [41m ВНИМАНИЕ! [m [92mЭто сбросит все изменения!  ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║                  ♦ Build ♦               │                ♦ Сборка ♦                ║
Echo  ║  Build - Esp32                      ► 4  │  Собрать для Esp32                       ║
Echo  ║  Build - Esp32 (DEBUG)              ► 4D │  Собрать для Esp32  (С ЛОГОМ)            ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║            ♦ Build and flash ♦           │          ♦ Сборка и прошивка ♦           ║
Echo  ║  Build and upload - Esp32           ► 5  │  Собрать и прошить - Esp32               ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║        ♦ Build and flash (DEBUG) ♦       │     ♦ Сборка и прошивка  (С ЛОГОМ) ♦     ║
Echo  ║  Build and upload - Esp32           ► 5D │  Собрать и прошить - Esp32               ║
Echo  ║  Serial port monitor                ► D  │  Вывод отладочной информации (ЛОГ)       ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
:Echo  ║             ♦ File System ♦              │           ♦ Файловая Система ♦           ║
:Echo  ║  Update FS data from framework      ► u  │  Обновить файлы ФС из фреймворка         ║
:Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║            ♦ FS for ESP32 ♦              │            ♦ ФС Для ESP32 ♦              ║
Echo  ║  Build File System image            ► b  │  Собрать образ Файловой Системы          ║
Echo  ║  Build and upload File System       ► f  │  Собрать и прошить Файловую Систему      ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║  Erase Flash    ESP32               ► e  │  Стереть флеш    ESP32                   ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║  Update libs and PIO Core           ► g  │  Обновить библиотеки и среду PIO Core    ║
Echo  ║  Clean up temp files .pio           ► c  │  Очистить временные файлы .pio           ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║  CMD window                         ► m  │  Открыть окно коммандной строки CMD      ║
Echo  ╠═════════════════════════════════════════════════════════════════════════════════════╣
Echo  ║  Remove Platformio installation     ► R  │  Полностью удалить Platformio с ПК       ║
Echo  ╚═════════════════════════════════════════════════════════════════════════════════════╝
Echo.
Set /p choice="Your choice [Ваш выбор]: ► " 

if "%choice%"=="1" (
	call update-DEV-from-Git.cmd 1
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" pkg update
)
if "%choice%"=="2" (
	call update-DEV-from-Git.cmd 2
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" pkg update
)
if "%choice%"=="1b" (
	call update-DEV-from-Git.cmd 2b
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" pkg update
)
if "%choice%"=="3" call update-DEV-from-Git.cmd 3

if "%choice%"=="4" (
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --environment esp32
		mkdir %workdir%\out_bin
	copy /z %workdir%\.pio\build\esp32\firmware.bin %workdir%\out_bin
)
if "%choice%"=="4D" (
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --environment esp32debug
	mkdir %workdir%\out_bin
	copy /z %workdir%\.pio\build\esp32debug\firmware.bin %workdir%\out_bin
)
if "%choice%"=="5" ("%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --target upload --environment esp32)
if "%choice%"=="5D" ("%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --target upload --environment esp32debug)
if "%choice%"=="D" (start %workdir%\SerialMonitor.cmd)
if "%choice%"=="u" (
	cd %workdir%\resources\
	start respack.cmd
	cd %workdir%
)
if "%choice%"=="b" (
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --target buildfs --environment esp32
	mkdir %workdir%\out_bin
	copy /z %workdir%\.pio\build\esp32\littlefs.bin %workdir%\out_bin
)
if "%choice%"=="f" ("%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --target uploadfs --environment esp32)
if "%choice%"=="e" ("%USERPROFILE%\.platformio\penv\Scripts\pio.exe" run --target erase --environment esp32)
if "%choice%"=="c" (
	pio system prune -f
	rmdir /S /Q %workdir%\.pio
	rmdir /S /Q %workdir%\out_bin
)
if "%choice%"=="g" (
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" update
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" upgrade
	"%USERPROFILE%\.platformio\penv\Scripts\pio.exe" pkg update
)
if "%choice%"=="m" (start cmd)
if "%choice%"=="R" (rmdir /S "%USERPROFILE%\.platformio")

Echo.
Echo.
Echo.
pause
del %workdir%\resources\.wget-hsts
cls
goto m1

exit
