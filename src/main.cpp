#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "//_CHAT_02_\\";
const int WIFI_CHANNEL = 1;

WebServer server(80);
DNSServer dnsServer;
IPAddress devkitIP(192, 168, 4, 2);

// HTML-страница для редиректа (Android понимает это лучше)
const char* redirect_html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0;url=http://192.168.4.2/'></head><body>Redirecting...</body></html>";

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n--- M5STICK PORTAL ---");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);

  // DNS перенаправляет всё на DevKit
  dnsServer.start(53, "*", devkitIP);

  // --- ОТВЕТЫ НА ПРОВЕРКИ (HTML с редиректом) ---
  server.on("/generate_204", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/gen_204", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/library/test/success.html", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/ncsi.txt", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/connecttest.txt", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/canonical.html", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });
  server.on("/success.txt", HTTP_GET, []() { server.send(200, "text/html", redirect_html); });

  // Все остальные запросы (включая google.com) — тоже редирект
  server.onNotFound([]() {
    server.send(200, "text/html", redirect_html);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
