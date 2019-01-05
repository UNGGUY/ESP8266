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

const char* host = "192.168.199.238";
const uint16_t port = 6000;

File file;
char con[200];
char buf[10240];
String webConfig_info[2];



void handleNotFound()
{
  memset(buf, 0, sizeof(buf) / sizeof(char));
  SPIFFS.begin();
  file = SPIFFS.open(webServer.uri(), "r");
  if (file)
  {
    file.readBytes(buf, file.size());
    file.close();
    SPIFFS.end();
    webServer.send(200, "text/css", buf);
  }
  else
  {
    file = SPIFFS.open("/index.html", "r");
    if (file) {
      file.readBytes(buf, file.size());
      file.close();
      SPIFFS.end();
      webServer.send(200, "text/html", buf);
    }
  }

}

void getWebConfig()
{
  memset(con, 0, sizeof(con) / sizeof(char));
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
        //Serial.println(webConfig_info[j]);
        j++;
      }
    }
  }
}


void setWebConfig()
{
  //  Serial.println(webServer.uri());
  Serial.println(webServer.arg("username"));
  Serial.println(webServer.arg("password"));
  WiFi.begin(webServer.arg("username"), webServer.arg("password"));

  for (uint8_t i = 0; i <= 10; i++)
  {
    delay(500);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED) {
      String str;
      for (int i = 0; i < webServer.args(); i++)
      {
        str += webServer.arg(i) + "\n";
      }
      Serial.println(str);
      memset(con, 0, sizeof(con) / sizeof(char));
      strcpy(con, str.c_str());
      Serial.println("config");
      SPIFFS.begin();
      file = SPIFFS.open("/config.txt", "w");
      if (file) {
        file.write((uint8_t*)con, sizeof(con));
        file.close();
        SPIFFS.end();
      }
      Serial.println("save");
      webServer.send(200, "text/html", WiFi.localIP().toString().c_str());
      Serial.println(WiFi.localIP().toString().c_str());
      for (uint8_t i = 0; i < 20; i++)
      {
        delay(500);
        Serial.print(".");
      }

      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      webServer.close();
      return;
    }
  }
  WiFi.disconnect(true);
  webServer.send(200, "text/plain", "Set Wifi config error!");
}



void setup() {
  Serial.begin(115200);
  Serial.println("\nHello");
  SPIFFS.begin();
  getWebConfig();
  Serial.println(webConfig_info[0]);
  Serial.println(webConfig_info[1]);
  WiFi.begin(webConfig_info[0], webConfig_info[1]);
  for (uint8_t i = 0; i <= 5; i++)
  {
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("ocnfig");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("DNSServer CaptivePortal example");

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", apIP);


    webServer.on("/api/wifi/config_set", setWebConfig);
    // replay to all requests with same HTML
    webServer.onNotFound(handleNotFound);
    webServer.begin();
  }

  else {
    WiFi.mode(WIFI_STA);
    Serial.println(WiFi.localIP().toString().c_str());
    Serial.println("WiFi Begin");
  }
}



void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
  else
  {
    memset(con, 0, sizeof(con) / sizeof(char));
    Serial.readBytes(con, sizeof(con));
    delay(1000);
    if (strlen(con)) {
      WiFiClient client;
      if (!client.connect(host, port)) {
        Serial.println("connection failed");
        Serial.println("wait 5 sec...");
        delay(5000);
      }
      else {
        client.println(con);
        String line = client.readStringUntil('\r');
        Serial.println(line);
        client.stop();
      }
    }
  }
}