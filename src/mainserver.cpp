#include "mainserver.h"

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
    
    // Sử dụng Queue (Zero-Global) để lấy dữ liệu cảm biến
    if (handles->qLcd != NULL) {
        xQueuePeek(handles->qLcd, &d, 0);
    }
    String json = "{\"temp\":" + String(d.temperature, 1) + ",\"hum\":" + String(d.humidity, 1) + "}";
    server.send(200, "application/json", json);
}

// Handler for LED 1 toggle (Zero-Global, Mutex protected)
void handleLed_1(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel& rgb_4_led) {
    if (server.hasArg("state")) {
        String state = server.arg("state");
        bool turnOn = (state == "on");
        
        // Cần bảo vệ `handles->deviceState` vì có nhiều tác vụ có thể truy cập state cùng một lúc.
        // Sử dụng Mutex để bảo vệ quyền truy cập vào Struct dữ liệu khi có nhiều Task cùng đọc/ghi.
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        
        handles->deviceState.led_1 = turnOn;
        rgb_4_led.setPixelColor(LED_1_PIN, turnOn ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
        rgb_4_led.show();
        
        xSemaphoreGive(handles->mutexDeviceState);
        
        server.send(200, "text/plain", turnOn ? "LED 1 ON" : "LED 1 OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

// Handler for LED 2 toggle (Zero-Global, Mutex protected)
void handleLed_2(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel& rgb_4_led) {
    if (server.hasArg("state")) {
        String state = server.arg("state");
        bool turnOn = (state == "on");

        // Sử dụng Mutex để bảo vệ quyền truy cập vào Struct dữ liệu khi có nhiều Task cùng đọc/ghi.
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        
        handles->deviceState.led_2 = turnOn;
        rgb_4_led.setPixelColor(LED_2_PIN, turnOn ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
        rgb_4_led.show();
        
        xSemaphoreGive(handles->mutexDeviceState);
        
        server.send(200, "text/plain", turnOn ? "LED 2 ON" : "LED 2 OFF");
    } else {
        server.send(400, "text/plain", "Missing state");
    }
}

// Handler to turn off everything (Zero-Global, Mutex protected)
void handleAllOff(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel& rgb_4_led) {
    // Sử dụng Mutex để tắt an toàn nhiều thiết bị, khóa mọi sự truy cập nội bộ khác cho đến khi xong.
    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
    
    handles->deviceState.led_1 = false;
    handles->deviceState.led_2 = false;
    rgb_4_led.setPixelColor(LED_1_PIN, rgb_4_led.Color(0, 0, 0));
    rgb_4_led.setPixelColor(LED_2_PIN, rgb_4_led.Color(0, 0, 0));
    rgb_4_led.show();
    
    xSemaphoreGive(handles->mutexDeviceState);
    server.send(200, "text/plain", "All devices OFF");
}

// Placeholder for WiFi connection
void handleConnect(WebServer& server) {
    server.send(200, "text/plain", "Connecting...");
}

// Start AP (Dual Mode Compliance)
void startAP() {
    // Fail-safe Network Manager: Thiết lập chế độ Dual Mode (AP + STA)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid.c_str(), password.c_str());
    Serial.println("ESP32 Started in AP+STA Mode.");
}

void main_server_task(void *pvParameters) {
    SystemHandles* handles = (SystemHandles*)pvParameters;
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }

    pinMode(LED_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(0, INPUT_PULLUP);

    // ZERO-GLOBAL: Object NeoPixel được khởi tạo toàn cục bên trong Task và truyền xuống bằng tham chiếu (reference).
    Adafruit_NeoPixel rgb_4_led(4, 8, NEO_GBR + NEO_KHZ800);
    rgb_4_led.begin();
    rgb_4_led.setBrightness(30);
    rgb_4_led.clear();
    rgb_4_led.show();

    // Local WebServer instance
    WebServer server(80);

    // Khởi tạo các Route tĩnh
    server.on("/", [&server]() { handleFile(server, "/index.html", "text/html"); });
    server.on("/style.css", [&server]() { handleFile(server, "/style.css", "text/css"); });
    server.on("/script.js", [&server]() { handleFile(server, "/script.js", "application/javascript"); });
    
    // Serve Icon Files
    server.onNotFound([&server]() {
        String uri = server.uri();
        if (uri.startsWith("/icon/")) {
            handleFile(server, uri.c_str(), "image/png");
        } else {
            server.send(404, "text/plain", "Not Found");
        }
    });

    // Zero-Global Handlers, truyền cả Server, Handles, và tham chiếu NeoPixel
    server.on("/sensors", HTTP_GET, [&server, handles]() { handleSensors(server, handles); });
    server.on("/led1", HTTP_GET, [&server, handles, &rgb_4_led]() { handleLed_1(server, handles, rgb_4_led); });
    server.on("/led2", HTTP_GET, [&server, handles, &rgb_4_led]() { handleLed_2(server, handles, rgb_4_led); });
    server.on("/all/off", HTTP_GET, [&server, handles, &rgb_4_led]() { handleAllOff(server, handles, rgb_4_led); });

    startAP();
    server.begin();

    // Vòng lặp Main Server hoạt động theo kiểu Non-blocking đối với luồng hệ thống RTOS
    while (1) {
        server.handleClient();
        
        // BOOT Button để ép lại AP mode khi cần
        if (digitalRead(0) == LOW) {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (digitalRead(0) == LOW) {
                startAP();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // prevent Watchdog timeout
    }
}