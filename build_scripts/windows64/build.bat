@echo off
if [%MACHINE%]==[] call setup_env.bat

set OLD_DIR=%CD%
cd /d %DIR%\caret7_source\build
cmake -G "NMake Makefiles" ../src
nmake 
cd /d %OLD_DIR%
set OLD_DIR=