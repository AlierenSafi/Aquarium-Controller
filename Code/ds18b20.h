#pragma once
#include <OneWire.h>
#include <Arduino.h>

/* DS18B20 durum makinesi */
enum DS18State { DS18_IDLE, DS18_CONVERTING };

class DS18B20 {
public:
    explicit DS18B20(uint8_t pin) : _ow(pin), _state(DS18_IDLE), _convStart(0) {}
    
    /* Non-blocking API */
    void startConversion();           // Dönüşüm başlat
    bool readResult(float& tempC);    // 750ms sonra sonucu oku
    bool isConverting() const { return _state == DS18_CONVERTING; }
    
    /* Eski blocking API - kullanmayın (delay(800) kullanır) */
    bool read(float& tempC);
    
private:
    OneWire _ow;
    DS18State _state;
    uint32_t _convStart;
};
