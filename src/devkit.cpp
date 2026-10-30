#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const char* password = "";

WebServer server(80);
bool globalFatal = false;

// Настройки статического IP
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Чат
struct ChatMessage { String name; String text; };
ChatMessage chatHistory[20];
int chatCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  
  Serial.println("\n\n=== START W1NSSPOT DEVKIT ===");
  
  Serial.println("[LittleFS] Монтирую...");
  if (LittleFS.begin(true)) {
    Serial.print("[LittleFS] OK! Занято: ");
    Serial.print(LittleFS.usedBytes() / 1024);
    Serial.println(" КБ");
  } else {
    Serial.println("[LittleFS] ОШИБКА МОНТИРОВАНИЯ!");
  }

  WiFi.config(local_IP, gateway, subnet);
  Serial.println("[WiFi] Подключаюсь к " + String(ssid) + "...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] ✅ IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WiFi] ❌ Не удалось подключиться!");
  }

  // --- ДЕТАЛЬНЫЕ ЛОГИ ---
  server.onNotFound([]() {
    Serial.println("[HTTP] Запрос на: " + server.uri() + " от " + server.client().remoteIP().toString());
    String path = server.uri();
    if (path == "/") path = "/index.html";
    
    if (!LittleFS.exists(path)) {
      Serial.println("[HTTP] ❌ 404: файл не найден " + path);
      server.send(404, "text/plain", "404 Not Found");
      return;
    }
    
    File file = LittleFS.open(path, "r");
    if (!file) {
      Serial.println("[HTTP] ❌ Не удалось открыть файл: " + path);
      server.send(500, "text/plain", "Error opening file");
      return;
    }
    
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".css")) type = "text/css";
    else if (path.endsWith(".js")) type = "application/javascript";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".mp4")) type = "video/mp4";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    
    Serial.println("[HTTP] ✅ Отдаю файл: " + path + " (" + type + ", " + String(file.size()) + " байт)");
    server.streamFile(file, type);
    file.close();
  });

  server.on("/", HTTP_GET, []() {
    Serial.println("[HTTP] Главная страница /");
    if (!LittleFS.exists("/index.html")) { server.send(404, "text/plain", "index.html not found"); return; }
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/stats", HTTP_GET, []() {
    String json = "{";
    json += "\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",";
    json += "\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",";
    json += "\"temp\":" + String(temperatureRead()) + ",";
    json += "\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",";
    json += "\"sd_status\":\"LittleFS OK\",";
    json += "\"users\":" + String(WiFi.softAPgetStationNum());
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("name") && server.hasArg("text")) {
      if (chatCount >= 20) {
        for (int i = 0; i < 19; i++) chatHistory[i] = chatHistory[i+1];
        chatCount = 19;
      }
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

  server.on("/fatal", HTTP_POST, []() { globalFatal = true; server.send(200, "text/plain", "OK"); });
  server.on("/get_fatal_status", HTTP_GET, []() { 
    String json = "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"; 
    server.send(200, "application/json", json); 
  });

  server.begin();
  Serial.println("[HTTP] ✅ SERVER STARTED (WAITING FOR CLIENTS)");
}

void loop() {
  server.handleClient();
}
