@echo off

set "commitMessage=%1"

:: Get the folder 3 levels up from the current script
set "currentFolder=%~dp0"
for %%i in ("%currentFolder%..\..\..") do set "parentFolder=%%~fi"

:: Close the software
taskkill /IM UDPClient.exe /F

:: Change directory to the folder 3 levels up
cd /d "%parentFolder%"
git add .
git commit -m commitMessage
git push -u origin main

cd /d "%currentFolder%"

:: Wait for 2 seconds before restarting
timeout /t 1 /nobreak > nul

:: Restart the software
start UDPClient.exe