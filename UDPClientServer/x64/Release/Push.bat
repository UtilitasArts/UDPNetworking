@echo off

set "commitMessage=%1"

:: Get the folder 3 levels up from the current script
set "currentFolder=%~dp0"
for %%i in ("%currentFolder%..\..\..") do set "parentFolder=%%~fi"

:: Change directory to the folder 3 levels up
cd /d "%parentFolder%"
git add .
git commit -m commitMessage
git push -u origin main

cd /d "%currentFolder%"
