@echo off
call "E:\UE5\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=%~dp0..\FoldlightDemo.uproject" -target=FoldlightDemo -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive "-archivedirectory=%~dp0..\Build" -utf8output -unattended
if errorlevel 1 exit /b %errorlevel%
if not exist "%~dp0..\Build\Windows\Licenses" mkdir "%~dp0..\Build\Windows\Licenses"
copy /Y "%~dp0..\Documentation\Fonts\AndroidOpenSourceProject_License.txt" "%~dp0..\Build\Windows\Licenses\DroidSansFallback.txt" >nul
copy /Y "%~dp0ExternalAssets\Prepared\KayKitAdventurers-CC0.txt" "%~dp0..\Build\Windows\Licenses\KayKitAdventurers-CC0.txt" >nul
copy /Y "%~dp0ExternalAssets\Prepared\KayKitDungeon-CC0.txt" "%~dp0..\Build\Windows\Licenses\KayKitDungeon-CC0.txt" >nul
exit /b %errorlevel%
