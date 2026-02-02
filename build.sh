CONFIG=Release
EXENAME=VulkanApp

mkdir -p build
cd build

if ! command -v cmake >/dev/null 2>/dev/null ; then
    echo CMake not found.
    exit 1
fi

if ! cmake .. -DCMAKE_BUILD_TYPE=$CONFIG >/dev/null 2>error.txt; then
    echo Failed to generate build files.
    cat error.txt
    exit 1
fi

if ! cmake --build . --config $CONFIG --parallel >/dev/null 2>error.txt ; then
    echo Failed to build program.
    cat error.txt
    exit 1
fi

echo Successfully compiled to $EXENAME

"./$EXENAME"