#pragma once
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <functional>

class OtaManager {
public:
    using OtaEventCallback = std::function<void()>;

    OtaManager(const char* hostname, const char* password);

    void begin();
    void update();

    void onStart(OtaEventCallback cb);
    void onEnd(OtaEventCallback cb);

private:
    const char* _hostname;
    const char* _password;

    OtaEventCallback _onStartCb;
    OtaEventCallback _onEndCb;
};
