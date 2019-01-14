#include <ESP8266WiFi.h>
typedef void (*Function)();
Function Init, myServer;

void setup() {
  MQTTSetUp();
  WifiSetUp();
}
void loop() {
  if (WiFi.status() == WL_CONNECTED)
    MQTT();
  else
    WifiConfig();
}
