#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"

const char* ssid = "iot-gateway";
const char* password = "iot-nw-pilot13";

const char* mqtt_server = "192.168.42.1";
const char* deviceID = "sht-9999";
const char* clientID = "sht-9999";

int infoLED = 14;

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

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

  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  
  connectWiFi();
  mqttClient.begin(mqtt_server, wifiClient);
}


void loop() {
  mqttClient.loop();
  delay(10);
  
  if(!mqttClient.connected()){
    reconnectMQTT();
  } else {
    float t = htu.readTemperature();
    float h = htu.readHumidity();
    float f = (t * 9/5) + 32;

    if (! isnan(t)) {  // check if 'is not a number'
      Serial.print("Temp *C = "); Serial.println(t);
      Serial.print("Temp *F = "); Serial.println(f);
    } else { 
      Serial.println("Failed to read temperature");
    }
    
    if (! isnan(h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); Serial.println(h);
    } else { 
      Serial.println("Failed to read humidity");
    }
    Serial.println();
    
    int batLevel = analogRead(A0);
    Serial.print("Analog reading =  ");
    Serial.println(batLevel);
    batLevel = map(batLevel, 580, 774, 0, 100);
    Serial.print("Battery level = ");
    Serial.print(batLevel);
    Serial.println("%");

    root["deviceID"] = deviceID;

    data["temp"] = double_with_n_digits(f,2);
    data["humidity"] = double_with_n_digits(h,2);
    data["battery"] = batLevel;
    
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
