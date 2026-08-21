@echo off
setlocal EnableExtensions
title FrameMeld Builder v1.0
echo ============================================
echo FrameMeld Builder v1.0
echo ============================================
where py >nul 2>&1
if errorlevel 1 goto NOPYTHON
if not exist ".venv\Scripts\python.exe" py -3.11 -m venv .venv
if errorlevel 1 goto NOPYTHON
call ".venv\Scripts\activate.bat"
python -m pip install --upgrade pip
python -m pip install PySide6 pyinstaller
if errorlevel 1 goto BUILDFAIL
echo Building FrameMeld.exe...
python -m PyInstaller --noconfirm --clean --onedir --windowed --name FrameMeld app\FrameMeld.py
if errorlevel 1 goto BUILDFAIL
if exist "dist\FrameMeld\tools" rmdir /s /q "dist\FrameMeld\tools"
robocopy "app\tools" "dist\FrameMeld\tools" /E /NFL /NDL /NJH /NJS /NP >nul
echo.
echo ============================================
echo DONE
echo ============================================
echo EXE: %CD%\dist\FrameMeld\FrameMeld.exe
echo.
echo Note: FrameMeld downloads its runtime (FFmpeg, Python, RIFE model)
echo automatically on first launch, into %%LOCALAPPDATA%%\FrameMeld\runtime
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