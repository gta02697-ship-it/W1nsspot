#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
WebServer server(80);

String chatMessages = ""; // Сообщения в формате "id|name|text\n"

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
      String id = String(millis());
      chatMessages += id + "|" + name + "|" + text + "\n";
      if (chatMessages.length() > 5000) { // Ограничение памяти
        chatMessages = chatMessages.substring(chatMessages.indexOf("\n") + 1);
      }
    }
    server.send(200, "text/plain", "OK");
  });

  // ЧАТ: Получение
  server.on("/get_messages", []() {
    String json = "[";
    int start = 0;
    bool first = true;
    while (start < chatMessages.length()) {
      int end = chatMessages.indexOf("\n", start);
      if (end == -1) end = chatMessages.length();
      String line = chatMessages.substring(start, end);
      
      int p1 = line.indexOf("|");
      int p2 = line.indexOf("|", p1 + 1);
      if (p1 > 0 && p2 > p1) {
        String id = line.substring(0, p1);
        String name = line.substring(p1 + 1, p2);
        String text = line.substring(p2 + 1);
        if (!first) json += ",";
        json += "{\"id\":" + id + ",\"name\":\"" + name + "\",\"text\":\"" + text + "\"}";
        first = false;
      }
      start = end + 1;
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
