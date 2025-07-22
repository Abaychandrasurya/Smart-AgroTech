// =======================================================================
//                  SMART FARM - SOIL MOISTURE NODE CODE
// =======================================================================
//
// Board: Arduino Nano
//
// This node reads three soil moisture sensors and a soil pH sensor.
// It averages the moisture readings, packages the data, and sends it
// to the Main Controller via LoRa before entering a sleep cycle.
//
// =======================================================================

// -- LIBRARY INCLUDES --
#include <SPI.h>
#include <LoRa.h>

// -- CONFIGURATION --
#define SOIL_NODE_ID           1 // IMPORTANT: Set a unique ID for each soil node

// -- PIN DEFINITIONS --
// Sensors
#define MOISTURE_PIN_UPPER     A0
#define MOISTURE_PIN_MID       A1
#define MOISTURE_PIN_DEEP      A2
#define PH_SENSOR_PIN          A3

// Indicator LEDs
#define LED_PIN_GREEN          3 // Power / OK
#define LED_PIN_BLUE           4 // Data Transmit

// LoRa Module Pins
#define LORA_SS_PIN            10
#define LORA_RST_PIN           9
#define LORA_DIO0_PIN          2

// -- SENSOR CALIBRATION (CRITICAL!) --
// You MUST calibrate your sensors for accurate readings.
// 1. Read the sensor value when it is completely dry ("in the air").
// 2. Read the sensor value when it is fully submerged in water.
const int SENSOR_AIR_VALUE = 580;   // Replace with your dry value
const int SENSOR_WATER_VALUE = 290; // Replace with your wet value

// -- GLOBAL VARIABLES --
float moisture_upper_percent = 0.0;
float moisture_mid_percent = 0.0;
float moisture_deep_percent = 0.0;
float moisture_avg_percent = 0.0;
float soil_ph = 0.0;

// Sleep interval: 15 minutes
const long sleepInterval = 900000; 

// =======================================================================
//                                SETUP
// =======================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Soil Moisture Node - Initializing...");
  Serial.print("Node ID: ");
  Serial.println(SOIL_NODE_ID);
  
  // Initialize Pins
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_BLUE, OUTPUT);
  
  // Startup LED blink
  digitalWrite(LED_PIN_GREEN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN_GREEN, LOW);

  // Initialize LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) { // Must match Main Controller frequency
    Serial.println("Starting LoRa failed!");
    while (1); // Halt on failure
  }
  Serial.println("LoRa Initialized.");
}

// =======================================================================
//                              MAIN LOOP
// =======================================================================
void loop() {
  digitalWrite(LED_PIN_GREEN, HIGH); // Green ON while working

  Serial.println("\n--- Waking up, taking readings ---");

  // 1. Read data from all connected sensors
  readAllSensors();

  // 2. Send the collected data in a single packet
  sendDataPacket();

  // 3. Go to sleep to save power
  digitalWrite(LED_PIN_GREEN, LOW);
  Serial.print("--- Readings sent. Sleeping for ");
  Serial.print(sleepInterval / 60000);
  Serial.println(" minutes. ---");
  
  delay(sleepInterval); // Note: For true low power, use a deep sleep library like LowPower.h
}

// =======================================================================
//                            HELPER FUNCTIONS
// =======================================================================

/**
 * @brief Reads all sensors and converts raw ADC values to percentages.
 */
void readAllSensors() {
  // Read raw ADC values
  int raw_upper = analogRead(MOISTURE_PIN_UPPER);
  int raw_mid = analogRead(MOISTURE_PIN_MID);
  int raw_deep = analogRead(MOISTURE_PIN_DEEP);
  int raw_ph = analogRead(PH_SENSOR_PIN);

  // Convert moisture readings to a percentage using the map() function
  moisture_upper_percent = map(raw_upper, SENSOR_AIR_VALUE, SENSOR_WATER_VALUE, 0, 100);
  moisture_mid_percent = map(raw_mid, SENSOR_AIR_VALUE, SENSOR_WATER_VALUE, 0, 100);
  moisture_deep_percent = map(raw_deep, SENSOR_AIR_VALUE, SENSOR_WATER_VALUE, 0, 100);

  // Constrain values to the 0-100 range in case of rogue readings
  moisture_upper_percent = constrain(moisture_upper_percent, 0, 100);
  moisture_mid_percent = constrain(moisture_mid_percent, 0, 100);
  moisture_deep_percent = constrain(moisture_deep_percent, 0, 100);
  
  // Calculate average moisture
  moisture_avg_percent = (moisture_upper_percent + moisture_mid_percent + moisture_deep_percent) / 3.0;

  // Convert pH reading. This is a placeholder and requires a specific
  // formula based on your pH sensor's amplifier board and calibration.
  // Example: soil_ph = map(raw_ph, 0, 1023, 0, 14);
  soil_ph = 7.0 + ((512.0 - raw_ph) / 100.0); // Example, needs real calibration!

  // Print readings for debugging
  Serial.print("Moisture: ");
  Serial.print(moisture_avg_percent);
  Serial.print("%, pH: ");
  Serial.println(soil_ph);
}

/**
 * @brief Packages and sends sensor data to the Main Controller via LoRa.
 */
void sendDataPacket() {
  digitalWrite(LED_PIN_BLUE, HIGH); // Blue ON indicates transmitting

  // Format: "S[NodeID],AvgMoisture,pH,Upper,Mid,Deep"
  // Example: "S1,55.3,6.8,60,55,51"
  String dataPacket = "S" + String(SOIL_NODE_ID) + ",";
  dataPacket += String(moisture_avg_percent, 1) + ","; // Avg moisture with 1 decimal
  dataPacket += String(soil_ph, 1) + ",";             // pH with 1 decimal
  dataPacket += String((int)moisture_upper_percent) + ",";
  dataPacket += String((int)moisture_mid_percent) + ",";
  dataPacket += String((int)moisture_deep_percent);

  // Send the packet
  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  Serial.print("Sent data packet: ");
  Serial.println(dataPacket);
  
  digitalWrite(LED_PIN_BLUE, LOW);
}
