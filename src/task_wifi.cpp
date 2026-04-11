#include "task_wifi.h"

static SystemHandles* pGlobalHandles = NULL;

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[WiFi] Đang bắt đầu chế độ Station...");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[WiFi] Đã kết nối với Router thành công.");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] Đã nhận IP: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("[WiFi] Mất kết nối. Đang ngầm dò sóng để tái kết nối (Zero-Delay)...");
            if (pGlobalHandles) {
                xSemaphoreTake(pGlobalHandles->mutexConfig, portMAX_DELAY);
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
