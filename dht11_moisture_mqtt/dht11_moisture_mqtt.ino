#include <ArduinoJson.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include <TimeLib.h>


#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int moisturePin = A0;
int moistureValue = 0;

int wifiLED = 14;
int mqttLED = 13;
int dataLED = 15;

const char* ssid = "VzMiFi6620L-BE7B";
const char* password = "4a23aab3";
const char* mqtt_server = "192.168.42.1";
const char* deviceName = "dht11_bornman";


WiFiClient wifiClient;
MQTTClient mqttClient;
DynamicJsonBuffer jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonArray& message = root.createNestedArray("message");
JsonObject& data = message.createNestedObject();

void connectWiFi(){
  delay(10);
  Serial.print("preparing wifi...");
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(wifiLED, LOW);
    Serial.print(".");
    delay(200);
  }
  digitalWrite(wifiLED, HIGH);
  Serial.println();
  Serial.println("Connected to WiFi!");
}

void reconnectMQTT(){
  while(!mqttClient.connected()){
    digitalWrite(mqttLED, LOW);
    Serial.println("Connecting to Mqtt Broker...");
    if(mqttClient.connect("ESP8266Client")){
      digitalWrite(mqttLED, HIGH);
      Serial.println("Connected to Broker!");
      mqttClient.subscribe("/moistureSensor/command");
    } else {
      Serial.println("Failed to connect to Mqtt Broker, retrying...");
      delay(2500);
    }
  }
}

void setup() {
  pinMode(wifiLED, OUTPUT);
  pinMode(mqttLED, OUTPUT);
  Serial.begin(9600);
  connectWiFi();
  mqttClient.begin(mqtt_server, wifiClient);
}


void loop() {
  mqttClient.loop();
  delay(10);

  if(!mqttClient.connected()){
    reconnectMQTT();
  } else {
    moistureValue = analogRead(moisturePin);
    float moistureF = (float) moistureValue;
    float h = dht.readHumidity();
    float t = dht.readTemperature(true);
    if (isnan(h) || isnan(t)){
      Serial.println("failed to read sensor");
      return;
    }
    
    
    root["deviceName"] = deviceName;

    data["temp"] = double_with_n_digits(t,2);
    data["humidity"] = double_with_n_digits(h,2);
    data["moisture"] = double_with_n_digits(moistureF,1);

    /*
     * {"message":[{"temp":75.20,"humidity":31.00,"moisture":2.0}],"deviceName":"dht11_bornman"}
     */
     
    root.printTo(Serial);
    Serial.println();

    char buffer[256];
    root.printTo(buffer, sizeof(buffer));
    //String jsonStr = String (buffer);
    
    mqttClient.publish("sensor", String (buffer));
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




