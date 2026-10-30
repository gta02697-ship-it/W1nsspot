#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  Serial.println("\n=== DEVKIT START ===");
  
  if (LittleFS.begin(true)) {
    Serial.print("[LittleFS] OK! Занято: ");
    Serial.println(LittleFS.usedBytes() / 1024);
  } else { Serial.println("[LittleFS] FAIL!"); }

  // ДевКит получает IP автоматически (должен быть .2 или .3)
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] IP: " + WiFi.localIP().toString());

  server.onNotFound([]() {
    Serial.println("[HTTP] Запрос: " + server.uri() + " от " + server.client().remoteIP().toString());
    String path = server.uri();
    if (path == "/") path = "/index.html";
    if (!LittleFS.exists(path)) { 
        Serial.println("[HTTP] 404: " + path);
        server.send(404, "text/plain", "404"); 
        return; 
    }
    File file = LittleFS.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".mp4")) type = "video/mp4";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    
    Serial.println("[HTTP] ✅ Отдаю: " + path);
    server.streamFile(file, type);
    file.close();
  });

  server.on("/", HTTP_GET, []() {
    if (!LittleFS.exists("/index.html")) { server.send(404, "text/plain", "no index"); return; }
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.begin();
  Serial.println("[HTTP] SERVER STARTED");
}
void loop() { server.handleClient(); }
