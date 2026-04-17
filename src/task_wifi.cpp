#include "task_wifi.h"

static SystemHandles* pGlobalHandles = NULL;

static int wifi_retry_count = 0;
const int MAX_WIFI_RETRIES = 5;

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[WiFi] Starting station mode");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[WiFi] Connected successful");
            wifi_retry_count = 0; //set counter retry = 0
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] Get IP: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            wifi_retry_count++;
            Serial.print("[WiFi] Disconnected (Time: ");
            Serial.print(wifi_retry_count);
            Serial.println(")- Reconnectting...");
            
            if (pGlobalHandles) {
                xSemaphoreTake(pGlobalHandles->mutexConfig, portMAX_DELAY);
                
                // Kích hoạt Fallback nếu thử quá số lần (sai pass hoặc trùng tên)
                if (wifi_retry_count >= MAX_WIFI_RETRIES && !pGlobalHandles->sysData.fallback_ssid.isEmpty()) {
                    Serial.println("[WiFi-Manager] Phát hiện lỗi mạng mới! Tự động khôi phục mạng WiFi cũ...");
                    pGlobalHandles->sysData.wifi_ssid = pGlobalHandles->sysData.fallback_ssid;
                    pGlobalHandles->sysData.wifi_pass = pGlobalHandles->sysData.fallback_pass;
                    // Đặt lại bộ đếm để tiếp tục retry cho mạng cũ thay vì loop fallback
                    wifi_retry_count = 0; 
                }

                String safe_ssid = pGlobalHandles->sysData.wifi_ssid;
                String safe_pass = pGlobalHandles->sysData.wifi_pass;
                xSemaphoreGive(pGlobalHandles->mutexConfig);
                
                // Tránh lỗi treo khi begin ngay trong Interrupt, ngắt trạm cũ và thiết lập lại
                WiFi.disconnect();
                WiFi.begin(safe_ssid.c_str(), safe_pass.c_str());
            }
            break;
        default: break;
    }
}

void init_wifi(SystemHandles* handles) {
    pGlobalHandles = handles;
    
    // Cấu hình Dual Mode ngay từ đầu theo Task 6
    WiFi.mode(WIFI_AP_STA);
    
    // Khởi tạo Event Handler
    WiFi.onEvent(WiFiEvent);
    
    // Lấy thông số từ struct (Zero Global)
    xSemaphoreTake(handles->mutexConfig, portMAX_DELAY);
    String target_ssid = handles->sysData.wifi_ssid;
    String target_pass = handles->sysData.wifi_pass;
    String target_ap_ssid = handles->sysData.ap_ssid;
    String target_ap_pass = handles->sysData.ap_pass;
    xSemaphoreGive(handles->mutexConfig);
    
    // Thiết lập AP Mode tại 192.168.4.1 ngay lập tức (Luôn duy trì)
    WiFi.softAP(target_ap_ssid.c_str(), target_ap_pass.c_str());
    Serial.print("\n[WiFi] Khởi động chế độ Access Point: ");
    Serial.println(WiFi.softAPIP());

    // Thiết lập kết nối STA (Non-blocking hoàn toàn)
    Serial.print("[WiFi] Đang dò sóng tới SSID: ");
    Serial.println(target_ssid);
    WiFi.begin(target_ssid.c_str(), target_pass.c_str());
}
