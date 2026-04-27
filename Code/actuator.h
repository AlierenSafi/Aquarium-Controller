#pragma once
#include "relays.h"
#include "config.h"

class ActuatorManager {
public:
    void init(RelayManager& rm);
    void setFanPercent(uint8_t pct);
    void autoFan(float tempC);
    void applyScenario(const Scenario& s);
private:
    RelayManager* _rm = nullptr;
    void _initPWM();
};

extern ActuatorManager actuators;
