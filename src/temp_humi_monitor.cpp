#include "temp_humi_monitor.h"
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

void temp_humi_monitor(void *pvParameters){
  Wire.begin(GPIO_NUM_11, GPIO_NUM_12);
  Serial.begin(115200);
  
  xSemaphoreTake(xI2CMutex, portMAX_DELAY);
  dht20.begin();
  xSemaphoreGive(xI2CMutex);
  
  while(1){
    xSemaphoreTake(xI2CMutex, portMAX_DELAY);
    dht20.read();
    float temperature = dht20.getTemperature();
    float humidity = dht20.getHumidity();
    xSemaphoreGive(xI2CMutex);

    set_sensor_data(temperature, humidity);
    Serial.print("Temp: "); Serial.print(temperature); Serial.print(" *C\n");
    Serial.print("Humi: "); Serial.print(humidity); Serial.print(" %\n");
    //Serial.println();

    vTaskDelay(5000);  
  }
}


void temp_humi_update_buffer_lcd(float temp, float humi){
    snprintf(lcd_buffer[0], sizeof(lcd_buffer[0]), "Temp: %.2fC", temp);
    snprintf(lcd_buffer[1], sizeof(lcd_buffer[1]), "Humi: %.2f%%", humi);

    // Condition for more state
}

void temp_humi_lcd_display(void *pvParameters){
    // Delay slightly to ensure Wire.begin() in temp_humi_monitor runs first
    vTaskDelay(100);

    xSemaphoreTake(xI2CMutex, portMAX_DELAY);
    lcd.begin();
    lcd.backlight();
    xSemaphoreGive(xI2CMutex);

    while(1){
      float temp, humi;
      get_sensor_data(&temp, &humi);
      temp_humi_update_buffer_lcd(temp, humi);
      xSemaphoreTake(xI2CMutex, portMAX_DELAY);
      lcd.setCursor(0, 0);
      lcd.print(lcd_buffer[0]);
      lcd.setCursor(0, 1);
      lcd.print(lcd_buffer[1]);
      xSemaphoreGive(xI2CMutex);
      vTaskDelay(1000);
    }
}