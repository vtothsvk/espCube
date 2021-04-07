git submodule update --init --recursive
rd build /s /q
md build
cd build
cmake .. -GNinja
set /p=Press ENTER to exit...