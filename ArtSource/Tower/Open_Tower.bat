@echo off
setlocal
set "TOWER_BLENDER=E:\Test\tools\blender-4.5.9-windows-x64\blender.exe"
if not exist "%TOWER_BLENDER%" set "TOWER_BLENDER=E:\Gametest\Tools\Blender\blender-4.5.3-windows-x64\blender.exe"
if not exist "%TOWER_BLENDER%" (
    echo Blender was not found. Open Foldlight_Tower.blend from Blender File - Open.
    pause
    exit /b 1
)
if not exist "%~dp0Foldlight_Tower.blend" (
    echo Foldlight_Tower.blend is missing from this folder.
    pause
    exit /b 1
)
start "" "%TOWER_BLENDER%" "%~dp0Foldlight_Tower.blend"
