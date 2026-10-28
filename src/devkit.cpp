#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "//_CHAT_02_\\";
#define SD_CS 5
WebServer server(80);

String chatMessages[20];
int messageCount = 0;

const char* file_manager_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>SD Files</title>
<style>
body { background:#080604; color:#fff5e5; font-family:sans-serif; padding:20px; }
h2 { color:#ff6a00; border-bottom:1px solid #ff6a00; padding-bottom:10px; }
.btn { background:#ff6a00; color:#000; padding:10px 15px; border-radius:5px; text-decoration:none; font-weight:bold; display:inline-block; margin-bottom:15px; }
.item { border:1px solid #333; padding:10px; margin-bottom:10px; display:flex; justify-content:space-between; align-items:center; background:#120b06; border-radius:5px; }
.del { color:#ff3939; text-decoration:none; font-weight:bold; padding:5px 10px; border:1px solid #ff3939; border-radius:5px; }
.del:hover { background:#ff3939; color:#000; }
</style></head><body>
<h2>УПРАВЛЕНИЕ SD-КАРТОЙ</h2>
<a href="/upload" class="btn">ЗАГРУЗИТЬ НОВЫЙ ФАЙЛ</a>
<div id="list"></div>
<script>
fetch('/list').then(r=>r.json()).then(data=>{
  let html='';
  data.forEach(f=>{
    html+='<div class="item"><span>'+f+'</span><a href="#" class="del" onclick="delFile(\''+f+'\')">УДАЛИТЬ</a></div>';
  });
  document.getElementById('list').innerHTML=html;
});
function delFile(name){
  if(confirm('Удалить '+name+'?')){
    fetch('/delete?file='+name).then(()=>location.reload());
  }
}
</script></body></html>
)rawliteral";

const char* upload_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Upload</title>
<style>
body { background:#080604; color:#fff5e5; font-family:sans-serif; text-align:center; padding-top:50px; }
h2 { color:#ff6a00; }
input[type="file"] { color:#fff; margin:20px 0; }
.btn { background:#ff6a00; color:#000; padding:10px 20px; border:none; border-radius:5px; font-weight:bold; cursor:pointer; }
a { color:#35ff9b; text-decoration:none; display:block; margin-top:20px; }
</style></head><body>
<h2>ЗАГРУЗКА НА SD</h2>
<form method="POST" action="/upload" enctype="multipart/form-data">
  <input type="file" name="file"><br><br>
  <button type="submit" class="btn">ЗАГРУЗИТЬ</button>
</form>
<a href="/files">← Назад к файлам</a>
</body></html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  if (!SD.begin(5, SPI, 10000000)) { Serial.println("SD FAIL"); } else { Serial.println("SD OK"); }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  server.on("/get_messages", HTTP_GET, []() {
    String json = "[";
    for (int i = 0; i < messageCount; i++) {
      json += "\"" + chatMessages[i] + "\"";
      if (i < messageCount - 1) json += ",";
    }
    json += "]";
    server.send(200, "application/json", json);
  });
  server.on("/send", HTTP_POST, []() {
    String text = server.arg("text");
    String name = server.arg("name");
    String fullMsg = name + ": " + text;
    if (messageCount < 20) { chatMessages[messageCount] = fullMsg; messageCount++; }
    else { for (int i = 0; i < 19; i++) chatMessages[i] = chatMessages[i+1]; chatMessages[19] = fullMsg; }
    server.send(200, "text/plain", "OK");
  });

  server.on("/files", HTTP_GET, []() { server.send(200, "text/html", file_manager_html); });
  server.on("/list", HTTP_GET, []() {
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
    if (!file.startsWith("/")) file = "/" + file;
    SD.remove(file);
    server.send(200, "text/plain", "OK");
  });

  server.on("/upload", HTTP_GET, []() { server.send(200, "text/html", upload_html); });
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "OK"); }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = "/" + upload.filename;
      File file = SD.open(filename, FILE_WRITE);
      if (file) file.close();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      File file = SD.open("/" + upload.filename, FILE_APPEND);
      if (file) { file.write(upload.buf, upload.currentSize); file.close(); }
    }
  });

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
    else if (path.endsWith(".mp4")) type = "video/mp4";
    server.streamFile(file, type);
    file.close();
  });

  server.begin();
  Serial.println("HTTP STARTED");
}

void loop() { server.handleClient(); }
