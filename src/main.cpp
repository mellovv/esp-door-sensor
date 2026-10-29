#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "secrets.h"
#include "LedController.h"
#include "DoorSensor.h"
#include "MqttManager.h"
#include "OtaManager.h"

// Hardware and Service instances
static WiFiClient wifiClient;
static LedController leds(PIN_LEFT_LED, PIN_RIGHT_LED, PIN_POWER);
static DoorSensor doorSensor(PIN_SENSOR, SENSOR_DEBOUNCE_MS);
static MqttManager mqtt(wifiClient, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASSWORD);
static OtaManager ota(DEVICE_ID, OTA_PASSWORD);

void setupWiFi() {
    Serial.printf("\n[WiFi] Connecting to SSID: '%s'...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);

#if USE_STATIC_IP
    IPAddress local_ip(STATIC_IP_ADDR);
    IPAddress gateway(STATIC_GATEWAY);
    IPAddress subnet(STATIC_SUBNET);
#ifdef STATIC_DNS
    IPAddress dns(STATIC_DNS);
    if (!WiFi.config(local_ip, gateway, subnet, dns)) {
#else
    if (!WiFi.config(local_ip, gateway, subnet)) {
#endif
        Serial.println("[WiFi] Static IP configuration failed!");
        leds.setPattern(LedPattern::ERROR_STATIC);
    } else {
        Serial.printf("[WiFi] Configured Static IP: %d.%d.%d.%d\n",
                      STATIC_IP_ADDR);
    }
#endif

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    leds.setPattern(LedPattern::WIFI_CONNECTING);
    unsigned long startAttempt = millis();
    unsigned long lastStatusPrint = 0;

    while (WiFi.status() != WL_CONNECTED) {
        leds.update();
        delay(10);

        if (millis() - lastStatusPrint >= 2000) {
            lastStatusPrint = millis();
            wl_status_t status = WiFi.status();
            const char* statusStr = "CONNECTING";
            if (status == WL_NO_SSID_AVAIL) statusStr = "SSID_NOT_FOUND (Verify 2.4GHz network)";
            else if (status == WL_CONNECT_FAILED) statusStr = "AUTH_FAILED (Check Password)";
            else if (status == WL_IDLE_STATUS) statusStr = "IDLE";
            else if (status == WL_DISCONNECTED) statusStr = "DISCONNECTED / WAITING";

            Serial.printf("[WiFi] %s (%lu s elapsed)\n",
                          statusStr, (millis() - startAttempt) / 1000);
        }
    }

    leds.setPattern(LedPattern::OFF);
    Serial.printf("[WiFi] Connected! IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(200);
    Serial.println("\n--- Starting ESP Door Sensor ---");

    // Initialize LEDs and Sensor
    leds.begin();
    doorSensor.begin();

    // Register callback for physical door movements
    doorSensor.onStateChange([](bool isOpen) {
        Serial.printf("[Sensor] Door %s\n", isOpen ? "OPENED" : "CLOSED");

        // Visual LED feedback (Right LED for Open, Left LED for Closed)
        if (isOpen) {
            leds.pulseRight(LED_PULSE_DURATION_MS);
        } else {
            leds.pulseLeft(LED_PULSE_DURATION_MS);
        }

        // Publish to Home Assistant via MQTT
        mqtt.publishDoorState(isOpen);
    });

    // Connect to local Wi-Fi
    setupWiFi();

    // Setup OTA Updates
    ota.onStart([]() {
        leds.setPattern(LedPattern::OTA_UPDATING);
    });
    ota.onEnd([]() {
        leds.setPattern(LedPattern::OFF);
    });
    ota.begin();

    // Setup MQTT with Home Assistant Discovery
    mqtt.onConnected([]() {
        // Publish current state immediately upon MQTT connection
        mqtt.publishDoorState(doorSensor.isOpen());
    });
    mqtt.begin();

    Serial.println("[System] Initialization complete.");
}

void loop() {
    leds.update();
    doorSensor.update();
    mqtt.update();
    ota.update();
}
