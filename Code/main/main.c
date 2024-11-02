#include "motor.h"
#include "pinout.h"

void app_main(void){
    struct Motor m1 = {};
    motorInit(&m1, MOTOR1_DIR, MOTOR1_STEP);

    struct Motor m2 = {};
    motorInit(&m2, MOTOR2_DIR, MOTOR2_STEP);

    struct Motor m3 = {};
    motorInit(&m3, MOTOR3_DIR, MOTOR3_STEP);

    struct Motor m4 = {};
    motorInit(&m4, MOTOR4_DIR, MOTOR4_STEP);

    struct Motor m5 = {};
    motorInit(&m5, MOTOR5_DIR, MOTOR5_STEP);
    
    while(1){
        // motorTest(&m1);
        motorTest(&m4);
    }
}
