#include "logger.h"
#include "globals.h"
#include "config.h"
#include "mutex.h"
#include <EEPROM.h>
#include <Arduino.h>

Logger logger;

static const uint16_t LB = EE_LOG;   /* log base address */

void Logger::begin() {
    if (!MUTEX_TAKE(xEEPROMMutex)) return;
    _head  = EEPROM.read(LB);
    _count = EEPROM.read(LB+1);
    if (_head >= LOG_SIZE || _count > LOG_SIZE) {
        _head = _count = 0;
        EEPROM.update(LB,   0);
        EEPROM.update(LB+1, 0);
    } else {
        for (uint8_t i = 0; i < _count; i++) {
            uint8_t s = (uint8_t)((_head + LOG_SIZE - _count + i) % LOG_SIZE);
            EEPROM.get(LB + 2 + s * sizeof(LogEntry), _buf[s]);
        }
    }
    MUTEX_GIVE(xEEPROMMutex);
}

void Logger::_persist(uint8_t slot) {
    EEPROM.put(LB + 2 + slot * sizeof(LogEntry), _buf[slot]);
    EEPROM.update(LB,   _head);
    EEPROM.update(LB+1, _count);
}

void Logger::log(EventCode code, uint8_t data) {
    uint8_t slot = _head;
    _buf[slot] = {(uint8_t)code, gHour, gMinute, (int8_t)gTempIn, data, gDay};
    _head = (_head + 1) % LOG_SIZE;
    if (_count < LOG_SIZE) _count++;
    if (MUTEX_TAKE(xEEPROMMutex)) {
        _persist(slot);
        MUTEX_GIVE(xEEPROMMutex);
    }
}

static const char* evtStr(uint8_t c) {
    switch(c) {
        case EVT_BOOT:          return "BOOT";
        case EVT_TEMP_HIGH:     return "T_HIGH";
        case EVT_TEMP_LOW:      return "T_LOW";
        case EVT_VOLT_LOW:      return "V_LOW";
        case EVT_BATT_LOW:      return "BAT_LOW";
        case EVT_RELAY_CHG:     return "RELAY";
        case EVT_SCENARIO:      return "SCEN";
        case EVT_ALARM_RECOVER: return "RECOV";
        case EVT_SCHEDULE:      return "SCHED";
        default:                return "???";
    }
}

static void printEntry(uint8_t idx, const LogEntry& e) {
    char b[48];
    snprintf(b, sizeof(b), "[%02u] %02u:%02u D%02u T%d %-8s d=%u",
             idx, e.hour, e.minute, e.day, (int)e.temp, evtStr(e.code), e.data);
    // gActivePort: Hangi porttan mesaj geldi?
    if (gActivePort == 1) {
        Serial3.println(b);
    } else {
        Serial.println(b);
    }
}

void Logger::printAll() {
    for (uint8_t i = 0; i < _count; i++) {
        uint8_t s = (uint8_t)((_head + LOG_SIZE - _count + i) % LOG_SIZE);
        printEntry(i, _buf[s]);
    }
}

void Logger::printLast(uint8_t n) {
    if (n > _count) n = _count;
    for (uint8_t i = _count - n; i < _count; i++) {
        uint8_t s = (uint8_t)((_head + LOG_SIZE - _count + i) % LOG_SIZE);
        printEntry(i, _buf[s]);
    }
}

void Logger::clear() {
    _head = _count = 0;
    if (MUTEX_TAKE(xEEPROMMutex)) {
        EEPROM.update(LB, 0);
        EEPROM.update(LB+1, 0);
        MUTEX_GIVE(xEEPROMMutex);
    }
}
