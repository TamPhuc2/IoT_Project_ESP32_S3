#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// threhold of temp and humi
#define TEMP_CRITICAL_COLD      10
#define TEMP_COOL               18
#define TEMP_NORMAL 		    30
#define TEMP_HOT		        35

#define HUMI_CRITICAL_COLD 	    30
#define HUMI_COOL 		        40
#define HUMI_NORMAL 		    60
#define HUMI_HOT 		        70

// state of LCD display
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


extern String WIFI_SSID;
extern String WIFI_PASS;
extern String CORE_IOT_TOKEN;
extern String CORE_IOT_SERVER;
extern String CORE_IOT_PORT;

// --- RTOS DATA STRUCTURES ---
// struct for data transmission across queues
struct SensorData {
    float temperature;
    float humidity;
    int state;
};

#define POWER_PIN 47
#define LED_PIN 38
#define FAN_PIN 48

#define LED_1_PIN   0
#define LED_2_PIN   1

// struct holding device states for Web Server
struct DeviceStates {
    bool powerOn;
    bool led_1;
    bool led_2;
};

// struct holding system handles injected into tasks
struct SystemHandles {
    QueueHandle_t qLed;         // queue for led_blinky task
    QueueHandle_t qNeo;         // queue for neo_blinky task
    QueueHandle_t qLcd;         // queue for temp_humi_lcd_display task
    SemaphoreHandle_t semLcd;   // binary semaphore to wake up LCD
    SemaphoreHandle_t mutexI2C; // mutex of I2C bus
    SemaphoreHandle_t mutexDeviceState; // mutex for DeviceStates
    DeviceStates deviceState;   // device states protected by mutex
};

#endif