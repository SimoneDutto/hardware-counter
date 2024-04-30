#include "SevSeg.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Arduino_JSON.h>

SevSeg sevseg; 
const char* ssid = "<your-wifi>";
const char* password = "<wifi-password>";
IPAddress local_IP(192, 168, 1, 199);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8); // this is optional
IPAddress secondaryDNS(8, 8, 4, 4); // this is optional

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
int i = 0;
void setup(){
  Serial.begin(9600);
  Serial.println("Starting board");
  setupSevSeg();
  setupWifi();
}

void setupWifi(){
  // Connect to Wi-Fi
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");  
}


void setupSevSeg(){
  Serial.println("Settting up sevseg");
  // set 
  byte numDigits = 3;
  byte digitPins[] = {12,13,15};
  byte segmentPins[] = {14,10,1,2,0,4,5,0};
  
  byte hardwareConfig = COMMON_CATHODE; 
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros);
  sevseg.setBrightness(200);
}

void loop() {
  static String response;
  if ((millis() - lastTime) > timerDelay) {
    if(WiFi.status()== WL_CONNECTED){
      int currentUser = httpGETRequest("<url-api>");
      sevseg.setNumber(currentUser, -1);
      lastTime = millis();
      i++;
    } else {
      sevseg.setNumber(998, -1);
    }
  }
  sevseg.refreshDisplay();
}

int httpGETRequest(const char* serverName) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  
  //create an HTTPClient instance
  HTTPClient https;
  String payload = "{}";

  if (https.begin(*client, serverName)) {  // HTTPS
    Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    https.setAuthorization("username", "password");
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        payload = https.getString();
        Serial.println(payload);
        https.end();
        JSONVar myObject = JSON.parse(payload);

        if (JSON.typeof(myObject) == "undefined") {
          return 999;
        }

        return int(myObject["num_games"]);
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    return httpCode;
  }
  return -1;
}
