#include <WiFi.h>
// Стик просто раздает Wi-Fi. Никаких DNS и редиректов.
void setup() {
  Serial.begin(115200);
  Serial.println("=== STICK START ===");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("༺✧₭Øⳑ⋮ⱱⱥ₦✧༻", "");
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
}
void loop() {}
