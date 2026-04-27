#include "relays.h"
#include "globals.h"
#include "mutex.h"
#include <Arduino.h>

RelayManager relayMgr;

void RelayManager::begin() {
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], HIGH); /* active LOW → off */
    }
    _mask = 0;
    gRelayMask = 0;
}

void RelayManager::_set(uint8_t idx, bool state) {
    if (idx >= 8) return;
    digitalWrite(RELAY_PINS[idx], state ? LOW : HIGH);
    if (state) _mask |=  (1u << idx);
    else        _mask &= ~(1u << idx);
    gRelayMask = _mask;
}

void RelayManager::on(uint8_t i)     { 
    if (MUTEX_TAKE(xRelayMutex)) {
        _set(i, true);  
        MUTEX_GIVE(xRelayMutex);
    }
}
void RelayManager::off(uint8_t i)    { 
    if (MUTEX_TAKE(xRelayMutex)) {
        _set(i, false); 
        MUTEX_GIVE(xRelayMutex);
    }
}
void RelayManager::toggle(uint8_t i) { 
    if (MUTEX_TAKE(xRelayMutex)) {
        bool current = (_mask >> i) & 1;
        _set(i, !current); 
        MUTEX_GIVE(xRelayMutex);
    }
}
void RelayManager::allOn()  { 
    if (MUTEX_TAKE(xRelayMutex)) {
        for (uint8_t i=0;i<8;i++) _set(i,true);  
        MUTEX_GIVE(xRelayMutex);
    }
}
void RelayManager::allOff() { 
    if (MUTEX_TAKE(xRelayMutex)) {
        for (uint8_t i=0;i<8;i++) _set(i,false); 
        MUTEX_GIVE(xRelayMutex);
    }
}

void RelayManager::applyMask(uint8_t mask) {
    if (MUTEX_TAKE(xRelayMutex)) {
        for (uint8_t i = 0; i < 8; i++) _set(i, (mask >> i) & 1);
        MUTEX_GIVE(xRelayMutex);
    }
}
