@echo off
setlocal

echo Installing Auto Hide Scenes...

set OBS_PLUGIN_PATH=%APPDATA%\obs-studio\plugins\auto-hide-scenes\bin\64bit
set OBS_DATA_PATH=%APPDATA%\obs-studio\plugins\auto-hide-scenes\data

mkdir "%OBS_PLUGIN_PATH%"
mkdir "%OBS_DATA_PATH%"

copy build\Release\auto-hide-scenes.dll "%OBS_PLUGIN_PATH%\"
xcopy /E /I /Y data\locale "%OBS_DATA_PATH%\locale"

echo Installation complete!
