#include "global.h"
float glob_temperature = 0;
float glob_humidity = 0;
int glob_humi_state = 1;
char lcd_buffer[2][17] = {"Temp: 0.00C", "Humi: 0.00"};

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

void set_humi_state(int state) {
    if (xDataMutex != NULL) {
        xSemaphoreTake(xDataMutex, portMAX_DELAY);
        glob_humi_state = state;
        xSemaphoreGive(xDataMutex);
    }
}

int get_humi_state() {
    int state = 1;
    if (xDataMutex != NULL) {
        xSemaphoreTake(xDataMutex, portMAX_DELAY);
        state = glob_humi_state;
        xSemaphoreGive(xDataMutex);
    }
    return state;
}