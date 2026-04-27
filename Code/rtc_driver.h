#pragma once
#include <RTClib.h>

class RtcDriver {
public:
    bool begin();
    void getDateTime();
    void setDateTime(uint16_t yr, uint8_t mo, uint8_t d,
                     uint8_t  h, uint8_t  m, uint8_t s);
    bool isRunning() const { return _ok; }
private:
    RTC_DS1307 _rtc;
    bool       _ok = false;
};

extern RtcDriver rtcDrv;
