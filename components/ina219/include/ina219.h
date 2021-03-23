#ifndef INA219_H_
#define INA219_H_

#include <stdint.h>
#include <math.h>
#include <esp_system.h>
#include "esp_err.h"
#include "i2cdev.h"
#include "esp_err.h"

/**
 * \defgroup driver-INA219
 * 
 * @brief Driver for the INA219 high-side current sensor
 */

/**
 * \defgroup ina219 class
 * \ingroup driver-INA219
 * @{
 */

#define PIN_CONFIG_TO_ADDR(a1, a0) ((0x40) | (a1 << 2) | (a0))
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

#define DEFAULT_CALIBRATION 256//LSB = 1mA @ .02R shunt
#define INA219_MSB 32768//2^15
#define INA219_CAL 0.04096//Calibration constant

#define INA219_DEFAULT_I2C_FREQ 1000000

/* *///Resolution//* */
#define RES_9b  0
#define RES_10b 1
#define RES_11b 2
#define RES_12b 3
#define INA219_DEFAULT_RESOLUTION  RES_12b

#define INA219_CONF_REG     0x00
#define INA219_I_REG        0x04
#define INA219_CAL_REG      0x05

/* *///Configuration//* */
#define INA219_CONFIG_BVOLTAGERANGE_MASK        0x2000  // Bus Voltage Range Mask
#define INA219_CONFIG_BVOLTAGERANGE_16V         0x0000  // 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V         0x2000  // 0-32V Range

#define INA219_CONFIG_GAIN_MASK                 0x1800  // Gain Mask
#define INA219_CONFIG_GAIN_1_40MV               0x0000  // Gain 1, 40mV Range
#define INA219_CONFIG_GAIN_2_80MV               0x0800  // Gain 2, 80mV Range
#define INA219_CONFIG_GAIN_4_160MV              0x1000  // Gain 4, 160mV Range
#define INA219_CONFIG_GAIN_8_320MV              0x1800  // Gain 8, 320mV Range

#define INA219_CONFIG_BADCRES_MASK              0x0780  // Bus ADC Resolution Mask
#define INA219_CONFIG_BADCRES_9BIT              0x0080  // 9-bit bus res = 0..511
#define INA219_CONFIG_BADCRES_10BIT             0x0100  // 10-bit bus res = 0..1023
#define INA219_CONFIG_BADCRES_11BIT             0x0200  // 11-bit bus res = 0..2047
#define INA219_CONFIG_BADCRES_12BIT             0x0400  // 12-bit bus res = 0..4097

#define INA219_CONFIG_SADCRES_MASK              0x0078  // Shunt ADC Resolution and Averaging Mask
#define INA219_CONFIG_SADCRES_9BIT_1S_84US      0x0000  // 1 x 9-bit shunt sample
#define INA219_CONFIG_SADCRES_10BIT_1S_148US    0x0008  // 1 x 10-bit shunt sample
#define INA219_CONFIG_SADCRES_11BIT_1S_276US    0x0010  // 1 x 11-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_1S_532US    0x0018  // 1 x 12-bit shunt sample
#define INA219_CONFIG_SADCRES_12BIT_2S_1060US   0x0048  // 2 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_4S_2130US   0x0050  // 4 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_8S_4260US   0x0058  // 8 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_16S_8510US  0x0060  // 16 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_32S_17MS    0x0068  // 32 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_64S_34MS    0x0070  // 64 x 12-bit shunt samples averaged together
#define INA219_CONFIG_SADCRES_12BIT_128S_69MS   0x0078  // 128 x 12-bit shunt samples averaged together

#define INA219_CONFIG_MODE_MASK                 0x0007  // Operating Mode Mask
#define INA219_CONFIG_MODE_POWERDOWN            0x0000
#define INA219_CONFIG_MODE_SVOLT_TRIGGERED      0x0001
#define INA219_CONFIG_MODE_BVOLT_TRIGGERED      0x0002
#define INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED  0x0003
#define INA219_CONFIG_MODE_ADCOFF               0x0004
#define INA219_CONFIG_MODE_SVOLT_CONTINUOUS     0x0005
#define INA219_CONFIG_MODE_BVOLT_CONTINUOUS     0x0006
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS 0x0007

/**
 * INA219 configuration parameters structure
 */
typedef struct ina219_config{
    uint8_t resolution;
    float shunt;
    float maxI;
}ina219_config_t;

/**
 * INA219 calibration parameters structure
 */
typedef struct ina219_calibaration{
    uint16_t cal;
    float lsb;
}ina219_calibration_t;

/**
 * Raw sensor values
 */
typedef struct ina219_data_raw{
    union{
        int rawCurrent;
        char rawCurrent_c[2];
    };
    union{
        int rawVoltage;
        char rawVoltage_c[2];
    };
}ina219_data_raw_t;

/**
 * Floating point sensor values
 */
typedef struct ina219_data{
    float current;
    float voltage;
}ina219_data_t;

/**
 * @brief INA219 driver class
 */
class ina219{
public:
    /**
     * @brief Creates an instance of the driver with the given parameters
     * 
     * @param addr I2C address
     * @param port I2C port number
     * @param sda I2C SDA GPIO pin
     * @param scl I2C SCL GPIO pin
     */
    ina219(uint8_t addr, i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

    /**
     * @brief Initialize INA219 sensor
     * 
     * @param config INA219 configuration
     * 
     * @returns
     *      ESP_OK if successfull
     *      ESP_ERR othervise 
     */
    esp_err_t init(ina219_config_t config);

    /**
     * @brief Polls for a measurement and get its result in floating point representation
     * 
     * @param data pointer to a data structure to be filled with the result
     * 
     * @returns
     *      ESP_OK if successfull
     *      ESP_ERR othervise 
     */
    esp_err_t getMeasurement(ina219_data_t* data);

    /**
     * @brief Reads data measured by the INA226 sensor
     * 
     * @returns
     *      ESP_OK if successfull
     *      ESP_ERR othervise 
     */ 
    esp_err_t readI(void);

    /**
     * @brief Gets result of a measurement in floating point representation
     * 
     * @param data pointer to a data structure to be filled with the result
     * 
     * @returns
     *      ESP_OK if successfull
     *      ESP_ERR othervise 
     */
    esp_err_t getResults(ina219_data_t* data);

    /**
     * @brief 2 byte I2C write operation to a given 8 register
     * 
     * @param reg Register address
     * @param data data to be written
     * 
     * @returns
     *      ESP_OK if successfull
     *      ESP_ERR othervise 
     */
    esp_err_t u16write(uint8_t reg, uint16_t data);

private:
    i2c_dev_t i2c;//i2c device descriptor
    ina219_config_t config;//sensor config
    ina219_calibration_t calibration;//sensor calibration
    ina219_data_raw_t data;//raw data buffer
};

/** @} */

#endif//INA219_H_