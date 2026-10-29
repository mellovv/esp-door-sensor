#include "DoorSensor.h"

volatile bool DoorSensor::_interruptTriggered = false;
DoorSensor* DoorSensor::_instance = nullptr;

void IRAM_ATTR DoorSensor::handleInterrupt() {
    _interruptTriggered = true;
}

DoorSensor::DoorSensor(uint8_t pin, unsigned long debounceDelayMs)
    : _pin(pin),
      _debounceDelayMs(debounceDelayMs),
      _currentState(false),
      _lastDebouncedState(false),
      _lastTransitionTime(0),
      _callback(nullptr) {
    _instance = this;
}

void DoorSensor::begin() {
    pinMode(_pin, INPUT_PULLUP);

    // Initial state (HIGH = Open, LOW = Closed with internal pullup)
    _currentState = (digitalRead(_pin) == HIGH);
    _lastDebouncedState = _currentState;
    _lastTransitionTime = millis();

    attachInterrupt(digitalPinToInterrupt(_pin), DoorSensor::handleInterrupt, CHANGE);
}

void DoorSensor::onStateChange(StateChangeCallback callback) {
    _callback = callback;
}

bool DoorSensor::isOpen() const {
    return _lastDebouncedState;
}

void DoorSensor::update() {
    unsigned long now = millis();

    if (_interruptTriggered) {
        _interruptTriggered = false;
        bool pinReading = (digitalRead(_pin) == HIGH);

        if (pinReading != _currentState) {
            _currentState = pinReading;
            _lastTransitionTime = now;
        }
    }

    // Debounce timer check
    if ((now - _lastTransitionTime) >= _debounceDelayMs) {
        if (_currentState != _lastDebouncedState) {
            _lastDebouncedState = _currentState;

            if (_callback) {
                _callback(_lastDebouncedState);
            }
        }
    }
}
