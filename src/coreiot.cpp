#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char* coreIOT_Server = "10.235.76.226";  
const char* coreIOT_Token = "ohvefr8ygpajb7f9fr9n";   // Device Access Token
const int   mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    //if (client.connect("ESP32Client", coreIOT_Token, NULL)) {
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
        
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
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
      Serial.println("Device turned ON.");
      //TODO

    } else {   
      Serial.println("Device turned OFF.");
      //TODO

    }
  } else {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}


void setup_coreiot(){

  //Serial.print("Connecting to WiFi...");
  //WiFi.begin(wifi_ssid, wifi_password);
  //while (WiFi.status() != WL_CONNECTED) {
  
  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while(1){
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY)) {
      break;
    }
    delay(500);
    Serial.print(".");
  }


  Serial.println(" Connected!");

  client.setServer(CORE_IOT_SERVER.c_str(), CORE_IOT_PORT.toInt());
  client.setCallback(callback);

}

void coreiot_task(void *pvParameters){
    SystemHandles* sysHandles = (SystemHandles*)pvParameters;
    setup_coreiot();

    String currentServer = CORE_IOT_SERVER;
    int currentPort = CORE_IOT_PORT.toInt();
    String currentToken = CORE_IOT_TOKEN;

    while(1){
        // Read dynamic config safely
        xSemaphoreTake(sysHandles->mutexConfig, portMAX_DELAY);
        String safeServer = CORE_IOT_SERVER;
        int safePort = CORE_IOT_PORT.toInt();
        String safeToken = CORE_IOT_TOKEN;
        xSemaphoreGive(sysHandles->mutexConfig);
        
        // Update MQTT Target if it changed
        if (safeServer != currentServer || safePort != currentPort) {
             client.disconnect();
             client.setServer(safeServer.c_str(), safePort);
             currentServer = safeServer;
             currentPort = safePort;
        }
        currentToken = safeToken;

        if (!client.connected()) {
            Serial.print("Attempting MQTT connection to ");
            Serial.print(currentServer);
            Serial.println("...");
            String clientId = "ESP32Client-";
            clientId += String(random(0xffff), HEX);
            
            // Connect with Token
            if (client.connect(clientId.c_str(), currentToken.c_str(), NULL)) {
                Serial.println("connected to CoreIOT Server!");
                client.subscribe("v1/devices/me/rpc/request/+");
            } else {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again later");
            }
        }
        client.loop();

        // Sample payload, publish to 'v1/devices/me/telemetry'
        float temp = 25.0; 
        float humi = 60.0;
        // get_sensor_data(&temp, &humi);
        String payload = "{\"temperature\":" + String(temp) +  ",\"humidity\":" + String(humi) + "}";
        
        if (client.connected()) {
            client.publish("v1/devices/me/telemetry", payload.c_str());
            Serial.println("Published payload: " + payload);
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Publish every 10 seconds (RTOS delay)
    }
}