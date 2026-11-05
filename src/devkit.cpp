#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "KOLIVAN-PRIME";
WebServer server(80);

String chatMessages = ""; // Глобальная переменная для сообщений

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

  if (LittleFS.begin(true)) {
    addLog("[LittleFS] ✅ OK");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      addLog("Файл: " + String(file.name()) + " (" + String(file.size()) + " байт)");
      file = root.openNextFile();
    }
  } else {
    addLog("[LittleFS] ❌ ОШИБКА");
  }

  Serial.print("\n[WiFi] Пытаюсь подключиться к: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  addLog("[WiFi] ✅ IP: " + WiFi.localIP().toString());

  // Обработчик отправки сообщения
  server.on("/send", []() {
    String name = server.arg("name");
    String text = server.arg("text");
    if (name.length() > 0 && text.length() > 0) {
      chatMessages += name + ": " + text + "\n";
    }
    server.send(200, "text/plain", "OK");
  });

  // Обработчик получения сообщений
  server.on("/get_messages", []() {
    server.send(200, "text/plain", chatMessages);
  });

  // Главная страница
  server.on("/", HTTP_GET, []() {
    Serial.println("[HTTP] Запрос: /");
    File file = LittleFS.open("/index.html", "r");
    if (!file) {
      Serial.println("[HTTP] ОШИБКА: /index.html НЕ НАЙДЕН!");
      server.send(404, "text/plain", "index.html not found");
      return;
    }
    Serial.println("[HTTP] ✅ Отдаю index.html");
    server.streamFile(file, "text/html");
    file.close();
  });

  // Универсальный обработчик
  server.onNotFound([]() {
    String path = server.uri();
    if (path == "/") path = "/index.html";
    Serial.println("[HTTP] Запрос: " + path);
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
    Serial.println("[HTTP] ✅ Отдаю: " + path);
    server.streamFile(file, type);
    file.close();
  });

  server.begin();
  addLog("[HTTP] ✅ СЕРВЕР ЗАПУЩЕН");
}

void loop() { server.handleClient(); }
