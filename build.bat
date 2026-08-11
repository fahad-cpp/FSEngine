@echo off
setlocal
set "EXENAME=VulkanApp.exe"
set "PRESET=clang-debug"

if not exist build mkdir build >nul 2>nul
if not exist CMakeCache.txt set "CLEANBUILD=YES"
if "%1" == "clean" set "CLEANBUILD=YES"

if "%CLEANBUILD%" == "YES" (
    rmdir /S /Q build
    cmake --preset %PRESET%
    if errorlevel 1 (
        echo Cmake Configuration Failed.
        type error.txt
        popd
        exit /b 1
    )
)

cmake --build --preset %PRESET%
if errorlevel 1 (
    echo CMake Build Failed.
    type error.txt
    popd
    exit /b 1
)
"bin\%EXENAME%"
endlocal
