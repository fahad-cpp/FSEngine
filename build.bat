@echo off
setlocal
set "EXENAME=VulkanApp.exe"
set "CONFIG=Debug"
set "CLEANBUILD=NO"

if not exist build mkdir build >nul 2>nul
if exist "bin\%EXENAME%" del "bin\%EXENAME%" >nul 2>nul
pushd build >nul 2>nul

if not exist CMakeCache.txt set "CLEANBUILD=YES"
if "%1" == "clean" set "CLEANBUILD=YES"

if "%CLEANBUILD%" == "YES" (
    cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE="%CONFIG%" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >nul 2>error.txt
    if errorlevel 1 (
        echo Cmake Configuration Failed.
        type error.txt
        popd
        exit /b 1
    )
)

cmake --build . --config "%CONFIG%" --parallel >error.txt 2>nul
if errorlevel 1 (
    echo CMake Build Failed.
    type error.txt
    popd
    exit /b 1
)
popd

"bin\%EXENAME%"
endlocal
