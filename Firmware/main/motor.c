#include "motor.h"

struct Motor motors[5] = {};

void motorsInit(){
    motorInit(&motors[0], MOTOR1_DIR, MOTOR1_STEP);
    motorInit(&motors[1], MOTOR2_DIR, MOTOR2_STEP);
    motorInit(&motors[2], MOTOR3_DIR, MOTOR3_STEP);
    motorInit(&motors[3], MOTOR4_DIR, MOTOR4_STEP);
    motorInit(&motors[4], MOTOR5_DIR, MOTOR5_STEP);
}

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

void motorRotateDeg(int id, int deg, int direction){
    bool dir = (direction != 0);
    const struct Motor *motor = &motors[id];
    gpio_set_level(motor->dirPin, dir);

    for(uint32_t i = 0; i < deg; i++){
        gpio_set_level(motor->stepPin, 1);
        esp_rom_delay_us(DELAY_MS);
        gpio_set_level(motor->stepPin, 0);
        esp_rom_delay_us(DELAY_MS);
    }
}
