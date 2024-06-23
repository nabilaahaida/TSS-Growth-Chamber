#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi
//const char *ssid = "BRIN_Net"; // Enter your WiFi name
//const char *password = NULL ;  // Enter WiFi password
const char *ssid = "BRINnet"; // Enter your WiFi name
const char *password = "brin@2022" ;  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "10.10.35.117";
//const char *mqtt_broker = "103.144.45.116";
const char *topic1 = "Shallot/Data";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

String inString;

WiFiClient espClient;
PubSubClient client(espClient);

void initWiFi() {
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void initMQTT() {
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "esp8266-client-";
      client_id += String(WiFi.macAddress());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("connected to the MQTT broker");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi(); 
  initMQTT(); 
}

void callback(char *topic, byte *payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
}

void loop() {
  while (Serial.available()) {
      char inChar = Serial.read();
      inString += inChar;
      String data1;
      String data2;
      String data3;
      String data4;
      if (inChar == '\n') {
        parsingData(inString, &data1, &data2, &data3, &data4);
        char data1_chr[data1.length()];
        char data2_chr[data2.length()];
        char data3_chr[data3.length()];
        char data4_chr[data4.length()];

        data1.toCharArray(data1_chr, data1.length());
        data2.toCharArray(data2_chr, data2.length());
        data3.toCharArray(data3_chr, data3.length());
        data4.toCharArray(data4_chr, data4.length());

        StaticJsonDocument<200> jsonDoc;
        jsonDoc["tempCom"] = data1_chr;
        jsonDoc["tempRad"] = data2_chr;
        jsonDoc["tempCham"] = data3_chr;
        jsonDoc["humCham"] = data4_chr;

        inString = "";

        String jsonString;
        serializeJson(jsonDoc, jsonString);
        client.publish(topic1, jsonString.c_str());
        delay(500); 
      }
  }
client.subscribe("Shallot/LED");
delay(500);
client.loop();
}
      
void parsingData(String dataIn, String *data1, String *data2, String *data3, String *data4) {
  int j = 0;
  String tmp[4];

  for (int i = 0; i < dataIn.length(); i++) {
    if(dataIn[i] == ',' or dataIn[i] == '\n') {
      j++;
    } else {
      tmp[j] += dataIn[i];
    }
  }

  *data1 = tmp[0];
  *data2 = tmp[1];
  *data3 = tmp[2];
  *data4 = tmp[3];
}
