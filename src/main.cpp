#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const int WIFI_CHANNEL = 1;

WebServer server(80);
DNSServer dnsServer;

void handleStats() {
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t usedHeap = totalHeap - freeHeap;
  
  // ЧЕСТНАЯ ТЕМПЕРАТУРА ПРОЦЕССОРА (без ограничений)
  float temp = temperatureRead();
  
  // Реальная частота процессора (240 МГц или 160 МГц)
  int cpuFreq = getCpuFrequencyMhz();
  
  String json = "{";
  json += "\"ram_used\":" + String(usedHeap / 1024) + ",";
  json += "\"ram_total\":" + String(totalHeap / 1024) + ",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"freq\":" + String(cpuFreq);
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
