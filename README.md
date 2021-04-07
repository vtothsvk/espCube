# espCube

ESP32 SoC based modular board fitted with a peristaltic pump

# SDK setup

Project based on the [ESP-IDF SKD](https://github.com/espressif/esp-idf).

* [Environment setup](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#installation-step-by-step)

# Building and flashing

ESP-IDF uses a CMake-based build system.
Namely:

* CMake to configure project and its components
* A command line build tool (either Ninja build or GNU Make)
* The espressif comand line tool, [esptool.py](https://github.com/espressif/esptool/#readme), for flashing the target

ESP-IDF provides its own comand line tool front-end tool to manage these stages of project development - idf.py

More info: [ESP-IDF build system](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)

## Building using cmake and ninja

1. Configure using cmake

```
mkdir build
cd build
cmake .. -DBLE_DEBUG=true -DDATA_DEBUG=true -GNinja
```

2. Build the project (can be skipped)

```
ninja
```

3. Flash the project using `flash` (automatically builds the poroject as well)

```
ninja flash
```

## Building using idf.py

1. Select target using `set-target`

```
idf.py set-target esp32
```

2. Build the project using `build` (can be skipped)

```
idf.py build
```

3. Flash the project using `flash` (automatically builds the poroject as well)

*optionally u can specify the target port using -p*

```
idf.py flash -p /dev/ttyS0
```

## Building using provided batch/bash scripts

* All project management scripts cane be found in the *scripts* folder

1. Set up an enviroment variable **ESP_IDF_PATH** with a path to your cloned ESP-IDF repository

2. Initialise project with the `init` script

3. (Optional) Build project with the `build` script

4. Flash the project using the `flash` script

* U can specify the target comport by invoking the script with the given port as an argument

```
flash.sh /dev/ttyS0
```

*this will flash the project onto the device on the port /dev/ttyS0*
