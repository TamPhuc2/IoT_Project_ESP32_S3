#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"

// Static instance holding our system handles (to be passed as a void* to tasks)
static SystemHandles sysHandles;

void setup()
{
  Serial.begin(115200);

  // Initialize Queues with capacity for 1 SensorData structure
  sysHandles.qLed = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qNeo = xQueueCreate(1, sizeof(SensorData));
  sysHandles.qLcd = xQueueCreate(1, sizeof(SensorData));

  // Initialize Binary Semaphore (for LCD) and Mutex (for I2C)
  sysHandles.semLcd = xSemaphoreCreateBinary();
  sysHandles.mutexI2C = xSemaphoreCreateMutex();

  // Tasks creation - passing the sysHandles pointer to pvParameters
  xTaskCreate(led_blinky, "Task LED Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, (void*)&sysHandles, 2, NULL);
  xTaskCreate(temp_humi_lcd_display, "Test LCD", 2048, (void*)&sysHandles, 2, NULL);
  
  //xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  //xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
}

void loop(){
  // FreeRTOS setup, empty loop
  vTaskDelete(NULL);
}