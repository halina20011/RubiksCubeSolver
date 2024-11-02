#include "motor.h"

void motorInit(struct Motor *motor, uint8_t dirPin, uint8_t stepPin){
    motor->dirPin = dirPin;
    motor->stepPin = stepPin;

    gpio_config_t pinConf = {};
    // dissable interrupt
    pinConf.intr_type = GPIO_INTR_DISABLE;
    // set them as output mode
    pinConf.mode = GPIO_MODE_OUTPUT;
    // bit masl for targeted pins
    pinConf.pin_bit_mask = (1ULL << dirPin) | (1ULL << stepPin);
    // dissable pull up, pull down
    pinConf.pull_down_en = 0;
    pinConf.pull_up_en = 0;
    gpio_config(&pinConf);
}

#define DELAY_MS    100
#define STEPS       5000

void motorTest(struct Motor *motor){
    gpio_set_level(motor->dirPin, 1);

    for(uint32_t i = 0; i < STEPS; i++){
        gpio_set_level(motor->stepPin, 1);
        esp_rom_delay_us(DELAY_MS);
        gpio_set_level(motor->stepPin, 0);
        esp_rom_delay_us(DELAY_MS);
    }

    vTaskDelay(100);
}
