#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1); // Стик сидит на .1
DNSServer dnsServer;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== STICK START (PORTAL) ===");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("༺✧₭Øⳑ⋮ⱱⱥ₦✧༻", ""); // Если нужен пароль, впиши в кавычки
  
  // Перехватываем все DNS-запросы (для Captive Portal)
  dnsServer.start(DNS_PORT, "*", apIP);

  // Любой запрос перекидываем на ДевКит (.2)
  server.onNotFound([]() {
    Serial.println("[PORTAL] Перехват: " + server.uri() + " -> 192.168.4.2");
    server.sendHeader("Location", "http://192.168.4.2/", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("[PORTAL] READY (REDIRECT TO .2)");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
