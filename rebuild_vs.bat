@echo off
setlocal EnableDelayedExpansion

:: --------------------------------------------------
:: Step 1: Delete Saved, Binaries, and Intermediate folders
:: --------------------------------------------------
echo Deleting Saved, Binaries, and Intermediate folders...
if exist "Saved" (
    rd /s /q "Saved"
    echo Deleted Saved
)
if exist "Binaries" (
    rd /s /q "Binaries"
    echo Deleted Binaries
)
if exist "Intermediate" (
    rd /s /q "Intermediate"
    echo Deleted Intermediate
)

:: --------------------------------------------------
:: Step 2: Locate the .uproject file in the current folder
:: --------------------------------------------------
set "PROJECT_FILE="
for %%i in (*.uproject) do (
    set "PROJECT_FILE=%%i"
)
if not defined PROJECT_FILE (
    echo ERROR: No .uproject file found in the current directory.
    pause
    exit /b 1
)
echo Found project file: %PROJECT_FILE%

:: --------------------------------------------------
:: Step 3: Locate UE5.5 installation directory via registry using InstalledDirectory
:: --------------------------------------------------
set "UE_INSTALL_DIR="

:: Try standard registry key first.
for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\EpicGames\Unreal Engine\5.6" /v InstalledDirectory 2^>nul') do (
    set "UE_INSTALL_DIR=%%B"
)

:: If not found, try the Wow6432Node key.
if not defined UE_INSTALL_DIR (
    for /f "tokens=2*" %%A in ('reg query "HKLM\SOFTWARE\Wow6432Node\EpicGames\Unreal Engine\5.6" /v InstalledDirectory 2^>nul') do (
        set "UE_INSTALL_DIR=%%B"
    )
)

if defined UE_INSTALL_DIR (
    :: Remove quotes from the registry value
    set "UE_INSTALL_DIR=%UE_INSTALL_DIR:"=%"

    :: Remove trailing backslash if present
    if "%UE_INSTALL_DIR:~-1%"=="\" (
        set "UE_INSTALL_DIR=%UE_INSTALL_DIR:~0,-1%"
    )

    set "UBT_PATH=!UE_INSTALL_DIR!\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
    echo UBT_PATH is: "!UBT_PATH!"

    if not exist "!UBT_PATH!" (
        echo UBT not found at expected registry location: !UBT_PATH!
        set "UBT_PATH="
    )
) else (
    echo ERROR: UE5.5 installation directory not found in the registry.
)

:: --------------------------------------------------
:: Step 4: Generate Visual Studio Project Files
:: --------------------------------------------------
echo Generating Visual Studio project files...
"%UBT_PATH%" -projectfiles -project="%CD%\%PROJECT_FILE%" -game -engine

echo.
echo Project files generated successfully.
pause