#include "rtc_driver.h"
#include "globals.h"
#include "mutex.h"
#include <Wire.h>

RtcDriver rtcDrv;

bool RtcDriver::begin() {
    _ok = _rtc.begin();
    if (_ok && !_rtc.isrunning())
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    return _ok;
}

void RtcDriver::getDateTime() {
    if (!_ok) return;
    if (!MUTEX_TAKE(xI2CMutex)) return;
    DateTime now = _rtc.now();
    MUTEX_GIVE(xI2CMutex);

    if (!MUTEX_TAKE(xRTCMutex)) return;
    gYear = now.year(); gMonth  = now.month();  gDay    = now.day();
    gHour = now.hour(); gMinute = now.minute();  gSecond = now.second();
    gDow  = now.dayOfTheWeek();
    MUTEX_GIVE(xRTCMutex);
}

void RtcDriver::setDateTime(uint16_t yr,uint8_t mo,uint8_t d,
                             uint8_t h, uint8_t m, uint8_t s) {
    if (!_ok) return;
    if (MUTEX_TAKE(xI2CMutex)) {
        _rtc.adjust(DateTime(yr,mo,d,h,m,s));
        MUTEX_GIVE(xI2CMutex);
    }
}
