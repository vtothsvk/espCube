#include "cube.h"

static ina226_t ina;
authHandler auth;

const char ssl_cert[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n"
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n"
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n"
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n"
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n"
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n"
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n"
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n"
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n"
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n"
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n"
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n"
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n"
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n"
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n"
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n"
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n"
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n"
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n"
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n"
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n"
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n"
"-----END CERTIFICATE-----";
// EOC

//ina219 myina(MYINA_ADDR, PORT, SDA_GPIO, SCL_GPIO);

ina226_config_t config {
    INA226_DEFAULT_RESOLUTION,
    INA226_DEFAULT_AVERAGING,
    INA226_DEFAULT_MODE,
    INA_SHUNT,
    INA_MAXI
};

ina219_config_t myconfig {
    INA219_DEFAULT_RESOLUTION,
    INA_SHUNT,
    INA_MAXI
};

static int64_t lastMeasure = 0;
static int64_t lastPol = 0;
static int64_t lastAdv = 0;
static bool polarity = DEFAULT_POL;
static int adcRaw = 0;
static float v = 0.00;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12; 

void bddRun() {
    printf("BDD START\r\n");

    wifiInit();
    wifiProvisioning();
    syncTime();

    esp_err_t ret = i2cdev_init();
    if(ret) {
        printf("I2C dev init: %d\r\n", ret);
    }

    polInit();
    pwmInit();
    adcInit();
    inaInit();

    lastMeasure = lastPol = lastAdv = esp_timer_get_time();

    while (true) {
        inaMeasure();
        polTime();
        advLoop();

        vTaskDelay(100 / portTICK_RATE_MS);
        //vTaskDelay(1000 / portTICK_RATE_MS);
    }//while (true)
}//bddRun

esp_err_t inaInit() {
    esp_err_t ret = i226InitDesc(&ina, INA_ADDR, PORT, SDA_GPIO, SCL_GPIO);
    if(ret) {
        printf("desc init: %d\r\n", ret);
    }

    ret = i226InitSensor(&ina, config);
    if(ret) {
        printf("sensor init: %d\r\n", ret);
    }

    /*ret = myina.init(myconfig);
    if(ret) {
        printf("sensor2 init: %d\r\n", ret);
    }*/

    return ret;
}//inaInit

void inaMeasure() {
    if ((esp_timer_get_time() - lastMeasure) >= (BDD_MTIME * S_TO_US)) {
        ina226_data_t data;
        esp_err_t ret = i226GetMeasurement(&ina, &data);
        if (ret) {
            printf("read ret: %d\r\n", ret);
        } else {
            printf("I: %.5fA\r\n", data.current);
        }//if (ret)
        float myv = 0;
        myv = readV();
        printf("myv: %.2f\r\n", myv);
        /*
        ina219_data_t mydata;
        ret = myina.getMeasurement(&mydata);
        if (ret) {
            printf("myina read ret: %d", ret);
        } else {
            printf("myI: %.5fA\r\n", data.current);
        }//if (ret)*/

        lastMeasure = esp_timer_get_time();
    }//if ((esp_timer_get_time() - lastMeasure) >= (BDD_POLTIME * S_TO_US))
}//inaMeasure

void polInit() {
    gpio_reset_pin(POL_GPIO);
    gpio_set_direction(POL_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(POL_GPIO, polarity);
}//polInit

void polTime() {
    if ((esp_timer_get_time() - lastPol) >= (BDD_POLTIME * S_TO_US)) {
        polarity = !polarity;
        gpio_set_level(POL_GPIO, polarity);
        lastPol = esp_timer_get_time();
    }//if ((esp_timer_get_time() - lastPol) >= (BDD_POLTIME * S_TO_US))
}//polTime

void pwmInit() {
    gpio_reset_pin(PWM_GPIO);
    gpio_set_direction(PWM_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(PWM_GPIO, BDD_DC);
}//pwmInit

void advLoop() {
    if ((esp_timer_get_time() - lastAdv) >= (ADV_INTERVAL * S_TO_US)) {
        advertiseData();
        lastAdv = esp_timer_get_time();
    }//if ((esp_timer_get_time() - this -> lastpol) >= (BDD_POL_TIME * S_TO_US))
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }//if (output_buffer == NULL) { ESP_LOGE...
                    }//if (output_buffer == NULL)
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }//if (evt->user_data)
                output_len += evt->data_len;
            }//if (!esp_http_client_is_chunked_response(evt->client))

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }//if (output_buffer != NULL)
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }//if (output_buffer != NULL)
            output_len = 0;
            break;
    }//switch(evt->event_id)
    return ESP_OK;
}//http_event_handler

esp_err_t advertiseData() {
    cJSON *root;
	root = cJSON_CreateObject();

    cJSON *data[1];

    cJSON *event[2];
    cJSON *array;

    event[0] = cJSON_CreateObject();

    cJSON_AddStringToObject(event[0], "Name", "Voltage");
    cJSON_AddNumberToObject(event[0], "Value", v);

    ina226_data_t inaData;
    esp_err_t ret = i226GetResults(&ina, &inaData);

    event[1] = cJSON_CreateObject();

    cJSON_AddStringToObject(event[1], "Name", "Current");
    cJSON_AddNumberToObject(event[1], "Value", inaData.current);

    
    array = Create_array_of_anything(event, 2);

    data[0] = cJSON_CreateObject();

    time_t epoch;
    time(&epoch);
    printf("epoch %ld\r\n", (long)epoch);

    cJSON_AddStringToObject(data[0], "LoggerName", "BDDshield");
    cJSON_AddNumberToObject(data[0], "Timestamp", (long)epoch);

    array = Create_array_of_anything(event, 1);

    cJSON_AddItemToObject(data[0], "MeasuredData", array);
    cJSON_AddArrayToObject(data[0], "ServiceData");
    cJSON_AddArrayToObject(data[0], "DebugData");

    cJSON_AddStringToObject(data[0], "DeviceId", myId);

    //cJSON *root;
    root = Create_array_of_anything(data, 1);

    //cJSON_AddItemToObject(root, "Data", array);
    const char *my_json_string = cJSON_Print(root);
	//ESP_LOGI(TAG, "my_json_string\n%s", my_json_string);
    printf("\r\n%s\r\n", my_json_string);
    cJSON_Delete(root);

    esp_http_client_config_t config = {
        .url = POST_ENDPOINT,
        .cert_pem = ssl_cert,
        .event_handler = http_event_handler
        //.transport_type = HTTP_TRANSPORT_OVER_TCP
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");

    char jwt[500] = "Bearer ";
    size_t jlen = strlen(jwt);
    auth.createJWT((uint8_t*)jwt + jlen, sizeof(jwt) - jlen, &jlen, (long)epoch);

    printf("jwt:\r\n%s\r\n", jwt);

    esp_http_client_set_header(client, "Authorization", jwt);

    esp_http_client_set_post_field(client, my_json_string, strlen(my_json_string));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
    ESP_LOGI(TAG, "Status = %d, content_length = %d",
           esp_http_client_get_status_code(client),
           esp_http_client_get_content_length(client));
    }//if (err == ESP_OK)
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return err;
}

esp_err_t adcInit() {
    gpio_num_t adc_gpio_num;

    esp_err_t ret = adc2_pad_get_io_num(ADC_GPIO, &adc_gpio_num);
    if (ret) return ret;

    adc2_pad_get_io_num(ADC_GPIO, &adc_gpio_num);
    vTaskDelay(2 / portTICK_RATE_MS);

    return ESP_OK;
}//adcInit

esp_err_t adcRead(int* raw) {
    return adc2_get_raw(ADC_GPIO, width, raw);
}//adcRead

float readV() {
    int32_t vraw = 0;
    int buffer = 0;
    uint8_t runs = 0;
    
    int64_t time = esp_timer_get_time();
    for(uint8_t i = 0; i < 20; i++) {
        if (adcRead(&buffer) == ESP_OK) {
            vraw += (int32_t)buffer;
            runs++;
            printf("siz\r\n");
        }
        vTaskDelay(1);
    }
    time = esp_timer_get_time() - time;
    printf("conversion time: %lld\r\n", time);
    printf("raw %d, lastbuff %d\r\n", vraw, buffer);

    return (runs) ? ((float)vraw / (float)runs) : (0.00);
}//readV