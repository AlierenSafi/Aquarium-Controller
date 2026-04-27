#include "alarm_manager.h"
#include "globals.h"
#include "logger.h"
#include <Arduino.h>

AlarmManager alarmMgr;

static const uint16_t DELAYS[ALM_COUNT] = {5000,5000,10000,0};
static const EventCode EVTS[ALM_COUNT]  = {
    EVT_TEMP_HIGH, EVT_TEMP_LOW, EVT_VOLT_LOW, EVT_BATT_LOW
};

void AlarmManager::begin() {
for (uint8_t i = 0; i < ALM_COUNT; i++) {
        _a[i].state = ALARM_IDLE;
        _a[i].start = 0;
        _a[i].interval = DELAYS[i]; 
    }
}

void AlarmManager::_fire(AlarmType t) {
    if (_a[t].state == ALARM_ACTIVE) {
        /* Already fired, don't repeat */
        return;
    }
    _a[t].state = ALARM_ACTIVE;
    digitalWrite(PIN_BUZZER, HIGH);
    logger.log(EVTS[t]);
}


void AlarmManager::_check(AlarmType t, bool state_input) {
    AlarmInstance &a = _a[t];
    if (state_input) {
        if (a.state == ALARM_IDLE) {
            /* First time condition met - start pending timer */
            a.start = millis();
            a.state = ALARM_PENDING;
        }
        if (a.state == ALARM_PENDING) {
            /* Check if interval elapsed */
            if ((millis() - a.start) >= a.interval) {
                _fire(t);
            }
        }
        /* If ALARM_ACTIVE, do nothing (already fired) */
    } else {
        /* Condition cleared - only reset if PENDING */
        /* ALARM_ACTIVE stays active until recover() is called */
        if (a.state == ALARM_PENDING) {
            a.state = ALARM_IDLE;
        }
    }
}

void AlarmManager::recover(AlarmType t) {
    _a[t].state = ALARM_IDLE;   /* ← ALARM_IDLE, not RECOVERED */
    logger.log(EVT_ALARM_RECOVER, (uint8_t)t);
    for (uint8_t i = 0; i < ALM_COUNT; i++)
        if (_a[i].state == ALARM_ACTIVE) return;
    digitalWrite(PIN_BUZZER, LOW);
}

void AlarmManager::recoverAll() {
    for (uint8_t i = 0; i < ALM_COUNT; i++) _a[i].state = ALARM_IDLE;
    digitalWrite(PIN_BUZZER, LOW);
    logger.log(EVT_ALARM_RECOVER, 0xFF);
}

void AlarmManager::update(float tIn, float v12, float vBat) {
    if (!gAlarmCfg.enabled) return;
    _check(ALM_TEMP_HIGH, tIn  >  gAlarmCfg.tempHigh);
    _check(ALM_TEMP_LOW,  tIn  <  gAlarmCfg.tempLow);
    _check(ALM_VOLT_LOW,  v12  <  gAlarmCfg.volt12Low);
    _check(ALM_BATT_LOW,  vBat <  gAlarmCfg.voltBatLow);
}
