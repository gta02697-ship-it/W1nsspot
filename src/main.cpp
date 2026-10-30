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
  
  // РЕАЛЬНАЯ ЧАСТОТА ПРОЦЕССОРА (240 МГц или 160 МГц)
  int cpuFreq = getCpuFrequencyMhz();
  
  String json = "{";
  json += "\"ram_used\":" + String(usedHeap / 1024) + ",";
  json += "\"ram_total\":" + String(totalHeap / 1024) + ",";
  json += "\"temp\":" + String(temp, 1) + ",";
  json += "\"freq\":" + String(cpuFreq);
  json += "}";
  
  // Разрешаем запросы с других IP (ДевКита)
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", json);
}

const char* portal_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<meta http-equiv="refresh" content="0; url=http://192.168.4.2/">
<title>W1NSSPOT</title>
<style>
body { background:#080604; color:#ffd05b; font-family:sans-serif; text-align:center; padding-top:40vh; margin:0; }
h1 { color:#ff6a00; font-size:24px; text-shadow:0 0 10px #ff6a00; }
p { color:#fff5e5; font-size:16px; }
</style></head><body>
<h1>W1NSSPOT</h1>
<p>Перенаправление на портал...</p>
</body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n--- M5STICK PORTAL (AUTO REDIRECT) ---");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Настройка DHCP: начинаем выдачу с .3, чтобы .2 остался для ДевКита
  tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  dhcps_lease_t lease;
  lease.enable = true;
  lease.start_ip.addr = (192 << 24) | (168 << 16) | (4 << 8) | 3;
  lease.end_ip.addr = (192 << 24) | (168 << 16) | (4 << 8) | 254;
  tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_DOMAIN_NAME_SERVER, &lease, sizeof(lease));
  tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Настройка DHCP: начинаем выдачу с .3, чтобы .2 остался для ДевКита
  tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
  dhcps_lease_t lease;
  lease.enable = true;
  lease.start_ip.addr = (192 << 24) | (168 << 16) | (4 << 8) | 3;
  lease.end_ip.addr = (192 << 24) | (168 << 16) | (4 << 8) | 254;
  tcpip_adapter_dhcps_option(TCPIP_ADAPTER_OP_SET, TCPIP_ADAPTER_DOMAIN_NAME_SERVER, &lease, sizeof(lease));
  tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);

  // DNS на себя (M5Stick), чтобы телефон стучался сюда
  dnsServer.start(53, "*", WiFi.softAPIP());

  // --- ЭНДПОИНТ СТАТИСТИКИ ---
  server.on("/stats", HTTP_GET, handleStats);

  // Отдаем HTML-страницу с авто-редиректом на все проверки
  server.on("/generate_204", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/gen_204", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/library/test/success.html", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/ncsi.txt", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/connecttest.txt", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/canonical.html", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/success.txt", HTTP_GET, []() { server.send(200, "text/html", portal_html); });

  // На любой другой запрос — тоже авто-редирект
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.onNotFound([]() { server.send(200, "text/html", portal_html); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
