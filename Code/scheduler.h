#pragma once
#include <Arduino.h>
#include <stdint.h>

/* Zamanlama yapisi */
struct Timer {
    uint32_t lastRun;
    uint32_t interval;
    bool enabled;
    
    Timer(uint32_t ms = 1000) : lastRun(0), interval(ms), enabled(true) {}
    
    bool check() {
        if (!enabled) return false;
        uint32_t now = millis();
        if (now - lastRun >= interval) {
            lastRun = now;
            return true;
        }
        return false;
    }
    
    void reset() { lastRun = millis(); }
    void setInterval(uint32_t ms) { interval = ms; }
};

/* Non-blocking bekleme */
class Delay {
private:
    uint32_t _start;
    uint32_t _duration;
    bool _running;
    
public:
    Delay() : _start(0), _duration(0), _running(false) {}
    
    void start(uint32_t ms) {
        _start = millis();
        _duration = ms;
        _running = true;
    }
    
    bool elapsed() {
        if (!_running) return true;
        if (millis() - _start >= _duration) {
            _running = false;
            return true;
        }
        return false;
    }
    
    bool isRunning() const { return _running; }
    void stop() { _running = false; }
};
