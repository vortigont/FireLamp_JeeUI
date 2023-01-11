color 0A
SETLOCAL EnableDelayedExpansion
set workdir=%~dp0
PATH=%PATH%;%workdir%;%USERPROFILE%\.platformio\penv\Scripts;
echo off
@chcp 1251>nul
mode con: cols=88 lines=40
cls

Echo  Attention. For install Python, run this script "As Administrator". 
Echo  And after completing this step 1, restart your computer.
Echo  .

:m1
Echo  #------------------------------------------#-----------------------------------------# 
Echo  *                  Commands                *                  Команды                * 
Echo  #------------------------------------------#-----------------------------------------# 
Echo  *             Install tools                *          Установка инструментов         * 
Echo  #------------------------------------------#-----------------------------------------#
Echo  *  Install Python          (step 1)      1 *  Установить Python             (Шаг 1)  * 
Echo  *  Install Platformio Core (step 2)      2 *  Установить PIO Core           (Шаг 2)  * 
Echo  *  Install Git for Windows (step 3)      3 *  Установить Git                (Шаг 3)  * 
Echo  *  Get firmware repository (step 4)      4 *  Получить репозиторий прошивки (Шаг 4)  *
Echo  #------------------------------------------#-----------------------------------------# 
Echo  *  Remove Platformio installation        R *  Удалить установку Платформио           * 
Echo  #----------------------------------------#-#-----------------------------------------#
Echo.
Set /p choice="Your choice (Ваш выбор): "

if "%choice%"=="1" (
	if not exist "%systemdrive%\Program Files (x86)" (
		%workdir%\resources\wget https://www.python.org/ftp/python/3.10.4/python-3.10.4.exe -O "%TMP%\python.exe"
	) else (
		%workdir%\resources\wget https://www.python.org/ftp/python/3.10.4/python-3.10.4-amd64.exe -O "%TMP%\python.exe"
	)
	
	%TMP%\python.exe /passive InstallAllUsers=0 PrependPath=1 Include_pip=1 Include_launcher=1 AssociateFiles=1 TargetDir=%USERPROFILE%\Python
	if exist "%USERPROFILE%\AppData\Local\Microsoft\WindowsApps\python.exe" (
		del %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\python.exe
		del %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\python3.exe
		del %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\pip.exe
		mklink %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\python.exe %USERPROFILE%\Python\python.exe
		mklink %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\python3.exe %USERPROFILE%\Python\python.exe
		mklink %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\pip.exe %USERPROFILE%\Python\Scripts\pip.exe
		mklink %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\pip3.exe %USERPROFILE%\Python\Scripts\pip.exe
	)
	if exist "%USERPROFILE%\Python\python.exe" (
		mklink %USERPROFILE%\Python\python3.exe %USERPROFILE%\Python\python.exe
		mklink %USERPROFILE%\Python\Scripts\pip3.exe %USERPROFILE%\Python\Scripts\pip.exe
		set PYTHONHOME=%USERPROFILE%\Python
		set PYTHONPATH=%USERPROFILE%\Python
	)
)

if "%choice%"=="2" (
	%workdir%\resources\wget https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -O %workdir%\get-platformio.py
	python %workdir%\get-platformio.py
	del %workdir%\get-platformio.py
)

if "%choice%"=="3" (
	if not exist "%systemdrive%\Program Files (x86)" (
		%workdir%\resources\wget https://github.com/git-for-windows/git/releases/download/v2.36.1.windows.1/Git-2.36.1-32-bit.exe -O %TMP%\git.exe
	) else (
		%workdir%\resources\wget https://github.com/git-for-windows/git/releases/download/v2.36.1.windows.1/Git-2.36.1-64-bit.exe -O %TMP%\git.exe
	)
	
	%TMP%\git.exe /SILENT
	del %TMP%\git.exe
)

if "%choice%"=="4" (
	Set /p diskInstal="Enter a drive letter C,D etc. (Введите букву диска C,D и т.п): "
	rem echo  test !%diskInstal!
	if not exist "!diskInstal!:\" (
		echo Disk letter is wrong!
		pause
		cls
		goto m1

	) else (
		echo "Firmware repo wil be instaled on disk !diskInstal!:"
		!diskInstal!:
		if exist "!diskInstal!:\FireLamp_JeeUI_original_int" (rmdir /S /Q !diskInstal!:\FireLamp_JeeUI_original_int)
		"%ProgramFiles%\Git\bin\git" clone https://github.com/andy040670/FireLamp_JeeUI_original_int.git
		start .\FireLamp_JeeUI_original_int
	)
)

if "%choice%"=="R" (rmdir /S %USERPROFILE%\.platformio)

Echo.
if "%choice%"=="1" (
	Echo Don't forget to restart your computer right now!
	pause >> nul
) else (
	pause
)
cls
goto m1