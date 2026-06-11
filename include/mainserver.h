#ifndef ___MAIN_SERVER__
#define ___MAIN_SERVER__
#include "global.h"
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

void main_server_task(void *pvParameters);

// Separate non-global handler prototypes

void handleFile(WebServer& server, const char* path, const char* type);
void handleSensors(WebServer& server, SystemHandles* handles);
void handlePower(WebServer& server, SystemHandles* handles);
void handleLed_1(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led);
void handleLed_2(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led);
void handleOff(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led);
void handleConnect(WebServer& server);
void connectToWiFi();
void startAP();





#endif