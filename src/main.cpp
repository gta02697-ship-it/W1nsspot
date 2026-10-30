#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n=====================================");
  Serial.println("=== STICK START (Wi-Fi & Portal)  ===");
  Serial.println("=====================================");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("༺✧₭Øⳑ⋮ⱱⱥ₦✧༻", "");
  
  Serial.println("[PORTAL] Точка доступа создана: ༺✧₭Øⳑ⋮ⱱⱥ₦✧༻");
  Serial.println("[PORTAL] Мой IP (Шлюз): " + apIP.toString());
  
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("[PORTAL] DNS-сервер запущен (перехват всех запросов)");

  server.onNotFound([]() {
    Serial.println("[PORTAL] Перехват: " + server.uri() + " -> Отправляю на http://192.168.4.2/");
    server.sendHeader("Location", "http://192.168.4.2/", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("[PORTAL] HTTP-сервер портала запущен!");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
