#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SSD1306Wire.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFS.h>

const char* ssid = ".";
const char* password = "12345678";

AsyncWebServer server(80);

Adafruit_INA219 ina219;
SSD1306Wire display(0x3c, 5, 4);

#define WIDTH 128
#define HIGHT 64
#define SCALE 0.005

float lastValues[WIDTH];
int lastValue = -100000;

#define MAXFILESIZE 72000

void setup() {
  display.init();
  //display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  Serial.begin(115200);

  SPIFFS.begin();

  IPAddress local_IP(2, 0, 0, 1);

  WiFi.persistent(false);
  WiFi.softAP(ssid, password);
  delay(2000);
  WiFi.softAPConfig(local_IP, local_IP, IPAddress(255, 255, 255, 0));

  File f = SPIFFS.open("/power.csv", "r");
  if (!f) {
    Serial.println("Failed to open file");
  }
  Serial.println(f.size());
  f.close();


  delay(1000);

  //Wire.begin(14,13);
  ina219.begin();//(&Wire);

  //server.serveStatic("/", SPIFFS, "/index.html");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.serveStatic("/d3.min.js", SPIFFS, "/d3.min.js");

  server.on("/power.csv", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/power.csv", "text/plain");
  });

  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.println("Measuring voltage and current with INA219 ...");
}


void loop() {
  unsigned long time = millis();
  float power_mW = 0;
  float current_mA = 0;
  int samples = 100;//10000

  for (int i = 0; i < samples; i++) {
    //shuntvoltage = ina219.getShuntVoltage_mV();
    //busvoltage = ina219.getBusVoltage_V();
    //loadvoltage = busvoltage + (shuntvoltage / 1000);
    current_mA += 0;//ina219.getCurrent_mA();
    power_mW += random(28000);//ina219.getPower_mW();

  }

  int power = (power_mW / samples) * ((current_mA >= 0) ? 1 : -1);
  Serial.print("Power:"); Serial.print(power); Serial.println(" ");
  Serial.print("Time:"); Serial.print(millis()-time); Serial.println(" ");

  File f = SPIFFS.open("/power.csv", "a");
  if (!f || f.size() > MAXFILESIZE) {
    f.close();
    f = SPIFFS.open("/power.csv", "w");
    f.println("power");
  }
  Serial.println(f.size());
  if (!f.println(String(power))) {
    Serial.println("Error writting file");
  }
  f.close();

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 10, "Power");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, String((float)power/1000) + " W");

  float offset = SCALE * (lastValue - power);
  lastValue = power;
  for (int i=WIDTH-1; i>0; i--) {
    lastValues[i] = lastValues[i-1];
  }
  lastValues[0] = 0;
  for (int i=1; i<WIDTH-1; i++) {
    display.setPixel(WIDTH-i, HIGHT/2 + lastValues[i]);
    lastValues[i] += offset;;
  }

  Serial.println(offset);

  display.display();
}
