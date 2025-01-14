color 0A
setlocal
set workdir=%~dp0
PATH=%PATH%;%workdir%;%USERPROFILE%\.platformio\penv\Scripts;
echo off
cls
mode con: cols=44

:m1
Echo  #****************************************#
Echo  #----------------------------------------#

Echo  # Uload by http firmware.bin        - ([97m1[92m)#
Echo  # Uload by http littlefs.bin        - ([97m2[92m)#
Echo  #****************************************#
Echo.
Set /p choice="Your choice: "

if not defined choice (
	echo.
	Echo Wrong command!
	goto m1
)
if "%choice%"=="1" (curl -v http://192.168.0.121/update -F "name=@.pio\build\esp8266@160\firmware.bin")
if "%choice%"=="2" (curl -v http://192.168.0.121/update -F "name=@.pio\build\esp8266@160\littlefs.bin")

Echo.
Echo.
Echo.
pause
del %workdir%\resources\.wget-hsts
cls
goto m1


exit