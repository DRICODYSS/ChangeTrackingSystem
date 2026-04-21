@echo off

setlocal enabledelayedexpansion

:ASK_PATH
set /p UE_PATH=Enter path to Unreal Engine root: 
if "%UE_PATH%"=="" (
    echo Error: No path entered.
    goto ASK_PATH
)
if not exist "%UE_PATH%" (
    echo Error: Path does not exist: %UE_PATH%
    goto ASK_PATH
)

echo UE Path set to: %UE_PATH%
echo.

set "PATCH_DIR=%~dp0Source"

echo Choose action:
echo 1) Dry-run patch
echo 2) Apply patch
echo 3) Rollback patch
set /p ACTION=Enter number: 

if "%ACTION%"=="1" (
    echo Running dry-run...
    python "%PATCH_DIR%\patcher.py" "%UE_PATH%" --dry-run
    goto END
)

if "%ACTION%"=="2" (
    echo Applying patch...
    python "%PATCH_DIR%\patcher.py" "%UE_PATH%"
    goto END
)

if "%ACTION%"=="3" (
    echo Rolling back patch...
    python "%PATCH_DIR%\patcher.py" "%UE_PATH%" --rollback
    goto END
)

echo Invalid option. Exiting.

:END
pause

:: D:\Unreal Engine\UE Source