#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
const char* password = "";

WebServer server(80);
bool globalFatal = false;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  delay(1000);
  
  Serial.println("\n\n======================================");
  Serial.println("=== DEVKIT START (LittleFS Server) ===");
  Serial.println("======================================");
  
  Serial.println("[LittleFS] Монтирую файловую систему...");
  if (LittleFS.begin(true)) {
    Serial.print("[LittleFS] ✅ OK! Занято: ");
    Serial.print(LittleFS.usedBytes() / 1024);
    Serial.print(" КБ из ");
    Serial.print(LittleFS.totalBytes() / 1024);
    Serial.println(" КБ");
    
    // Проверяем наличие главного файла
    if (LittleFS.exists("/index.html")) {
      Serial.println("[LittleFS] ✅ index.html НАЙДЕН!");
    } else {
      Serial.println("[LittleFS] ❌ index.html НЕ НАЙДЕН!");
    }
  } else {
    Serial.println("[LittleFS] ❌ ОШИБКА МОНТИРОВАНИЯ!");
  }

  Serial.println("[WiFi] Подключаюсь к: " + String(ssid));
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] ✅ УСПЕХ! Мой IP: " + WiFi.localIP().toString());
    Serial.println("[WiFi] Шлюз (Стик): " + WiFi.gatewayIP().toString());
  } else {
    Serial.println("\n[WiFi] ❌ НЕ ПОДКЛЮЧИЛСЯ!");
  }

  // Ловим ВСЕ запросы
  server.onNotFound([]() {
    String clientIP = server.client().remoteIP().toString();
    String path = server.uri();
    Serial.println("[HTTP] >>> Запрос от " + clientIP + " на " + path);
    
    if (path == "/") path = "/index.html";
    
    if (!LittleFS.exists(path)) { 
        Serial.println("[HTTP] ❌ 404 ОШИБКА: Файл не найден " + path);
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
    
    Serial.println("[HTTP] ✅ Отдаю файл: " + path + " (" + String(file.size()) + " байт, " + type + ")");
    server.streamFile(file, type);
    file.close();
  });

  server.on("/", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос на главную страницу (/)");
    if (!LittleFS.exists("/index.html")) { 
      Serial.println("[HTTP] ❌ Нет index.html");
      server.send(404, "text/plain", "no index"); return; 
    }
    File file = LittleFS.open("/index.html", "r");
    Serial.println("[HTTP] ✅ Отдаю index.html");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/stats", HTTP_GET, []() {
    String json = "{\"ram_used\":" + String((ESP.getHeapSize() - ESP.getFreeHeap()) / 1024) + ",\"ram_total\":" + String(ESP.getHeapSize() / 1024) + ",\"temp\":" + String(temperatureRead()) + ",\"load\":" + String(100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize())) + ",\"sd_status\":\"LittleFS OK\",\"users\":" + String(WiFi.softAPgetStationNum()) + "}";
    server.send(200, "application/json", json);
  });

  server.on("/fatal", HTTP_POST, []() { 
    Serial.println("[ADMIN] ⚠️ ПОЛУЧЕНА КОМАНДА FATAL!");
    globalFatal = true; 
    server.send(200, "text/plain", "OK"); 
  });
  
  server.on("/get_fatal_status", HTTP_GET, []() { 
    String json = "{\"fatal\": " + String(globalFatal ? "true" : "false") + "}"; 
    server.send(200, "application/json", json); 
  });

  server.begin();
  Serial.println("[HTTP] ✅ ВЕБ-СЕРВЕР ЗАПУЩЕН! ЖДУ ПОДКЛЮЧЕНИЙ...");
}

void loop() {
  server.handleClient();
}
