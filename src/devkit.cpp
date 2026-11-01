#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);
bool globalFatal = false;

struct ChatMessage { String name; String text; bool isAdmin; };
ChatMessage chatHistory[20];
int chatCount = 0;

// Логи
String logBuffer[50];
int logIndex = 0;
void addLog(String msg) {
  Serial.println(msg);
  logBuffer[logIndex] = msg;
  logIndex = (logIndex + 1) % 50;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  addLog("\n=== DEVKIT START ===");
  
  if (LittleFS.begin(true)) addLog("[LittleFS] ✅ OK");
  else addLog("[LittleFS] ❌ ОШИБКА");

  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  addLog("[WiFi] ✅ IP: " + WiFi.localIP().toString());

  // Универсальный обработчик файлов
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

  // Главная
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Чат
  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("name") && server.hasArg("text")) {
      if (chatCount >= 20) { for (int i = 0; i < 19; i++) chatHistory[i] = chatHistory[i+1]; chatCount = 19; }
      chatHistory[chatCount].name = server.arg("name");
      chatHistory[chatCount].text = server.arg("text");
      chatHistory[chatCount].isAdmin = server.hasArg("isAdmin") && server.arg("isAdmin") == "true";
      chatCount++;
      server.send(200, "text/plain", "OK");
    } else { server.send(400, "text/plain", "Bad Request"); }
  });

  server.on("/get_messages", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < chatCount; i++) {
      json += "{\"id\":" + String(i) + ",\"name\":\"" + chatHistory[i].name + "\",\"text\":\"" + chatHistory[i].text + "\",\"isAdmin\":" + (chatHistory[i].isAdmin ? "true" : "false") + "}";
      if (i < chatCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // Админка (логи и очистка)
  server.on("/clear_chat", HTTP_POST, []() {
    chatCount = 0;
    addLog("[ADMIN] ЧАТ ОЧИЩЕН");
    server.send(200, "text/plain", "OK");
  });

  server.on("/get_logs", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < 50; i++) {
      int idx = (logIndex + i) % 50;
      if (logBuffer[idx] != "") {
        String line = logBuffer[idx];
        line.replace("\"", "\\\"");
        json += "\"" + line + "\"";
        if (i < 49) json += ",";
      }
    }
    if (json.endsWith(",")) json.remove(json.length() - 1);
    json += "]";
    server.send(200, "application/json", json);
  });

  // Статистика
  server.on("/stats", HTTP_GET, []() {
    String json = "{\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",\"temp\":" + String(temperatureRead()) + ",\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",\"sd_status\":\"LittleFS OK\",\"users\":" + String(WiFi.softAPgetStationNum()) + "}";
    server.send(200, "application/json", json);
  });

  server.begin();
  addLog("[HTTP] ✅ СЕРВЕР ЗАПУЩЕН");
}

void loop() { server.handleClient(); }
