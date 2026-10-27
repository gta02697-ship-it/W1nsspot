#include <WiFi.h>
#include <DNSServer.h>

const char* ssid = "//_CHAT_02_\\";
const int WIFI_CHANNEL = 1;
DNSServer dnsServer;

void setup() {
  Serial.begin(115200);
  delay(200);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);
  
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 2));
}

void loop() {
  dnsServer.processNextRequest();
}
