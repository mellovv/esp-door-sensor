#pragma once
#include <Arduino.h>

// ==========================================
// Hardware Pin Definitions (NodeMCU ESP8266)
// ==========================================
#define PIN_LEFT_LED   D6
#define PIN_RIGHT_LED  D7
#define PIN_BUTTON     D8
#define PIN_SENSOR     D1
#define PIN_POWER      D2

// ==========================================
// Operational Parameters
// ==========================================
#define SERIAL_BAUD_RATE      115200
#define SENSOR_DEBOUNCE_MS    100      // Debounce window in milliseconds
#define MQTT_RECONNECT_MS     5000     // Reconnect retry interval
#define LED_PULSE_DURATION_MS 250      // Duration of LED flash on state change

// ==========================================
// Device & Home Assistant Configuration
// ==========================================
#define DEVICE_ID             "esp_door_sensor"
#define DEVICE_NAME           "ESP Door Sensor"
#define DEVICE_MANUFACTURER   "DIY / NodeMCU"
#define DEVICE_MODEL          "ESP8266 Door Sensor"
#define DEVICE_SW_VERSION     "2.0.0"

// MQTT Topics (HA standard: keep state/availability outside the homeassistant/ discovery tree)
#define HA_DISCOVERY_PREFIX   "homeassistant"
#define MQTT_TOPIC_STATE      DEVICE_ID "/door/state"
#define MQTT_TOPIC_AVAIL      DEVICE_ID "/status"
#define MQTT_TOPIC_CONFIG     HA_DISCOVERY_PREFIX "/binary_sensor/" DEVICE_ID "/door/config"

// Payloads
#define PAYLOAD_OPEN          "ON"
#define PAYLOAD_CLOSED        "OFF"
#define PAYLOAD_ONLINE        "online"
#define PAYLOAD_OFFLINE       "offline"
