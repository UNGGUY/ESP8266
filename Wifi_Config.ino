#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>

File file;
char con[200];
char buf[10240];
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String FILETYPE[5] = {".css", ".img", ".jpe", ".jpg", ".js"};
String CONTENTTYPE[5] = {"text/css", "image/jpeg", "image/jpeg", "image/jpeg", "application/x-javascript"};

void Wifi()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void WifiSetUp()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ESP8266 WiFi Config");
  webServer.begin();
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/api/wifi/config_set", setWebConfig);
  // replay to all requests with same HTML
  webServer.onNotFound(handleNotFound);
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
    content_type = "text/html";
  }
  else
  {
    fileType = strrchr(webServer.uri().c_str(), '.');
    for (int i = 0; i < 5; i++)
    {
      if (strcmp(fileType, FILETYPE[i].c_str()) == 0)
        content_type = CONTENTTYPE[i];
    }
  }
  if (file) {
    file.readBytes(buf, file.size());
    webServer.send(200, content_type, buf);
    file.close();
    SPIFFS.end();
  }
}




void setWebConfig()
{
  WiFi.begin(webServer.arg("username"), webServer.arg("password"));
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
}
