typedef void (*Function)();
Function Init,l;

void setup() {
  if (digitalRead(4)) {
    Init = WifiSetUp;
    l = Wifi;
  }
  else {
    Init = MQTTSetUp;
    l = MQTT;
  }
  Init();
}
void loop() {
  l();
}
