@echo off
setlocal EnableDelayedExpansion

:: ============================================================
:: build_windows.bat  -  Build SailkapenaHG on Windows
:: Requires: Qt6, CMake and a C++ compiler (MSVC or MinGW)
:: ============================================================

:: --- Configuration: adjust these paths to your Qt installation ---
:: Examples:
::   C:\Qt\6.10.2\msvc2022_64
::   C:\Qt\6.10.2\mingw_64
set QT_DIR=

:: Build output directory
set BUILD_DIR=build_windows

:: ---------------------------------------------------------------
:: Auto-detect Qt if QT_DIR is empty
:: ---------------------------------------------------------------
if "%QT_DIR%"=="" (
    echo Buscando Qt6 en C:\Qt ...
    for /f "delims=" %%d in ('dir /b /ad "C:\Qt" 2^>nul') do (
        :: Prefer msvc2022_64, then msvc2019_64, then mingw_64
        for %%s in (msvc2022_64 msvc2019_64 mingw_64 mingw_32) do (
            if exist "C:\Qt\%%d\%%s\lib\cmake\Qt6\Qt6Config.cmake" (
                if "!QT_DIR!"=="" (
                    set QT_DIR=C:\Qt\%%d\%%s
    echo Qt6 found at: !QT_DIR!
                )
            )
        )
    )
)

if "%QT_DIR%"=="" (
    echo.
    echo ERROR: Qt6 not found automatically.
    echo Edit this script and set QT_DIR manually.
    echo Example: set QT_DIR=C:\Qt\6.10.2\msvc2022_64
    pause
    exit /b 1
)

:: ---------------------------------------------------------------
:: Verify CMake
:: ---------------------------------------------------------------
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Install it from https://cmake.org/download/
    pause
    exit /b 1
)

:: ---------------------------------------------------------------
:: Detect compiler (MSVC or MinGW)
:: ---------------------------------------------------------------
set GENERATOR=
set TOOLCHAIN=

:: Check if Qt path contains "mingw"
echo %QT_DIR% | findstr /i "mingw" >nul
if not errorlevel 1 (
    :: MinGW: look for Ninja or mingw32-make
    where ninja >nul 2>&1
    if not errorlevel 1 (
        set GENERATOR=-G "Ninja"
    ) else (
        set GENERATOR=-G "MinGW Makefiles"
    )
    echo Using compiler: MinGW
    goto :configure
)

:: Default: MSVC (Visual Studio)
:: Detect installed Visual Studio version
for %%v in ("Visual Studio 17 2022" "Visual Studio 16 2019" "Visual Studio 15 2017") do (
    if "!GENERATOR!"=="" (
        cmake --help 2>&1 | findstr /i "%%~v" >nul
        if not errorlevel 1 (
            set GENERATOR=-G "%%~v" -A x64
            echo Using generator: %%~v
        )
    )
)

if "!GENERATOR!"=="" (
    echo Visual Studio not detected. Trying with the default generator...
)

:configure
:: ---------------------------------------------------------------
:: Configure with CMake
:: ---------------------------------------------------------------
echo.
echo === Configuring CMake ===
echo Build dir : %BUILD_DIR%
echo Qt dir    : %QT_DIR%
echo.

cmake -S . -B "%BUILD_DIR%" ^
    !GENERATOR! ^
    -DCMAKE_PREFIX_PATH="%QT_DIR%" ^
    -DCMAKE_BUILD_TYPE=Release

if errorlevel 1 (
    echo.
    echo ERROR in CMake configuration.
    pause
    exit /b 1
)

:: ---------------------------------------------------------------
:: Build
:: ---------------------------------------------------------------
echo.
echo === Building ===
cmake --build "%BUILD_DIR%" --config Release --parallel

if errorlevel 1 (
    echo.
    echo ERROR during build.
    pause
    exit /b 1
)

:: ---------------------------------------------------------------
:: Copy Qt DLLs next to the executable (windeployqt)
:: ---------------------------------------------------------------
echo.
echo === Copying Qt DLLs (windeployqt) ===

set WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe

:: Normalize exe path: if there is no Release subfolder, copy into it
:: so that installer.iss always points to build_windows\Release\
if not exist "%BUILD_DIR%\Release" mkdir "%BUILD_DIR%\Release"
if not exist "%BUILD_DIR%\Release\SailkapenaHG.exe" (
    if exist "%BUILD_DIR%\SailkapenaHG.exe" (
        xcopy /E /Y "%BUILD_DIR%\*" "%BUILD_DIR%\Release\" >nul
    )
)
set EXE_PATH=%BUILD_DIR%\Release\SailkapenaHG.exe

if exist "%WINDEPLOYQT%" (
    if exist "%EXE_PATH%" (
        "%WINDEPLOYQT%" --release "%EXE_PATH%"
        echo DLLs copied successfully.
    ) else (
        echo Warning: executable not found at %EXE_PATH%
    )
) else (
    echo Warning: windeployqt not found at %WINDEPLOYQT%
    echo Copy the Qt DLLs manually next to the executable.
)

:: ---------------------------------------------------------------
:: Create installer with Inno Setup (if installed)
:: ---------------------------------------------------------------
echo.
echo === Creating installer ===

set ISCC=
for %%p in (
    "C:\Program Files (x86)\Inno Setup 6\iscc.exe"
    "C:\Program Files\Inno Setup 6\iscc.exe"
    "C:\Program Files (x86)\Inno Setup 5\iscc.exe"
) do (
    if "!ISCC!"=="" if exist %%p set ISCC=%%~p
)

if "!ISCC!"=="" (
    where iscc >nul 2>&1
    if not errorlevel 1 (
        for /f "delims=" %%i in ('where iscc') do set ISCC=%%i
    )
)

if "!ISCC!"=="" (
    echo Inno Setup not found. Installer was NOT generated.
    echo Download it for free at: https://jrsoftware.org/isdl.php
    echo Then run:  iscc installer\installer.iss
) else (
    if not exist installer_output mkdir installer_output
    "!ISCC!" installer\installer.iss
    if errorlevel 1 (
        echo ERROR creating the installer.
    ) else (
        echo Installer generated in: installer_output\
    )
)

echo.
echo === Done ===
echo Ejecutable : %EXE_PATH%
echo Installer  : installer_output\SailkapenaHG_Setup_0.1.exe  (if Inno Setup was installed)
echo.
pause
