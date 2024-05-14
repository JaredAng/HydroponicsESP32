#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "webpage.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <ESP32Time.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define solenoid1 18
#define solenoid2 16
#define solenoid3 5
#define powerSensor 17

// TDs Sensor
#define TdsSensorPin 34
#define VREF 3.3              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point

// Temp Sensor
#define TEMP_PIN 32
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

//pH Sensor
#define phPIN 35
float ph_act = 0.0f;
//int buffer_index = 0, buffer_arr[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, temp = 0;

//SD Card
#define SCK  14
#define MISO  23
#define MOSI  13
#define SDPin 27
//Generic variables
String filePath = "/data.txt";

unsigned long time_sec = 1714247798; // epoch time
ESP32Time rtc(3600 * 8); // offset in seconds GMT+8
int timer = 15;
String watch = rtc.getTime("%d%b %H:%M:%S");

//Network
const char* wifi_network_ssid = "Hydroponics Access Point";
const char* wifi_network_password =  "57790855";

const char *soft_ap_ssid = "Hydroponics_Automated";
const char *soft_ap_password = "testpassword";

AsyncWebServer server(80);

//Timers
unsigned long TDSsampleTimer = millis(), TDSreaderTimer = millis(),
              tempTimer = millis(), phLevelTimer = millis(), WriteTimer = millis(),
              SwitchSensorTimer_on = millis(), SwitchSensorTimer_off = millis(),
              SwitchNutrientTimer_on = millis(), SwitchNutrientTimer_off = millis(),
              SwitchAcidTimer_on = millis(), SwitchAcidTimer_off = millis(),
              SwitchBaseTimer_on = millis(), SwitchBaseTimer_off = millis();
unsigned long delay_Relay = 5000;
bool isTDS_on = true;

void setup() {
  Serial.begin(115200);
  rtc.setTime(time_sec);
  //    WiFi.mode(WIFI_MODE_AP);

  //  WiFi.softAP(soft_ap_ssid, soft_ap_password);
  //  Serial.print("ESP32 IP as soft AP: ");
  //  Serial.println(WiFi.softAPIP());
  connectToWifi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/ph", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", pH.c_str());
  });
  server.on("/ec", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", EC.c_str());
  });

  server.on("/Limit/ph/lw", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", phLimitLW.c_str());
  });
  server.on("/Limit/ph/up", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", phLimitUP.c_str());
  });
  server.on("/Limit/ec/lw", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", ecLimitLW.c_str());
  });
  server.on("/Limit/ec/up", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", ecLimitUP.c_str());
  });

  server.on("/solenoids/acid", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", solenoid[0].c_str());
  });
  server.on("/solenoids/base", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", solenoid[1].c_str());
  });
  server.on("/solenoids/nutrient", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", solenoid[2].c_str());
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
    String posted[2] = {"", ""};
    if (request->hasParam("ecLimitLW", true)) {
      posted[0] = request->getParam("ecLimitUP", true)->value();
      posted[1] = request->getParam("ecLimitLW", true)->value();
      if (posted[0] != "" && posted[0].toFloat() > ecLimitLW.toFloat())
        ecLimitUP = posted[0];
      if (posted[1] != "" && posted[1].toFloat() < ecLimitUP.toFloat())
        ecLimitLW = posted[1];
    }
    if (request->hasParam("phLimitUP", true)) {
      posted[0] = request->getParam("phLimitUP", true)->value();
      posted[1] = request->getParam("phLimitLW", true)->value();
      if (posted[0] != "" && posted[0].toFloat() > phLimitLW.toFloat() )
        phLimitUP = posted[0];
      if (posted[1] != "" && posted[1].toFloat() < phLimitUP.toFloat())
        phLimitLW = posted[1];
    }
    posted[0] = "";
    posted[1] = "";
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/message", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", message.c_str());
  });

  // Start server
  server.begin();

  //filePath = "/" + client_id + "_" + watch + ".log";
  SD_Card_setup(filePath);
  watch = rtc.getTime("%d%b %H:%M:%S");
  message = "timeStamp,tempC,EC,pH";
  if (file_available(SD, filePath)) {
    appendFile(SD, filePath, message);
  } else {
    writeFile(SD, filePath, message);
  }

  // Temp Sensor
  sensors.begin();
  temperatureC = readDSTemperatureC();
  delay(50);

  // pH Sensor
  Wire.begin();
  pinMode(phPIN, INPUT);
  delay(50);

  //TDS Sensor
  pinMode(TdsSensorPin, INPUT);
  delay(50);

  pinMode(solenoid1, OUTPUT);
  pinMode(solenoid2, OUTPUT);
  pinMode(solenoid3, OUTPUT);
  pinMode(powerSensor, OUTPUT);

  digitalWrite(solenoid1, HIGH);
  digitalWrite(solenoid2, HIGH);
  digitalWrite(solenoid3, HIGH);
  digitalWrite(powerSensor, HIGH);
  delay(50);
}

void loop() {

  //TDS
  if (millis() - TDSreaderTimer > 800 && isTDS_on) {
    TDSreaderTimer = millis();
    EC = readTDS();
    //    isTDS_on = false;
  }
  if (millis() - TDSsampleTimer > 40U && isTDS_on) { //every 40 milliseconds,read the analog value from the ADC
    TDSsampleTimer = millis();
    get_analogSampleTimepoint();
  }
  //TDS
  if (millis() - phLevelTimer > 600 && !isTDS_on) {
    phLevelTimer = millis();
    pH = readPHLevel();
  }

  if (millis() - SwitchSensorTimer_off > 2000 && !isTDS_on) {
//    SwitchSensorTimer_on = millis();
    isTDS_on = true;
//     pH = readPHLevel();
    digitalWrite(powerSensor, HIGH);
  }
  if (millis() - SwitchSensorTimer_off > delay_Relay && isTDS_on) {
    SwitchSensorTimer_off = millis();
    isTDS_on = false;
    digitalWrite(powerSensor, LOW);
  }

  if (millis() - tempTimer > 1000) {
    tempTimer = millis();
    temperatureC = readDSTemperatureC();
  }

  if (phLimitLW.toFloat() <= ph_act && ph_act <= phLimitUP.toFloat()) {
    solenoid[0] = "False";
    solenoid[1] = "False";
  } else if (ph_act > phLimitUP.toFloat()) {
    solenoid[0] = "False";
    solenoid[1] = "True";
  } else if (ph_act < phLimitLW.toFloat()) {
    solenoid[0] = "True";
    solenoid[1] = "False";
  }

  if (ecLimitLW.toFloat() <= EC.toFloat() && EC.toFloat() <= ecLimitUP.toFloat()) {
    solenoid[2] = "False";
  } else if (ecLimitLW.toFloat() > EC.toFloat()) {
    solenoid[2] = "True";
  }

  if (millis() - SwitchAcidTimer_on > 2000 && solenoid[0] == "False") {
    SwitchAcidTimer_on = millis();
    digitalWrite(solenoid1, HIGH);
    solenoid[0] = "True";
  } else if (millis() - SwitchAcidTimer_off >  delay_Relay && solenoid[0] == "True") {
    SwitchAcidTimer_off = millis();
    digitalWrite(solenoid1, LOW);
    solenoid[0] = "False";
  }

  if (millis() - SwitchBaseTimer_on >  2000 && solenoid[1] == "False") {
    SwitchBaseTimer_on = millis();
    digitalWrite(solenoid2, HIGH);
    solenoid[1] = "True";
  } else if (millis() - SwitchBaseTimer_off >  delay_Relay && solenoid[1] == "True") {
    SwitchBaseTimer_off = millis();
    digitalWrite(solenoid2, LOW);
    solenoid[1] = "False";
  }

  if (millis() - SwitchNutrientTimer_on >  2000 && solenoid[2] == "False") {
    SwitchNutrientTimer_on = millis();
    digitalWrite(solenoid3, HIGH);
    solenoid[2] = "True";
  } else if (millis() - SwitchNutrientTimer_off >  delay_Relay && solenoid[2] == "True") {
    SwitchNutrientTimer_off = millis();
    digitalWrite(solenoid3, LOW);
    solenoid[2] = "False";
  }

  if (millis() - WriteTimer > 5000) {
    WriteTimer = millis();
    if (message.indexOf("error") > 0) {
      SD_Card_setup(filePath);
    } else {
      watch = rtc.getTime("%d%B%Y %H:%M:%S");
      //    message = temperatureC + "," + EC + "," + pH;
      //    message = watch + "," + temperatureC + "," + EC + "," + pH;
      if (file_available(SD, filePath)) {
        message = appendFile(SD, filePath, watch + "," + temperatureC + "," + EC + "," + pH);
      } else {
        message = "timeStamp,tempC,EC,pH";
        writeFile(SD, filePath, message);
        message = appendFile(SD, filePath, watch + "," + temperatureC + "," + EC + "," + pH);
      }
    }

  }
}
