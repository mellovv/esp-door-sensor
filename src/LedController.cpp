#include "LedController.h"

LedController::LedController(uint8_t leftPin, uint8_t rightPin, uint8_t powerPin)
    : _leftPin(leftPin),
      _rightPin(rightPin),
      _powerPin(powerPin),
      _currentPattern(LedPattern::OFF),
      _lastToggleTime(0),
      _blinkState(false),
      _pulseLeftUntil(0),
      _pulseRightUntil(0) {}

void LedController::begin() {
    pinMode(_leftPin, OUTPUT);
    pinMode(_rightPin, OUTPUT);
    pinMode(_powerPin, OUTPUT);

    // Replicate power rail setup from original code
    digitalWrite(_powerPin, LOW);
    applyOutputs(false, false);
}

void LedController::setPattern(LedPattern pattern) {
    if (_currentPattern != pattern) {
        _currentPattern = pattern;
        _lastToggleTime = millis();
        _blinkState = false;

        if (pattern == LedPattern::ERROR_STATIC) {
            applyOutputs(true, true);
        } else if (pattern == LedPattern::OFF) {
            applyOutputs(false, false);
        }
    }
}

void LedController::pulseLeft(unsigned long durationMs) {
    _pulseLeftUntil = millis() + durationMs;
}

void LedController::pulseRight(unsigned long durationMs) {
    _pulseRightUntil = millis() + durationMs;
}

void LedController::applyOutputs(bool leftOn, bool rightOn) {
    digitalWrite(_leftPin, leftOn ? HIGH : LOW);
    digitalWrite(_rightPin, rightOn ? HIGH : LOW);
}

void LedController::update() {
    unsigned long now = millis();

    // Check temporary pulses first
    bool pulseLeftActive = (now < _pulseLeftUntil);
    bool pulseRightActive = (now < _pulseRightUntil);

    if (pulseLeftActive || pulseRightActive) {
        applyOutputs(pulseLeftActive, pulseRightActive);
        return;
    }

    // Pattern-based updates
    switch (_currentPattern) {
        case LedPattern::WIFI_CONNECTING:
            if (now - _lastToggleTime >= 200) {
                _lastToggleTime = now;
                _blinkState = !_blinkState;
                applyOutputs(_blinkState, !_blinkState);
            }
            break;

        case LedPattern::OTA_UPDATING:
            if (now - _lastToggleTime >= 100) {
                _lastToggleTime = now;
                _blinkState = !_blinkState;
                applyOutputs(_blinkState, _blinkState);
            }
            break;

        case LedPattern::ERROR_STATIC:
            applyOutputs(true, true);
            break;

        case LedPattern::OFF:
        default:
            applyOutputs(false, false);
            break;
    }
}
