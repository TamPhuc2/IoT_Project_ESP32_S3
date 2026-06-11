#include "global.h"

#define NODE_MODE_A // Mạch A: Get data from sensors, Send data(Auto Fallback)
//#define NODE_MODE_B // Mạch B: Actuator, GET data from Cloud, Subscribe MQTT

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"
#include "task_wifi.h"

void node_b_actuator_task(void *pvParameters);

// Static instance holding our system handles (to be passed as a void* to tasks)
static SystemHandles sysHandles;

void setup()
{
  Serial.begin(115200);

  // Initialize Queues with capacity for 1 SensorData structure
  sysHandles.qLed = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qNeo = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qLcd = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qTinyML  = xQueueCreate(1, sizeof(TinyMLData));
  sysHandles.qTrigger = xQueueCreate(1, sizeof(int));

  // Initialize Binary Semaphore (for LCD) and Mutex (for I2C and Device States)
  sysHandles.semLcd = xSemaphoreCreateBinary();
  sysHandles.mutexI2C = xSemaphoreCreateMutex();
  sysHandles.mutexDeviceState = xSemaphoreCreateMutex();
  sysHandles.mutexConfig = xSemaphoreCreateMutex();

  // Initialize device default states
  sysHandles.deviceState.led_1 = false;
  sysHandles.deviceState.led_2 = false;
  sysHandles.deviceState.tinyml_mode = false;

    // Initialize Zero-Global credentials Default
  sysHandles.sysData.wifi_ssid = "tp";
  sysHandles.sysData.wifi_pass = "0123456789"; // Requirement
  sysHandles.sysData.fallback_ssid = "";
  sysHandles.sysData.fallback_pass = "";
  sysHandles.sysData.coreiot_server = "app.coreiot.io";
  sysHandles.sysData.coreiot_port = "1883"; 
  // sysHandles.sysData.coreiot_token = "ohvefr8ygpajb7f9fr9n";
  sysHandles.sysData.coreiot_token = "CooZmkhZjSLxzxm6spkH";
  sysHandles.sysData.ap_ssid = "MY ESP32_S3 NETWORK";
  sysHandles.sysData.ap_pass = "12345678";

  // Init Event-Driven WiFi (Zero-blocking)
  init_wifi(&sysHandles);
// ========================================================
// NODE A: Sensor & CoreIOT Client Task
#ifdef NODE_MODE_A
  Serial.println(">>> NODE A <<<");
  
  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_lcd_display, "Test LCD", 4096, (void*)&sysHandles, 2, NULL);
  xTaskCreate(tiny_ml_task, "Tiny ML Task", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(coreiot_task, "CoreIOT/FallBack Task", 5120, (void*)&sysHandles, 2, NULL);

// ========================================================
// NODE B: Actuator & MQTT Client Task
#elif defined(NODE_MODE_B)
  Serial.println(">>> NODE B  <<<");
  xTaskCreate(node_b_actuator_task, "Node B Task", 5120, (void*)&sysHandles, 2, NULL);
#endif

  // Web server always runs (AP/STA)
  xTaskCreate(main_server_task, "Task Main Server", 8192, (void*)&sysHandles, 2, NULL);
}

void loop(){
  vTaskDelete(NULL);
}

// ========================================================
// NODE B  MQTT CLIENT CONFIGURATION
// ========================================================
#ifdef NODE_MODE_B
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel* nodeBStrip = NULL;
static bool isWarning = false; // Biến trạng thái cảnh báo

static void node_b_mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Parsing payload
  String message = "";
  for (int i = 0; i < length; i++) message += (char)payload[i];
  
  StaticJsonDocument<200> doc;
  if (!deserializeJson(doc, message)) {
      const char* cmd = doc["cmd"];
      if (cmd) {
         if (nodeBStrip == NULL) return;

         if (strcmp(cmd, "ON") == 0) {
             isWarning = true; 
             Serial.println("[NodeB] Received ON -> Start Blinking Mode");
         } else if (strcmp(cmd, "OFF") == 0) {
             isWarning = false;
             Serial.println("[NodeB] Received OFF -> Stop Blinking/Turn Off");
         }
      }
  }
}

void node_b_actuator_task(void *pvParameters) {
  SystemHandles* sysHandles = (SystemHandles*)pvParameters;
  
  if (nodeBStrip == NULL) {
      nodeBStrip = new Adafruit_NeoPixel(1, ACTUATOR_1_PIN, NEO_GRB + NEO_KHZ800);
      nodeBStrip->begin();
      nodeBStrip->setBrightness(50);
      nodeBStrip->clear();
      nodeBStrip->show();
  }

  pinMode(ACTUATOR_2_PIN, OUTPUT);
  digitalWrite(ACTUATOR_2_PIN, LOW);

  WiFiClient espClient;
  PubSubClient client(espClient);
  client.setCallback(node_b_mqtt_callback);

  uint32_t lastBlinkTime = 0;
  bool blinkState = false;

  while (1) {
    if (WiFi.status() == WL_CONNECTED) {
       // Reconnect logic
       if (!client.connected()) {
          client.setServer(LOCAL_BROKER_IP, 1883);
          String clientId = "NodeBClient-";
          clientId += String(random(0xffff), HEX);
          if (client.connect(clientId.c_str())) {
              Serial.println("[NodeB] Connected to Edge Gateway!");
              client.subscribe("home/actuators/led1");
          } else {
              vTaskDelay(5000 / portTICK_PERIOD_MS);
          }
       }
       client.loop();
    }

    // --- LOGIC CHỚP ĐÈN (BLINKING LOGIC) ---
    if (isWarning) {
      if (millis() - lastBlinkTime >= 500) { // Chớp mỗi 500ms
          lastBlinkTime = millis();
          blinkState = !blinkState;

          if (blinkState) {
              digitalWrite(ACTUATOR_2_PIN, HIGH);
              nodeBStrip->setPixelColor(0, nodeBStrip->Color(255, 0, 0)); // Đỏ
          } else {
              digitalWrite(ACTUATOR_2_PIN, LOW);
              nodeBStrip->setPixelColor(0, nodeBStrip->Color(0, 0, 0)); // Tắt
          }
          nodeBStrip->show();
      }
    } else {
      // Khi không cảnh báo, đảm bảo đèn luôn tắt
      digitalWrite(ACTUATOR_2_PIN, LOW);
      if (nodeBStrip) {
          nodeBStrip->setPixelColor(0, nodeBStrip->Color(0,0,0));
          nodeBStrip->show();
      }
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); // Tăng tần suất kiểm tra để nháy mượt hơn
  }
}
#endif
