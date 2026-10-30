#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const char* password = "";

WebServer server(80);
bool globalFatal = false;

// Чат
struct ChatMessage { String name; String text; };
ChatMessage chatHistory[20];
int chatCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  
  Serial.println("\n\n======================================");
  Serial.println("=== DEVKIT START (LittleFS Server) ===");
  Serial.println("======================================");
  
  if (LittleFS.begin(true)) {
    Serial.println("[LittleFS] ✅ OK! index.html: " + String(LittleFS.exists("/index.html") ? "ЕСТЬ" : "НЕТ"));
  } else {
    Serial.println("[LittleFS] ❌ ОШИБКА!");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] ✅ IP: " + WiFi.localIP().toString());

  // 1. Отдача главной страницы
  server.on("/", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /");
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // 2. Отдача картинок и шрифтов (явные пути)
  server.on("/mem.png", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /mem.png");
    File file = LittleFS.open("/mem.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });
  server.on("/vasa.png", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /vasa.png");
    File file = LittleFS.open("/vasa.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });
  server.on("/vas.png", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /vas.png");
    File file = LittleFS.open("/vas.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });
  server.on("/Minecraftia-Regular.ttf", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /Minecraftia-Regular.ttf");
    File file = LittleFS.open("/Minecraftia-Regular.ttf", "r");
    server.streamFile(file, "font/ttf");
    file.close();
  });
  server.on("/error.mp4", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /error.mp4");
    File file = LittleFS.open("/error.mp4", "r");
    server.streamFile(file, "video/mp4");
    file.close();
  });

  // 3. API Чата и Статистики
  server.on("/get_messages", HTTP_GET, []() {
    Serial.println("[API] Запрос: /get_messages");
    String json = "[";
    for (int i = 0; i < chatCount; i++) {
      json += "{\"id\":" + String(i) + ",\"name\":\"" + chatHistory[i].name + "\",\"text\":\"" + chatHistory[i].text + "\"}";
      if (i < chatCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/send", HTTP_POST, []() {
    Serial.println("[API] Запрос: /send");
    if (server.hasArg("name") && server.hasArg("text")) {
      if (chatCount >= 20) { for (int i = 0; i < 19; i++) chatHistory[i] = chatHistory[i+1]; chatCount = 19; }
      chatHistory[chatCount].name = server.arg("name");
      chatHistory[chatCount].text = server.arg("text");
      chatCount++;
      server.send(200, "text/plain", "OK");
    } else { server.send(400, "text/plain", "Bad Request"); }
  });

  server.on("/stats", HTTP_GET, []() {
    String json = "{\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",\"temp\":" + String(temperatureRead()) + ",\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",\"sd_status\":\"LittleFS OK\",\"users\":" + String(WiFi.softAPgetStationNum()) + "}";
    server.send(200, "application/json", json);
  });

  // 4. FATAL
  server.on("/fatal", HTTP_POST, []() { globalFatal = true; server.send(200, "text/plain", "OK"); });
  server.on("/get_fatal_status", HTTP_GET, []() { server.send(200, "application/json", "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"); });

  // 5. Если запрос на что-то другое (например, favicon.ico) - просто 404
  server.onNotFound([]() {
    Serial.println("[HTTP] ❌ 404: " + server.uri());
    server.send(404, "text/plain", "404");
  });

  server.begin();
  Serial.println("[HTTP] ✅ ВЕБ-СЕРВЕР ЗАПУЩЕН!");
}

void loop() {
  server.handleClient();
}
