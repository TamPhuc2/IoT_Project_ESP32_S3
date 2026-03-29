#include "mainserver.h"

// Shared static NeoPixel object for web server LED control
// Declared at file scope so both handleLed_1 and handleLed_2 share the same pixel buffer
static Adafruit_NeoPixel rgb_4_led(4, 8, NEO_GBR + NEO_KHZ800);
static bool rgb_4_led_initialized = false;

static void ensureRgbInit() {
    if (!rgb_4_led_initialized) {
        rgb_4_led.begin();
        rgb_4_led.setBrightness(30);
        rgb_4_led.clear();
        rgb_4_led.show();
        rgb_4_led_initialized = true;
    }
}


// Helper to serve files from SPIFFS
void handleFile(WebServer& server, const char* path, const char* type) {
    File file = SPIFFS.open(path, "r");
    if (file) {
        server.streamFile(file, type);
        file.close();
    } else {
        server.send(404, "text/plain", "File not found");
    }
}

// Handler for sensor data JSON
void handleSensors(WebServer& server, SystemHandles* handles) {
    SensorData d = {0, 0, 0};
    // Peek from queue if available
    if (handles->qLcd != NULL) {
        xQueuePeek(handles->qLcd, &d, 0);
    }
    String json = "{\"temp\":" + String(d.temperature, 1) + ",\"hum\":" + String(d.humidity, 1) + "}";
    server.send(200, "application/json", json);
}

// Handler for Power toggle
void handlePower(WebServer& server, SystemHandles* handles) {
    if (server.hasArg("state")) {
        String state = server.arg("state");
        bool turnOn = (state == "on");
        
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        handles->deviceState.powerOn = turnOn;
        digitalWrite(POWER_PIN, turnOn ? HIGH : LOW);
        xSemaphoreGive(handles->mutexDeviceState);
        
        server.send(200, "text/plain", turnOn ? "Power ON" : "Power OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

// Handler for LED toggle
void handleLed_1(WebServer& server, SystemHandles* handles) {
    if (server.hasArg("state")) {
        String state = server.arg("state");
        bool turnOn = (state == "on");

        ensureRgbInit(); // Dùng shared object, không tạo mới
        
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        handles->deviceState.led_1 = turnOn;
        // Set pixel LED_1 theo state, pixel LED_2 giữ nguyên (không reset)
        rgb_4_led.setPixelColor(LED_1_PIN, turnOn ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
        rgb_4_led.show();
        xSemaphoreGive(handles->mutexDeviceState);
        
        server.send(200, "text/plain", turnOn ? "LED ON" : "LED OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

// Handler for Fan toggle
void handleLed_2(WebServer& server, SystemHandles* handles) {
    if (server.hasArg("state")) {
        String state = server.arg("state");
        bool turnOn = (state == "on");

        ensureRgbInit(); // Dùng shared object, không tạo mới

        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        handles->deviceState.led_2 = turnOn;
        // Set pixel LED_2 theo state, pixel LED_1 giữ nguyên (không reset)
        rgb_4_led.setPixelColor(LED_2_PIN, turnOn ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
        rgb_4_led.show();
        xSemaphoreGive(handles->mutexDeviceState);
        
        server.send(200, "text/plain", turnOn ? "Fan ON" : "Fan OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

// Placeholder for WiFi connection (can be expanded)
void handleConnect(WebServer& server) {
    server.send(200, "text/plain", "Connecting...");
}

void startAP() {
    WiFi.mode(WIFI_AP);
    // Use globals for ssid/password as they are defined in global.cpp
    WiFi.softAP(ssid.c_str(), password.c_str());
}

void main_server_task(void *pvParameters) {
    SystemHandles* handles = (SystemHandles*)pvParameters;
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }
    else {
        Serial.println("Ok roi ku");
    }

    // Initialize Pins
    pinMode(POWER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(0, INPUT_PULLUP); // BOOT Button

    // Local WebServer instance
    WebServer server(80);

    // Setup Routes using Lambdas to capture server and handles
    server.on("/", [&server]() { handleFile(server, "/index.html", "text/html"); });
    server.on("/style.css", [&server]() { handleFile(server, "/style.css", "text/css"); });
    server.on("/script.js", [&server]() { handleFile(server, "/script.js", "application/javascript"); });
    
    // Serve icons dynamically
    server.onNotFound([&server]() {
        String uri = server.uri();
        if (uri.startsWith("/icon/")) {
            handleFile(server, uri.c_str(), "image/png");
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    });

    server.on("/sensors", HTTP_GET, [&server, handles]() { handleSensors(server, handles); });
    server.on("/power", HTTP_GET, [&server, handles]() { handlePower(server, handles); });
    server.on("/led", HTTP_GET, [&server, handles]() { handleLed_1(server, handles); });
    server.on("/fan", HTTP_GET, [&server, handles]() { handleLed_2(server, handles); });

    startAP();
    server.begin();

    while (1) {
        server.handleClient();
        
        // BOOT Button to force AP mode (if switched to STA)
        if (digitalRead(0) == LOW) {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (digitalRead(0) == LOW) {
                startAP();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // prevent Watchdog timeout
    }
}