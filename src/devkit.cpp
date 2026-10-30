#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);
bool globalFatal = false;

struct ChatMessage { String name; String text; };
ChatMessage chatHistory[20];
int chatCount = 0;

// Буфер для логов (последние 50 строк)
String logBuffer[50];
int logIndex = 0;

void addLog(String msg) {
  Serial.println(msg);
  logBuffer[logIndex] = msg;
  logIndex = (logIndex + 1) % 50;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  addLog("\n=== DEVKIT START ===");
  
  if (LittleFS.begin(true)) addLog("[LittleFS] ✅ OK!");
  else addLog("[LittleFS] ❌ ОШИБКА!");

  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  addLog("\n[WiFi] ✅ IP: " + WiFi.localIP().toString());

  // Главная
  server.on("/", HTTP_GET, []() {
    addLog("[HTTP] Запрос: /");
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Кэширование
  auto serveCachedFile = [](String path, String type) {
    File file = LittleFS.open(path, "r");
    if (!file) { server.send(404, "text/plain", "404"); return; }
    server.sendHeader("Cache-Control", "max-age=31536000, public");
    server.streamFile(file, type);
    file.close();
  };

  server.on("/mem.png", HTTP_GET, [serveCachedFile]() { serveCachedFile("/mem.png", "image/png"); });
  server.on("/vasa.png", HTTP_GET, [serveCachedFile]() { serveCachedFile("/vasa.png", "image/png"); });
  server.on("/vas.png", HTTP_GET, [serveCachedFile]() { serveCachedFile("/vas.png", "image/png"); });
  server.on("/Minecraftia-Regular.ttf", HTTP_GET, [serveCachedFile]() { serveCachedFile("/Minecraftia-Regular.ttf", "font/ttf"); });
  server.on("/error.mp4", HTTP_GET, [serveCachedFile]() { serveCachedFile("/error.mp4", "video/mp4"); });
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204, "text/plain", ""); });

  // API Чата
  server.on("/get_messages", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < chatCount; i++) {
      json += "{\"id\":" + String(i) + ",\"name\":\"" + chatHistory[i].name + "\",\"text\":\"" + chatHistory[i].text + "\"}";
      if (i < chatCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("name") && server.hasArg("text")) {
      if (chatCount >= 20) { for (int i = 0; i < 19; i++) chatHistory[i] = chatHistory[i+1]; chatCount = 19; }
      chatHistory[chatCount].name = server.arg("name");
      chatHistory[chatCount].text = server.arg("text");
      addLog("[CHAT] Новое сообщение от " + server.arg("name"));
      chatCount++;
      server.send(200, "text/plain", "OK");
    } else { server.send(400, "text/plain", "Bad Request"); }
  });

  server.on("/stats", HTTP_GET, []() {
    String json = "{\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",\"temp\":" + String(temperatureRead()) + ",\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",\"sd_status\":\"LittleFS OK\",\"users\":" + String(WiFi.softAPgetStationNum()) + "}";
    server.send(200, "application/json", json);
  });

  // --- НОВЫЕ API ДЛЯ АДМИНКИ ---
  
  // 1. Очистка чата
  server.on("/clear_chat", HTTP_POST, []() {
    chatCount = 0;
    addLog("[ADMIN] 🗑️ ЧАТ ОЧИЩЕН!");
    server.send(200, "text/plain", "OK");
  });

  // 2. Получение логов
  server.on("/get_logs", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < 50; i++) {
      int idx = (logIndex + i) % 50;
      if (logBuffer[idx] != "") {
        // Экранируем кавычки для JSON
        String line = logBuffer[idx];
        line.replace("\"", "\\\"");
        json += "\"" + line + "\"";
        if (i < 49) json += ",";
      }
    }
    // Убираем лишнюю запятую в конце, если есть
    if (json.endsWith(",")) json.remove(json.length() - 1);
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/fatal", HTTP_POST, []() { globalFatal = true; server.send(200, "text/plain", "OK"); });
  server.on("/get_fatal_status", HTTP_GET, []() { server.send(200, "application/json", "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"); });

  server.begin();
  addLog("[HTTP] ✅ СЕРВЕР ЗАПУЩЕН!");
}
void loop() { server.handleClient(); }
