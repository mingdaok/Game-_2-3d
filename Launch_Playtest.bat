@echo off
setlocal
set "FOLDLIGHT_ROOT=%~dp0"
if exist "%FOLDLIGHT_ROOT%Build\Windows\FoldlightDemo.exe" (
    start "Foldlight" "%FOLDLIGHT_ROOT%Build\Windows\FoldlightDemo.exe" -windowed -ResX=1280 -ResY=720
) else (
    start "Foldlight" "E:\UE5\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "%FOLDLIGHT_ROOT%FoldlightDemo.uproject" -game -windowed -ResX=1280 -ResY=720 -NoSplash
)
