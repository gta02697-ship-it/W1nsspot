#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
#define SD_CS 5
WebServer server(80);

String chatMessages[20];
int messageCount = 0;
IPAddress connectedIPs[20];
int connectedCount = 0;

const char* file_manager_html = R"rawliteral(
<!DOCTYPE html>
<html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>SD Управление</title>
<style>
body { background:#080604; color:#fff5e5; font-family:monospace; padding:15px; }
h2 { color:#ff6a00; border-bottom:1px solid #ff6a00; padding-bottom:10px; }
.file-item { border:1px solid #333; padding:10px; margin-bottom:10px; display:flex; justify-content:space-between; align-items:center; background:#120b06; border-radius:5px; }
.del-btn { color:#ff3939; text-decoration:none; font-weight:bold; border:1px solid #ff3939; padding:5px 10px; border-radius:5px; cursor:pointer; }
.del-btn:active { background:#ff3939; color:#000; }
.upload-box { border:1px solid #ff6a00; padding:15px; border-radius:5px; margin-bottom:20px; background:#100a05; }
input[type="file"] { color:#fff; margin-bottom:10px; }
button { background:#ff6a00; color:#000; border:none; padding:10px 15px; font-weight:bold; border-radius:5px; cursor:pointer; }
a.back { color:#35ff9b; text-decoration:none; display:block; margin-top:20px; font-size:14px; }
</style></head><body>
<h2>УПРАВЛЕНИЕ SD-КАРТОЙ</h2>
<div class="upload-box">
  <form method="POST" action="/upload" enctype="multipart/form-data">
    <input type="file" name="file" required>
    <br>
    <button type="submit">ЗАГРУЗИТЬ ФАЙЛ</button>
  </form>
</div>
<div id="file-list">Загрузка списка...</div>
<a href="/" class="back">← Вернуться на главную</a>
<script>
fetch('/list').then(r => r.json()).then(files => {
  let html = '';
  if (files.length === 0) {
    html = '<p style="color:#888">Файлов нет</p>';
  } else {
    files.forEach(f => {
      html += '<div class="file-item"><span>' + f + '</span><button class="del-btn" onclick="delFile(\'' + f + '\')">УДАЛИТЬ</button></div>';
    });
  }
  document.getElementById('file-list').innerHTML = html;
});
function delFile(name) {
  if (confirm('Точно удалить ' + name + '?')) {
    fetch('/delete?file=' + name).then(() => location.reload());
  }
}
</script>
</body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- DEVKIT START ---");

  SPI.begin(18, 19, 23, 5);
  
  bool sdOk = false;
  uint32_t speeds[] = {20000000, 10000000, 6000000, 4000000, 2000000, 1000000};
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("[SD] Пробую скорость %d Гц...\n", speeds[i]);
    if (SD.begin(5, SPI, speeds[i])) {
      Serial.printf("✅ SD OK на скорости %d Гц\n", speeds[i]);
      sdOk = true;
      break;
    }
    SD.end();
    delay(100);
  }

  if (!sdOk) {
    Serial.println("❌ SD CARD NOT FOUND. Работаем без карты.");
  }

  WiFi.mode(WIFI_STA);
  Serial.printf("[WiFi] Подключаюсь к %s...\n", ssid);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("[WiFi] ✅ IP: "); Serial.println(WiFi.localIP());

  server.on("/stats", HTTP_GET, [sdOk]() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    
    // ЧЕСТНАЯ ТЕМПЕРАТУРА ПРОЦЕССОРА
    float temp = temperatureRead();
    
    // РЕАЛЬНАЯ ЧАСТОТА ПРОЦЕССОРА
    int cpuFreq = getCpuFrequencyMhz();
    
    String json = "{";
    json += "\"ram_used\":" + String(usedHeap / 1024) + ",";
    json += "\"ram_total\":" + String(totalHeap / 1024) + ",";
    json += "\"temp\":" + String(temp, 1) + ",";
    json += "\"freq\":" + String(cpuFreq) + ",";
    json += "\"users\":" + String(connectedCount) + ",";
    json += "\"sd_status\":\"" + String(sdOk ? "OK" : "ERROR") + "\"";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
  });

  server.on("/", []() {
    if (!SD.exists("/index.html")) {
      server.send(200, "text/html", "<h2 style='color:red;font-family:monospace'>ОШИБКА: index.html НЕ НАЙДЕН!</h2><a href='/files' style='color:green;'>→ ФАЙЛОВЫЙ МЕНЕДЖЕР</a>");
      return;
    }
    File file = SD.open("/index.html", "r");
    if (file) { server.streamFile(file, "text/html"); file.close(); }
    else { server.send(500, "text/plain", "Ошибка чтения"); }
  });

  server.on("/files", HTTP_GET, [sdOk]() { 
    if (!sdOk) { server.send(500, "text/plain", "SD not initialized"); return; }
    server.send(200, "text/html", file_manager_html); 
  });

  server.on("/list", HTTP_GET, [sdOk]() {
    if (!sdOk) { server.send(500, "text/plain", "SD not initialized"); return; }
    String json = "["; File root = SD.open("/"); File file = root.openNextFile(); bool first = true;
    while (file) { if (!file.isDirectory()) { if (!first) json += ","; json += "\"" + String(file.name()) + "\""; first = false; } file = root.openNextFile(); }
    json += "]"; server.send(200, "application/json", json);
  });

  server.on("/delete", HTTP_GET, [sdOk]() { 
    if (!sdOk) { server.send(500, "text/plain", "SD not initialized"); return; }
    String file = server.arg("file"); if (!file.startsWith("/")) file = "/" + file; 
    if (SD.remove(file)) server.send(200, "text/plain", "OK");
    else server.send(500, "text/plain", "Error");
  });

  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "OK"); }, [sdOk]() {
    if (!sdOk) return;
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) { File f = SD.open("/" + upload.filename, FILE_WRITE); if (f) f.close(); }
    else if (upload.status == UPLOAD_FILE_WRITE) { File f = SD.open("/" + upload.filename, FILE_APPEND); if (f) { f.write(upload.buf, upload.currentSize); f.close(); } }
  });

  server.on("/get_messages", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < messageCount; i++) {
      String msg = chatMessages[i]; int colon = msg.indexOf(": ");
      String name = (colon > 0) ? msg.substring(0, colon) : "???"; String text = (colon > 0) ? msg.substring(colon + 2) : msg;
      name.replace("\"", "'"); text.replace("\"", "'");
      json += "{\"id\":\"srv_" + String(i) + "\",\"name\":\"" + name + "\",\"text\":\"" + text + "\"}";
      if (i < messageCount - 1) json += ",";
    }
    json += "]"; server.send(200, "application/json", json);
  });

  server.on("/send", HTTP_POST, []() {
    String text = server.arg("text"), name = server.arg("name");
    String fullMsg = name + ": " + text;
    if (messageCount < 20) { chatMessages[messageCount] = fullMsg; messageCount++; }
    else { for (int i = 0; i < 19; i++) chatMessages[i] = chatMessages[i+1]; chatMessages[19] = fullMsg; }
    IPAddress ip = server.client().remoteIP(); bool exists = false;
    for (int i = 0; i < connectedCount; i++) if (connectedIPs[i] == ip) exists = true;
    if (!exists && connectedCount < 20) { connectedIPs[connectedCount] = ip; connectedCount++; }
    server.send(200, "text/plain", "OK");
  });

  server.onNotFound([sdOk]() {
    if (!sdOk) { server.send(404, "text/plain", "404 (SD not init)"); return; }
    String path = server.uri(); if (!SD.exists(path)) { server.send(404, "text/plain", "404"); return; }
    File file = SD.open(path, "r"); String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html"; else if (path.endsWith(".png")) type = "image/png"; else if (path.endsWith(".ttf")) type = "font/ttf"; else if (path.endsWith(".mp4")) type = "video/mp4";
    server.streamFile(file, type); file.close();
  });

  server.begin();
  Serial.println("✅ HTTP STARTED");
}

void loop() { server.handleClient(); }
