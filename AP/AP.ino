// display
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI display = TFT_eSPI(135, 240);

#include <Button2.h>
Button2 btn1(35);
Button2 btn2(0);

#include <EEPROM.h>

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>

// Wi-Fi credentials
const char *ssid = "SecPriPC";
#define number_passwords 3
String passwords[number_passwords] = { "thisisapassword", "thunderstorms", "theblackpearl" };

// Server login credentials
const char* login_username = "sppc-admin";
const char* login_password = "adianiz";

// Settings for handling authentication
const char* realm = "Login Required";
const String auth_fail_response = "Authentication Failed";

int index_password = 0;
int number_visits = 0;

WebServer server(80);

void handle_home() {
  server.send(200, "text/plain", "Welcome home! You can find a special message by logging in and going to /special");
}

void handle_login() {
  if (!server.authenticate(login_username, login_password)) {
    return server.requestAuthentication(DIGEST_AUTH, realm, auth_fail_response);
  }
  server.send(200, "text/plain", "Login OK! Go to /special");
}

void handle_special() {
  if (!server.authenticate(login_username, login_password)) {
    server.send(200, "text/plain", "You must be logged in to see the special message. Go to /login\n(Safari Browser is not supported)");
  } else {
    ++number_visits;
    String special_message = "Congratulations! The special message is: InventingPasswordsIsReallyHard. The server has booted <t> seconds ago and there have been <v> visits so far.";
    String time_elapsed = (String)(millis()/1000);
    special_message.replace("<t>", time_elapsed);
    special_message.replace("<v>", (String)number_visits);
    server.send(200, "text/plain", special_message);
    display_visits();
  }
}

void setup_server() {
  server.on("/", handle_home);
  server.on("/login", handle_login);
  server.on("/special", handle_special);
  server.begin();

  Serial.println("Server started");
  display.println("Server started");
}



void setup_WIFI() {
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_MODE_AP);
  WiFi.softAP(ssid, passwords[index_password].c_str(), 1);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setup_display() {
  display.init();
  display.setTextColor(TFT_GREEN, TFT_BLACK);
  display.fillScreen(TFT_BLACK);
  display.setRotation(1);
  display.setTextSize(2);
}

void setup_button() {
    btn1.setPressedHandler([](Button2 & b) {
      EEPROM.write(0, (number_passwords + index_password-1)%number_passwords);
      EEPROM.commit();
      display.println("restart");
      ESP.restart();
    });

    btn2.setPressedHandler([](Button2 & b) {
        EEPROM.write(0, (index_password+1)%number_passwords);
        EEPROM.commit();
        display.println("restart");
        ESP.restart();
    });
}

void display_visits() {
  display.setTextColor(TFT_RED, TFT_BLACK);
  display.drawString("visits", 160, 0, 1);
  display.drawString(String(number_visits), 200, 20, 4);
  display.setTextColor(TFT_GREEN, TFT_BLACK);
}

void setup() {
  setup_button();
  setup_display();

  Serial.begin(115200);
  EEPROM.begin(1);

  index_password = EEPROM.read(0) % number_passwords;
  display.println();
  display.println("Groupe " + String(index_password+1));
  display.println("PW: " + passwords[index_password].substring(0,5) + "***");
  display.println();

  display_visits();

  setup_WIFI();

  setup_server();
}

void loop() {
  btn1.loop();
  btn2.loop();

  server.handleClient();
}
