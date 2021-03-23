#include "ina219.h"

ina219::ina219(uint8_t addr, i2c_port_t port, gpio_num_t sda, gpio_num_t scl) {
    this -> i2c.port = port;
    this -> i2c.addr = addr;
    this -> i2c.cfg.sda_io_num = sda;
    this -> i2c.cfg.scl_io_num = scl;
    this -> i2c.cfg.master.clk_speed = INA219_DEFAULT_I2C_FREQ;
}//ina219

esp_err_t ina219::init(ina219_config_t config) {
    esp_err_t ret = i2c_dev_create_mutex(&this -> i2c);
    if (ret) return ret;

    this -> calibration.lsb = this -> config.maxI / INA219_MSB;
    this -> calibration.cal = (uint16_t)trunc(INA219_CAL / (this -> calibration.lsb * this -> config.shunt));

    ret = this -> u16write(INA219_CAL_REG, this -> calibration.cal);
    if (ret) return ret;

    uint16_t res;

    switch(this -> config.resolution){
        case RES_9b:
            res = INA219_CONFIG_BADCRES_9BIT | INA219_CONFIG_SADCRES_9BIT_1S_84US;
        break;

        case RES_10b:
            res = INA219_CONFIG_BADCRES_10BIT | INA219_CONFIG_SADCRES_10BIT_1S_148US;
        break;

        case RES_11b:
            res = INA219_CONFIG_BADCRES_11BIT | INA219_CONFIG_SADCRES_11BIT_1S_276US;
        break;

        default:
            res = INA219_CONFIG_BADCRES_12BIT | INA219_CONFIG_SADCRES_12BIT_128S_69MS;
        break;
    }
    
    ret = this -> u16write(INA219_CONF_REG, INA219_CONFIG_BVOLTAGERANGE_16V | INA219_CONFIG_GAIN_1_40MV | res | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS);

    return ret;
}//init

esp_err_t ina219::getMeasurement(ina219_data_t* data) {
    esp_err_t ret = this -> readI();
    if (ret) return ret;

    this -> getResults(data);

    return ret;
}//getMeasurement

esp_err_t ina219::readI(void) {
    esp_err_t ret = i2c_dev_read_reg(&this -> i2c, INA219_I_REG, this -> data.rawCurrent_c, sizeof(this -> data.rawCurrent_c));
    if (ret) return ret;
    
    this -> data.rawCurrent = (this -> data.rawCurrent_c[0] << 8) | (this -> data.rawCurrent_c[1]);

    #ifdef _DEBUG
    //cout << "raw uint16 data: " << this -> data.rawCurrent + 0 << endl;
    
    #endif

    return ret;
}//readI

esp_err_t ina219::getResults(ina219_data_t* data) {
    data -> current = (float)this -> data.rawCurrent * this -> calibration.lsb;
    return ESP_OK;
}//getResults

esp_err_t ina219::u16write(uint8_t reg, uint16_t data) {
    uint8_t buffer[2];
    buffer[1] = (data >> 8) & 0xff;
    buffer[2] = data & 0xff;

    esp_err_t ret = i2c_dev_write(&this -> i2c, &reg, sizeof(reg), buffer, sizeof(buffer));

    return ret;
}//u16write