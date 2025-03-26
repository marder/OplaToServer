#include <SPI.h>
#include <WiFiNINA.h>
#include "ArduinoJson.h"
#include "arduino_secrets.h"

#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;

int status = WL_IDLE_STATUS;

WiFiSSLClient client;

char server[] = "marder.bieda.it";
unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 10L * 1000L;

void setup() {

  carrier.noCase();
  carrier.begin();

  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  printWifiStatus();
}

void loop() {

  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

void httpRequest() {

  client.stop();

  JsonDocument doc;
  doc["description"] = "arduino-opla";
  doc["temperature"] = carrier.Env.readTemperature();
  doc["humidity"] = carrier.Env.readHumidity();
  doc["pressure"] = carrier.Pressure.readPressure(MILLIBAR);

  if (client.connect(server, 443)) {
    Serial.println("connecting...");
    Serial.println("POST /dhtData?api_key=1qazxsw23 HTTP/1.1");
    client.println("POST /dhtData?api_key=1qazxsw23 HTTP/1.1");
    Serial.println("content-type: application/json");
    client.println("content-type: application/json");
    Serial.println("Host: marder.bieda.it");
    client.println("Host: marder.bieda.it");
    Serial.print("content-length: ");
    client.print("content-length: ");
    Serial.println(measureJson(doc));
    client.println(measureJson(doc));
    client.println();
    serializeJson(doc, Serial);
    serializeJson(doc, client);
    client.println();
    lastConnectionTime = millis();

  } else {
    Serial.println("connection failed");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}