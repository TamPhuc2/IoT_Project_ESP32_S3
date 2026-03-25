#include "neo_blinky.h"


    
void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.setBrightness(30); // set brightness 
    strip.clear();
    strip.show();

    float current_r = 0, current_g = 0, current_b = 0;
    float target_r = 0, target_g = 0, target_b = 0;
    float currentHumi = 50.0; // Khởi tạo an toàn (không chớp)
    
    int tick_count = 0;

    while(1){                          
        // Cập nhật giá trị độ ẩm sau mỗi 1 giây (50 ticks của 20ms)
        if (tick_count % 50 == 0) {
            get_sensor_data(NULL, &currentHumi);
            
            // Giới hạn giá trị độ ẩm
            if(currentHumi < 0) currentHumi = 0;
            if(currentHumi > 100) currentHumi = 100;

            // Tính toán màu mục tiêu tương ứng với độ ẩm
            if (currentHumi <= 50.0) {
                // Từ 0% -> 50%: Màu xám (128,128,128) -> Xanh lá (0,255,0)
                float ratio = currentHumi / 50.0;
                target_r = 128.0 - 128.0 * ratio;
                target_g = 128.0 + 127.0 * ratio;
                target_b = 128.0 - 128.0 * ratio;
            } else {
                // Từ 50% -> 100%: Xanh lá (0,255,0) -> Đỏ (255,0,0)
                float ratio = (currentHumi - 50.0) / 50.0;
                target_r = 255.0 * ratio;
                target_g = 255.0 - 255.0 * ratio;
                target_b = 0;
            }
        }

        // Hiệu ứng mượt: màu sắc hiện tại sẽ tiến từ từ về màu mục tiêu
        current_r += (target_r - current_r) * 0.05;
        current_g += (target_g - current_g) * 0.05;
        current_b += (target_b - current_b) * 0.05;

        // Xử lý logic nhấp nháy
        bool led_on = true; // Mặc định hiển thị bình thường khi [40, 60]
        if (currentHumi < 40.0 || currentHumi > 60.0) {
            // Nhấp nháy mỗi 200ms (10 tick * 20ms)
            led_on = ((tick_count / 10) % 2 == 0);
        }

        if (led_on) {
            strip.setPixelColor(0, strip.Color((uint8_t)current_r, (uint8_t)current_g, (uint8_t)current_b));
        } else {
            strip.setPixelColor(0, strip.Color(0, 0, 0)); // Tắt LED
        }
        strip.show();

        tick_count++;
        vTaskDelay(pdMS_TO_TICKS(20)); // Delay 20ms tạo hiệu ứng 50 FPS mượt mà
    }
}