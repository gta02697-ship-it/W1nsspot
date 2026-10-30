#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 2); // Стик теперь .2
DNSServer dnsServer;
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== STICK START ===");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("༺✧₭Øⳑ⋮ⱱⱥ₦✧༻", "");
  
  dnsServer.start(DNS_PORT, "*", apIP);

  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
  });
  
  server.begin();
  Serial.println("STICK READY (REDIRECT TO .1)");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
