@echo off
call "E:\UE5\UE_5.7\Engine\Build\BatchFiles\Build.bat" FoldlightDemoEditor Win64 Development "-Project=%~dp0..\FoldlightDemo.uproject" -WaitMutex -NoHotReloadFromIDE
exit /b %errorlevel%
