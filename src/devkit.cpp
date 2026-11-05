#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);

// Массив для сообщений
struct Message {
  String id;
  String name;
  String text;
};

Message messages[50]; // Максимум 50 сообщений
int messageCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== DEVKIT START ===");

  if (LittleFS.begin(true)) {
    Serial.println("[LittleFS] OK");
  } else {
    Serial.println("[LittleFS] FAIL");
  }

  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\n[WiFi] IP: " + WiFi.localIP().toString());

  // ЧАТ: Отправка
  server.on("/send", []() {
    String name = server.arg("name");
    String text = server.arg("text");
    if (name.length() > 0 && text.length() > 0) {
      if (messageCount < 50) {
        String id = String(millis());
        messages[messageCount] = {id, name, text};
        messageCount++;
      }
    }
    server.send(200, "text/plain", "OK");
  });

  // ЧАТ: Получение
  server.on("/get_messages", []() {
    String json = "[";
    for (int i = 0; i < messageCount; i++) {
      if (i > 0) json += ",";
      json += "{\"id\":" + messages[i].id + ",\"name\":\"" + messages[i].name + "\",\"text\":\"" + messages[i].text + "\"}";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // Главная
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    if (!file) { server.send(404, "text/plain", "Not found"); return; }
    server.streamFile(file, "text/html");
    file.close();
  });

  // Остальное
  server.onNotFound([]() {
    String path = server.uri();
    if (path == "/") path = "/index.html";
    if (!LittleFS.exists(path)) { server.send(404, "text/plain", "404"); return; }
    File file = LittleFS.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    server.streamFile(file, type);
    file.close();
  });

  server.begin();
  Serial.println("[HTTP] SERVER STARTED");
}

void loop() { server.handleClient(); }
