git submodule update --init --recursive
md build
cd build
cmake .. -GNinja
set /p=Press ENTER to exit...