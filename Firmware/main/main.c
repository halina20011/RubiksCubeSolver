#include "motor.h"
#include "pinout.h"

#include "nvs_flash.h"

#include "sensor.h"
#include "wifi.h"
#include "mcode.h"

void mainLoop(){
    while(1){
        processCommandQueue();
    }
}

void app_main(void){
    ESP_ERROR_CHECK(nvs_flash_init());

    commandQueueInit();
    // sensorInit(1);
    motorsInit();
    wifiInit();
    xTaskCreate(mainLoop, "MainLoop", 4096, NULL, 2, NULL);
    
    // bool led = 0;
    // struct Color color = {};
    // while(1){
    //     // motorTest(&m1);
    //     // motorTest(&m4);
    //     // sensorLed(led);
    //     getColor(&color);
    //     printf("%i %i %i\n", color.r, color.g, color.b);
    //     // led = !led;
    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
}
