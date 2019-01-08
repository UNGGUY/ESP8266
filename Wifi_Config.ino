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
  memset(con, 0, sizeof(con) / sizeof(char));
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
