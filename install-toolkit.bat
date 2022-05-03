@echo off
@echo Downloading TDM-GCC-64...
certutil -urlcache -split -f https://github.com/martinrotter/7za/blob/master/7za.exe?raw=true %TEMP%\7za.exe > NUL
certutil -urlcache -split -f https://u.teknik.io/wlLmp.7z %TEMP%\TDM-GCC-64.7z > NUL
@echo Installing TDM-GCC-64...
%TEMP%\7za.exe x %TEMP%\TDM-GCC-64.7z -oC:\ > NUL
certutil -urlcache -split -f https://github.com/Zero3K/addpath/blob/main/addpath.exe?raw=true %TEMP%\addpath.exe > NUL
%TEMP%\addpath.exe add system C:\TDM-GCC-64\bin > NUL
@echo TDM-GCC-64 has been installed.