@echo off
REM chcp 65001: bat console codepage for English prompts only; demo CP comes from arg UTF8
chcp 65001 >nul
cd /d "%~dp0"
REM CONFIRM default OFF. To use ON: change last arg OFF -> ON
REM Optional 3rd arg AUTO on exe: auto-keys to CT page (self-test only)
demo_gtwvt.exe UTF8 OFF