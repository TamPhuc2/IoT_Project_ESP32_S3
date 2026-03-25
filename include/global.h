#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Humidity Thresholds
#define THRESHOLD_HUMI_CRITICAL_LOW   (10.0f)
#define THRESHOLD_HUMI_WARNING_LOW    (30.0f)
#define THRESHOLD_HUMI_WARNING_HIGH   (70.0f)
#define THRESHOLD_HUMI_CRITICAL_HIGH  (90.0f)

extern float glob_temperature;
extern float glob_humidity;

extern char lcd_buffer[2][17];
extern SemaphoreHandle_t xI2CMutex;

extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;
extern boolean isWifiConnected;


extern boolean isWifiConnected;
extern SemaphoreHandle_t xBinarySemaphoreInternet;

extern SemaphoreHandle_t xDataMutex;
void set_sensor_data(float temp, float humi);
void get_sensor_data(float *temp, float *humi);
void set_humi_state(int state);
int get_humi_state();

#endif