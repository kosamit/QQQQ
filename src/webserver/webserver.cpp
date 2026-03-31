/*
 * @Description: Web file manager for SD card
 * @Author: kosamit
 */

#include "webserver.h"

WebServer server(80);
bool webServerRunning = false;

// HTML ページ（ファイル一覧 + アップロード + 削除）
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>QQQQ SD File Manager</title>
<style>
  body { font-family: -apple-system, sans-serif; background: #1a1a2e; color: #eee; margin: 0; padding: 20px; }
  h1 { color: #e94560; margin-bottom: 5px; }
  .info { color: #888; margin-bottom: 20px; font-size: 14px; }
  .upload-area { background: #16213e; border: 2px dashed #e94560; border-radius: 10px; padding: 20px; margin-bottom: 20px; text-align: center; }
  .upload-area input[type=file] { margin: 10px 0; }
  .btn { background: #e94560; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; font-size: 14px; }
  .btn:hover { background: #c73e54; }
  .btn-del { background: #555; padding: 5px 12px; font-size: 12px; }
  .btn-del:hover { background: #e94560; }
  table { width: 100%; border-collapse: collapse; }
  th, td { text-align: left; padding: 10px; border-bottom: 1px solid #333; }
  th { color: #e94560; }
  a { color: #0fbcf9; text-decoration: none; }
  a:hover { text-decoration: underline; }
  .size { color: #888; }
  #progress { display: none; margin-top: 10px; }
  #progress-bar { width: 100%; height: 20px; border-radius: 10px; }
  #status { margin-top: 10px; color: #0fbcf9; }
</style>
</head>
<body>
<h1>QQQQ SD File Manager</h1>
<div class="info" id="sd-info"></div>
<div class="upload-area">
  <p>Upload files to SD card</p>
  <input type="file" id="fileInput" multiple>
  <button class="btn" onclick="uploadFiles()">Upload</button>
  <div id="progress"><progress id="progress-bar" value="0" max="100"></progress></div>
  <div id="status"></div>
</div>
<table>
  <thead><tr><th>File</th><th>Size</th><th></th></tr></thead>
  <tbody id="filelist"></tbody>
</table>
<script>
function formatSize(b) {
  if (b < 1024) return b + ' B';
  if (b < 1048576) return (b/1024).toFixed(1) + ' KB';
  return (b/1048576).toFixed(1) + ' MB';
}
function loadFiles() {
  fetch('/api/list').then(r=>r.json()).then(data => {
    document.getElementById('sd-info').textContent =
      'Total: ' + formatSize(data.total) + ' / Used: ' + formatSize(data.used) + ' / Free: ' + formatSize(data.total - data.used);
    let html = '';
    data.files.forEach(f => {
      html += '<tr><td><a href="/api/download?file=' + encodeURIComponent(f.name) + '">' + f.name + '</a></td>'
            + '<td class="size">' + formatSize(f.size) + '</td>'
            + '<td><button class="btn btn-del" onclick="delFile(\'' + f.name + '\')">Delete</button></td></tr>';
    });
    document.getElementById('filelist').innerHTML = html || '<tr><td colspan="3">No files</td></tr>';
  });
}
function uploadFiles() {
  const files = document.getElementById('fileInput').files;
  if (!files.length) return;
  const status = document.getElementById('status');
  const prog = document.getElementById('progress');
  prog.style.display = 'block';
  let i = 0;
  function next() {
    if (i >= files.length) { status.textContent = 'Done!'; loadFiles(); return; }
    const f = files[i];
    status.textContent = 'Uploading ' + f.name + '...';
    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/upload?filename=/' + encodeURIComponent(f.name));
    xhr.upload.onprogress = e => {
      if (e.lengthComputable) document.getElementById('progress-bar').value = (e.loaded/e.total)*100;
    };
    xhr.onload = () => { i++; next(); };
    xhr.onerror = () => { status.textContent = 'Error uploading ' + f.name; };
    xhr.send(f);
  }
  next();
}
function delFile(name) {
  if (!confirm('Delete ' + name + '?')) return;
  fetch('/api/delete?file=' + encodeURIComponent(name), {method:'DELETE'}).then(() => loadFiles());
}
loadFiles();
</script>
</body>
</html>
)rawliteral";

// ファイル一覧API
static void handleList() {
    String json = "{\"files\":[";
    File root = SD.open("/");
    bool first = true;
    if (root) {
        while (true) {
            File entry = root.openNextFile();
            if (!entry) break;
            if (!entry.isDirectory()) {
                if (!first) json += ",";
                json += "{\"name\":\"" + String(entry.name()) + "\",\"size\":" + String(entry.size()) + "}";
                first = false;
            }
            entry.close();
        }
        root.close();
    }
    json += "],\"total\":" + String(SD.totalBytes()) + ",\"used\":" + String(SD.usedBytes()) + "}";
    server.send(200, "application/json", json);
}

// ファイルアップロードAPI
static File uploadFile;
static void handleUploadStart() {
    server.send(200, "text/plain", "OK");
}

static void handleUploadData() {
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        String filename = server.arg("filename");
        if (filename.isEmpty()) filename = "/" + upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;
        Serial.printf("Upload: %s\n", filename.c_str());
        uploadFile = SD.open(filename, FILE_WRITE);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
            Serial.printf("Upload complete: %u bytes\n", upload.totalSize);
        }
    }
}

// ファイルダウンロードAPI
static void handleDownload() {
    String filename = server.arg("file");
    if (!filename.startsWith("/")) filename = "/" + filename;

    File file = SD.open(filename);
    if (!file) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    String contentType = "application/octet-stream";
    String nameLower = filename;
    nameLower.toLowerCase();
    if (nameLower.endsWith(".mp3")) contentType = "audio/mpeg";
    else if (nameLower.endsWith(".wav")) contentType = "audio/wav";
    else if (nameLower.endsWith(".txt")) contentType = "text/plain";
    else if (nameLower.endsWith(".json")) contentType = "application/json";

    server.streamFile(file, contentType);
    file.close();
}

// ファイル削除API
static void handleDelete() {
    String filename = server.arg("file");
    if (!filename.startsWith("/")) filename = "/" + filename;

    if (SD.remove(filename)) {
        server.send(200, "text/plain", "Deleted");
        Serial.printf("Deleted: %s\n", filename.c_str());
    } else {
        server.send(500, "text/plain", "Delete failed");
    }
}

// インデックスページ
static void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void startWebServer() {
    if (webServerRunning) return;
    if (!Wifi_Connection_Flag) {
        Serial.println("WiFi not connected - cannot start web server");
        return;
    }

    server.on("/", handleRoot);
    server.on("/api/list", HTTP_GET, handleList);
    server.on("/api/upload", HTTP_POST, handleUploadStart, handleUploadData);
    server.on("/api/download", HTTP_GET, handleDownload);
    server.on("/api/delete", HTTP_DELETE, handleDelete);

    server.begin();
    webServerRunning = true;

    Serial.println("Web server started");
    Serial.print("URL: http://");
    Serial.println(WiFi.localIP());
}

void stopWebServer() {
    if (!webServerRunning) return;
    server.close();
    webServerRunning = false;
    Serial.println("Web server stopped");
}

void handleWebServer() {
    if (webServerRunning) {
        server.handleClient();
    }
}
