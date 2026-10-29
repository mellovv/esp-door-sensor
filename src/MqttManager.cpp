#include "MqttManager.h"
#include "config.h"

MqttManager::MqttManager(WiFiClient& wifiClient, const char* host, uint16_t port,
                         const char* user, const char* password)
    : _wifiClient(wifiClient),
      _mqtt(wifiClient),
      _host(host),
      _port(port),
      _user(user),
      _password(password),
      _lastReconnectAttempt(0),
      _onConnectCallback(nullptr) {}

void MqttManager::begin() {
    _mqtt.setServer(_host, _port);
    // Expand buffer from default 256 bytes to handle Home Assistant discovery JSON payloads
    _mqtt.setBufferSize(768);
}

bool MqttManager::isConnected() {
    return _mqtt.connected();
}

void MqttManager::onConnected(ConnectCallback callback) {
    _onConnectCallback = callback;
}

bool MqttManager::reconnect() {
    Serial.printf("[MQTT] Connecting to broker at %s:%d...\n", _host, _port);

    bool connected = false;
    const char* clientId = DEVICE_ID;

    // Connect with Last Will and Testament (LWT)
    if (_user && strlen(_user) > 0) {
        connected = _mqtt.connect(clientId, _user, _password,
                                  MQTT_TOPIC_AVAIL, 1, true, PAYLOAD_OFFLINE);
    } else {
        connected = _mqtt.connect(clientId,
                                  MQTT_TOPIC_AVAIL, 1, true, PAYLOAD_OFFLINE);
    }

    if (connected) {
        Serial.println("[MQTT] Connected successfully!");
        publishAvailability(true);
        publishDiscoveryConfig();

        if (_onConnectCallback) {
            _onConnectCallback();
        }
    } else {
        Serial.printf("[MQTT] Connection failed, rc=%d. Will retry in %d ms.\n",
                      _mqtt.state(), MQTT_RECONNECT_MS);
    }

    return connected;
}

void MqttManager::publishAvailability(bool online) {
    if (!_mqtt.connected()) return;
    _mqtt.publish(MQTT_TOPIC_AVAIL, online ? PAYLOAD_ONLINE : PAYLOAD_OFFLINE, true);
}

void MqttManager::publishDiscoveryConfig() {
    if (!_mqtt.connected()) return;

    JsonDocument doc;
    doc["name"] = "Door";
    doc["device_class"] = "door";
    doc["state_topic"] = MQTT_TOPIC_STATE;
    doc["payload_on"] = PAYLOAD_OPEN;
    doc["payload_off"] = PAYLOAD_CLOSED;
    doc["availability_topic"] = MQTT_TOPIC_AVAIL;
    doc["payload_available"] = PAYLOAD_ONLINE;
    doc["payload_not_available"] = PAYLOAD_OFFLINE;
    doc["unique_id"] = String(DEVICE_ID) + "_door";
    doc["has_entity_name"] = true;

    JsonObject device = doc["device"].to<JsonObject>();
    JsonArray identifiers = device["identifiers"].to<JsonArray>();
    identifiers.add(DEVICE_ID);
    device["name"] = DEVICE_NAME;
    device["model"] = DEVICE_MODEL;
    device["manufacturer"] = DEVICE_MANUFACTURER;
    device["sw_version"] = DEVICE_SW_VERSION;

    String output;
    serializeJson(doc, output);

    bool ok = _mqtt.publish(MQTT_TOPIC_CONFIG, output.c_str(), true);
    if (ok) {
        Serial.println("[MQTT] Home Assistant discovery config published.");
    } else {
        Serial.println("[MQTT] ERROR: Failed to publish discovery config (buffer overflow?)");
    }
}

void MqttManager::publishDoorState(bool isOpen) {
    if (!_mqtt.connected()) return;

    const char* payload = isOpen ? PAYLOAD_OPEN : PAYLOAD_CLOSED;
    _mqtt.publish(MQTT_TOPIC_STATE, payload, true);
    Serial.printf("[MQTT] Published door state: %s\n", payload);
}

void MqttManager::update() {
    if (!WiFi.isConnected()) {
        return;
    }

    if (!_mqtt.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= MQTT_RECONNECT_MS) {
            _lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        _mqtt.loop();
    }
}
