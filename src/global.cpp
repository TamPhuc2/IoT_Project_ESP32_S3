#include "global.h"

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN;
String CORE_IOT_SERVER; 
String CORE_IOT_PORT;

String ssid = "TP NETWOK";
String password = "12345678";
String wifi_ssid = "tp";
String wifi_password = "tamphuc123";
boolean isWifiConnected = false;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();