#pragma once
#include <Arduino.h>
#include <stdint.h>

/* RGB LED Strip Control Class
 * Pin 10: Red
 * Pin 9:  Green
 * Pin 8:  Blue
 * 
 * Note: Arduino Mega pins 8, 9, 10 are PWM capable (Timer1/Timer2)
 */

class RGBStrip {
public:
    // Initialize
    void begin();
    
    // Set RGB values (0-255)
    void setRGB(uint8_t r, uint8_t g, uint8_t b);
    
    // Set individual color channel
    void setRed(uint8_t value);
    void setGreen(uint8_t value);
    void setBlue(uint8_t value);
    
    // Predefined colors
    void setOff();
    void setWhite(uint8_t brightness = 255);
    void setRedColor(uint8_t brightness = 255);
    void setGreenColor(uint8_t brightness = 255);
    void setBlueColor(uint8_t brightness = 255);
    void setYellow(uint8_t brightness = 255);
    void setCyan(uint8_t brightness = 255);
    void setMagenta(uint8_t brightness = 255);
    void setOrange(uint8_t brightness = 255);
    void setPurple(uint8_t brightness = 255);
    
    // Brightness control (0-100%, changes current color brightness)
    void setBrightness(uint8_t percent);
    
    // Get current values
    uint8_t getRed() const { return _r; }
    uint8_t getGreen() const { return _g; }
    uint8_t getBlue() const { return _b; }
    uint8_t getBrightness() const { return _brightness; }
    
    // Check status
    bool isOn() const { return (_r > 0) || (_g > 0) || (_b > 0); }
    
    // Get current color as string (format: R:xxx G:xxx B:xxx)
    void getColorString(char* buffer, size_t size) const;

private:
    uint8_t _r = 0;           // Red value (0-255)
    uint8_t _g = 0;           // Green value (0-255)
    uint8_t _b = 0;           // Blue value (0-255)
    uint8_t _brightness = 100; // Brightness percentage (0-100)
    
    // Calculate and apply PWM value
    void _applyPWM();
    uint8_t _calcPWM(uint8_t value);
};

// Global instance
extern RGBStrip rgbStrip;
