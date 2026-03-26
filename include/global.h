#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"


#define TEMP_CRITICAL_COLD      10
#define TEMP_COOL               18
#define TEMP_NORMAL 		    30
#define TEMP_HOT		        35

#define HUMI_CRITICAL_COLD 	    30
#define HUMI_COOL 		        40
#define HUMI_NORMAL 		    60
#define HUMI_HOT 		        70


#define DEFAULT_STATE       0
#define CRITICAL_COLD       1
#define COOL                2
#define NORMAL              3
#define HOT                 4
#define CRITICAL_HOT        5

extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;
extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

// --- RTOS DATA STRUCTURES ---

// Struct for data transmission across queues
struct SensorData {
    float temperature;
    float humidity;
    int state;
};

// Struct holding system handles injected into tasks
struct SystemHandles {
    QueueHandle_t qLed;         // Queue for led_blinky task
    QueueHandle_t qNeo;         // Queue for neo_blinky task
    QueueHandle_t qLcd;         // Queue for temp_humi_lcd_display task
    SemaphoreHandle_t semLcd;   // Binary semaphore to wake up LCD
    SemaphoreHandle_t mutexI2C; // Mutex to protect I2C line
};

#endif