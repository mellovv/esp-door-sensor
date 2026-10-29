#pragma once
#include <Arduino.h>
#include <functional>

class DoorSensor {
public:
    using StateChangeCallback = std::function<void(bool isOpen)>;

    DoorSensor(uint8_t pin, unsigned long debounceDelayMs = 100);

    void begin();
    void update();
    bool isOpen() const;
    void onStateChange(StateChangeCallback callback);

    // Called from ISR
    static void handleInterrupt();

private:
    uint8_t _pin;
    unsigned long _debounceDelayMs;
    bool _currentState;
    bool _lastDebouncedState;
    unsigned long _lastTransitionTime;
    static volatile bool _interruptTriggered;
    static DoorSensor* _instance;

    StateChangeCallback _callback;
};
