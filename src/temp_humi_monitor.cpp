#include "temp_humi_monitor.h"

// format lcd buffer 
static void temp_humi_update_buffer_lcd(char buf[2][17], float temp, float humi, int state, bool isTinyML, String predict_state){
    String statusStr = "";
    if (isTinyML) {
        statusStr = "tinyml- " + predict_state;
    } else {
        if (state == CRITICAL_COLD) statusStr = "CRITICAL_COLD";
        else if (state == COOL) statusStr = "COOL";
        else if (state == NORMAL) statusStr = "NORMAL";
        else if (state == HOT) statusStr = "HOT";
        else if (state == CRITICAL_HOT) statusStr = "CRITICAL_HOT";
    }

    // center align
    int len = statusStr.length();
    int padding = 0;
    if (len < 16) {
        padding = (16 - len) / 2;
    }
    
    // clear previous screen
    char tempBuf[17];
    memset(tempBuf, ' ', 16);
    tempBuf[16] = '\0';
    
    // print statusStr into LCD
    memcpy(tempBuf + padding, statusStr.c_str(), len > 16 ? 16 : len);
    snprintf(buf[0], 17, "%s", tempBuf);

    // format temp and humi value
    snprintf(buf[1], 17, "T:%2.1fC . H:%2.1f%%", temp, humi);
}

void temp_humi_monitor(void *pvParameters){
  DHT20 dht20;
  SystemHandles* handles = (SystemHandles*)pvParameters;
  int lastState = -1;

  Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
  Serial.begin(115200);
  
  xSemaphoreTake(handles->mutexI2C, portMAX_DELAY);
  dht20.begin();
  xSemaphoreGive(handles->mutexI2C);
  
  while(1){
    xSemaphoreTake(handles->mutexI2C, portMAX_DELAY);
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();
    xSemaphoreGive(handles->mutexI2C);

    // checks extreme states across BOTH sensors to determine overall LCD Status
    int currentState = DEFAULT_STATE; 
    // CRITICAL COLD
    if(temperature < TEMP_CRITICAL_COLD){
        currentState = CRITICAL_COLD;
    }
    // COOL
    else if(temperature >= TEMP_CRITICAL_COLD && temperature < TEMP_COOL){
        currentState = COOL; 
    } 
    // CRITICAL HOT
    else if(temperature >= TEMP_HOT){
        currentState = CRITICAL_HOT; 
    } 
    // HOT
    else if(temperature >= TEMP_NORMAL && temperature < TEMP_HOT){
        currentState = HOT; 
    }
    //NORMAL
    else {
        currentState = NORMAL; 
    }

    SensorData freshData = {temperature, humidity, currentState};

    // share data to queues
    xQueueOverwrite(handles->qLed, &freshData);
    xQueueOverwrite(handles->qNeo, &freshData);
    xQueueOverwrite(handles->qLcd, &freshData);

    // Get current tinyml_mode
    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
    bool is_tinyml = handles->deviceState.tinyml_mode;
    xSemaphoreGive(handles->mutexDeviceState);

    if (is_tinyml) {
        // Send trigger to TinyML task
        int trigger = 1;
        xQueueSend(handles->qTrigger, &trigger, 0);
        // Note: semLcd for TinyML mode will be given by the TinyML task when its predict_state changes!
    } else {
        // Give Semaphore to wake up LCD task if there's a state change or if it's the very first loop!
        if (currentState != lastState) {
            xSemaphoreGive(handles->semLcd);
            lastState = currentState;
        }
    }

    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" *C ");
    Serial.print("Humi: "); Serial.print(humidity); Serial.print(" %");
    Serial.println();
    vTaskDelay(5000);  
  }
}

void temp_humi_lcd_display(void *pvParameters){
    LiquidCrystal_I2C lcd(33,16,2);
    SystemHandles* handles = (SystemHandles*)pvParameters;
    SensorData data;
    char local_lcd_buffer[2][17] = {"                ", "                "};

    // delay slightly to ensure Wire.begin() in temp_humi_monitor run first
    vTaskDelay(100); 

    xSemaphoreTake(handles->mutexI2C, portMAX_DELAY);
    lcd.begin();
    lcd.backlight();
    xSemaphoreGive(handles->mutexI2C);

    while(1){
      // suspend task until a semaphore is given 
      xSemaphoreTake(handles->semLcd, portMAX_DELAY);

      // peek the data from queue (non-blocking)
      if (xQueuePeek(handles->qLcd, &data, 0) == pdTRUE) {
          xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
          bool is_tinyml = handles->deviceState.tinyml_mode;
          xSemaphoreGive(handles->mutexDeviceState);

          String predict_state = "WAITING";
          if (is_tinyml) {
              TinyMLData ml_data = {0, ""};
              if (handles->qTinyML != NULL && xQueuePeek(handles->qTinyML, &ml_data, 0) == pdTRUE) {
                  predict_state = ml_data.predict_state;
              }
          }

          temp_humi_update_buffer_lcd(local_lcd_buffer, data.temperature, data.humidity, data.state, is_tinyml, predict_state);
          
          xSemaphoreTake(handles->mutexI2C, portMAX_DELAY);
          lcd.setCursor(0, 0);
          lcd.print(local_lcd_buffer[0]);
          lcd.setCursor(0, 1);
          lcd.print(local_lcd_buffer[1]);
          xSemaphoreGive(handles->mutexI2C);
      }
    }
}