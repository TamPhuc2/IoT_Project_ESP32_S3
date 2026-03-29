#ifndef ___MAIN_SERVER__
#define ___MAIN_SERVER__
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"

#define LED1_PIN 48
#define LED2_PIN 41
#define BOOT_PIN 0
String mainPage(bool led1, bool led2, float temp, float humi);
String settingsPage();

void main_server_task(void *pvParameters);

#endif