#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(49, 53); // CE, CSN

const byte address1[6] = "NODE1";
const byte address2[6] = "NODE2";

struct SoilData {
  int soilTop;
  int soilDeep;
};

struct FireVoltData {
  bool flameDetected;
  float voltage;
};

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, address1);
  radio.openReadingPipe(2, address2);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    byte pipe;
    radio.available(&pipe);

    if (pipe == 1) {
      SoilData data;
      radio.read(&data, sizeof(data));
      Serial.print("Soil Top: "); Serial.print(data.soilTop);
      Serial.print(" Soil Deep: "); Serial.println(data.soilDeep);
      // Send to Blynk here
    }
    else if (pipe == 2) {
      FireVoltData data;
      radio.read(&data, sizeof(data));
      Serial.print("Flame: "); Serial.print(data.flameDetected);
      Serial.print(" Voltage: "); Serial.println(data.voltage);
      // Send to Blynk here
    }
  }
}
