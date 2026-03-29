#include "mainserver.h"

String mainPage(bool led1, bool led2, float temp, float humi)
{
  String l1 = led1 ? "ON" : "OFF";
  String l2 = led2 ? "ON" : "OFF";
  String l1_class = led1 ? "btn on" : "btn off";
  String l2_class = led2 ? "btn on" : "btn off";

  return String(R"rawliteral(
  <!DOCTYPE html>
  <html lang="vi">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
    <style>
      :root {
        --bg: #0f172a;
        --card-bg: rgba(30, 41, 59, 0.7);
        --text: #f8fafc;
        --accent: #38bdf8;
      }
      body {
        font-family: 'Inter', 'Segoe UI', sans-serif;
        background: var(--bg);
        color: var(--text);
        text-align: center;
        margin: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        min-height: 100vh;
      }
      .container {
        background: var(--card-bg);
        padding: 40px;
        border-radius: 24px;
        box-shadow: 0 8px 32px rgba(0,0,0,0.5);
        backdrop-filter: blur(12px);
        width: 90%;
        max-width: 450px;
        border: 1px solid rgba(255,255,255,0.1);
      }
      h1 { 
        font-size: 2em; 
        margin-bottom: 30px; 
        background: linear-gradient(135deg, #38bdf8, #818cf8); 
        -webkit-background-clip: text; 
        -webkit-text-fill-color: transparent; 
      }
      .sensor-grid { 
        display: grid; 
        grid-template-columns: 1fr 1fr; 
        gap: 20px; 
        margin-bottom: 30px; 
      }
      .sensor-card { 
        background: rgba(0,0,0,0.2); 
        padding: 20px; 
        border-radius: 16px; 
        border: 1px solid rgba(255,255,255,0.05); 
        transition: transform 0.3s ease; 
      }
      .sensor-card:hover { transform: translateY(-5px); }
      .sensor-val { font-size: 1.8em; font-weight: bold; color: var(--accent); margin-top: 10px; }
      .controls { display: flex; flex-direction: column; gap: 15px; }
      .btn { 
        display: inline-block; padding: 15px 30px; font-size: 1.2em; font-weight: 600; text-decoration: none; border-radius: 12px; transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        text-transform: uppercase; letter-spacing: 1px; color: white; border: none; cursor: pointer;
      }
      .btn.on { background: linear-gradient(135deg, #16a34a, #22c55e); box-shadow: 0 4px 15px rgba(34, 197, 94, 0.4); }
      .btn.off { background: linear-gradient(135deg, #dc2626, #ef4444); box-shadow: 0 4px 15px rgba(239, 68, 68, 0.4); }
      .btn:hover { transform: scale(1.05) translateY(-2px); box-shadow: 0 8px 25px rgba(0,0,0,0.4); filter: brightness(1.1); }
      .btn:active { transform: scale(0.98); }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>ESP32 Control Panel</h1>
      <div class="sensor-grid">
        <div class="sensor-card">
          <div>🌡️ Nhiệt độ</div>
          <div class="sensor-val">)rawliteral" + String(temp, 1) + R"rawliteral(&deg;C</div>
        </div>
        <div class="sensor-card">
          <div>💧 Độ ẩm</div>
          <div class="sensor-val">)rawliteral" + String(humi, 1) + R"rawliteral(%</div>
        </div>
      </div>
      <div class="controls">
        <a href="/led1/toggle" class=")rawliteral" + l1_class + R"rawliteral(">💡 LED 1: )rawliteral" + l1 + R"rawliteral(</a>
        <a href="/led2/toggle" class=")rawliteral" + l2_class + R"rawliteral(">💡 LED 2: )rawliteral" + l2 + R"rawliteral(</a>
      </div>
    </div>
  </body>
  </html>
  )rawliteral");
}

String settingsPage()
{
  return String(R"rawliteral(
  <!DOCTYPE html>
  <html lang="vi">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Cài đặt Wi-Fi</title>
    <style>
      :root {
        --bg: #0f172a;
        --card-bg: rgba(30, 41, 59, 0.7);
        --text: #f8fafc;
      }
      body {
        font-family: 'Inter', sans-serif;
        background: var(--bg);
        color: var(--text);
        text-align: center;
        margin: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        min-height: 100vh;
      }
      .container {
        background: var(--card-bg);
        padding: 40px;
        border-radius: 24px;
        box-shadow: 0 8px 32px rgba(0,0,0,0.5);
        backdrop-filter: blur(12px);
        width: 90%;
        max-width: 400px;
        border: 1px solid rgba(255,255,255,0.1);
      }
      h1 { font-size: 1.8em; margin-bottom: 25px; background: linear-gradient(135deg, #38bdf8, #818cf8); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
      input[type=text], input[type=password] {
        width: 100%;
        padding: 15px;
        border: 1px solid rgba(255,255,255,0.1);
        border-radius: 12px;
        background: rgba(0,0,0,0.2);
        color: white;
        font-size: 1em;
        box-sizing: border-box;
        margin-bottom: 20px;
        outline: none;
        transition: border 0.3s;
      }
      input:focus { border-color: #38bdf8; }
      button, .btn-back {
        display: inline-block;
        background: linear-gradient(135deg, #38bdf8, #818cf8);
        color: white;
        font-weight: bold;
        text-decoration: none;
        border: none;
        border-radius: 12px;
        padding: 12px 25px;
        cursor: pointer;
        transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        font-size: 1em;
        margin: 5px;
      }
      button:hover, .btn-back:hover {
        transform: scale(1.05) translateY(-2px);
        box-shadow: 0 8px 25px rgba(56, 189, 248, 0.4);
      }
      .btn-back { background: rgba(255,255,255,0.1); }
      .btn-back:hover { box-shadow: 0 8px 25px rgba(255,255,255,0.1); }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>⚙️ Wi-Fi Settings</h1>
      <form action="/connect" method="get">
        <input name="ssid" type="text" placeholder="Tên Wi-Fi (SSID)" required><br>
        <input name="pass" type="password" placeholder="Mật khẩu (bỏ trống nếu không có)"><br>
        <button type="submit">Kết nối</button>
        <a href="/" class="btn-back">Quay lại</a>
      </form>
    </div>
  </body>
  </html>
  )rawliteral");
}

void main_server_task(void *pvParameters)
{
  SystemHandles* handles = (SystemHandles*)pvParameters;

  // Init LEDs
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BOOT_PIN, INPUT_PULLUP);

  // init rgb leds
  Adafruit_NeoPixel rgb_4_led(4, 8, NEO_GBR + NEO_KHZ800);
  rgb_4_led.begin();
  rgb_4_led.setBrightness(30);

  // Sync initial hardware to device states
  xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
  rgb_4_led.setPixelColor(0, handles->deviceState.led1 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
  rgb_4_led.setPixelColor(1, handles->deviceState.led2 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
  rgb_4_led.show();
  xSemaphoreGive(handles->mutexDeviceState);

  // Note: 'ssid' & 'password' are provided by global.cpp
  String local_wifi_ssid = wifi_ssid;
  String local_wifi_password = wifi_password;
  bool isAPMode = true;
  bool connecting = false;
  unsigned long connect_start_ms = 0;

  // Local WebServer instance to comply with zero-globals
  WebServer server(80);

  // ================= Routes =================
  server.on("/", HTTP_GET, [&server, handles]() {
    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
    bool l1 = handles->deviceState.led1;
    bool l2 = handles->deviceState.led2;
    xSemaphoreGive(handles->mutexDeviceState);

    // Default reading
    float temp = 25.0; float hum = 60.0;
    SensorData d;
    if (handles->qLcd != NULL && xQueuePeek(handles->qLcd, &d, 0) == pdTRUE) {
      temp = d.temperature;
      hum = d.humidity;
    }

    server.send(200, "text/html", mainPage(l1, l2, temp, hum));
  });

  server.on("/led1/toggle", HTTP_GET, [&server, handles, &rgb_4_led]() {
    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
    handles->deviceState.led1 = !handles->deviceState.led1;
    rgb_4_led.setPixelColor(0, handles->deviceState.led1 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
    rgb_4_led.show();
    xSemaphoreGive(handles->mutexDeviceState);
    
    server.sendHeader("Location", "/", true);
    server.send(303, "text/plain", "Redirecting...");
  });

  server.on("/led2/toggle", HTTP_GET, [&server, handles, &rgb_4_led]() {
    xSemaphoreTake(handles->mutexDeviceState, portMAX_DELAY);
    handles->deviceState.led2 = !handles->deviceState.led2;
    rgb_4_led.setPixelColor(1, handles->deviceState.led2 ? rgb_4_led.Color(255, 255, 255) : rgb_4_led.Color(0, 0, 0));
    rgb_4_led.show();
    xSemaphoreGive(handles->mutexDeviceState);
    
    server.sendHeader("Location", "/", true);
    server.send(303, "text/plain", "Redirecting...");
  });

  server.on("/settings", HTTP_GET, [&server]() {
    server.send(200, "text/html", settingsPage());
  });

  server.on("/connect", HTTP_GET, [&server, &local_wifi_ssid, &local_wifi_password, &isAPMode, &connecting, &connect_start_ms]() {
    local_wifi_ssid = server.arg("ssid");
    local_wifi_password = server.arg("pass");
    
    server.send(200, "text/html", "<html><body><h2>Connecting to " + local_wifi_ssid + "...</h2><a href='/'>Quay lại</a></body></html>");
    
    isAPMode = false;
    connecting = true;
    connect_start_ms = millis();
    
    WiFi.mode(WIFI_STA);
    if(local_wifi_password.isEmpty()) {
      WiFi.begin(local_wifi_ssid.c_str());
    } else {
      WiFi.begin(local_wifi_ssid.c_str(), local_wifi_password.c_str());
    }
  });

  // ================= First AP Start =================
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("Web Server Task Started. AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.begin();

  // ================= Loop =================
  while (1)
  {
    server.handleClient();

    // BOOT Button to force AP Mode
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW && !isAPMode)
      {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid.c_str(), password.c_str());
        isAPMode = true;
        connecting = false;
        Serial.println("Switched to AP Mode via Switch");
      }
    }

    // Handle STA Connecting State
    if (connecting)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; 
        if(xBinarySemaphoreInternet != NULL) {
            xSemaphoreGive(xBinarySemaphoreInternet);
        }
        isAPMode = false;
        connecting = false;
      }
      else if (millis() - connect_start_ms > 10000)
      { 
        // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid.c_str(), password.c_str());
        connecting = false;
        isWifiConnected = false;
        isAPMode = true;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20)); // prevent Watchout starvation
  }
}