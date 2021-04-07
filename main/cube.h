#ifndef CUBE_H_
#define CUBE_H_

#include "main.h"

//System
#define S_TO_MS     1000
#define S_TO_US     1000000
#define POSITIVE    true
#define NEGATIVE    false

//I2C config
#define SDA_GPIO GPIO_NUM_21
#define SCL_GPIO GPIO_NUM_22
#define PORT 0

#define INA_SHUNT   0.02//R
#define INA_MAXI    10//A
#define INA_ADDR    PIN_CONFIG_TO_ADDR(0, 0)

#define MYINA_ADDR  PIN_CONFIG_TO_ADDR(1, 1)

//BDD shield config
#define PWM_GPIO    GPIO_NUM_26
#define POL_GPIO    GPIO_NUM_19
#define ADC_GPIO    ADC2_CHANNEL_4
//#define ADC_GPIO    ADC1_CHANNEL_5 

//BDD config
#define BDD_MTIME       2//s
#define BDD_POLTIME     60//s
#define BDD_DC          0//*100%
#define PWM_FREQ        100//Hz

#define DEFAULT_POL     NEGATIVE

//Cube config
#define TAG "espCube"
#define MEAS_INTERVAL   1//s
#define ADV_INTERVAL    5//s
#define LOOP_DELAY      100//ms

//HTTP config
#define POST_ENDPOINT   "https://fei.edu.r-das.sk:51415/Auth"

void bddRun(void);
esp_err_t inaInit(void);
void inaMeasure(void);
void polInit(void);
void polTime(void);
void pwmInit(void);
void pwm(void);
void advLoop(void);
esp_err_t advertiseData(void);
esp_err_t adcInit(void);
esp_err_t adcRead(int* raw);
float readV(void);

#endif