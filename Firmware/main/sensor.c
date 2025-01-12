#include "sensor.h"

#define E ESP_ERROR_CHECK
#define I2C_TIMEOUT_MS 1000

i2c_master_dev_handle_t devHandle = NULL;

esp_err_t writeRegister(uint8_t regAddr, uint8_t value){
    uint8_t data[2] = {COMMAND_BIT | regAddr, value};
    return i2c_master_transmit(devHandle, data, sizeof(data), I2C_TIMEOUT_MS);
}

esp_err_t readRegisterN(uint8_t regAddr, uint8_t *data, size_t size){
    uint8_t writeBuffer = COMMAND_BIT | regAddr;
    return i2c_master_transmit_receive(devHandle, &writeBuffer, 1, data, size, I2C_TIMEOUT_MS);
}

uint16_t readRegister16(uint8_t regAddr){
    uint8_t buffer[2] = {(uint8_t)COMMAND_BIT | regAddr, 0};
    i2c_master_transmit_receive(devHandle, buffer, 2, buffer, 2, I2C_TIMEOUT_MS);
    
    uint16_t low = buffer[0] & 0xff;
    uint16_t high = (uint16_t)((uint16_t)(buffer[1]) << 8);
    
    return high | low;
}

void getRawColor(struct RawColor *color){
    color->c = readRegister16(REGISTER_CDATAL);
    color->r = readRegister16(REGISTER_RDATAL);
    color->g = readRegister16(REGISTER_GDATAL);
    color->b = readRegister16(REGISTER_BDATAL);
}

void getColor(struct Color *color){
    struct RawColor rawColor = {};
    getRawColor(&rawColor);
    if(rawColor.c == 0){
        return;
    }

    color->r = (float)rawColor.r / rawColor.c * 255;
    color->g = (float)rawColor.g / rawColor.c * 255;
    color->b = (float)rawColor.b / rawColor.c * 255;
}

void sensorLed(bool led){
    gpio_set_level(SENSOR_LED_PIN, led);
}

void sensorInit(bool led){
    gpio_config_t pinConf = {};
    // dissable interrupt
    pinConf.intr_type = GPIO_INTR_DISABLE;
    // set them as output mode
    pinConf.mode = GPIO_MODE_OUTPUT;
    // bit masl for targeted pins
    pinConf.pin_bit_mask = (1ULL << SENSOR_LED_PIN);
    // dissable pull up, pull down
    pinConf.pull_down_en = 0;
    pinConf.pull_up_en = 0;
    gpio_config(&pinConf);

    sensorLed(led);

    // initialize sensor
    i2c_master_bus_config_t busConfig = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = SENSOR_PORT,
        .scl_io_num = SENSOR_SCL,
        .sda_io_num = SENSOR_SDA,
    };
    i2c_master_bus_handle_t busHandle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&busConfig, &busHandle));

    i2c_device_config_t sensor = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SENSOR_ADDRESS,
        .scl_speed_hz = 100000
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(busHandle, &sensor, &devHandle));

    // write to enable register
    uint8_t enableRegister = (1 << PON);
    // power on
    E(writeRegister(REGISTER_ENABLE, enableRegister));
    vTaskDelay(pdMS_TO_TICKS(3));
    
    // enable RGBC
    E(writeRegister(REGISTER_ENABLE, (1 << AEN) | (1 << PON)));

    // integration time: 100ms
    E(writeRegister(REGISTER_ATIME, 0xd6));

    // set gain to 4x
    E(writeRegister(REGISTER_CONTROL, 0x01));
}
