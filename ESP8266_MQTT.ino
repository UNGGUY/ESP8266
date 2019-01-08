#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <FS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "192.168.199.238";
const char* client_id = "clientId-ApjJZcy9Dh";
const char* mqtt_username = "admin";
const char* mqtt_password = "public";
long lastMsg;
File file;
char con[200];
char buf[10240];
String webConfig_info[2];


void setup() {
  Serial.begin(115200);
  getWebConfig();
  WiFi.begin(webConfig_info[0], webConfig_info[1]);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/api/wifi/config_set", setWebConfig);
  // replay to all requests with same HTML
  webServer.onNotFound(handleNotFound);

}



void loop() {

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    if (Serial.available()) {
      memset(con, 0, sizeof(con) / sizeof(char));
      Serial.readBytes(con, sizeof(con));
      client.publish("home/status/", con);
    }
  }
  else {
    if (WiFi.waitForConnectResult() == WL_NO_SSID_AVAIL || WiFi.waitForConnectResult() == WL_CONNECT_FAILED) {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP("ESP8266 WiFi config");
      webServer.begin();
    }
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
}
