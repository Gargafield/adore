
set -xe

os_type="$(uname)"
BUILD_ROOT=""
EXE_PATH=adore/cli/adore
if [[ "$os_type" == "Darwin" ]]; then
  BUILD_ROOT=build/xcode
elif [[ "$os_type" == MINGW* || "$os_type" == MSYS* || "$os_type" == CYGWIN* ]]; then
  BUILD_ROOT=build/vs2022
  EXE_PATH+=".exe"
  OUT_BINARY+=.exe
else
  BUILD_ROOT=build
fi

BUILD_PATH=$BUILD_ROOT/debug
cmake -G=Ninja -B $BUILD_PATH -DCMAKE_BUILD_TYPE=Debug

# build lute0
ninja -C $BUILD_PATH $EXE_PATH

if [[ $? -ne 0 ]]; then
  echo "Build failed"
  exit 1
fi

echo "Build succeeded. Executable located at $BUILD_PATH/$EXE_PATH"

# Optionally run the built executable given as an option to the script
if [[ "$1" == "run" ]]; then
  $BUILD_PATH/$EXE_PATH main.luau
fi