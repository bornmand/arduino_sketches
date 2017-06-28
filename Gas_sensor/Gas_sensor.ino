#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "iot-gateway";
const char* password = "iot-nw-pilot13";

const char* mqtt_server = "192.168.42.1";
const char* deviceID = "gas-mq4-001";
const char* clientID = "gas-mq4-001";

int minVal = 1000;
int maxVal = 0;

int infoLED = 14;

WiFiClient wifiClient;
MQTTClient mqttClient;
DynamicJsonBuffer jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonArray& message = root.createNestedArray("readings");
JsonObject& data = message.createNestedObject();

void connectWiFi(){
  delay(10);
  Serial.print("preparing wifi...");
  
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    
    delay(200);
  }
  Serial.println();
  Serial.println("Connected to WiFi!");
}

void reconnectMQTT(){
  while(!mqttClient.connected()){
    Serial.println("Connecting to Mqtt Broker...");
    if(mqttClient.connect(clientID)){
      Serial.println("Connected to Broker!");
      mqttClient.subscribe("/sensors/command");
    } else {
      Serial.println("Failed to connect to Mqtt Broker, retrying...");
      delay(2500);
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(infoLED, OUTPUT);

  connectWiFi();
  mqttClient.begin(mqtt_server, wifiClient);
}

void loop() {
  mqttClient.loop();
  delay(10);
  
  if(!mqttClient.connected()){
    reconnectMQTT();
  } else {
    int gasLevel = analogRead(A0);
    if (gasLevel < minVal){
      minVal = gasLevel;
    }
    if (gasLevel > maxVal){
      maxVal = gasLevel;
    }
    Serial.print("level =  "); Serial.print(gasLevel);
    Serial.print("\t"); Serial.print("min="); Serial.print(minVal);
    Serial.print("\t"); Serial.print("max="); Serial.print(maxVal);

    root["deviceID"] = deviceID;
    data["gas"] = gasLevel;

    Serial.print("\t");
    root.printTo(Serial);
    Serial.println();
    char buffer[256];
    root.printTo(buffer, sizeof(buffer));

    digitalWrite(infoLED, HIGH);
    mqttClient.publish("sensor", String (buffer));
    delay(100);
    digitalWrite(infoLED, LOW);
    
    delay(1000);
    
  }
}

void messageReceived(String topic, String payload, char* bytes, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] Payload: [");
  Serial.print(payload);
  Serial.print("]");
  Serial.println();
}

