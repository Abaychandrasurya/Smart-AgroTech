// =======================================================================
//                   SMART FARM - MOTOR CONTROL NODE CODE
// =======================================================================
//
// Board: ESP32 Dev Kit
//
// This code manages the main water pump and all associated safety sensors.
// It only acts on commands from the Main Controller and sends back a
// continuous stream of diagnostic data.
//
// =======================================================================

// -- LIBRARY INCLUDES --
#include <SPI.h>
#include <LoRa.h>

// -- PIN DEFINITIONS --
// Actuators
#define MOTOR_RELAY_PIN           26
#define EXTINGUISHER_RELAY_PIN    27

// Digital Sensors
#define FLAME_SENSOR_PIN          25
#define WATER_LEVEL_PIN           32 // Simple float switch

// Analog Sensors
#define VOLTAGE_SENSOR_PIN        34
#define CURRENT_SENSOR_PIN        35
#define TDS_SENSOR_PIN            36
#define PH_SENSOR_PIN             39
#define PRESSURE_SENSOR_PIN       4

// Indicator LEDs
#define LED_PIN_GREEN             15 // Status OK
#define LED_PIN_YELLOW            12 // Motor ON
#define LED_PIN_RED               13 // FAULT

// LoRa Module Pins
#define LORA_SS_PIN               5
#define LORA_RST_PIN              14
#define LORA_DIO0_PIN             2

// -- CONFIGURATION & CONSTANTS --
#define NODE_ID                   2   // Unique ID for this node
#define VOLTAGE_DIVIDER_FACTOR    11.0 // Factor for voltage sensor (e.g., for a 10:1 divider)
#define MIN_WATER_LEVEL           LOW  // For float switch (LOW = water present)
#define MAX_CURRENT_AMPS          8.0  // Max safe current for your pump

// -- GLOBAL VARIABLES --
// Status
bool isMotorOn = false;
bool isFault = false;

// Sensor Readings
float batteryVoltage = 0.0;
float motorCurrent = 0.0;
float waterTDS = 0.0;
float waterPH = 0.0;
float waterPressure = 0.0;
bool waterLevelOk = true;
bool flameDetected = false;

// Timers
unsigned long lastDataSend = 0;
const long dataSendInterval = 15000; // Send data every 15 seconds

// =======================================================================
//                                SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Motor Control Node - Initializing...");

  // Initialize Pins
  pinMode(MOTOR_RELAY_PIN, OUTPUT);
  pinMode(EXTINGUISHER_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_RELAY_PIN, LOW);      // Ensure motor is OFF on startup
  digitalWrite(EXTINGUISHER_RELAY_PIN, LOW);

  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(WATER_LEVEL_PIN, INPUT_PULLUP); // Use internal pullup for float switch

  // Startup LED sequence
  digitalWrite(LED_PIN_RED, HIGH); delay(500); digitalWrite(LED_PIN_RED, LOW);
  digitalWrite(LED_PIN_YELLOW, HIGH); delay(500); digitalWrite(LED_PIN_YELLOW, LOW);
  digitalWrite(LED_PIN_GREEN, HIGH); // Green ON indicates ready

  // Initialize LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    isFault = true;
    while (1);
  }
  Serial.println("LoRa Initialized.");
}

// =======================================================================
//                              MAIN LOOP
// =======================================================================
void loop() {
  // 1. Listen for commands from the Main Controller
  listenForLoRaCommands();

  // 2. Read all diagnostic and safety sensors
  readAllSensors();

  // 3. Check for any fault conditions that require an emergency stop
  checkForFaults();

  // 4. Periodically send a data packet back to the Main Controller
  if (millis() - lastDataSend > dataSendInterval) {
    sendDataPacket();
    lastDataSend = millis();
  }
  
  // 5. Update the status indicator lights
  updateIndicators();

  delay(200);
}

// =======================================================================
//                            HELPER FUNCTIONS
// =======================================================================

/**
 * @brief Reads all connected sensors and updates global variables.
 */
void readAllSensors() {
  // Note: These are raw readings. Calibration is required for accuracy.
  int rawVoltage = analogRead(VOLTAGE_SENSOR_PIN);
  batteryVoltage = (rawVoltage / 4095.0) * 3.3 * VOLTAGE_DIVIDER_FACTOR;

  int rawCurrent = analogRead(CURRENT_SENSOR_PIN);
  // Example ACS712 20A calibration: (rawADC - offset) * sensitivity
  motorCurrent = ((rawCurrent / 4095.0 * 3.3) - 2.5) / 0.100; // Needs calibration!

  waterTDS = analogRead(TDS_SENSOR_PIN); // Placeholder, requires calibration formula
  waterPH = analogRead(PH_SENSOR_PIN); // Placeholder, requires calibration formula
  waterPressure = analogRead(PRESSURE_SENSOR_PIN); // Placeholder, requires calibration
  
  waterLevelOk = (digitalRead(WATER_LEVEL_PIN) == MIN_WATER_LEVEL);
  flameDetected = (digitalRead(FLAME_SENSOR_PIN) == HIGH);
}

/**
 * @brief Checks for any critical faults. If found, triggers an emergency stop.
 */
void checkForFaults() {
  if (isFault) return; // Already in a fault state

  if (flameDetected) {
    Serial.println("CRITICAL: Flame Detected!");
    triggerEmergencyStop(true); // true = activate extinguisher
  } else if (isMotorOn && motorCurrent > MAX_CURRENT_AMPS) {
    Serial.println("CRITICAL: Motor Overcurrent!");
    triggerEmergencyStop(false);
  } else if (isMotorOn && !waterLevelOk) {
    Serial.println("CRITICAL: Water level dropped during operation!");
    triggerEmergencyStop(false);
  }
}

/**
 * @brief Listens for and parses commands received via LoRa.
 */
void listenForLoRaCommands() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String command = "";
    while (LoRa.available()) {
      command += (char)LoRa.read();
    }
    Serial.print("Received command: ");
    Serial.println(command);

    if (command == "MOTOR_START" && !isFault) {
      // Pre-start safety check
      readAllSensors(); // Get latest data before starting
      if(waterLevelOk) {
        digitalWrite(MOTOR_RELAY_PIN, HIGH);
        isMotorOn = true;
      } else {
        Serial.println("Motor start aborted: Low water level!");
        sendDataPacket(); // Send an immediate update
      }
    } else if (command == "MOTOR_STOP" || command == "ALL_STOP") {
      digitalWrite(MOTOR_RELAY_PIN, LOW);
      isMotorOn = false;
    }
  }
}

/**
 * @brief Packages and sends all sensor data to the Main Controller.
 */
void sendDataPacket() {
  // Format: "NodeID,MotorStatus,Voltage,Current,WaterLevel,TDS,pH,Flame"
  String dataPacket = String(NODE_ID) + ",";
  dataPacket += String(isMotorOn) + ",";
  dataPacket += String(batteryVoltage) + ",";
  dataPacket += String(motorCurrent) + ",";
  dataPacket += String(waterLevelOk) + ",";
  dataPacket += String(waterTDS) + ",";
  dataPacket += String(waterPH) + ",";
  dataPacket += String(flameDetected);

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();
  
  Serial.print("Sent data packet: ");
  Serial.println(dataPacket);
}

/**
 * @brief Shuts down the motor and handles emergency actions.
 * @param activateExtinguisher Set to true if a fire is detected.
 */
void triggerEmergencyStop(bool activateExtinguisher) {
  digitalWrite(MOTOR_RELAY_PIN, LOW);
  isMotorOn = false;
  isFault = true;

  if (activateExtinguisher) {
    digitalWrite(EXTINGUISHER_RELAY_PIN, HIGH);
  }
  sendDataPacket(); // Send a final packet with the fault status
}

/**
 * @brief Updates the onboard LEDs to show current status.
 */
void updateIndicators() {
  if (isFault) {
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
    digitalWrite(LED_PIN_RED, HIGH); // Solid RED for fault
  } else {
    digitalWrite(LED_PIN_RED, LOW);
    digitalWrite(LED_PIN_GREEN, HIGH);
    digitalWrite(LED_PIN_YELLOW, isMotorOn); // Yellow light follows motor status
  }
}
