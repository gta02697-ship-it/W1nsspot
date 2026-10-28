#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const char* ssid = "//_CHAT_02_\\";
const int WIFI_CHANNEL = 1;

WebServer server(80);
DNSServer dnsServer;
IPAddress devkitIP(192, 168, 4, 2);

// Красивая страница-портал
const char* portal_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>W1NSSPOT PORTAL</title>
<style>
@font-face { font-family:"W1nssMinecraft"; src:url("http://192.168.4.2/Minecraftia-Regular.ttf") format("truetype"); }
body { background:#080604; color:#ffd05b; font-family:"W1nssMinecraft",monospace; text-align:center; padding-top:20vh; margin:0; background-image:linear-gradient(#ff79000b 1px,transparent 1px),linear-gradient(90deg,#ff79000b 1px,transparent 1px); background-size:22px 22px; }
h1 { color:#ff6a00; font-size:32px; text-shadow:0 0 10px #ff6a00; }
p { color:#fff5e5; font-size:18px; }
a { display:inline-block; margin-top:30px; padding:15px 30px; background:#ff6a00; color:#000; text-decoration:none; border-radius:10px; font-size:20px; font-weight:bold; box-shadow:0 0 20px #ff6a00; }
a:hover { background:#ffd05b; }
</style></head><body>
<h1>W1NSSPOT</h1>
<p>Добро пожаловать в школьную сеть!</p>
<p>Нажми на кнопку ниже, чтобы войти.</p>
<a href="http://192.168.4.2">ВОЙТИ НА ПОРТАЛ</a>
</body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n--- M5STICK PORTAL ---");

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  WiFi.softAP(ssid, nullptr, WIFI_CHANNEL, false, 12);

  dnsServer.start(53, "*", devkitIP);

  // Отдаем страницу-портал на все запросы
  server.on("/generate_204", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/gen_204", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.on("/", HTTP_GET, []() { server.send(200, "text/html", portal_html); });
  server.onNotFound([]() { server.send(200, "text/html", portal_html); });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
