idf_component_register(SRCS "wifiManager.c"
                    REQUIRES esp_wifi freertos nvs_flash wifi_provisioning
                    INCLUDE_DIRS include
                    )