setlocal
set param=%1
echo off
set workdir=%~dp0
PATH=%PATH%;%workdir%;%ProgramFiles%\Git;%ProgramFiles%\Git\bin
cls
Echo.


where /q git
IF ERRORLEVEL 1 (
    ECHO "Git not in path. Assume it is '%pfilespath%\Git\bin\git'"
    set "gitcmd=%pfilespath%\Git\bin\git"
) ELSE (
    ECHO Git найден в PATH
    set "gitcmd=git"
)


if "%param%"=="1" ( rem переключаем на мастер
    %gitcmd% switch master
    %gitcmd% pull
)
if "%param%"=="2" ( rem переключаем на релизную ветку
    %gitcmd% switch v3.4
    %gitcmd% pull
)
if "%param%"=="3" ( rem сброс состояния локального репозитория
    %gitcmd% reset --hard
    rem "%ProgramFiles%\Git\bin\git" checkout .
    %gitcmd% pull
)
Echo.
