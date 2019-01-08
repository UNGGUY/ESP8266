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

const char* host = "192.168.199.238";
const uint16_t port = 6000;


File file;
char con[200];
char buf[10240];
String webConfig_info[2];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);   // 打印主题信息
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); // 打印主题内容
  }
  //Serial.println();
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(client_id, mqtt_username, mqtt_password)) {
//      Serial1.println("connected");
      client.subscribe("home/status/");
    } else {
//      Serial1.print("failed, rc=");
//      Serial1.print(client.state());
//      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void handleNotFound()
{
  String content_type;
  char *fileType;
  memset(buf, 0, sizeof(buf) / sizeof(char));
  SPIFFS.begin();
  file = SPIFFS.open(webServer.uri(), "r");
  if (!file)
  {
    file = SPIFFS.open("/index.html", "r");
    content_type="text/html";
  }
  else
  {
    fileType = strrchr(webServer.uri().c_str(), '.');
    if (strcmp(fileType, ".css")==0)
      content_type = "text/css";
    else if (strcmp(fileType, ".img")==0 || strcmp(fileType, ".jpe")==0 || strcmp(fileType, ".jpg")==0)
      content_type = "image/jpeg";
    else if (strcmp(fileType, ".png")==0)
      content_type = "image/png";
    else if (strcmp(fileType, ".js")==0)
      content_type = "application/x-javascript";
  }
  if (file) {
    file.readBytes(buf, file.size());
    webServer.send(200, content_type, buf);
    file.close();
    SPIFFS.end();
  }
}

void getWebConfig()
{
  memset(con, 0, sizeof(con) / sizeof(char));
  SPIFFS.begin();
  file = SPIFFS.open("/config.txt", "r");
  if (file) {
    file.readBytes(con, file.size());
    int i = 0, j = 0;
    for (; con[i] != '\0'; i++)
    {
      if (con[i] != '\n')
        webConfig_info[j] += con[i];
      else
      {
        j++;
      }
    }
  }
  file.close();
  SPIFFS.end();
}


void setWebConfig()
{
  WiFi.begin(webServer.arg("username"), webServer.arg("password"));
  //  if (WiFi.waitForConnectResult() == WL_NO_SSID_AVAIL || WiFi.waitForConnectResult() == WL_CONNECT_FAILED) {
  //    WiFi.disconnect(true);
  //    webServer.send(200, "text/plain", "SSID OR PASSWORD ERROR");
  //  }
  //else {
  String str;
  for (int i = 0; i < webServer.args(); i++)
  {
    str += webServer.arg(i) + "\n";
  }
  memset(con, 0, sizeof(con) / sizeof(char));
  strcpy(con, str.c_str());
  SPIFFS.begin();
  file = SPIFFS.open("/config.txt", "w");
  if (file) {
    file.write((uint8_t*)con, sizeof(con));
    file.close();
    SPIFFS.end();
  }
  webServer.send(200, "text/plain", WiFi.localIP().toString().c_str());
  for (uint8_t i = 0; i < 20; i++)
  {
    delay(500);
    Serial.print(".");
  }
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  webServer.close();
  return;
  //  }
}



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
