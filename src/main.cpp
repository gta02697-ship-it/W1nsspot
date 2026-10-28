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
  Serial.println("\n--- M5STICK PORTAL ---");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);

  dnsServer.start(53, "*", devkitIP);

  // Логируем ВСЕ запросы
  server.onNotFound([]() {
    Serial.print("REQUEST: ");
    Serial.println(server.uri());
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
