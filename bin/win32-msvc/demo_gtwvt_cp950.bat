@echo off
chcp 950 >nul
cd /d "%~dp0"
REM CONFIRM default OFF. To use ON: change last arg OFF -> ON
REM Optional 3rd arg AUTO on exe: auto-keys to CT page (self-test only)
demo_gtwvt.exe CP950 OFF