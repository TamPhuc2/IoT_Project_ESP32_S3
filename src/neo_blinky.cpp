#include "neo_blinky.h"

void neo_blinky(void *pvParameters){
    SystemHandles* handles = (SystemHandles*)pvParameters;
    SensorData data;
    
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.setBrightness(30); // set brightness 
    strip.clear();
    strip.show();

    // rgb color set up
    float current_r = 0, current_g = 0, current_b = 0;
    float target_r = 0, target_g = 0, target_b = 0;
    float currentHumi = 50.0; // init value
    
    int tick_count = 0;

    while(1){   
        // update humi value after 1s              
        if(tick_count % TICKS_PER_UPDATE == 0){
            if(xQueuePeek(handles->qNeo, &data, 0) == pdTRUE){
                currentHumi = data.humidity;
            }
            
            // humidity threshold
            if(currentHumi < 0) currentHumi = 0;
            if(currentHumi > 100) currentHumi = 100;

            //mapping rgb color with humidity value
            if(currentHumi <= 50.0) {
                // 0% -> 50%: grey (128,128,128) -> green (0,255,0)
                float ratio = currentHumi / 50.0;
                target_r = 128.0 - 128.0 * ratio;
                target_g = 128.0 + 127.0 * ratio;
                target_b = 128.0 - 128.0 * ratio;
            } 
            else{
                // 50% -> 100%: green (0,255,0) -> red (255,0,0)
                float ratio = (currentHumi - 50.0) / 50.0;
                target_r = 255.0 * ratio;
                target_g = 255.0 - 255.0 * ratio;
                target_b = 0;
            }
        }

        // smoothy animation 
        current_r += (target_r - current_r) * 0.05;
        current_g += (target_g - current_g) * 0.05;
        current_b += (target_b - current_b) * 0.05;

        // Logic blinky rgb led and state
        bool led_on = true;
        
        // blinky each 200ms - fast
        if(currentHumi < HUMI_CRITICAL_COLD || currentHumi >= HUMI_HOT) {
            led_on = ((tick_count / TICKS_BLINK_FAST) % 2 == 0); 
        }
        else if(currentHumi >= HUMI_COOL && currentHumi < HUMI_NORMAL) {
            led_on = true;                         
        }
        // blinky each 500ms - slow
        else {
            led_on = ((tick_count / TICKS_BLINK_SLOW) % 2 == 0);
        }

        if(led_on){
            strip.setPixelColor(0, strip.Color((uint8_t)current_r, (uint8_t)current_g, (uint8_t)current_b));
        }
        else{
            strip.setPixelColor(0, strip.Color(0, 0, 0));
        }
        strip.show();

        tick_count++;
        vTaskDelay(TASK_PERIOD_MS); // Delay 20ms for smoothy 50 FPS 
    }
}