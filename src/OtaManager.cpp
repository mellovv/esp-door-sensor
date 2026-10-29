#include "OtaManager.h"

OtaManager::OtaManager(const char* hostname, const char* password)
    : _hostname(hostname),
      _password(password),
      _onStartCb(nullptr),
      _onEndCb(nullptr) {}

void OtaManager::begin() {
    if (_hostname && strlen(_hostname) > 0) {
        ArduinoOTA.setHostname(_hostname);
    }
    if (_password && strlen(_password) > 0) {
        ArduinoOTA.setPassword(_password);
    }

    ArduinoOTA.onStart([this]() {
        Serial.println("[OTA] Update started.");
        if (_onStartCb) _onStartCb();
    });

    ArduinoOTA.onEnd([this]() {
        Serial.println("\n[OTA] Update completed successfully.");
        if (_onEndCb) _onEndCb();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
    Serial.println("[OTA] Ready for updates.");
}

void OtaManager::update() {
    ArduinoOTA.handle();
}

void OtaManager::onStart(OtaEventCallback cb) {
    _onStartCb = cb;
}

void OtaManager::onEnd(OtaEventCallback cb) {
    _onEndCb = cb;
}
