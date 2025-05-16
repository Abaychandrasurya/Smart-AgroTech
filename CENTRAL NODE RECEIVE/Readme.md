# 🌱 Smart Agriculture System

A multi-node IoT-based Smart Agriculture system using **Arduino Mega 2560**, **Arduino Nano**, and **nRF24L01** modules to monitor and manage soil, environmental, and fire safety conditions across agricultural zones.

---

## 🧭 Node Overview

### 🔹 1. Central Node – Arduino Mega 2560

**Purpose**: Collects data from local sensors and receives wireless data from remote nodes.

#### 🧪 Reads:
- 📡 **From Remote Node 1 & 2** via nRF24L01
- 🌡️ DHT22 – Temperature & Humidity
- 🌱 NPK Sensor – Soil Nutrients (N, P, K)
- ⚡ Soil pH Sensor
- 💡 Light Sensor
- Any other onboard analog/digital sensors

#### 🔌 nRF24L01 Wiring to Mega:

| **nRF24L01 Pin** | **Arduino Mega Pin** |
|------------------|----------------------|
| VCC              | 3.3V                 |
| GND              | GND                  |
| CE               | 49                   |
| CSN              | 53                   |
| SCK              | 52                   |
| MOSI             | 51                   |
| MISO             | 50                   |

---

### 🔹 2. Remote Node 1 – Arduino Nano + nRF24L01

**Purpose**: Monitors soil moisture levels at two different depths and transmits the data wirelessly to the central node.

#### 🧪 Sensors:
- 🌱 Soil Moisture Sensor – *Upper Level*
- 🌱 Soil Moisture Sensor – *Deeper Level*

#### 📤 Sends:
- Soil Moisture Data (2 readings) → **Central Node (Mega 2560)**

#### 🛠 Wiring:

| **Component**            | **Arduino Nano Pin** |
|--------------------------|----------------------|
| Soil Moisture (Upper)    | A0                   |
| Soil Moisture (Deeper)   | A1                   |
| nRF24L01 (VCC)           | 3.3V                 |
| nRF24L01 (CE, CSN, etc.) | Custom SPI pins      |

> ⚠️ Use a capacitor (10µF–100µF) between VCC and GND of nRF24L01 for stable operation.

---

### 🔹 3. Remote Node 2 – Arduino Nano + nRF24L01

**Purpose**: Monitors fire hazards and voltage levels, and activates a relay to trigger a fire extinguisher in emergencies.

#### 🧪 Sensors:
- 🔥 Flame Sensor – Detects fire
- ⚡ Voltage Monitoring Sensor – Analog voltage sensing
- ⚙️ Relay Module – Controls fire extinguisher or buzzer

#### 📤 Sends:
- Flame Detection (Boolean)
- Voltage Level (Analog)  
→ **Central Node (Mega 2560)**

#### 🛠 Wiring:

| **Component**            | **Arduino Nano Pin** |
|--------------------------|----------------------|
| Flame Sensor             | D2                   |
| Voltage Sensor (Analog)  | A0                   |
| Relay IN                 | D3                   |
| nRF24L01 (VCC)           | 3.3V                 |
| nRF24L01 (CE, CSN, etc.) | Custom SPI pins      |

> Relay is triggered automatically if flame is detected or abnormal voltage is observed.

---

## 📡 Communication Protocol

- **Wireless Module**: nRF24L01 (2.4 GHz SPI-based transceiver)
- **Topology**: Star network (all nodes communicate directly with the central node)
- **Protocol**: Custom payload format using `RF24` Arduino library

---

## 🔋 Power Supply Notes

- **Arduino Nano** can be powered via USB or 7–12V on VIN.
- Use a **3.3V LDO regulator** (e.g., AMS1117) for powering nRF24L01 modules reliably.
- Always place a capacitor across **VCC–GND** of nRF24L01 to prevent brownouts.

---

## 🔧 Required Libraries

Install via Arduino Library Manager:

- `RF24` by TMRh20 – nRF24L01 communication
- `DHT` & `Adafruit Sensor` libraries (for DHT22)
- Any analog sensor libraries if required

---

## 🔄 Data Transmission Format

Each remote node transmits data packets structured like:

**Remote Node 1:**
```json
{
  "node": 1,
  "soil_upper": 45,
  "soil_deep": 38
}

