#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const int WIFI_CHANNEL = 1;

WebServer server(80);
DNSServer dnsServer;

// Переменные для замера нагрузки
unsigned long lastIdleTime = 0;
unsigned long lastTotalTime = 0;
float cpuLoad = 0.0;

void calculateCPU() {
  // Получаем время простоя Idle Task (в тиках FreeRTOS)
  unsigned long idleTime = xTaskGetIdleTickCount(0); // Core 0
  unsigned long totalTime = xTaskGetTickCount();
  
  if (lastTotalTime > 0) {
    unsigned long deltaIdle = idleTime - lastIdleTime;
    unsigned long deltaTotal = totalTime - lastTotalTime;
    
    if (deltaTotal > 0) {
      float idlePercent = (float)deltaIdle / (float)deltaTotal * 100.0;
      cpuLoad = 100.0 - idlePercent; // Нагрузка = 100% - простой
      if (cpuLoad < 0) cpuLoad = 0;
      if (cpuLoad > 100) cpuLoad = 100;
    }
  }
  lastIdleTime = idleTime;
  lastTotalTime = totalTime;
}

void handleStats() {
  calculateCPU();
  
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t usedHeap = totalHeap - freeHeap;
  
  // ЧЕСТНАЯ ТЕМПЕРАТУРА ПРОЦЕССОРА (без ограничений)
  float temp = temperatureRead();
  
  String json = "{";
  json += "\"ram_used\":" + String(usedHeap / 1024) + ",";
  json += "\"ram_total\":" + String(totalHeap / 1024) + ",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"load\":" + String((int)cpuLoad);
  json += "}";
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

const char* portal_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="0; url=http://192.168.4.2/">
<title>W1NSSPOT</title>
<style>body { background:#080604; color:#ffd05b; font-family:sans-serif; text-align:center; padding-top:40vh; margin:0; } h1 { color:#ff6a00; font-size:24px; text-shadow:0 0 10px #ff6a00; } p { color:#fff5e5; font-size:16px; }</style></head><body>
<h1>W1NSSPOT</h1><p>Перенаправление на портал...</p></body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(200);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Инициализация счетчиков для нагрузки
  lastIdleTime = xTaskGetIdleTickCount(0);
  lastTotalTime = xTaskGetTickCount();

  server.on("/stats", HTTP_GET, handleStats);
  server.on("/generate_204", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.onNotFound([]() { server.send(200, "text/html", portal_html); });

  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
