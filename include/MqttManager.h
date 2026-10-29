#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>

class MqttManager {
public:
    using ConnectCallback = std::function<void()>;

    MqttManager(WiFiClient& wifiClient, const char* host, uint16_t port,
                const char* user = nullptr, const char* password = nullptr);

    void begin();
    void update();
    bool isConnected();

    void publishDiscoveryConfig();
    void publishAvailability(bool online);
    void publishDoorState(bool isOpen);

    void onConnected(ConnectCallback callback);

private:
    WiFiClient& _wifiClient;
    PubSubClient _mqtt;

    const char* _host;
    uint16_t _port;
    const char* _user;
    const char* _password;

    unsigned long _lastReconnectAttempt;
    ConnectCallback _onConnectCallback;

    bool reconnect();
};
