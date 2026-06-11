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

void handleStatus(WebServer& server, SystemHandles* handles) {
  String json = "{";
  json += "\"led1\":" + String(handles->deviceState.led_1 ? 1 : 0);
  json += ",\"led2\":" + String(handles->deviceState.led_2 ? 1 : 0);
  json += "}";
  Serial.println(json);
  server.send(200, "application/json", json);
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

void handleLed_1(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led) {

    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);

    if (server.hasArg("state")) {
        String state = server.arg("state");
        handles->deviceState.led_1 = (state == "on");
    } else {
        handles->deviceState.led_1 = !handles->deviceState.led_1;
    }

    bool led1 = handles->deviceState.led_1;
    bool led2 = handles->deviceState.led_2;

    rgb_4_led.setPixelColor(LED_1_PIN,
        led1 ? rgb_4_led.Color(255,255,255)
             : rgb_4_led.Color(0,0,0));
    rgb_4_led.show();

    xSemaphoreGive(handles->mutexDeviceState);

    // ===== JSON LOG =====
    String json = "{";
    json += "\"led1\":" + String(led1 ? 1 : 0);
    json += ",\"led2\":" + String(led2 ? 1 : 0);
    json += "}";
    Serial.println(json);

    server.send(200, "application/json", json);
}

void handleLed_2(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led) {

    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);

    if (server.hasArg("state")) {
        String state = server.arg("state");
        handles->deviceState.led_2 = (state == "on");
    } else {
        handles->deviceState.led_2 = !handles->deviceState.led_2;
    }

    bool led1 = handles->deviceState.led_1;
    bool led2 = handles->deviceState.led_2;

    rgb_4_led.setPixelColor(LED_2_PIN,
        led2 ? rgb_4_led.Color(255,255,255)
             : rgb_4_led.Color(0,0,0));
    rgb_4_led.show();

    xSemaphoreGive(handles->mutexDeviceState);

    // ===== JSON LOG =====
    String json = "{";
    json += "\"led1\":" + String(led1 ? 1 : 0);
    json += ",\"led2\":" + String(led2 ? 1 : 0);
    json += "}";
    Serial.println(json);

    server.send(200, "application/json", json);
}

void handleOff(WebServer& server, SystemHandles* handles, Adafruit_NeoPixel &rgb_4_led) {

    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);

    handles->deviceState.led_1 = false;
    handles->deviceState.led_2 = false;

    rgb_4_led.setPixelColor(LED_1_PIN, rgb_4_led.Color(0,0,0));
    rgb_4_led.setPixelColor(LED_2_PIN, rgb_4_led.Color(0,0,0));
    rgb_4_led.show();

    xSemaphoreGive(handles->mutexDeviceState);

    // ===== JSON LOG =====
    String json = "{\"led1\":0,\"led2\":0}";
    Serial.println(json);

    server.send(200, "application/json", json);
}

void handleTinyML(WebServer& server, SystemHandles* handles) {
    String switchParam = server.arg("switch");
    String json;

    if (switchParam == "1") {
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        if (!handles->deviceState.tinyml_mode) {
            handles->deviceState.tinyml_mode = true;
            xSemaphoreGive(handles->mutexDeviceState);
            xSemaphoreGive(handles->semLcd); // wake up LCD
        } else {
            xSemaphoreGive(handles->mutexDeviceState);
        }

        // Switch bật lấy thông tin tinyML
        int trigger = 1;
        xQueueSend(handles->qTrigger, &trigger, 0);
        TinyMLData predict_data = {0, ""};
        // Đọc kết quả TinyML
        if (handles->qTinyML != NULL && 
            xQueuePeek(handles->qTinyML, &predict_data, 100) == pdTRUE) {
            
            json = "{";
            json += "\"state\":\"on\",";
            json += "\"label\":\"" + String(predict_data.predict_state) + "\",";
            json += "\"value\":" + String(predict_data.predict_value);
            json += "}";
            Serial.println(predict_data.predict_value);
            Serial.println(predict_data.predict_state);
        } else {
            json = "{\"state\":\"on\",\"label\":\"WAITING\"}";
        }
    } else {
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        if (handles->deviceState.tinyml_mode) {
            handles->deviceState.tinyml_mode = false;
            xSemaphoreGive(handles->mutexDeviceState);
            xSemaphoreGive(handles->semLcd); // wake up LCD
        } else {
            xSemaphoreGive(handles->mutexDeviceState);
        }

        json = "{\"state\":\"\"}";
    }

    server.send(200, "application/json", json);
}



// Handle dynamic config from test.html
void handleConnect(WebServer& server, SystemHandles* handles) {
    if (server.hasArg("ssid") && server.hasArg("pass") && server.hasArg("token")) {
        // Safe Zero-Global update using mutex
        xSemaphoreTake(handles->mutexConfig, portMAX_DELAY);
        
        // Lưu lại cấu hình cũ phòng trường hợp mất mạng
        handles->sysData.fallback_ssid = handles->sysData.wifi_ssid;
        handles->sysData.fallback_pass = handles->sysData.wifi_pass;

        handles->sysData.wifi_ssid = server.arg("ssid");
        handles->sysData.wifi_pass = server.arg("pass");
        handles->sysData.coreiot_server = server.arg("server");
        handles->sysData.coreiot_port = server.arg("port");
        handles->sysData.coreiot_token = server.arg("token");
        xSemaphoreGive(handles->mutexConfig);
        
        server.send(200, "text/plain", "Cấu hình thành công! ESP32 đang khởi động lại kết nối...");
        
        // Disconnect forces STA task (via WiFiEvent) to retry with new credentials
        WiFi.disconnect();
    } else {
        server.send(400, "text/plain", "Thiếu tham số bắt buộc");
    }
}


void main_server_task(void *pvParameters) {
    SystemHandles* handles = (SystemHandles*)pvParameters;
    
    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }

    // Initialize Pins
    pinMode(POWER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    pinMode(0, INPUT_PULLUP); // BOOT Button

    // Init NeoPixel local
    Adafruit_NeoPixel rgb_4_led(4, 8, NEO_GBR + NEO_KHZ800);
    rgb_4_led.begin();
    rgb_4_led.setBrightness(30);
    rgb_4_led.clear();
    rgb_4_led.show();


    // Local WebServer instance
    WebServer server(80);

    // Setup Routes using Lambdas to capture server and handles
    server.on("/", [&server]() { handleFile(server, "/index.html", "text/html"); });
    server.on("/style.css", [&server]() { handleFile(server, "/style.css", "text/css"); });
    server.on("/chart.js", [&server]() {handleFile(server, "/chart.js", "application/javascript"); }); 
    server.on("/script.js", [&server]() { handleFile(server, "/script.js", "application/javascript"); });

    //
    server.on("/connect", HTTP_GET, [&server, handles]() { handleConnect(server, handles); });

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
    server.on("/led1", HTTP_GET, [&server, handles, &rgb_4_led]() { handleLed_1(server, handles, rgb_4_led); });
    server.on("/led2", HTTP_GET, [&server, handles, &rgb_4_led]() { handleLed_2(server, handles, rgb_4_led); });
    server.on("/status", HTTP_GET, [&server, handles]() { handleStatus(server, handles); });
    server.on("/off", HTTP_GET, [&server, handles, &rgb_4_led]() { handleOff(server, handles, rgb_4_led); });
    server.on("/tinyML", HTTP_GET, [&server, handles]() { handleTinyML(server, handles); });
    server.begin();

    // Variables for hardware-state tracking
    bool last_led_1 = false;
    bool last_led_2 = false;

    while (1) {
        server.handleClient();

        // Zero-Global Hardware Polling
        xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
        bool current_led_1 = handles->deviceState.led_1;
        bool current_led_2 = handles->deviceState.led_2;
        xSemaphoreGive(handles->mutexDeviceState);

        // Apply physical changes if state has diverged from tracking
        if (current_led_1 != last_led_1) {
            rgb_4_led.setPixelColor(LED_1_PIN, current_led_1 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
            rgb_4_led.show();
            last_led_1 = current_led_1;
        }

        if (current_led_2 != last_led_2) {
            rgb_4_led.setPixelColor(LED_2_PIN, current_led_2 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
            rgb_4_led.show();
            last_led_2 = current_led_2;
        }
        
        // BOOT Button to force AP mode (if switched to STA)
        if (digitalRead(0) == LOW) {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (digitalRead(0) == LOW) {
                //startAP();
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // prevent Watchdog timeout
    }
}