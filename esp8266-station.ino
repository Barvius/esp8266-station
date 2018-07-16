#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define CONFIG_PIN 13
#define CHARGE_BEGIN_PIN 14
#define CHARGE_END_PIN 12

Adafruit_BMP085 bmp;
ESP8266WebServer HTTP(80);

void ConnectWiFi() {

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
  httpClient.setUserAgent("ESP8266");

  String req = "cid=" + String(ESP.getChipId());
  req += "&temperature_" + String(ESP.getChipId()) + "=";
  req += bmp.readTemperature();
  req += "&pressure_" + String(ESP.getChipId()) + "=";
  req += bmp.readPressure() / 133.3;
  req += "&voltage_" + String(ESP.getChipId()) + "=";
  req += String(5.3 * ((float)analogRead(A0) / 1023));
  req += "&charge_begin_" + String(ESP.getChipId()) + "=";
  req += String(digitalRead(CHARGE_BEGIN_PIN));
  req += "&charge_end_" + String(ESP.getChipId()) + "=";
  req += String(digitalRead(CHARGE_END_PIN));

  httpClient.begin("http://192.168.1.141/update_station.php?" + req);
 
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
  
  pinMode(CONFIG_PIN, INPUT);
  pinMode(CHARGE_BEGIN_PIN, INPUT);
  pinMode(CHARGE_END_PIN, INPUT);
  
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
