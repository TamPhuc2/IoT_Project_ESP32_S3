#include "led_blinky.h"

void led_blinky(void *pvParameters){
    SystemHandles* handles = (SystemHandles*)pvParameters;
    SensorData data;
    float currentTemp = 25.0f; // init with safe normal value

    pinMode(LED_GPIO, OUTPUT);
  
    while(1){
        // non-blocking peek from queue
        if(xQueuePeek(handles->qLed, &data, 0) == pdTRUE){
          // read success
          currentTemp = data.temperature;
        }

        int delay_time = 1000; 

        // set delay time of each state
        // critical cold or critical hot
        if(currentTemp < TEMP_CRITICAL_COLD || currentTemp >= TEMP_HOT){
          delay_time = 100;  
        } 
        // normal
        else if(currentTemp >= TEMP_COOL && currentTemp < TEMP_NORMAL){
          delay_time = 1000; 
        } 
        // cool or hot
        else{
          delay_time = 500;
        }

        digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
        vTaskDelay(delay_time);
        digitalWrite(LED_GPIO, LOW);   // turn the LED OFF
        vTaskDelay(delay_time);
    }
}
