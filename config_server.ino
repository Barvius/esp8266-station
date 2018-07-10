#include <FS.h>


void available_networks_handler() {
  String json = "[";
  int n = WiFi.scanNetworks();
  if (n) {
    for (int i = 0; i < n; ++i) {
      if (i) {
        json += ",";
      }
      json += "{";
      json += "\"ssid\":\"";
      json += WiFi.SSID(i);
      json += "\",\"rssi\":";
      json += WiFi.RSSI(i);
      json += ",\"encryption\":\"";
      //      json += WiFi.encryptionType(i);
      switch (WiFi.encryptionType(i)) {
        case ENC_TYPE_NONE:
          json += "NONE";
          break;
        case ENC_TYPE_WEP:
          json += "WEP";
          break;
        case ENC_TYPE_TKIP:
          json += "TKIP";
          break;
        case ENC_TYPE_CCMP:
          json += "CCMP";
          break;
        case ENC_TYPE_AUTO:
          json += "AUTO";
          break;
        default:
          json += "?";
          break;
      }
      json += "\"}";
      delay(10);
    }
  }
  json += "]";
  HTTP.send(200, "text/json", json);
}

void restart_handler() {
  HTTP.send(200, "text / plain", "Reset OK");
  ESP.restart();
}

void connect_handler(){
  if (HTTP.argName(0) == "ssid" && HTTP.argName(1) == "psk") {
    
    if(WiFi.begin(HTTP.arg("ssid").c_str(),HTTP.arg("psk").c_str())){
      HTTP.send(200, "text/plain", "OK"); // отправляем ответ о выполнении
      ESP.restart();
    } else {
      HTTP.send(403, "text/plain", "FAIL"); // отправляем ответ о выполнении
    }
    
    
    
  }
}
String getContentType(String filename) {
  if (HTTP.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {  
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void ConfigServer(){
  SPIFFS.begin();
    {
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
      }
    }
    HTTP.on("/restart", restart_handler);
    HTTP.on("/available_networks", available_networks_handler);
    HTTP.on("/connect", connect_handler);
    HTTP.onNotFound([]() {
      if (!handleFileRead(HTTP.uri()))
        HTTP.send(404, "text/plain", "FileNotFound");

    });
    HTTP.begin();
}

