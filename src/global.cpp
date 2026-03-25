#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;
char lcd_buffer[2][16] = {"Temp: 0.00C", "Humi: 0.00"};

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;
SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();

SemaphoreHandle_t xI2CMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t xDataMutex = xSemaphoreCreateMutex();

void set_sensor_data(float temp, float humi) {
    if (xDataMutex != NULL) {
        xSemaphoreTake(xDataMutex, portMAX_DELAY);
        glob_temperature = temp;
        glob_humidity = humi;
        xSemaphoreGive(xDataMutex);
    }
}

void get_sensor_data(float *temp, float *humi) {
    if (xDataMutex != NULL) {
        xSemaphoreTake(xDataMutex, portMAX_DELAY);
        if (temp) *temp = glob_temperature;
        if (humi) *humi = glob_humidity;
        xSemaphoreGive(xDataMutex);
    }
}