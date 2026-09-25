@echo off
setlocal
set "EXENAME=FSEngine.exe"
set "PRESET=clang-debug"

if not exist build mkdir build >nul 2>nul
if not exist CMakeCache.txt set "CLEANBUILD=YES"
if exist bin\%EXENAME% del bin\%EXENAME%
if "%1" == "clean" set "CLEANBUILD=YES"

start "" /b compileShader.bat

if "%CLEANBUILD%" == "YES" (
    rmdir /S /Q build
    rmdir /S /Q bin
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

copy build\%PRESET%\compile_commands.json build\ >nul
"bin\%EXENAME%"
endlocal
