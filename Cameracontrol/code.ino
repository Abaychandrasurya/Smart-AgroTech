// =======================================================================
//                    SMART FARM - CAMERA NODE CODE
// =======================================================================
//
// Board: AI Thinker ESP32-CAM
//
// This node operates in a low-power deep sleep mode. It wakes up when the
// PIR sensor detects motion, sends a single alert packet via LoRa,
// blinks its onboard LED, and goes back to sleep.
//
// =======================================================================

// -- LIBRARY INCLUDES --
#include "esp_camera.h"
#include <SPI.h>
#include <LoRa.h>

// -- CONFIGURATION --
#define CAMERA_NODE_ID        1 // Unique ID for this camera node

// -- PIN DEFINITIONS --
// PIR Motion Sensor
#define PIR_PIN               16

// LoRa Module Pins
#define LORA_SS_PIN           15
#define LORA_RST_PIN          2
#define LORA_DIO0_PIN         4
// Standard SPI pins for ESP32-CAM
#define LORA_SCK_PIN          14
#define LORA_MISO_PIN         12
#define LORA_MOSI_PIN         13

// Onboard LED
#define ONBOARD_LED_PIN       33

// -- DEEP SLEEP CONFIGURATION --
// RTC_DATA_ATTR allows this variable to survive a deep sleep cycle
RTC_DATA_ATTR int bootCount = 0;
const int sleepDurationSeconds = 10; // Sleep for 10s after sending alert to avoid spam

// =======================================================================
//                                SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("Camera Node Booting...");

  // Increment boot count and print it
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  // If bootCount is 1, it's the first time power-on.
  // If > 1, it means we woke up from sleep.
  if (bootCount > 1) {
    // We woke up from motion, so send the alert.
    sendMotionAlert();
  }

  // Now, set up the deep sleep wake-up trigger
  goToSleep();
}

// =======================================================================
//                              MAIN LOOP
// =======================================================================
void loop() {
  // The loop is intentionally empty. All logic is handled in setup()
  // and the device goes to sleep immediately after.
}

// =======================================================================
//                            HELPER FUNCTIONS
// =======================================================================

/**
 * @brief Initializes LoRa, sends a motion alert packet, and blinks the LED.
 */
void sendMotionAlert() {
  Serial.println("Motion detected! Sending LoRa alert...");

  // Configure the SPI bus for LoRa
  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  
  // Initialize LoRa
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    return; // Can't send alert if LoRa fails
  }

  // Format the data packet: "C[NodeID],ALERT"
  // Example: "C1,ALERT"
  String dataPacket = "C" + String(CAMERA_NODE_ID) + ",ALERT";

  // Send the packet
  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  Serial.print("Sent data packet: ");
  Serial.println(dataPacket);
  
  // Blink the onboard LED to confirm alert was sent
  pinMode(ONBOARD_LED_PIN, OUTPUT);
  for (int i=0; i<5; i++) {
    digitalWrite(ONBOARD_LED_PIN, LOW); // LED is active LOW
    delay(100);
    digitalWrite(ONBOARD_LED_PIN, HIGH);
    delay(100);
  }
}

/**
 * @brief Configures the wake-up source and puts the ESP32 to sleep.
 */
void goToSleep() {
  Serial.println("Configuring wake-up on motion detect pin...");
  // Configure PIR pin as the external wake-up source.
  // ESP32 can wake up when a specific pin is HIGH.
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 1); // 1 = High level trigger

  Serial.println("Going to sleep now.");
  Serial.flush(); // Ensure all serial messages are sent before sleeping
  esp_deep_sleep_start();
}
