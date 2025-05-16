# 🔥 Remote Node 2 – Flame Detection, Voltage Monitoring & Relay Control

This document describes **Remote Node 2** in a wireless monitoring and control system. This node integrates a **flame sensor**, a **voltage monitoring circuit**, and a **relay module** for automated control based on real-time sensor data. It communicates wirelessly using the **nRF24L01 transceiver**.

---

## 🧪 Components Used

| Component               | Quantity | Description                                                          |
|-------------------------|----------|----------------------------------------------------------------------|
| Arduino Nano            | 1        | Microcontroller to read sensors and control relay                    |
| Flame Sensor            | 1        | Digital sensor to detect flame or high-intensity infrared light     |
| Voltage Divider Circuit | 1        | Measures voltage by scaling it down (used with analog pin)          |
| Relay Module            | 1        | Used to control AC/DC devices remotely                              |
| nRF24L01 Module         | 1        | Wireless transceiver for node-to-node communication                 |
| Jumper Wires            | -        | For making necessary electrical connections                         |
| Power Supply            | 1        | External 5V supply recommended (with 3.3V regulator if needed)       |

---

## 🔌 Wiring Instructions

### Wiring Table

| Component         | Arduino Nano Pin |
|------------------|------------------|
| Flame Sensor OUT  | D2               |
| Voltage Sensor    | A0               |
| Relay IN          | D3               |
| nRF24L01 VCC      | 3.3V             |
| nRF24L01 GND      | GND              |
| nRF24L01 CE       | D9               |
| nRF24L01 CSN      | D10              |
| nRF24L01 SCK      | D13              |
| nRF24L01 MOSI     | D11              |
| nRF24L01 MISO     | D12              |

> ⚠️ **Note**: Always use a stable **3.3V** supply for the nRF24L01. If you're using an external 5V source, add a 3.3V voltage regulator (like AMS1117) for the radio module.

---

## 🔍 Functional Overview

This Remote Node performs the following functions:

1. **Flame Detection**  
   Detects fire or high-intensity infrared light via digital output from the flame sensor.  
   - HIGH = No flame  
   - LOW = Flame detected

2. **Voltage Monitoring**  
   Measures analog voltage using a voltage divider circuit connected to A0.  
   Ideal for monitoring battery or input voltage.

3. **Relay Control**  
   A relay module is used to control connected electrical devices (like an alarm or water pump).  
   Can be turned ON/OFF based on instructions from the base station or sensor input.

4. **Wireless Communication**  
   Sends real-time sensor data wirelessly to a central Base Station Node using the nRF24L01 module.

---

## 📡 Data Format (Example)

```json
{
  "flame": 0,
  "voltage": 3.72,
  "relayState": 1
}
