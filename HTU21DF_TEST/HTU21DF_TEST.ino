/*************************************************** 
  This is an example for the HTU21D-F Humidity & Temp Sensor

  Designed specifically to work with the HTU21D-F sensor from Adafruit
  ----> https://www.adafruit.com/products/1899

  These displays use I2C to communicate, 2 pins are required to  
  interface
 ****************************************************/

#include <Wire.h>
#include "Adafruit_HTU21DF.h"

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (A5 on UNO)
// Connect SDA to I2C data pin (A4 on UNO)

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
}


void loop() {
  float t = htu.readTemperature();
  float f = (t * 9/5) + 32;
  float h = htu.readHumidity();
  Serial.print("Temp: "); Serial.print(f); Serial.print("*F");
  Serial.print("\tHum: "); Serial.print(h); Serial.println("%");

  int batLevel = analogRead(A0);
  Serial.print("Analog reading =  ");
  Serial.println(batLevel);
  batLevel = map(batLevel, 576, 756, 0, 100);
  Serial.print("Battery level = ");
  Serial.print(batLevel);
  Serial.println("%");

  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
      
}
