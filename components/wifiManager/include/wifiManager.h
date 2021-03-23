#ifndef wifiManager_h
#define wifiManager_h

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <esp_system.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include <esp_event.h>

#include <wifi_provisioning/manager.h>
#ifdef PROV_BLE
#include <wifi_provisioning/scheme_ble.h>
#else
#include <wifi_provisioning/scheme_softap.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define WIFI_MANAGER_TAG "WiFi Manager"
#define WIFI_CONNECTED_EVENT BIT0
#define WIFI_FAIL_EVENT      BIT1

#define ESP_MAXIMUM_RETRY  10

esp_err_t wifiInit(void);
esp_err_t wifiDisconnect(void);
esp_err_t wifiConnect(void);
esp_err_t wifiProvisioning(void);
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data);
esp_err_t getRSSI(int8_t* rssi);

void syncTime(void);

#ifdef __cplusplus
}
#endif

#endif