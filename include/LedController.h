#pragma once
#include <Arduino.h>

enum class LedPattern {
    OFF,
    WIFI_CONNECTING,
    OTA_UPDATING,
    ERROR_STATIC
};

class LedController {
public:
    LedController(uint8_t leftPin, uint8_t rightPin, uint8_t powerPin);

    void begin();
    void update();

    void setPattern(LedPattern pattern);
    void pulseLeft(unsigned long durationMs = 250);
    void pulseRight(unsigned long durationMs = 250);

private:
    uint8_t _leftPin;
    uint8_t _rightPin;
    uint8_t _powerPin;

    LedPattern _currentPattern;
    unsigned long _lastToggleTime;
    bool _blinkState;

    unsigned long _pulseLeftUntil;
    unsigned long _pulseRightUntil;

    void applyOutputs(bool leftOn, bool rightOn);
};
