#ifndef __NEO_BLINKY__
#define __NEO_BLINKY__
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"


#define TASK_PERIOD_MS          20   // task run per 20ms
#define UPDATE_DATA_INTERVAL    1000 // update data per 1s
#define BLINK_FAST_MS           200  // blink 200ms
#define BLINK_SLOW_MS           500  // blink 500ms

// revert ms to tick
#define TICKS_PER_UPDATE     (UPDATE_DATA_INTERVAL / TASK_PERIOD_MS) // = 50
#define TICKS_BLINK_FAST     (BLINK_FAST_MS / TASK_PERIOD_MS)        // = 10
#define TICKS_BLINK_SLOW     (BLINK_SLOW_MS / TASK_PERIOD_MS)        // = 25

#define NEO_PIN 45
#define LED_COUNT 1 

void neo_blinky(void *pvParameters);

#endif