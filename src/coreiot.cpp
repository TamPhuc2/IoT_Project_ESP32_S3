#include "coreiot.h"

WiFiClient espClient;
PubSubClient client(espClient);

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

void coreiot_task(void *pvParameters){
    SystemHandles* sysHandles = (SystemHandles*)pvParameters;
    pGlobalHandles_coreIOT = sysHandles;
    
    // Set callback initially
    client.setCallback(callback);

    String currentServer = "";
    int currentPort = 0;
    String currentToken = "";

    while(1){
        // Only attempt communication if WiFi is actually connected
        if (WiFi.status() == WL_CONNECTED) {
            
            // Read dynamic config safely - Zero Global Protocol
            xSemaphoreTake(sysHandles->mutexConfig, portMAX_DELAY);
            String safeServer = sysHandles->sysData.coreiot_server;
            int safePort = sysHandles->sysData.coreiot_port.toInt();
            String safeToken = sysHandles->sysData.coreiot_token;
            xSemaphoreGive(sysHandles->mutexConfig);
            
            // Re-setup server target if the config changed (from the UI)
            if (safeServer != currentServer || safePort != currentPort) {
                 client.disconnect();
                 currentServer = safeServer;
                 currentPort = safePort;
                 client.setServer(currentServer.c_str(), currentPort);
            }
            currentToken = safeToken;

            // Maintain MQTT connection
            if (!client.connected() && !safeServer.isEmpty() && !safeToken.isEmpty()) {
                Serial.print("[CoreIOT] Attempting MQTT connection to ");
                Serial.print(currentServer);
                Serial.println("...");
                String clientId = "ESP32Client-";
                clientId += String(random(0xffff), HEX);
                
                // Connect with Token as Username for CoreIOT
                if (client.connect(clientId.c_str(), currentToken.c_str(), NULL)) {
                    Serial.println("[CoreIOT] connected to CoreIOT Server!");
                    client.subscribe("v1/devices/me/rpc/request/+");
                } else {
                    Serial.print("[CoreIOT] failed, rc=");
                    Serial.print(client.state());
                    Serial.println(" try again later");
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

                String payload = "{\"temperature\":" + String(temp, 1) +  ",\"humidity\":" + String(humi, 1) + "}";
                
                client.publish("v1/devices/me/telemetry", payload.c_str());
                Serial.print("[CoreIOT] Published payload: ");
                Serial.println(payload);
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