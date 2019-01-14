#include <PubSubClient.h>
#include <FS.h>
WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "192.168.199.238";
const char* client_id = "clientId-ApjJZcy9Dh";
const char* mqtt_username = "admin";
const char* mqtt_password = "public";
String webConfig_info[2];
char configInfo[200];
int num = 0;
File f;


void getWebConfig()
{
  memset(configInfo, 0, sizeof(configInfo) / sizeof(char));
  SPIFFS.begin();
  f = SPIFFS.open("/config.txt", "r");
  if (f) {
    f.readBytes(configInfo, f.size());
    int i = 0, j = 0;
    for (; configInfo[i] != '\0'; i++)
    {
      if (configInfo[i] != '\n')
        webConfig_info[j] += configInfo[i];
      else
      {
        j++;
      }
    }
  }
  f.close();
  SPIFFS.end();
  memset(configInfo, 0, sizeof(configInfo) / sizeof(char));
}


void MQTTSetUp()
{
  Serial.begin(115200);
  getWebConfig();
  WiFi.mode(WIFI_STA);
  WiFi.begin(webConfig_info[0], webConfig_info[1]);
  for (int i = 0; i < 10; i++)
  {
    delay(500);
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.softAPdisconnect(true);
      break;
    }
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void MQTT()
{
  if (Serial.available()) {
    configInfo[num] = Serial.read();
    if (configInfo[0] == 0xA1&& configInfo[1] == 0xA1) {
      WiFi.begin("####", "####");
      memset(configInfo, 0, sizeof(configInfo) / sizeof(char));
      num = 0;
    }
    else if (configInfo[0] == 0xA5 && configInfo[1] == 0xA5) {
      client.publish("home/status/", configInfo);
      memset(configInfo, 0, sizeof(configInfo) / sizeof(char));
      num = 0;
    }
    else if (num = 1 && ((configInfo[0] != 'A' && configInfo[1] != 'A') && (configInfo[0] != 0xA5 && configInfo[1] != 0xA5)))
      num = -1;
    num++;
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);   // 打印主题信息
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); // 打印主题内容
  }
  Serial.println();
}


void reconnect() {
  while (!client.connected()) {
    // Serial.print("Attempting MQTT connection...");
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
