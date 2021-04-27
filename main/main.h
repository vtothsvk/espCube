#ifndef MAIN_H_
#define MAIN_H_

//Standard libraries
#include <stdio.h>
#include <math.h>
#include <stdint.h>

//ESP-IDF components
//System
#include "esp_system.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "soc/soc.h"
#include "soc/rtc_io_reg.h"
//FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
//Drivers
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/adc.h"
//WiFi and HTTP client
#include "esp_wifi.h"
#include "esp_http_client.h"

//Additional project components
//ESP-IDF lib
#include "i2cdev.h"
//Custom
#include "ina226.h"
#include "ina219.h"
#include "wifiManager.h"
#include "newAuth.h"
#include "esp_json.h"
#include "pwmOut.h"

#include <stdio.h>
#include <math.h>
#include <esp_system.h>

#include "esp_err.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

//Cube 
#include "cube.h"

//C wrapper for main func
extern "C" {
    void app_main(void);
}

#endif//MAIN_H_