# ESP Door Sensor

An ESP8266 (NodeMCU v2) firmware for a magnetic reed switch door sensor with **Home Assistant MQTT Auto-Discovery**, **LED status indicators**, and **ArduinoOTA** support.

---

## Features

- **Interrupt-Driven Door Sensing**: Uses hardware interrupts (`CHANGE`) with software debouncing (`100 ms` default) on an internal pull-up input pin.
- **Home Assistant Auto-Discovery**: Automatically registers as a `door` binary sensor device in Home Assistant via MQTT discovery.
- **Availability & Last Will (LWT)**: Publishes online/offline availability states (`esp_door_sensor/status`) with retained MQTT Last Will and Testament payloads.
- **Non-Blocking LED Feedback**: Dual-LED controller providing visual status patterns for Wi-Fi connection, OTA updates, configuration errors, and momentary pulses when the door opens or closes.
- **Over-The-Air (OTA) Updates**: Built-in `ArduinoOTA` support with password authentication for wireless firmware updates.
- **Flexible Networking**: Supports DHCP out of the box with optional Static IP configuration.

---

## Hardware & Pinout

- **Board**: NodeMCU v2 (ESP8266 / `esp12e`)
- **Sensor**: Magnetic reed switch connected between `D1` and `GND` (uses internal pull-up resistor: `LOW` = Closed, `HIGH` = Open).

| Component | NodeMCU Pin | Macro (`include/config.h`) | Description |
| :--- | :---: | :--- | :--- |
| **Door Reed Switch** | `D1` | `PIN_SENSOR` | Input with `INPUT_PULLUP` (`HIGH` = Open, `LOW` = Closed) |
| **Power / Ground Rail** | `D2` | `PIN_POWER` | Driven `LOW` as a common return/sink rail |
| **Left LED** | `D6` | `PIN_LEFT_LED` | Pulses when door **closes**; used in status patterns |
| **Right LED** | `D7` | `PIN_RIGHT_LED` | Pulses when door **opens**; used in status patterns |
| **Button (Reserved)** | `D8` | `PIN_BUTTON` | Defined in [`include/config.h`](include/config.h) |

### LED Status Patterns

| State / Event | Left LED (`D6`) | Right LED (`D7`) | Behavior |
| :--- | :---: | :---: | :--- |
| **Idle / Connected** | OFF | OFF | Both LEDs off to conserve power / avoid distraction |
| **Door Opened** | OFF | ON (`250 ms`) | Single pulse on Right LED |
| **Door Closed** | ON (`250 ms`) | OFF | Single pulse on Left LED |
| **Wi-Fi Connecting** | Alternating | Alternating | Alternates every `200 ms` |
| **OTA Updating** | Flashing | Flashing | Both flash synchronously every `100 ms` |
| **Static IP Error** | ON | ON | Both solid ON |

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- An MQTT broker (e.g., Mosquitto add-on in Home Assistant)

### 1. Configure Credentials

Copy the example secrets file to create your local configuration (which is git-ignored):

```bash
cp include/secrets.h.example include/secrets.h
```

Edit `include/secrets.h` with your Wi-Fi, MQTT broker, and OTA credentials:

```cpp
// Wi-Fi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Optional Static IP (set to false to use DHCP)
#define USE_STATIC_IP false
#define STATIC_IP_ADDR    10, 21, 37, 3
#define STATIC_GATEWAY    10, 21, 37, 1
#define STATIC_SUBNET     255, 255, 255, 0
#define STATIC_DNS        10, 21, 37, 1

// MQTT Broker Configuration (Home Assistant Mosquitto)
#define MQTT_HOST "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USER "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_password"

// OTA (Over-The-Air) Update Password
#define OTA_PASSWORD "your_ota_password"
```

### 2. Customize Device Parameters (Optional)

Hardware pins, debounce timing, device metadata, and MQTT topics can be adjusted in [`include/config.h`](include/config.h).

### 3. Build and Flash

#### Initial Flash via USB

```bash
# Build the project
pio run

# Upload firmware over USB
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

#### Wireless Updates via OTA

Once the device is connected to Wi-Fi, you can upload firmware over the network by passing the device IP (or `esp_door_sensor.local`) and OTA flags, or by adding an OTA environment to [`platformio.ini`](platformio.ini):

```bash
pio run --target upload --upload-port <DEVICE_IP>
```

---

## MQTT & Home Assistant Integration

When connected to the MQTT broker, the sensor automatically publishes a retained Home Assistant MQTT Discovery configuration payload. No manual YAML configuration in Home Assistant is required.

| Purpose | Topic | Payload(s) | Retained |
| :--- | :--- | :--- | :---: |
| **Discovery Config** | `homeassistant/binary_sensor/esp_door_sensor/door/config` | JSON discovery metadata | Yes |
| **Door State** | `esp_door_sensor/door/state` | `ON` (Open) / `OFF` (Closed) | Yes |
| **Availability (LWT)** | `esp_door_sensor/status` | `online` / `offline` | Yes |

---

## Project Structure

```text
├── include/
│   ├── config.h           # Pin definitions, timings, device metadata, and MQTT topics
│   ├── secrets.h.example  # Template for Wi-Fi, MQTT, and OTA credentials
│   ├── DoorSensor.h       # Interrupt-driven reed switch handler with debounce
│   ├── LedController.h    # Non-blocking LED pattern and pulse controller
│   ├── MqttManager.h      # MQTT client wrapper with HA auto-discovery and LWT
│   └── OtaManager.h       # ArduinoOTA wrapper with start/end callbacks
├── src/
│   ├── main.cpp           # Application entry point, Wi-Fi setup, and main loop
│   ├── DoorSensor.cpp
│   ├── LedController.cpp
│   ├── MqttManager.cpp
│   └── OtaManager.cpp
└── platformio.ini         # PlatformIO environment and library dependencies
```

## Dependencies

Managed automatically via [`platformio.ini`](platformio.ini):
- [`knolleary/PubSubClient`](https://github.com/knolleary/pubsubclient) (`^2.8`)
- [`bblanchon/ArduinoJson`](https://github.com/bblanchon/ArduinoJson) (`^7.0.0`)
