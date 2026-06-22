@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo Locating Visual Studio Toolchain...
echo ===================================================

:: Use vswhere to find the latest installation of VS or Build Tools
set "VSWHERE_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "!VSWHERE_PATH!" (
    echo [ERROR] vswhere.exe not found at default location.
    echo Please make sure Visual Studio or VS Build Tools are installed.
    pause
    exit /b 1
)

:: Get the installation path of the newest layout
for /f "usebackq tokens=*" %%i in (`"!VSWHERE_PATH!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
    echo [ERROR] Could not find a valid Visual Studio installation with C++ Build Tools.
    pause
    exit /b 1
)

:: Path to the x64 native environment initialization script
set "VCVARS_PATH=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

if not exist "!VCVARS_PATH!" (
    echo [ERROR] Found Visual Studio, but vcvars64.bat is missing.
    pause
    exit /b 1
)

echo Found VS at: "%VS_PATH%"
echo Initializing 64-bit environment...
echo ---------------------------------------------------

:: Call the environment setup script (must use 'call' so it stays in this batch context)
call "!VCVARS_PATH!"

echo ---------------------------------------------------
echo Environment loaded. Building library...
echo ===================================================

:: --- YOUR BUILD COMMANDS GO HERE ---

:: Example: Generating user32_undoc.lib from a module definition (.def) file
if exist "user32_undoc.def" (
    echo Creating library from user32_undoc.def...
    lib /def:user32_undoc.def /out:user32_undoc.lib /machine:x64
) else (
    echo [WARNING] user32_undoc.def not found in current folder. 
    echo Running raw 'lib' command syntax check instead:
    lib /?
)

echo ===================================================
echo Done!
echo ===================================================
pause