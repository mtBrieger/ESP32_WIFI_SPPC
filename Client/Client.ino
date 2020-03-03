// display
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI display = TFT_eSPI(135, 240);

#include <Button2.h>
Button2 btn1(35);
Button2 btn2(0);

#include <EEPROM.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <base64.h>

// Wi-Fi credentials
const char *ssid = "SecPriPC";
#define number_passwords 3
String passwords[number_passwords] = { "thisisapassword", "thunderstorms", "theblackpearl" };

// Server login credentials
String login_username = "sppc-admin";
String login_password = "adianiz";

int index_password = 0;

void setup_display() {
  display.init();
  display.setTextColor(TFT_BLUE, TFT_BLACK);
  display.fillScreen(TFT_BLACK);
  display.setRotation(1);
  display.setTextSize(2);
}

void setup_button() {
    btn1.setPressedHandler([](Button2 & b) {
      EEPROM.write(0, (number_passwords + index_password-1)%number_passwords);
      EEPROM.commit();
      display.println();
      display.println("restart");
      ESP.restart();
    });

    btn2.setPressedHandler([](Button2 & b) {
        EEPROM.write(0, (index_password+1)%number_passwords);
        EEPROM.commit();
        display.println();
        display.println("restart");
        ESP.restart();
    });
}

unsigned long start_time;

void connect_WIFI() {
    //delay(5000);

    // Connect to access point
    display.print("Connecting");
    WiFi.enableSTA(true);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, passwords[index_password].c_str(), 1);

    start_time = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        display.print(".");
        btn1.loop();
        btn2.loop();
        // Reboot board if connection is not working
        if (millis() - start_time > 5000) {
            ESP.restart();
        }
    }
    display.println();
    display.println("Connected");

    Serial.println();
    Serial.println("Wi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress());

    // Make HTTP request
    Serial.println();
    Serial.println("GET request");

    HTTPClient client;
    client.begin("http://192.168.4.1/login");
    String auth = base64::encode(login_username + ':' + login_password);
    client.addHeader("Authorization", "Basic " + auth);

    int httpCode = client.GET();

    display.print("GET code: ");
    display.println(httpCode);
    if (httpCode > 0) {
      display.print("Payload:  ");
      display.println(client.getString());
    }

    delay(5000);

    Serial.println();
    Serial.println("closing WiFi");

    WiFi.disconnect();
    ESP.restart();
}

void setup() {
  setup_button();
  setup_display();

  Serial.begin(115200);
  EEPROM.begin(1);

  index_password = EEPROM.read(0) % number_passwords;
  display.println("Groupe " + String(index_password+1));
  display.println("PW: " + passwords[index_password].substring(0,5) + "***");
  display.println();
}

void loop() {
  btn1.loop();
  btn2.loop();

  connect_WIFI();
}
