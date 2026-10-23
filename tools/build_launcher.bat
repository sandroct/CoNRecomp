@echo off
rem Build do Launcher.exe (Win32 nativo, sem CMake).
rem Saida: build\port\bin\Launcher.exe (ao lado do con.exe).
cd /d C:\recomp\CoNRecomp
call tools\win\vsenv.cmd >nul 2>&1
if not exist build\launcher mkdir build\launcher
cl /nologo /EHsc /std:c++17 /O1 /W3 /DUNICODE /D_UNICODE ^
   src\launcher\launcher.cpp ^
   /Fo:build\launcher\ /Fd:build\launcher\launcher ^
   /Fe:build\port\bin\Launcher.exe
exit /b %ERRORLEVEL%
