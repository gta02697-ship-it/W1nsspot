#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const char* password = "";

WebServer server(80);
bool sdOk = false;
bool globalFatal = false;

// Простая структура для чата
struct ChatMessage {
  String name;
  String text;
};
ChatMessage chatHistory[20];
int chatCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  Serial.println("\n\n=== START W1NSSPOT DEVKIT ===");
  
  // 1. Инициализация LittleFS
  Serial.println("[LittleFS] Монтирую...");
  if (LittleFS.begin(true)) {
    sdOk = true;
    Serial.print("[LittleFS] OK! Занято: ");
    Serial.print(LittleFS.usedBytes() / 1024);
    Serial.println(" КБ");
  } else {
    Serial.println("[LittleFS] ОШИБКА МОНТИРОВАНИЯ!");
  }

  // 2. Подключение к Wi-Fi
  Serial.println("[WiFi] Подключаюсь к " + String(ssid) + "...");
  IPAddress local_IP(192, 168, 4, 2);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
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

  // 3. Обработчики сервера
  server.on("/", HTTP_GET, []() {
    if (!sdOk) { server.send(500, "text/plain", "LittleFS Error"); return; }
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      server.send(404, "text/html", "<h2 style='color:red;font-family:monospace'>ОШИБКА: index.html НЕ НАЙДЕН В LittleFS!</h2><a href='/files' style='color:green;'>→ ФАЙЛОВЫЙ МЕНЕДЖЕР</a>");
      return;
    }
    server.streamFile(file, "text/html");
    file.close();
  });

  // Статика (картинки, видео)
  server.onNotFound([]() {
    String path = server.uri();
    if (path == "/") path = "/index.html";
    if (!LittleFS.exists(path)) { 
        server.send(404, "text/plain", "404 Not Found"); 
        return; 
    }
    File file = LittleFS.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".css")) type = "text/css";
    else if (path.endsWith(".js")) type = "application/javascript";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".mp4")) type = "video/mp4";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    server.streamFile(file, type);
    file.close();
  });

  // API Статистики
  server.on("/stats", HTTP_GET, []() {
    String json = "{";
    json += "\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",";
    json += "\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",";
    json += "\"temp\":" + String(temperatureRead()) + ",";
    json += "\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",";
    json += "\"sd_status\":\"" + String(sdOk ? "LittleFS OK" : "ERROR") + "\",";
    json += "\"users\":" + String(WiFi.softAPgetStationNum());
    json += "}";
    server.send(200, "application/json", json);
  });

  // Чат
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
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
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

  // FATAL (Админка)
  server.on("/fatal", HTTP_POST, []() { 
    globalFatal = true; 
    server.send(200, "text/plain", "OK"); 
  });
  
  server.on("/get_fatal_status", HTTP_GET, []() { 
    String json = "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"; 
    server.send(200, "application/json", json); 
  });

  // Файловый менеджер (заглушка, но рабочая)
  server.on("/files", HTTP_GET, []() {
    String html = "<html><body style='background:#111;color:#ff8c00;font-family:monospace;'><h2>LittleFS Files</h2><ul>";
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      html += "<li>" + String(file.name()) + " (" + String(file.size()) + " bytes)</li>";
      file = root.openNextFile();
    }
    html += "</ul></body></html>";
    server.send(200, "text/html", html);
  });

  server.onNotFound([]() {
    Serial.println("[HTTP] Запрос: " + server.uri() + " от " + server.client().remoteIP().toString());
    String path = server.uri();
    if (path == "/") path = "/index.html";
    if (!LittleFS.exists(path)) { 
        Serial.println("[HTTP] ❌ 404: " + path);
        server.send(404, "text/plain", "404 Not Found"); 
        return; 
    }
    File file = LittleFS.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".css")) type = "text/css";
    else if (path.endsWith(".js")) type = "application/javascript";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".mp4")) type = "video/mp4";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    server.streamFile(file, type);
    file.close();
    Serial.println("[HTTP] ✅ Отдан файл: " + path);
  });

  server.begin();
  Serial.println("[HTTP] ✅ SERVER STARTED");
}

void loop() {
  server.handleClient();
}
