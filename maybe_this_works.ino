
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
/*ADD YOUR PASSWORD BELOW*/
// const char* ssid = "USC Guest Wireless"; // SSID - your WiFi"s name
// const char* password = ""; // Password
const char* ssid = "oplus_co_appqoh"; // SSID - your WiFi"s name
const char* password = "ecbo4562"; // Password

WiFiServer server(80);

bool enableHeater = false;
uint8_t loopCnt = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);

  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");


     while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

   Serial.println("WiFi connected");
  /* Start the server */
  server.begin();
  Serial.println("Server started");
  /* Print the IP address on serial monitor */
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");    /* URL IP to be typed in mobile/desktop browser */
  Serial.print(WiFi.localIP());
  Serial.println("/");
}




void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  /* Wait until the client sends some data */
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }
  /* Read the first line of the request */
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }

  StaticJsonDocument<200> data;
  data["temp"] = t;
  data["humidity"] = h;

  String dataBody;
  serializeJson(data, dataBody);
  Serial.println(dataBody);

  /* Return the response */
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json; charset=utf-8");
  client.println(""); /* do not forget this one */
  client.println(dataBody);
  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");

  // client.println("HTTP/1.1 200 OK");
  // client.println("Content-Type: text/html");
  // client.println(""); /* do not forget this one */
  // client.println("<!DOCTYPE HTML>");
  // client.println("<html>");
  // client.println("<h1 align=center> Hopefully this works</h1><br><br>");
  

  delay(1000);

  // Toggle heater enabled state every 30 seconds
  // An ~3.0 degC temperature increase can be noted when heater is enabled
  if (loopCnt >= 30) {
    enableHeater = !enableHeater;
    sht31.heater(enableHeater);
    Serial.print("Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");

    loopCnt = 0;
  }
  loopCnt++;
}
