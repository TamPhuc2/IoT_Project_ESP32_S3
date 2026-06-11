#include "coreiot.h"

static SystemHandles* pGlobalHandles_coreIOT = NULL;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[CoreIOT] Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("[CoreIOT] Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* method = doc["method"];
  if (strcmp(method, "setStateLED") == 0) {
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
    const char* params = doc["params"];

    if (strcmp(params, "ON") == 0) {
      Serial.println("[CoreIOT] Device turned ON via RPC.");
      if (pGlobalHandles_coreIOT) {
          xSemaphoreTake(pGlobalHandles_coreIOT->mutexDeviceState, portMAX_DELAY);
          pGlobalHandles_coreIOT->deviceState.led_1 = true;
          xSemaphoreGive(pGlobalHandles_coreIOT->mutexDeviceState);
      }
    } else {   
      Serial.println("[CoreIOT] Device turned OFF via RPC.");
      if (pGlobalHandles_coreIOT) {
          xSemaphoreTake(pGlobalHandles_coreIOT->mutexDeviceState, portMAX_DELAY);
          pGlobalHandles_coreIOT->deviceState.led_1 = false;
          xSemaphoreGive(pGlobalHandles_coreIOT->mutexDeviceState);
      }
    }
  } else {
    Serial.print("[CoreIOT] Unknown method: ");
    Serial.println(method);
  }
}

#include "global.h"

void coreiot_task(void *pvParameters){
    SystemHandles* sysHandles = (SystemHandles*)pvParameters;
    pGlobalHandles_coreIOT = sysHandles;

    WiFiClient espClient;
    PubSubClient client(espClient);
    
    // Set callback initially
    client.setCallback(callback);

    String currentServer = "";
    int currentPort = 0;
    String currentToken = "";
    
    bool usingLocalBroker = false;

    while(1){
        // Only attempt communication if WiFi is actually connected
        if (WiFi.status() == WL_CONNECTED) {
            
            // Read dynamic config safely - Zero Global Protocol
            xSemaphoreTake(sysHandles->mutexConfig, portMAX_DELAY);
            String safeServer = sysHandles->sysData.coreiot_server;
            int safePort = sysHandles->sysData.coreiot_port.toInt();
            String safeToken = sysHandles->sysData.coreiot_token;
            xSemaphoreGive(sysHandles->mutexConfig);
            
            // Maintain MQTT connection
            if (!client.connected()) {
                // prepare client ID and connection parameters
                String clientId = "NodeAClient-";
                clientId += String(random(0xffff), HEX);
                
                // Connect to local broker (Edge Gateway) first
                Serial.print("[CoreIOT/Fallback] Attemping Local Broker: ");
                Serial.println(LOCAL_BROKER_IP);
                client.setServer(LOCAL_BROKER_IP, 1883);
                
                if (client.connect(clientId.c_str())) {
                    Serial.println("[CoreIOT] Connected to Local Broker (M2M)!");
                    usingLocalBroker = true;
                } else {
                    // Failed to connect to local broker =>  try CoreIOT Cloud
                    Serial.println("[CoreIOT] Local Broker Failed. Falling back to CoreIOT Cloud!");
                    if (!safeServer.isEmpty() && !safeToken.isEmpty()) {
                        client.setServer(safeServer.c_str(), safePort);
                        // Connect with Token as Username
                        if (client.connect(clientId.c_str(), safeToken.c_str(), NULL)) {
                            Serial.println("[CoreIOT] Connected safely to CoreIOT Cloud Server!");
                            client.subscribe("v1/devices/me/rpc/request/+"); // Open RPC Cloud
                            usingLocalBroker = false;
                        } else {
                            Serial.print("[CoreIOT] Cloud Failed rc=");
                            Serial.print(client.state());
                            Serial.println(". Reconnecting...");
                        }
                    }
                }
            }
            
            // Keep MQTT session alive and process incoming RPC
            client.loop(); 

            // Payload generation and transmission
            if (client.connected()) {
                // Peek latest queue data safely
                float temp = 0.0; 
                float humi = 0.0;
                SensorData d = {0, 0, 0};
                
                if (sysHandles->qLcd != NULL) {
                    if (xQueuePeek(sysHandles->qLcd, &d, 0) == pdTRUE) {
                        temp = d.temperature;
                        humi = d.humidity;
                    }
                }

                String payload = "";
                String targetTopic = "";

                if (usingLocalBroker) {
                    // Cấu trúc Payload và Topic dành cho Edge Gateway (Python Rule Engine)
                    payload = "{\"id\":\"Node_A\",\"temperature\":" + String(temp, 1) +  ",\"humidity\":" + String(humi, 1) + "}";
                    targetTopic = "home/sensors";
                // } else {
                //     // Cấu trúc Payload và Topic đẩy nguyên bản lên CoreIOT Cloud
                //     //payload = "{\"temperature\": " + String(temp, 1) +  ",\"humidity\": " + String(humi, 1) + "}";
                //     payload = "{\"id\":\"Node_A1\",\"temperature\":" + String(temp, 1) +  ",\"humidity\":" + String(humi, 1) + "}";
                //     // targetTopic = "v1/devices/me/telemetry";
                //    // payload = "{\"id\":\"Node_A1\",\"temperature\":" + String(temp, 1) +  ",\"humidity\":" + String(humi, 1) + "}";
                //     targetTopic = "v1/gateway/telemetry";
                // }
                } else {
    
                    String device_name = "Node_A";
                    unsigned long ts = millis(); // Giả lập timestamp hoặc dùng giá trị thực
    
                    payload = "{\"" + device_name + "\": [";
                    payload += "{";
                    payload += "\"ts\":" + String(ts) + ",";
                    payload += "\"values\": {";
                    payload += "\"temperature\":" + String(temp, 1) + ",";
                    payload += "\"humidity\":" + String(humi, 1);
                    payload += "}"; // đóng values
                    payload += "}"; // đóng object trong array
                    payload += "]}"; // đóng array và object tổng
    
                    targetTopic = "v1/gateway/telemetry";
                }
                if (client.publish(targetTopic.c_str(), payload.c_str())) {
                    Serial.print("[CoreIOT/Telemetry] Published to ");
                    Serial.print(usingLocalBroker ? "LOCAL: " : "CLOUD: ");
                    Serial.println(payload);
                } else {
                    Serial.println("[CoreIOT/Telemetry] Error: Data Publish failed.");
                }
            }
        } else {
            // Mất mạng nội hạt, ngắt kết nối PubSub
            if (client.connected()) {
                client.disconnect();
            }
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Publish every 10 seconds
    }
}