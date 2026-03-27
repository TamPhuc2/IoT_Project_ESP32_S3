#include "global.h"

String ssid = "ESP32-YOUR NETWORK HERE!!!";
String password = "12345678";
String wifi_ssid = "abcde";
String wifi_password = "123456789";
boolean isWifiConnected = false;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();