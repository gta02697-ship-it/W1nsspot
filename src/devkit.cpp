#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);
bool globalFatal = false;

// Жестко ставим ДевКит на .2
IPAddress local_IP(192, 168, 4, 2);
IPAddress gateway(192, 168, 4, 1); // Шлюз (Стик) на .1
IPAddress subnet(255, 255, 255, 0);

struct ChatMessage { String name; String text; };
ChatMessage chatHistory[20];
int chatCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  Serial.println("\n=== DEVKIT START (.2) ===");
  
  if (LittleFS.begin(true)) {
    Serial.print("[LittleFS] OK! Занято: ");
    Serial.print(LittleFS.usedBytes() / 1024);
    Serial.println(" КБ");
  } else { Serial.println("[LittleFS] FAIL!"); }

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid);
  
  Serial.print("[WiFi] Подключение");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] ✅ IP: " + WiFi.localIP().toString());

  // Обработчик всех файлов (картинки, шрифты)
  server.onNotFound([]() {
    String path = server.uri();
    if (path == "/") path = "/index.html";
    
    Serial.println("[HTTP] Запрос: " + path + " от " + server.client().remoteIP().toString());
    
    if (!LittleFS.exists(path)) { 
        Serial.println("[HTTP] ❌ 404: " + path);
        server.send(404, "text/plain", "404 Not Found"); 
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

  // Главная страница
  server.on("/", HTTP_GET, []() {
    if (!LittleFS.exists("/index.html")) { server.send(404, "text/plain", "no index"); return; }
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Статистика
  server.on("/stats", HTTP_GET, []() {
    String json = "{\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",\"temp\":" + String(temperatureRead()) + ",\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",\"sd_status\":\"LittleFS OK\",\"users\":" + String(WiFi.softAPgetStationNum()) + "}";
    server.send(200, "application/json", json);
  });

  // Чат
  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("name") && server.hasArg("text")) {
      if (chatCount >= 20) { for (int i = 0; i < 19; i++) chatHistory[i] = chatHistory[i+1]; chatCount = 19; }
      chatHistory[chatCount].name = server.arg("name");
      chatHistory[chatCount].text = server.arg("text");
      chatCount++;
      server.send(200, "text/plain", "OK");
    } else { server.send(400, "text/plain", "Bad Request"); }
  });

  server.on("/get_messages", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < chatCount; i++) {
      json += "{\"id\":" + String(i) + ",\"name\":\"" + chatHistory[i].name + "\",\"text\":\"" + chatHistory[i].text + "\"}";
      if (i < chatCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // FATAL
  server.on("/fatal", HTTP_POST, []() { globalFatal = true; server.send(200, "text/plain", "OK"); });
  server.on("/get_fatal_status", HTTP_GET, []() { server.send(200, "application/json", "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"); });

  server.begin();
  Serial.println("[HTTP] ✅ SERVER STARTED");
}
void loop() { server.handleClient(); }
