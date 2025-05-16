#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "NODE2";

struct FireVoltData {
  bool flameDetected;
  float voltage;
};

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);  // Flame sensor
  pinMode(3, OUTPUT); // Relay
  digitalWrite(3, LOW);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  FireVoltData data;
  data.flameDetected = digitalRead(2) == LOW; // Adjust if logic is reversed
  data.voltage = analogRead(A0) * (5.0 / 1023.0) * (40); // replace with actual ratio

  if (data.flameDetected) {
    digitalWrite(3, HIGH); // Activate extinguisher
  } else {
    digitalWrite(3, LOW);
  }

  radio.write(&data, sizeof(data));
  delay(2000);
}
