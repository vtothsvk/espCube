#include "wifiManager.h"

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void wifi_init_sta(void);
static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;
static bool voluntaryDisconnect = false;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(WIFI_MANAGER_TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(WIFI_MANAGER_TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(WIFI_MANAGER_TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(WIFI_MANAGER_TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_MANAGER_TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        /* Signal main application to continue execution */
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            s_retry_num++;
            ESP_LOGI(WIFI_MANAGER_TAG, "Disconnected. Connecting to the AP again...");
            esp_wifi_connect();
        } else {
            ESP_LOGI(WIFI_MANAGER_TAG, "Connection failed....");
            if (!voluntaryDisconnect) {
                nvs_flash_init();
                nvs_flash_erase();
            }

            voluntaryDisconnect = false;
        }
    }
}//event handler

static void wifi_init_sta(void){
    /* Start Wi-Fi in station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t wifiInit() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    esp_event_loop_delete_default();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    //init netif with default config
    esp_netif_create_default_wifi_sta();

    return ESP_OK;
}

esp_err_t wifiDisconnect() {
    voluntaryDisconnect = true;

    esp_err_t ret = esp_wifi_disconnect();
    if (ret) return ret;

    ret = esp_wifi_stop();
    if (ret) return ret;

    return esp_wifi_deinit();
}

esp_err_t wifiConnect() {
    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //wifi sta connect
    wifi_init_sta();

    //wait for connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, 60 * 1000 / portTICK_RATE_MS);

    //clean up
    vEventGroupDelete(wifi_event_group);

    return ESP_OK;
}//wifiConnect

esp_err_t wifiProvisioning() {
    wifi_event_group = xEventGroupCreate();

#ifndef PROV_BLE
    esp_netif_create_default_wifi_ap();
#endif

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    //provisioning manager config
    wifi_prov_mgr_config_t config = {
#ifdef PROV_BLE
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
#else
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
#endif   
    };

    //init provis. manager
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

    bool provisioned = false;
    //check if provisioned
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        ESP_LOGI(WIFI_MANAGER_TAG, "Starting provisioning");

    /* generate device name */

    //security level
    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

    //proof of posession
    const char *pop = "abcd1234";

    //prov. ap password
    const char *service_key = NULL;

#ifdef PROV_BLE
    uint8_t custom_service_uuid[] = {
        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
    };

    wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
#endif

    //optional endpoint for custom data 
    wifi_prov_mgr_endpoint_create("custom-data");
        
    //start provisioning
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, "bleSniffer", service_key));

    //register custom endpoint
    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

    //wait for connection
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, 300 * 1000 / portTICK_RATE_MS);

    //clean up
    vEventGroupDelete(wifi_event_group);
    } else {
        wifi_prov_mgr_deinit();
        wifi_init_sta();
    }

    return ESP_OK;
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data){
    if (inbuf) {
        ESP_LOGI(WIFI_MANAGER_TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(WIFI_MANAGER_TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}

esp_err_t getRSSI(int8_t* rssi){
    //init an empty ap_record struct
    wifi_ap_record_t ap_record;

    //get ap details
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_record);

    //output rssi
    *rssi = ap_record.rssi;

    return ret;
}

void syncTime(void){
    //synch immediately after response
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    //set synch mode and server - poll every 60 min
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    //w8 to synch
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        vTaskDelay(100 / portTICK_RATE_MS);
    }

    //set and get epoch
    time_t now = 0;
    struct tm mytime = {0};
    time(&now);
    setenv("TZ", "GMT+1", 1);
    tzset();
    localtime_r(&now, &mytime);

    printf("epoch: %ld\r\n", (long)now);
}//synch time
