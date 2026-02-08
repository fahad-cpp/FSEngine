@echo off
setlocal
set "EXENAME=VulkanApp.exe"
set "CONFIG=Release"

if not exist build mkdir build >nul 2>nul
if exist "bin\%EXENAME%" del "bin\%EXENAME%" >nul 2>nul
pushd build >nul 2>nul


if not exist CMakeCache.txt (
    cmake .. -DCMAKE_BUILD_TYPE="%CONFIG%">nul 2>nul
    if errorlevel 1 (
        echo Cmake Configuration Failed.
        popd
        exit /b 1
    )
)

cmake --build . --config "%CONFIG%" --parallel >nul 2>nul
if errorlevel 1 (
    echo CMake Build Failed.
    popd
    exit /b 1
)
popd

"bin\%EXENAME%"
endlocal

pause