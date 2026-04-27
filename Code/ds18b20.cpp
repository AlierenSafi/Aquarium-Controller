#include "ds18b20.h"
#include <Arduino.h>

/* ========== NON-BLOCKING API ========== */

void DS18B20::startConversion() {
    if (_state != DS18_IDLE) return;  // Zaten dönüşüm devam ediyor
    
    if (!_ow.reset()) return;
    _ow.write(0xCC); /* skip ROM */
    _ow.write(0x44); /* convert T */
    
    _state = DS18_CONVERTING;
    _convStart = millis();
}

bool DS18B20::readResult(float& tempC) {
    if (_state != DS18_CONVERTING) return false;
    
    // 750ms bekleme süresi doldu mu?
    if (millis() - _convStart < 750) return false;
    
    if (!_ow.reset()) {
        _state = DS18_IDLE;
        return false;
    }
    _ow.write(0xCC);
    _ow.write(0xBE); /* read scratchpad */
    
    uint8_t d[9];
    for (uint8_t i = 0; i < 9; i++) d[i] = _ow.read();
    
    if (OneWire::crc8(d, 8) != d[8]) {
        _state = DS18_IDLE;
        return false;
    }
    
    int16_t raw = (int16_t)((d[1] << 8) | d[0]);
    tempC = (float)raw / 16.0f;
    
    _state = DS18_IDLE;
    return true;
}

/* ========== BLOCKING API (eski) ========== */

bool DS18B20::read(float& tempC) {
    // Non-blocking API kullanarak blocking davranış
    startConversion();
    
    // 750ms bekle (blocking)
    uint32_t start = millis();
    while (millis() - start < 800) {
        // Busy wait
    }
    
    return readResult(tempC);
}
