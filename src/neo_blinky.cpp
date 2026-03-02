#include "neo_blinky.h"


void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.setBrightness(30); // set brightness 
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    while(1) {                          
        // strip.setPixelColor(0, strip.Color(100, 45, 200)); // Set pixel 0 to red
        // strip.show(); // Update the strip

        // // Wait for 500 milliseconds
        // vTaskDelay(500);

        // // Set the pixel to off
        // strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel 0 off
        // strip.show(); // Update the strip

        // // Wait for another 500 milliseconds
        // vTaskDelay(500);

        double currentTemp = glob_temperature;
        //temp < 10, blinky led neo 
        if(currentTemp < 10){
            strip.setPixelColor(0, strip.Color(0, 0, 255)); // Set pixel 0 to blue
            strip.show(); // Update the strip

            // Wait for 500 milliseconds
            vTaskDelay(200);

            // Set the pixel to off
            strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel 0 off
            strip.show(); // Update the strip

            // Wait for another 500 milliseconds
            vTaskDelay(200);
        }
        //temp > 50, blinky led neo
        else if(currentTemp > 40){
            strip.setPixelColor(0, strip.Color(255, 0, 0)); // Set pixel 0 to red
            strip.show(); // Update the strip

            // Wait for 200 milliseconds
            vTaskDelay(200);

            // Set the pixel to off
            strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel 0 off
            strip.show(); // Update the strip

            // Wait for another 200 milliseconds
            vTaskDelay(200);
        }
        else{
        //different temp
        int colorStep = (int)(currentTemp / 2);
        double ratio = colorStep / 100.0;

        uint8_t red = (uint8_t)(ratio * 255);
        uint8_t blue = 255 - red;
        uint8_t green = 0;

        strip.setPixelColor(0, strip.Color(red, green, blue));
        strip.show();

        // Serial.print("Color: ("); Serial.print(red); Serial.print(", ");
        //                         Serial.print(green); Serial.print(", ");
        //                         Serial.print(blue); Serial.print(")");
        // Serial.println();
        vTaskDelay(1000);
        }
    }
}