#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "༺✧₭Øⳑ⋮ⱱⱥ₦✧༻";
#define SD_CS 5
WebServer server(80);

String chatMessages[20];
int messageCount = 0;

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
      html += '<div class="file-item">';
      html += '<span>' + f + '</span>';
      html += '<button class="del-btn" onclick="delFile(\'' + f + '\')">УДАЛИТЬ</button>';
      html += '</div>';
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

  int sd_attempts = 0;
  while (!SD.begin(5, SPI, 10000000) && sd_attempts < 10) {
    Serial.printf("[SD] Попытка %d/10: ошибка\n", sd_attempts + 1);
    delay(500);
    sd_attempts++;
  }

  if (sd_attempts >= 10) {
    Serial.println("❌ SD CARD NOT FOUND");
  } else {
    Serial.println("✅ SD OK");
  }

  WiFi.mode(WIFI_STA);
  Serial.printf("[WiFi] Подключаюсь к %s...\n", ssid);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("[WiFi] ✅ IP: "); Serial.println(WiFi.localIP());

  server.on("/", []() {
    Serial.println("[HTTP] GET / (главная страница)");
    if (!SD.exists("/index.html")) {
      Serial.println("❌ index.html НЕ НАЙДЕН! Перенаправляю в файловый менеджер.");
      server.send(200, "text/html", "<h2 style='color:red;font-family:monospace'>ОШИБКА: index.html УДАЛЕН!</h2><p style='font-family:monospace'>Перейдите по ссылке ниже, чтобы загрузить его обратно:</p><a href='/files' style='color:green;font-family:monospace;font-size:20px;'>→ ФАЙЛОВЫЙ МЕНЕДЖЕР</a>");
      return;
    }
    File file = SD.open("/index.html", "r");
    if (file) {
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(500, "text/plain", "Ошибка чтения файла");
    }
  });

  server.on("/files", HTTP_GET, []() {
    Serial.println("[HTTP] GET /files (менеджер)");
    server.send(200, "text/html", file_manager_html);
  });

  server.on("/list", HTTP_GET, []() {
    Serial.println("[HTTP] GET /list (список файлов)");
    String json = "[";
    File root = SD.open("/");
    File file = root.openNextFile();
    bool first = true;
    while (file) {
      if (!file.isDirectory()) {
        if (!first) json += ",";
        json += "\"" + String(file.name()) + "\"";
        first = false;
      }
      file = root.openNextFile();
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/delete", HTTP_GET, []() {
    String file = server.arg("file");
    Serial.printf("[HTTP] DELETE %s\n", file.c_str());
    if (!file.startsWith("/")) file = "/" + file;
    if (SD.remove(file)) {
      server.send(200, "text/plain", "OK");
    } else {
      server.send(500, "text/plain", "Ошибка удаления");
    }
  });

  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "OK");
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = "/" + upload.filename;
      Serial.printf("[UPLOAD] Начало: %s\n", filename.c_str());
      File file = SD.open(filename, FILE_WRITE);
      if (file) file.close();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      File file = SD.open("/" + upload.filename, FILE_APPEND);
      if (file) { 
        file.write(upload.buf, upload.currentSize); 
        file.close(); 
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      Serial.printf("[UPLOAD] Завершено: %s (%d байт)\n", upload.filename.c_str(), upload.totalSize);
    }
  });

  server.on("/get_messages", HTTP_GET, []() {
    Serial.println("[HTTP] GET /get_messages");
    String json = "[";
    for (int i = 0; i < messageCount; i++) {
      String msg = chatMessages[i];
      int colonIndex = msg.indexOf(": ");
      String name = (colonIndex > 0) ? msg.substring(0, colonIndex) : "???";
      String text = (colonIndex > 0) ? msg.substring(colonIndex + 2) : msg;
      
      name.replace("\"", "'");
      text.replace("\"", "'");
      
      json += "{\"id\":\"srv_" + String(i) + "\",\"name\":\"" + name + "\",\"text\":\"" + text + "\"}";
      if (i < messageCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  server.on("/send", HTTP_GET, []() {
    String text = server.arg("text");
    String name = server.arg("name");
    Serial.printf("[HTTP] GET /send от %s: %s\n", name.c_str(), text.c_str());
    String fullMsg = name + ": " + text;
    if (messageCount < 20) { chatMessages[messageCount] = fullMsg; messageCount++; }
    else { for (int i = 0; i < 19; i++) chatMessages[i] = chatMessages[i+1]; chatMessages[19] = fullMsg; }
    server.send(200, "text/plain", "OK");
  });

  server.on("/send", HTTP_POST, []() {
    String text = server.arg("text");
    String name = server.arg("name");
    Serial.printf("[HTTP] POST /send от %s: %s\n", name.c_str(), text.c_str());
    String fullMsg = name + ": " + text;
    if (messageCount < 20) { chatMessages[messageCount] = fullMsg; messageCount++; }
    else { for (int i = 0; i < 19; i++) chatMessages[i] = chatMessages[i+1]; chatMessages[19] = fullMsg; }
    server.send(200, "text/plain", "OK");
  });

  server.onNotFound([]() {
    String path = server.uri();
    if (!SD.exists(path)) {
      server.send(404, "text/plain", "404 Not Found");
      return;
    }
    File file = SD.open(path, "r");
    String type = "text/plain";
    if (path.endsWith(".html")) type = "text/html";
    else if (path.endsWith(".png")) type = "image/png";
    else if (path.endsWith(".ttf")) type = "font/ttf";
    else if (path.endsWith(".mp4")) type = "video/mp4";
    server.streamFile(file, type);
    file.close();
  });

  server.begin();
  Serial.println("✅ HTTP STARTED");
}

void loop() { server.handleClient(); }
