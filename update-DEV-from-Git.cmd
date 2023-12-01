setlocal
set param=%1
echo off
set workdir=%~dp0
PATH=%PATH%;%workdir%;%ProgramFiles%\Git;%ProgramFiles%\Git\bin
cls
Echo.

if "%param%"=="1" ( rem переключаем на мастер
	"%ProgramFiles%\Git\bin\git" switch master
	"%ProgramFiles%\Git\bin\git" pull
)
if "%param%"=="2" ( rem переключаем на релизную ветку
	"%ProgramFiles%\Git\bin\git" switch v3.4
	"%ProgramFiles%\Git\bin\git" pull
)
if "%param%"=="3" ( rem сброс состояния локального репозитория
	"%ProgramFiles%\Git\bin\git" reset --hard
	rem "%ProgramFiles%\Git\bin\git" checkout .
	"%ProgramFiles%\Git\bin\git" pull
)
Echo.




