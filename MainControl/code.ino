// =======================================================================
//                    SMART FARM - MAIN CONTROLLER CODE
// =======================================================================
//
// Board: Arduino Mega 2560
//
// This code acts as the central hub (master node) for the smart farm.
// It reads local environmental sensors, receives data from remote nodes
// via LoRa, makes decisions, and uploads all data to Blynk via GSM.
//
// =======================================================================

// -- LIBRARY INCLUDES --
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <BH1750.h>
#include <SoftwareSerial.h>

// -- PIN DEFINITIONS --
// Local Sensors
#define DHT_PIN                 22
#define MQ135_PIN               A0
#define RAIN_SENSOR_PIN         A1
// Anemometer pin is an interrupt, defined below

// Indicator LEDs
#define LED_PIN_GREEN           24
#define LED_PIN_BLUE            26
#define LED_PIN_RED             28

// LoRa Module Pins
#define LORA_SS_PIN             53
#define LORA_RST_PIN            9
#define LORA_DIO0_PIN           3

// GSM Module Pins (SoftwareSerial)
#define GSM_RX_PIN              10
#define GSM_TX_PIN              11

// -- SENSOR & MODULE SETUP --
#define DHT_TYPE                DHT22
DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter(0x23); // I2C address 0x23

SoftwareSerial gsmSerial(GSM_RX_PIN, GSM_TX_PIN);

// -- BLYNK & GSM CONFIGURATION --
String BLYNK_AUTH_TOKEN = "YOUR_BLYNK_AUTH_TOKEN";
String BLYNK_APN = "www"; // Your network provider's APN (e.g., "airtelgprs.com")
String BLYNK_SERVER = "blynk-cloud.com";

// -- DATA STRUCTURES FOR REMOTE NODES --
// A structure to hold data from a single soil node
struct SoilNodeData {
  int nodeId;
  float moisture_avg;
  float soil_ph;
  float valve_pressure;
  bool valve_open;
};

// A structure to hold data from the motor node
struct MotorNodeData {
  bool motor_on;
  float voltage;
  float current;
  float water_level;
  bool fire_alert;
};

// Create instances of the data structures
SoilNodeData soilNode1 = {1, 0.0, 0.0, 0.0, false};
MotorNodeData motorNode = {false, 0.0, 0.0, 0.0, false};

// -- GLOBAL VARIABLES --
float ambient_temp = 0.0;
float ambient_humidity = 0.0;
float air_quality = 0.0;
float light_intensity = 0.0;
int rain_value = 0;
bool isRaining = false;

// Timers for non-blocking operations
unsigned long lastBlynkUpload = 0;
const long blynkUploadInterval = 300000; // 5 minutes

// =======================================================================
//                                SETUP
// =======================================================================
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Smart Farm Main Controller - Initializing...");

  // Initialize Indicator LEDs
  pinMode(LED_PIN_GREEN, OUTPUT);
  pinMode(LED_PIN_BLUE, OUTPUT);
  pinMode(LED_PIN_RED, OUTPUT);
  digitalWrite(LED_PIN_GREEN, HIGH); // Green ON indicates power
  digitalWrite(LED_PIN_BLUE, LOW);
  digitalWrite(LED_PIN_RED, LOW);

  // Initialize Local Sensors
  dht.begin();
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 Initialized.");
  } else {
    Serial.println("Error initializing BH1750");
  }

  // Initialize LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) { // 433 MHz for India
    Serial.println("Starting LoRa failed!");
    digitalWrite(LED_PIN_RED, HIGH);
    while (1);
  }
  Serial.println("LoRa Initialized.");
  
  // Initialize GSM
  gsmSerial.begin(9600);
  Serial.println("Initializing GSM... (May take a minute)");
  // Basic GSM check - more robust checks would use AT command libraries
  delay(5000);
  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CPIN?");
  delay(1000);
  gsmSerial.println("AT+CREG?");
  delay(1000);
  Serial.println("GSM initialization commands sent.");
  digitalWrite(LED_PIN_BLUE, HIGH); // Blue ON indicates GSM is likely ready
}

// =======================================================================
//                              MAIN LOOP
// =======================================================================
void loop() {
  // 1. Read all sensors connected directly to the Mega
  readLocalSensors();

  // 2. Check for any incoming data packets from remote nodes
  listenForLoRaData();

  // 3. Apply the core logic (e.g., check rain status)
  applySmartLogic();

  // 4. Periodically upload all data to Blynk Cloud via GSM
  if (millis() - lastBlynkUpload > blynkUploadInterval) {
    uploadDataToBlynk();
    lastBlynkUpload = millis();
  }

  // Optional: Add logic to listen for commands from Blynk
  
  delay(1000); // Small delay to prevent busy-looping
}

// =======================================================================
//                            HELPER FUNCTIONS
// =======================================================================

/**
 * @brief Reads all sensors physically connected to the Arduino Mega.
 */
void readLocalSensors() {
  ambient_temp = dht.readTemperature();
  ambient_humidity = dht.readHumidity();
  air_quality = analogRead(MQ135_PIN); // This is a raw value, needs calibration
  light_intensity = lightMeter.readLightLevel();
  rain_value = analogRead(RAIN_SENSOR_PIN);

  // Simple check for NaN (Not a Number) from DHT
  if (isnan(ambient_temp) || isnan(ambient_humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temp: "); Serial.print(ambient_temp);
  Serial.print(" *C, Humidity: "); Serial.print(ambient_humidity);
  Serial.print("%, Light: "); Serial.print(light_intensity);
  Serial.println(" lx");
}

/**
 * @brief Listens for incoming LoRa packets and parses them.
 */
void listenForLoRaData() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    digitalWrite(LED_PIN_BLUE, !digitalRead(LED_PIN_BLUE)); // Blink blue on receive
    Serial.print("Received packet with RSSI ");
    Serial.println(LoRa.packetRssi());

    // Simple protocol: NodeID,data1,data2,data3...
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    // Example parsing for soil node (ID 1)
    // Format: "1,45.5,6.8,15.2,1" (ID,moisture,ph,pressure,valve_status)
    if (receivedData.startsWith("1,")) {
      // Parse the data for soil node 1
      // Note: This is a basic parser. A more robust solution would use libraries
      // like ArduinoJson or strtok.
      sscanf(receivedData.c_str(), "1,%f,%f,%f,%d", 
             &soilNode1.moisture_avg, 
             &soilNode1.soil_ph, 
             &soilNode1.valve_pressure, 
             &soilNode1.valve_open);
      Serial.println("Parsed data for Soil Node 1.");
    }
    // Add 'else if' blocks for other node IDs (e.g., motor node "2,...")
  }
}

/**
 * @brief Central logic hub for making farming decisions.
 */
void applySmartLogic() {
  // Rule 1: Check for rain. If it's raining, stop all irrigation.
  // A lower analog value from the sensor means more rain.
  if (rain_value < 500) { 
    isRaining = true;
    // Send a "STOP" command to all valve/motor nodes via LoRa
    LoRa.beginPacket();
    LoRa.print("ALL_STOP");
    LoRa.endPacket();
    Serial.println("Rain detected! Sending STOP command.");
  } else {
    isRaining = false;
  }

  // Rule 2: If soil moisture is low and it's not raining, request motor start
  if (soilNode1.moisture_avg < 30.0 && !isRaining) {
    // Command the motor node to start the pump
    LoRa.beginPacket();
    LoRa.print("MOTOR_START"); // Assuming motor node listens for this
    LoRa.endPacket();

    // Command the specific valve to open
    LoRa.beginPacket();
    LoRa.print("VALVE_1_OPEN"); // Assuming valve node listens for this
    LoRa.endPacket();
    Serial.println("Low moisture. Sending MOTOR_START and VALVE_1_OPEN.");
  }
}


/**
 * @brief Connects to GPRS and sends all data to Blynk virtual pins.
 */
void uploadDataToBlynk() {
  Serial.println("--- Uploading data to Blynk ---");
  digitalWrite(LED_PIN_BLUE, HIGH);

  // Establish TCP connection to Blynk server
  gsmSerial.println("AT+CIPSTART=\"TCP\",\"" + BLYNK_SERVER + "\",80");
  delay(2000); // Wait for connection

  // Construct the GET request string. Update virtual pins as needed.
  String dataUrl = "GET /" + BLYNK_AUTH_TOKEN + "/update";
  dataUrl += "/V0?value=" + String(ambient_temp);
  dataUrl += "&V1?value=" + String(ambient_humidity);
  dataUrl += "&V2?value=" + String(light_intensity);
  dataUrl += "&V3?value=" + String(air_quality);
  dataUrl += "&V4?value=" + String(isRaining);
  dataUrl += "&V10?value=" + String(soilNode1.moisture_avg);
  dataUrl += "&V11?value=" + String(soilNode1.soil_ph);
  // Add more virtual pins for all other data points

  // Send the request
  gsmSerial.println("AT+CIPSEND");
  delay(1000);
  gsmSerial.println(dataUrl + " HTTP/1.1");
  delay(500);
  gsmSerial.println("Host: " + BLYNK_SERVER);
  delay(500);
  gsmSerial.println("Connection: close");
  delay(500);
  gsmSerial.println();
  delay(1000);
  gsmSerial.write(26); // Send Ctrl+Z to end the command
  delay(2000);

  // Close the TCP connection
  gsmSerial.println("AT+CIPCLOSE");
  
  Serial.println("--- Upload complete ---");
  digitalWrite(LED_PIN_BLUE, LOW);
}
