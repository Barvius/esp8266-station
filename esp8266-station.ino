#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

//#define CONFIG_PIN 13

Adafruit_BMP085 bmp;
ESP8266WebServer HTTP(80);


unsigned long timer = 0;

void ConnectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  timer = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - timer < 10000) {
    delay(500);
    Serial.print(".");
    //    timer = millis();
  }
  if (millis() - timer > 10000) {
    ESP.deepSleep(5 * 60 * 1000 * 1000);
  }
  //Serial.println("");

  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void UpdateFirmware() {
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.141", 80, "/esp8266/", String(ESP.getSketchSize()));
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
  //  httpClient.setUserAgent("ESP8266");
  

  String req = "cid=" + String(ESP.getChipId());
  req += "&temperature_" + String(ESP.getChipId()) + "=";
  req += bmp.readTemperature();
  
  double P = 0;
  for (int i = 0; i < 5; i++) {
    P += bmp.readPressure();
  }
  P /= 5;
  
  req += "&pressure_" + String(ESP.getChipId()) + "=";
  req += P / 133.3;

  req += "&voltage_" + String(ESP.getChipId()) + "=";
  req += String(5.2 * ((float)analogRead(A0) / 1023));

  httpClient.begin("http://192.168.1.141/esp8266/update.php?" + req);

  httpClient.GET();
  httpClient.end();


}


void setup() {
  Serial.begin(115200);

  //  pinMode(CONFIG_PIN, INPUT);
  //  if (digitalRead(CONFIG_PIN) == HIGH) {
  //    WiFi.softAP(String("ESP_" + String(ESP.getChipId())).c_str(), String("ESP_" + String(ESP.getChipId())).c_str());
  //    ConfigServer();
  //  }

  //  pinMode(CONFIG_PIN, INPUT);

  ConnectWiFi();

  bmp.begin();
  SendMeasure();
  UpdateFirmware();
  ESP.deepSleep(5 * 60 * 1000 * 1000);


}

void loop() {
  //  HTTP.handleClient();
  //delay(1);
}
