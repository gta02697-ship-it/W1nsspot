#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "//_CHAT_02_\\";
#define SD_CS 5
WebServer server(80);

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  
  // Принудительно снижаем скорость до 10 МГц
  if (!SD.begin(5, SPI, 10000000)) { 
    Serial.println("SD FAIL"); 
  } else {
    Serial.println("SD OK");
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  server.on("/", []() {
    File file = SD.open("/index.html", "r");
    if (file) { server.streamFile(file, "text/html"); file.close(); }
    else server.send(404, "text/plain", "404");
  });

  server.onNotFound([]() {
    String path = server.uri();
    if (!SD.exists(path)) { server.send(404, "text/plain", "404"); return; }
    File file = SD.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    server.streamFile(file, type);
    file.close();
  });

  server.begin();
  Serial.println("HTTP STARTED");
}

void loop() {
  server.handleClient();
}
