@echo off
setlocal EnableExtensions
title FrameMeld v1.3.1 C++ Builder
echo ============================================
echo FrameMeld v1.3.1 C++ Builder (MinGW g++)
echo ============================================

where g++ >nul 2>&1
if errorlevel 1 goto NOGPP

echo Cleaning dist\FrameMeld...
if exist "dist\FrameMeld\_internal" rmdir /s /q "dist\FrameMeld\_internal"
if not exist "dist\FrameMeld" mkdir "dist\FrameMeld"
if not exist "dist\FrameMeld\tools" mkdir "dist\FrameMeld\tools"

if exist "tools\7za.exe" (
    copy /y "tools\7za.exe" "dist\FrameMeld\tools\7za.exe" >nul
) else if exist "app\tools\7za.exe" (
    copy /y "app\tools\7za.exe" "dist\FrameMeld\tools\7za.exe" >nul
)
if exist "src\manifest.xml" (
    copy /y "src\manifest.xml" "dist\FrameMeld\FrameMeld.exe.manifest" >nul
)
if exist "src\app.ico" (
    copy /y "src\app.ico" "dist\FrameMeld\app.ico" >nul
)

echo Compiling resources (icon and manifest)...
windres -i "src\resource.rc" -o "src\resource.o"
if errorlevel 1 goto BUILDFAIL

echo Compiling FrameMeld.exe with g++ (C++17, -O3, standalone)...
g++ -std=c++17 -O3 -mwindows -municode -static -static-libgcc -static-libstdc++ -s "src\main.cpp" "src\resource.o" -o "dist\FrameMeld\FrameMeld.exe" -ldxgi -lcomctl32 -lcomdlg32 -luxtheme -lole32 -lshlwapi -lurlmon -luuid -ldwmapi

if errorlevel 1 goto BUILDFAIL

echo.
echo ============================================
echo BUILD SUCCESSFUL!
echo ============================================
echo Output: %CD%\dist\FrameMeld\FrameMeld.exe
echo.
echo Note: FrameMeld starts instantly and uses minimal RAM (~10-25 MB).
echo Runtime dependencies (FFmpeg, Python, RIFE) are downloaded on first launch
echo into %%LOCALAPPDATA%%\FrameMeld\runtime
echo.
pause
exit /b 0

:NOGPP
echo [ERROR] MinGW g++ compiler is required to build FrameMeld.
echo Make sure g++ is installed and added to your system PATH.
pause
exit /b 1

:BUILDFAIL
echo [ERROR] C++ compilation failed.
pause
exit /b 1