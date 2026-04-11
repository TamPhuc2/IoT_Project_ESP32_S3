#include "task_wifi.h"

// void startAP()
// {
//     WiFi.mode(WIFI_AP);
//     WiFi.softAP(String(SSID_AP), String(PASS_AP));
//     Serial.print("AP IP: ");
//     Serial.println(WiFi.softAPIP());
// }

void wifi_task(void *pvParameters) {
    SystemHandles* sysHandles = (SystemHandles*)pvParameters;
    
    // Always keep AP running for Fail-safe
    WiFi.mode(WIFI_AP_STA);
    
    String current_ssid = "";
    String current_pass = "";

    while (1) {
        // Safely fetch wifi credentials using Mutex
        xSemaphoreTake(sysHandles->mutexConfig, portMAX_DELAY);
        String safe_ssid = wifi_ssid;
        String safe_pass = wifi_password;
        xSemaphoreGive(sysHandles->mutexConfig);

        // If credentials changed or wifi disconnected, try connecting
        if (WiFi.status() != WL_CONNECTED || current_ssid != safe_ssid || current_pass != safe_pass) {
            
            // If they changed, disconnect first
            if (current_ssid != safe_ssid || current_pass != safe_pass) {
                WiFi.disconnect();
                current_ssid = safe_ssid;
                current_pass = safe_pass;
            }

            if (!current_ssid.isEmpty()) {
                Serial.print("[WiFi] Đang bắt đầu kết nối với SSD: ");
                Serial.println(current_ssid);
                
                if (current_pass.isEmpty()) {
                    WiFi.begin(current_ssid.c_str());
                } else {
                    WiFi.begin(current_ssid.c_str(), current_pass.c_str());
                }

                int retryCount = 0;
                while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    Serial.print(".");
                    retryCount++;
                }
                Serial.println();

                if (WiFi.status() == WL_CONNECTED) {
                    xSemaphoreGive(xBinarySemaphoreInternet);
                    Serial.print("[WiFi] Đã kết nối thành công! IP: ");
                    Serial.println(WiFi.localIP());
                } else {
                    Serial.println("[WiFi] Kết nối thất bại. Sẽ thử lại sau 10s...");
                }
            } else {
                Serial.println("[WiFi] Chưa có SSID cấu hình. Chạy AP Mode...");
            }
        }
        
        // Loop delay
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
