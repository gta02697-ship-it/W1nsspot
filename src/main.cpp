#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "//_CHAT_02_\\";
const int WIFI_CHANNEL = 1;

WebServer server(80);
DNSServer dnsServer;
IPAddress devkitIP(192, 168, 4, 2);

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n--- M5STICK PORTAL ROUTER ---");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);

  // DNS перенаправляет все запросы на DevKit
  dnsServer.start(53, "*", devkitIP);

  // --- ХИТРЫЕ ОБРАБОТЧИКИ ДЛЯ ANDROID / IOS ---
  // Android (проверка на captive portal)
  server.on("/generate_204", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  server.on("/gen_204", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  // iOS / macOS
  server.on("/hotspot-detect.html", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  server.on("/library/test/success.html", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  // Windows
  server.on("/ncsi.txt", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  server.on("/connecttest.txt", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  // Firefox / Chrome
  server.on("/canonical.html", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });
  server.on("/success.txt", HTTP_GET, []() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });

  // Все остальные запросы — на DevKit
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.2");
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
