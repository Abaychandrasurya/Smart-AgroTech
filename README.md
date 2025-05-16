# Smart-Agricultural-Management-System
To develop a smart farming system that monitors various environmental factors and automates tasks like irrigation and fertilization to optimize crop yields and resource usage.

# IoT Project

## Overview
A complete IoT and AI-based solution for modern farming, integrating real-time environmental monitoring, automated irrigation, and disease detection to improve productivity and sustainability.

## 🚀 Features

- 🌧️ Automatic irrigation based on real-time soil moisture
- 🌡️ Monitors environmental factors: temperature, humidity, light, wind
- 🧪 Analyzes soil health: NPK, pH, moisture
- 💧 Water quality and level monitoring
- 💨 Air quality monitoring for sensitive crops
- 🤖 Live camera feed for security
- 🚷 Motion Based Protection and Alert System
- 📲 GSM-based alerts via SMS / HTTP
- 🌐 Cloud dashboard for live monitoring & control
- 🔌 Automated control of pumps, lights via relays
- 🔋 Solar + battery-powered operation for field deployment

---

## 🧰 Components & Uses

| **Component**                  | **Use**                                                                 |
|-------------------------------|--------------------------------------------------------------------------|
| **Arduino Mega 2560**         | Main microcontroller to connect multiple sensors                        |
| **Arduino Nano**              | microcontroller to monitor remote sensors                               |
| **NRF24L01+PA+LNA**           | data transmission between microcontrollers                              |
| **ESP32-CAM**                 | Captures plant images for disease/pest detection                        |
| **Soil Moisture Sensor**      | Automates irrigation based on moisture level                            |
| **DHT22 / SHT31**             | Measures temperature and humidity                                       |
| **NPK Sensor**                | Detects nitrogen, phosphorus, potassium in soil                         |
| **Anemometer (Wind Sensor)**  | Monitors wind speed for crop protection                                 |
| **MQ135 Air Quality Sensor**  | Detects air pollutants for sensitive plant areas                        |
| **TDS Sensor**                | Checks Total Dissolved Solids in water                                  |
| **Turbidity Sensor**          | Detects suspended particles in irrigation water                         |
| **Water Level Sensor**        | Monitors reservoir or tank levels                                       |
| **DS18B20 Temperature Sensor**| Measures soil or water temperature                                      |
| **Water Pressure Sensor**     | Detects pipeline pressure for leakage detection                         |
| **Water pH Sensor**           | Measures acidity/alkalinity of irrigation water                         |
| **Soil pH Sensor**            | Monitors soil health by measuring pH                                    |
| **LDR / BH1750 Light Sensor** | Controls lighting/shading systems                                       |
| **Rain Sensor**               | Detects rainfall to pause irrigation                                    |
| **Flame Sensor**              | Detects fire in the starters of motor                                   |
| **Voltage Monitoring**        | Monitors the voltage at the input of mains at starter                   |
| **Motion Sensor**             | Detects movement in the Field and provides Alert                        |
| **SIM800L / SIM900A Module**  | Sends data/alerts via SMS or HTTP over GSM                              |
| **Relay Module**              | Controls pumps, fans, lights                                            |
| **Water Pump**                | Irrigates the crops automatically                                       |
| **Solenoid Valve**            | Controls water flow electronically                                      |
| **Jumper Wires**              | Wiring for sensors and modules                                          |
| **Breadboard / PCB**          | Circuit prototyping and development                                     |
| **Power Supply Module**       | Provides regulated power to components                                  |
| **Battery / Li-ion Cell**     | Backup power for outdoor deployment                                     |
| **Solar Panel (Optional)**    | Power system in remote, off-grid locations                              |
| **Voltage Regulator (LM7805)**| Maintains safe voltage to sensors                                       |
| **ThingSpeak / Firebase**     | IoT cloud platform for real-time data visualization                     |

## 🧠 Software Stack

| **Software/Tool**       | **Purpose**                                        |
|-------------------------|----------------------------------------------------|
| **Arduino IDE**         | Programming Arduino and ESP32 boards              |
| **Node.js / Express.js**| Backend server for data management and API        |
| **MongoDB / MySQL**     | Stores sensor and image data                      |
| **Firebase / Vercel**   | Hosts the dashboard frontend                      |
| **HTML / CSS / JS**     | Dashboard UI                                      |
| **Blynk / ThingSpeak**  | Alternate IoT platforms for visualization         |


### Monitoring
- Collect sensor data and visualize it on a dashboard.
- Set up alerts for critical events (e.g., low soil moisture, extreme temperatures).
- Monitor the health and performance of the IoT devices.

### Data Management
- Use a time-series database (e.g., InfluxDB) to store sensor data.
- Implement data processing and analysis to gain insights.

### Cloud Platform
- Host the web application and database.
- Use cloud services for data storage, processing, and analysis.
- Manage and monitor the IoT devices.

### Software Components
- **Microcontroller Firmware**: Code to read sensor data and control actuators.
- **Web Application**: A user interface to visualize sensor data, control the system, and configure settings.
- **Data Processing and Analysis**: Scripts or applications to process and analyze the sensor data.

## Getting Started
1. Clone the repository: `https://github.com/Abaychandrasurya/Smart-Farming-System.git`
2. Install the necessary dependencies.
3. Upload the firmware to the microcontrollers.
4. Deploy the web application to the cloud platform.
5. Configure the IoT devices and sensors.
6. Set up monitoring and alerts.

## Contributing
Contributions are welcome! Please fork the repository and create a pull request with your changes.

