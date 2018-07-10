#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define CONFIG_PIN 13

Adafruit_BMP085 bmp;
ESP8266WebServer HTTP(80);

void ConnectWiFi() {
  //
  //  unsigned long wifiConnectStart = millis();
  //  while (WiFi.status() != WL_CONNECTED) {
  //    // Check to see if
  //    if (WiFi.status() == WL_CONNECT_FAILED) {
  //      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
  //      delay(10000);
  //    }
  //    delay(500);
  //    Serial.println("...");
  //    // Only try for 5 seconds.
  //    if (millis() - wifiConnectStart > 15000) {
  //      Serial.println("Failed to connect to WiFi");
  //      return;
  //    }
  //  }
  Serial.print("Connecting to ");
  

  WiFi.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void UpdateFirmware() {
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.141", 80, "/esp8266/FWU.php", String(ESP.getSketchSize()));
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}

void SendMeasure() {

  HTTPClient httpClient;
  httpClient.setUserAgent("ShinePhone 1.3 (iPhone; iOS 9.0.2; Scale/1.0)");

  //  int deviceCount = sensors.getDeviceCount();  // узнаем количество подключенных градусников
  //sensors.requestTemperatures();

  String req = "cid=" + String(ESP.getChipId());

  req += "&temperature_" + String(ESP.getChipId()) + "=";
  req += bmp.readTemperature();
  req += "&pressure_" + String(ESP.getChipId()) + "=";
  req += bmp.readPressure() / 133.3;
  req += "&val_" + String(ESP.getChipId()) + "=";
  req += String(digitalRead(CONFIG_PIN));
  //    for (int i = 0; i <= deviceCount - 1; i++) {
  //      DeviceAddress Address18b20;
  //      sensors.getAddress(Address18b20, i);
  //      String tmp = "";
  //      String t = "";
  //      for (int i = 0; i < 8; i++) {
  //        t = String(Address18b20[i], HEX);
  //        while ( t.length() < 2)  t = "0" +  t;
  //        tmp += t;
  //      }
  //      req += "&" +tmp+"=";
  //      req += sensors.getTempC(Address18b20);
  //    }
  httpClient.begin("http://192.168.1.141/update.php?" + req);
  httpClient.GET();
  httpClient.end();
}


void setup() {
  Serial.begin(115200);
  Serial.println(WiFi.psk());
  pinMode(CONFIG_PIN, INPUT);
  if (digitalRead(CONFIG_PIN) == HIGH) {
    WiFi.softAP(String("ESP_" + String(ESP.getChipId())).c_str(), String("ESP_" + String(ESP.getChipId())).c_str());
    ConfigServer();
  } else {
    ConnectWiFi();
    bmp.begin();
    SendMeasure();
    UpdateFirmware();
    ESP.deepSleep(5 * 60 * 1000 * 1000);
  }

}

void loop() {
  HTTP.handleClient();
  delay(1);
}
