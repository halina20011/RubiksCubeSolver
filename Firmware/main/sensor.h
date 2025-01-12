#ifndef __SENSOR__
#define __SENSOR__

#include <freertos/FreeRTOS.h>
#include "driver/gpio.h"
#include <driver/i2c_master.h>

//       --- i2c start -->     
// sleep                   idle <--------- rgbc adc
//       <-- !PON --------  |               ^
//                          |               |       
//                    !WEN & AEN-------> rgbc init

// PON      power on      
// AEN      RGBC enable
// WEN      wait enable

#define SENSOR_ADDRESS      0x29
#define SENSOR_PORT         I2C_NUM_0
#define SENSOR_SCL          22
#define SENSOR_SDA          21
#define SENSOR_LED_PIN      19

#define COMMAND_BIT         (0x80)

#define REGISTER_ENABLE     (0x00)
#define REGISTER_ATIME      (0x01)
#define REGISTER_CONTROL    (0x0F)

// Clear data low byte
#define REGISTER_CDATAL     (0x14)
#define REGISTER_RDATAL     (0x16)
#define REGISTER_GDATAL     (0x18)
#define REGISTER_BDATAL     (0x1A)

#define PON 0
#define AEN 1

struct RawColor{
    uint16_t r, g, b, c;
};

struct Color{
    uint8_t r, g, b;
};

esp_err_t writeRegister(uint8_t regAddr, uint8_t value);
esp_err_t readRegisterN(uint8_t regAddr, uint8_t *data, size_t size);
uint16_t readRegister16(uint8_t regAddr);

void getRawColor(struct RawColor *color);
void getColor(struct Color *color);

void sensorLed(bool led);
void sensorInit(bool led);

#endif
