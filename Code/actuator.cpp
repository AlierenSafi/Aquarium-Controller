#include "actuator.h"
#include "globals.h"
#include <Arduino.h>

ActuatorManager actuators;

/* Timer5 / OC5C / PL5 (pin 44) — Fast PWM 8-bit, prescaler /8 */
void ActuatorManager::_initPWM() {
    TCCR5A = _BV(COM5C1) | _BV(WGM50);
    TCCR5B = _BV(WGM52)  | _BV(CS51);
    OCR5C  = 0;
    DDRL  |= _BV(PL5);
}

void ActuatorManager::init(RelayManager& rm) {
    _rm = &rm;
    _initPWM();
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
}

void ActuatorManager::setFanPercent(uint8_t pct) {
    if (pct > 100) pct = 100;
    uint8_t duty = (uint8_t)((uint16_t)pct * 255 / 100);
    OCR5C    = duty;
    gFanDuty = duty;
}

/* Linear ramp: 0% @ <=24°C, 100% @ >=30°C */
void ActuatorManager::autoFan(float t) {
    uint8_t pct;
    if      (t <= 24.0f) pct = 0;
    else if (t >= 30.0f) pct = 100;
    else pct = (uint8_t)((t - 24.0f) * 100.0f / 6.0f);
    setFanPercent(pct);
}

void ActuatorManager::applyScenario(const Scenario& s) {
    if (_rm) _rm->applyMask(s.relayMask);
    setFanPercent(s.fanSpeed);
    gSettings.targetTemp = s.targetTemp;
}
