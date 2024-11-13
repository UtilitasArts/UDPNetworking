@echo off

cd /d "%~dp0"
set currentDir=%cd%
cd ..
set parentDir=%cd%
setx gitudp "%parentDir%"
setx Path "%Path%;%currentDir%"

