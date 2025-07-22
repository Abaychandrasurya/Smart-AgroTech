// =======================================================================
//                   SMART FARM - VALVE CONTROL NODE CODE
// =======================================================================
//
// Board: NodeMCU 1.0 (ESP8266)
//
// This node controls a single solenoid valve and monitors water pressure.
// It receives commands from the Main Controller to open or close the
// valve and sends back status updates and fault alerts.
//
// =======================================================================

// -- LIBRARY INCLUDES --
#include <SPI.h>
#include <LoRa.h>

// -- CONFIGURATION --
#define VALVE_NODE_ID           1  // IMPORTANT: Set a unique ID for each valve node (1, 2, 3...)

// -- PIN DEFINITIONS --
// Actuators
#define VALVE_RELAY_PIN         D1 // GPIO 5

// Analog Sensors
#define PRESSURE_SENSOR_PIN     A0

// Indicator LEDs
#define LED_PIN_GREEN           D2 // GPIO 4
#define LED_PIN_BLUE            RX // GPIO 3
#define LED_PIN_YELLOW          TX // GPIO 1

// LoRa Module Pins
#define LORA_SS_PIN             D8 // GPIO 15
#define LORA_RST_PIN            D0 // GPIO 16
#define LORA_DIO0_PIN           D3 // GPIO 0

// -- GLOBAL VARIABLES --
bool isValveOpen = false;
float waterPressure = 0.0;
bool fault_NoPressure = false;

unsigned long lastPressureCheck = 0;
const long pressureCheckInterval = 5000; // Check for pressure every 5 seconds when valve is open

// =======================================================================
//                                SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Valve Control Node - Initializing...");
  Serial.print("Node ID: ");
  Serial.println(VALVE_NODE_ID);

  // Initialize Pins
  pinMode(VALVE_RELAY_PIN, OUTPUT);
  digitalWrite(VALVE_RELAY_PIN, LOW); // Ensure valve is CLOSED on startup

  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_BLUE, OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);

  // Startup LED sequence
  digitalWrite(LED_PIN_GREEN, HIGH);
  digitalWrite(LED_PIN_BLUE, HIGH);
  digitalWrite(LED_PIN_YELLOW, HIGH);
  delay(1000);
  digitalWrite(LED_PIN_GREEN, HIGH); // Green ON indicates power OK
  digitalWrite(LED_PIN_BLUE, LOW);
  digitalWrite(LED_PIN_YELLOW, LOW);

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
  // 1. Listen for commands from the Main Controller
  listenForLoRaCommands();

  // 2. Periodically check pressure if the valve is open
  if (isValveOpen && (millis() - lastPressureCheck > pressureCheckInterval)) {
    checkPressureAndFaults();
    lastPressureCheck = millis();
  }
  
  // 3. Update status LEDs
  updateIndicators();

  delay(100);
}

// =======================================================================
//                            HELPER FUNCTIONS
// =======================================================================

/**
 * @brief Listens for and parses commands targeted at this node.
 */
void listenForLoRaCommands() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    digitalWrite(LED_PIN_BLUE, HIGH); // Blink blue on receive
    
    String command = "";
    while (LoRa.available()) {
      command += (char)LoRa.read();
    }
    Serial.print("Received command: ");
    Serial.println(command);

    // Command format is "VALVE_[ID]_[ACTION]" e.g., "VALVE_1_OPEN"
    String openCommand = "VALVE_" + String(VALVE_NODE_ID) + "_OPEN";
    String closeCommand = "VALVE_" + String(VALVE_NODE_ID) + "_CLOSE";
    
    if (command == openCommand) {
      Serial.println("Command accepted: Opening valve.");
      digitalWrite(VALVE_RELAY_PIN, HIGH);
      isValveOpen = true;
      fault_NoPressure = false; // Reset fault status on new command
      sendDataPacket();
    } else if (command == closeCommand || command == "ALL_STOP") {
      Serial.println("Command accepted: Closing valve.");
      digitalWrite(VALVE_RELAY_PIN, LOW);
      isValveOpen = false;
      sendDataPacket();
    }
    delay(100);
    digitalWrite(LED_PIN_BLUE, LOW);
  }
}

/**
 * @brief Reads the pressure sensor and checks for a no-pressure fault.
 */
void checkPressureAndFaults() {
  // Read sensor and convert to a pseudo-pressure value
  // NOTE: This requires calibration with your specific sensor.
  int rawValue = analogRead(PRESSURE_SENSOR_PIN);
  waterPressure = rawValue / 1023.0 * 100.0; // Example conversion to a percentage

  Serial.print("Pressure check: ");
  Serial.println(waterPressure);

  // If the valve is supposed to be open but there's no pressure, it's a fault.
  if (isValveOpen && waterPressure < 5.0) { // Using 5.0 as a threshold for "no pressure"
    fault_NoPressure = true;
    Serial.println("FAULT: Valve is open but no water pressure detected!");
    sendDataPacket(); // Immediately report the fault
  } else {
    fault_NoPressure = false;
  }
}

/**
 * @brief Packages and sends its status to the Main Controller.
 */
void sendDataPacket() {
  // Format: "V[NodeID],ValveStatus,Pressure,FaultStatus"
  // Example: "V1,1,65.2,0" (Valve 1, Open, 65.2 pressure, No Fault)
  // Example: "V1,1,2.1,1"  (Valve 1, Open, 2.1 pressure, FAULT)
  
  String dataPacket = "V" + String(VALVE_NODE_ID) + ",";
  dataPacket += String(isValveOpen) + ",";
  dataPacket += String(waterPressure, 1) + ","; // Send pressure with 1 decimal place
  dataPacket += String(fault_NoPressure);

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();
  
  Serial.print("Sent data packet: ");
  Serial.println(dataPacket);
}

/**
 * @brief Updates the onboard LEDs to show current status.
 */
void updateIndicators() {
  // Green LED is always on for power
  digitalWrite(LED_PIN_GREEN, HIGH);
  
  // Yellow LED shows if the valve is open
  digitalWrite(LED_PIN_YELLOW, isValveOpen);

  // Red (simulated by blinking blue) indicates a fault
  if (fault_NoPressure) {
    digitalWrite(LED_PIN_BLUE, !digitalRead(LED_PIN_BLUE)); // Blink blue rapidly for fault
    delay(200);
  }
}
