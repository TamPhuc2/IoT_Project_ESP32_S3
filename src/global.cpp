#include "global.h"

String WIFI_SSID;
String WIFI_PASS;
String CORE_IOT_TOKEN = "ohvefr8ygpajb7f9fr9n";
String CORE_IOT_SERVER = "app.coreiot.io"; 
String CORE_IOT_PORT = "1883";

String ssid = "TP NETWOK";
String password = "12345678";
String wifi_ssid = "tp";
String wifi_password = "tamphuc123";
boolean isWifiConnected = false;

SemaphoreHandle_t xBinarySemaphoreInternet = xSemaphoreCreateBinary();