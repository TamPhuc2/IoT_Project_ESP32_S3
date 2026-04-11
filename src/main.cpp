#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"
#include "task_wifi.h"

// Static instance holding our system handles (to be passed as a void* to tasks)
static SystemHandles sysHandles;

void setup()
{
  Serial.begin(115200);

  // Initialize Queues with capacity for 1 SensorData structure
  sysHandles.qLed = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qNeo = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qLcd = xQueueCreate(1, sizeof(SensorData));

  // Initialize Binary Semaphore (for LCD) and Mutex (for I2C and Device States)
  sysHandles.semLcd = xSemaphoreCreateBinary();
  sysHandles.mutexI2C = xSemaphoreCreateMutex();
  sysHandles.mutexDeviceState = xSemaphoreCreateMutex();
  sysHandles.mutexConfig = xSemaphoreCreateMutex();
  
  // Initialize device default states
  sysHandles.deviceState.led_1 = false;
  sysHandles.deviceState.led_2 = false;

  // Initialize Zero-Global credentials Default
  sysHandles.sysData.wifi_ssid = "tp";
  sysHandles.sysData.wifi_pass = "tamphuc12345"; // Requirement
  sysHandles.sysData.coreiot_server = "app.coreiot.io";
  sysHandles.sysData.coreiot_port = "1883"; // Note: HTTP will just use the server root, port 1883 might not be needed for HTTP
  sysHandles.sysData.coreiot_token = "ohvefr8ygpajb7f9fr9n";
  sysHandles.sysData.ap_ssid = "TP NETWOK";
  sysHandles.sysData.ap_pass = "12345678";

  // Init Event-Driven WiFi (Zero-blocking)
  init_wifi(&sysHandles);

  // Tasks creation - passing the sysHandles pointer to pvParameters
  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_lcd_display, "Test LCD", 2048, (void*)&sysHandles, 2, NULL);
  
  xTaskCreate(main_server_task, "Task Main Server" ,8192, (void*)&sysHandles, 2, NULL);
  
  // Start CoreIOT background REST API task
  // wifi_task removed as it is now event driven on CPU0
  xTaskCreate(coreiot_task, "CoreIOT HTTP Task", 8192, (void*)&sysHandles, 2, NULL);
  
  //xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop(){
  // FreeRTOS setup, empty loop
  vTaskDelete(NULL);
}