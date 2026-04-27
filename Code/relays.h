#pragma once
#include "config.h"

class RelayManager {
public:
    void    begin();
    void    on(uint8_t idx);
    void    off(uint8_t idx);
    void    toggle(uint8_t idx);
    void    allOn();
    void    allOff();
    void    applyMask(uint8_t mask);
    uint8_t getMask() const { return _mask; }
private:
    uint8_t _mask = 0;
    void    _set(uint8_t idx, bool state);
};

extern RelayManager relayMgr;
