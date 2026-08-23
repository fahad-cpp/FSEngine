EXENAME=VulkanApp
PRESET="clang-debug"

mkdir -p build

if ! command -v cmake >/dev/null 2>/dev/null ; then
    echo CMake not found.
    exit 1
fi

if ! cmake --preset $PRESET; then
    echo Failed to generate build files.
    exit 1
fi

if ! cmake --build --preset $PRESET; then
    echo Failed to build program.
    exit 1
fi

cp build/$PRESET/compile_commands.json build/
echo Successfully compiled to $EXENAME

"./bin/$EXENAME"
