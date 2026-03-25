#include "led_blinky.h"

void led_blinky(void *pvParameters){
    pinMode(LED_GPIO, OUTPUT);
  
    while(1){
        float currentTemp = 25.0f; // init with safe normal value
        get_sensor_data(&currentTemp, NULL);
        int delay_time = 1000; 

        //(,10] [50,)
        if(currentTemp <= THRESHOLD_TEMP_CRITICAL_LOW || currentTemp >= THRESHOLD_TEMP_CRITICAL_HIGH){
          delay_time = 100;  
        } 
        //[20, 40)
        else if(currentTemp >= THRESHOLD_TEMP_WARNING_LOW && currentTemp < THRESHOLD_TEMP_WARNING_HIGH){
          delay_time = 1000; 
        } 
        else{
          delay_time = 500;
        }

        digitalWrite(LED_GPIO, HIGH);  // turn the LED ON
        vTaskDelay(delay_time);
        digitalWrite(LED_GPIO, LOW);   // turn the LED OFF
        vTaskDelay(delay_time);
    }
}
