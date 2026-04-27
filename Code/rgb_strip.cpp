#include "rgb_strip.h"
#include "config.h"
#include "globals.h"
#include <Arduino.h>

RGBStrip rgbStrip;

void RGBStrip::begin() {
    // Pinleri çıkış olarak ayarla
    pinMode(PIN_RGB_RED, OUTPUT);
    pinMode(PIN_RGB_GREEN, OUTPUT);
    pinMode(PIN_RGB_BLUE, OUTPUT);
    
    // Başlangıçta kapalı
    setOff();
}

void RGBStrip::setRGB(uint8_t r, uint8_t g, uint8_t b) {
    _r = r;
    _g = g;
    _b = b;
    _applyPWM();
}

void RGBStrip::setRed(uint8_t value) {
    _r = value;
    _applyPWM();
}

void RGBStrip::setGreen(uint8_t value) {
    _g = value;
    _applyPWM();
}

void RGBStrip::setBlue(uint8_t value) {
    _b = value;
    _applyPWM();
}

void RGBStrip::setOff() {
    _r = _g = _b = 0;
    _brightness = 100;
    _applyPWM();
}

void RGBStrip::setWhite(uint8_t brightness) {
    _brightness = brightness;
    _r = _g = _b = 255;
    _applyPWM();
}

void RGBStrip::setRedColor(uint8_t brightness) {
    _brightness = brightness;
    _r = 255;
    _g = _b = 0;
    _applyPWM();
}

void RGBStrip::setGreenColor(uint8_t brightness) {
    _brightness = brightness;
    _g = 255;
    _r = _b = 0;
    _applyPWM();
}

void RGBStrip::setBlueColor(uint8_t brightness) {
    _brightness = brightness;
    _b = 255;
    _r = _g = 0;
    _applyPWM();
}

void RGBStrip::setYellow(uint8_t brightness) {
    _brightness = brightness;
    _r = 255;
    _g = 255;
    _b = 0;
    _applyPWM();
}

void RGBStrip::setCyan(uint8_t brightness) {
    _brightness = brightness;
    _r = 0;
    _g = 255;
    _b = 255;
    _applyPWM();
}

void RGBStrip::setMagenta(uint8_t brightness) {
    _brightness = brightness;
    _r = 255;
    _g = 0;
    _b = 255;
    _applyPWM();
}

void RGBStrip::setOrange(uint8_t brightness) {
    _brightness = brightness;
    _r = 255;
    _g = 165;
    _b = 0;
    _applyPWM();
}

void RGBStrip::setPurple(uint8_t brightness) {
    _brightness = brightness;
    _r = 128;
    _g = 0;
    _b = 128;
    _applyPWM();
}

void RGBStrip::setBrightness(uint8_t percent) {
    if (percent > 100) percent = 100;
    _brightness = percent;
    _applyPWM();
}

void RGBStrip::_applyPWM() {
    // PWM değerlerini hesapla ve uygula
    uint8_t pwmR = _calcPWM(_r);
    uint8_t pwmG = _calcPWM(_g);
    uint8_t pwmB = _calcPWM(_b);
    
    // NOT: RGB LED sürücü devre inverted (ters) mantık kullanıyor olabilir
    // PCB şemasında transistör sürücü varsa, LOW = LED ON, HIGH = LED OFF
    // Eğer direkt PWM ise: analogWrite kullanılır
    // Şemada 10k dirençler var, muhtemelen transistör sürücü
    
    // Arduino Mega: 8, 9, 10 pinleri PWM destekli
    analogWrite(PIN_RGB_RED, pwmR);
    analogWrite(PIN_RGB_GREEN, pwmG);
    analogWrite(PIN_RGB_BLUE, pwmB);
}

uint8_t RGBStrip::_calcPWM(uint8_t value) {
    // Parlaklık yüzdesini uygula
    uint16_t result = (uint16_t)value * _brightness / 100;
    if (result > 255) result = 255;
    return (uint8_t)result;
}

void RGBStrip::getColorString(char* buffer, size_t size) const {
    snprintf(buffer, size, "R:%u G:%u B:%u (%u%%)", 
             (unsigned)_r, (unsigned)_g, (unsigned)_b, (unsigned)_brightness);
}
