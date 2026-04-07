#include "task_wifi.h"

// void startAP()
// {
//     WiFi.mode(WIFI_AP);
//     WiFi.softAP(String(SSID_AP), String(PASS_AP));
//     Serial.print("AP IP: ");
//     Serial.println(WiFi.softAPIP());
// }

void startSTA()
{
    if (WIFI_SSID.isEmpty())
    {
        vTaskDelete(NULL);
    }

    // Use AP_STA to ensure AP remains active while searching for STA
    WiFi.mode(WIFI_AP_STA);

    if (WIFI_PASS.isEmpty())
    {
        WiFi.begin(WIFI_SSID.c_str());
    }
    else
    {
        WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
    }

    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 20) // Try for ~10 seconds
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        xSemaphoreGive(xBinarySemaphoreInternet);
        Serial.println("STA Connected successfully.");
    }
    else
    {
        Serial.println("STA Connection failed. Periodic retries will continue.");
    }
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    startSTA();
    return false;
}
