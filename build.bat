@echo off
setlocal
set "EXENAME=VulkanApp.exe"
set "CONFIG=Debug"
set "CLEANBUILD=NO"

if not exist build mkdir build >nul 2>nul
if exist "bin\%EXENAME%" del "bin\%EXENAME%" >nul 2>nul

if not exist CMakeCache.txt set "CLEANBUILD=YES"
if "%1" == "clean" set "CLEANBUILD=YES"

if "%CLEANBUILD%" == "YES" (
    rmdir /S /Q build
    cmake --preset default
    if errorlevel 1 (
        echo Cmake Configuration Failed.
        type error.txt
        popd
        exit /b 1
    )
)

cmake --build build --config "%CONFIG%" --parallel 
if errorlevel 1 (
    echo CMake Build Failed.
    type error.txt
    popd
    exit /b 1
)
"bin\%EXENAME%"
endlocal
