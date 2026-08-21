@echo off
setlocal EnableExtensions
title FrameForge Builder v1.0
echo ============================================
echo FrameForge Builder v1.0
echo ============================================
where py >nul 2>&1
if errorlevel 1 goto NOPYTHON
if not exist ".venv\Scripts\python.exe" py -3.11 -m venv .venv
if errorlevel 1 goto NOPYTHON
call ".venv\Scripts\activate.bat"
python -m pip install --upgrade pip
python -m pip install PySide6 pyinstaller
if errorlevel 1 goto BUILDFAIL
echo Building FrameForge.exe...
python -m PyInstaller --noconfirm --clean --onedir --windowed --name FrameForge app\FrameForge.py
if errorlevel 1 goto BUILDFAIL
if exist "dist\FrameForge\tools" rmdir /s /q "dist\FrameForge\tools"
robocopy "app\tools" "dist\FrameForge\tools" /E /NFL /NDL /NJH /NJS /NP >nul
echo.
echo ============================================
echo DONE
echo ============================================
echo EXE: %CD%\dist\FrameForge\FrameForge.exe
echo.
echo Note: FrameForge downloads its runtime (FFmpeg, Python, RIFE model)
echo automatically on first launch, into %%LOCALAPPDATA%%\FrameForge\runtime
echo No Flowframes install or manual FFmpeg download is needed to build.
echo.
pause
exit /b 0
:NOPYTHON
echo [ERROR] Python 3.11+ with py.exe is required for this BUILD step.
pause
exit /b 1
:BUILDFAIL
echo [ERROR] PyInstaller build failed.
pause
exit /b 1