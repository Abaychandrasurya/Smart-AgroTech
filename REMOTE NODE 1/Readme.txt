# 🌱 Remote Node 1 – Soil Moisture Monitoring System

This document describes the setup and configuration of **Remote Node 1**, a part of a wireless smart agriculture monitoring system. This node is responsible for monitoring **soil moisture levels** using two sensors and transmitting the data to a central unit using the **nRF24L01 wireless transceiver module**.

---

## 📦 Components Used

| Component           | Quantity | Description                                                                 |
|---------------------|----------|-----------------------------------------------------------------------------|
| Arduino Nano        | 1        | Microcontroller board for processing sensor data                           |
| nRF24L01 Module     | 1        | Wireless transceiver module for data transmission                          |
| Soil Moisture Sensor| 2        | Analog sensors to measure soil moisture level                              |
| Jumper Wires        | -        | For electrical connections between components                              |
| Breadboard (optional)| 1       | For prototyping without soldering (optional)                               |

---

## ⚙️ Circuit Diagram & Wiring Instructions

Below is the wiring connection between the components:

### 🔌 Wiring Table

| Component            | Arduino Nano Pin |
|----------------------|------------------|
| Soil Moisture Sensor 1 | A0               |
| Soil Moisture Sensor 2 | A1               |
| nRF24L01 VCC         | 3.3V             |
| nRF24L01 GND         | GND              |
| nRF24L01 CE          | D9               |
| nRF24L01 CSN         | D10              |
| nRF24L01 SCK         | D13              |
| nRF24L01 MOSI        | D11              |
| nRF24L01 MISO        | D12              |

> ⚠️ **Note:** The nRF24L01 operates at **3.3V**, not 5V. Supplying 5V may damage the module. Ensure your Arduino Nano can provide stable 3.3V, or use an external voltage regulator (like AMS1117-3.3V) if needed.

---

## 📡 System Overview

This Remote Node performs the following tasks:

1. **Reads analog values** from two soil moisture sensors.
2. **Processes and formats** the data.
3. **Transmits** the sensor data wirelessly to the **Base Station Node** using the nRF24L01 transceiver.

---

## 🧠 Arduino Sketch Overview

The Arduino code (not included here) will:
- Initialize the nRF24L01 module using the RF24 library.
- Continuously read analog values from A0 and A1.
- Format the data into a structure.
- Send it over the air using the RF24 `write()` function.

> Sample data format:
```json
{
  "sensor1": 645,
  "sensor2": 712
}
