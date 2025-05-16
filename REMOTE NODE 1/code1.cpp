#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "NODE1";

struct SoilData {
  int soilTop;
  int soilDeep;
};

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  SoilData data;
  data.soilTop = analogRead(A0);
  data.soilDeep = analogRead(A1);

  radio.write(&data, sizeof(data));
  delay(2000); // Adjust as needed
}
