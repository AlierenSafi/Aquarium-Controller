#pragma once
#include "config.h"

struct AlarmInstance {
    AlarmState state;
    uint32_t start;
    uint32_t interval; // 'delay' yerine 'interval' kullanın 

    AlarmInstance(AlarmState s = ALARM_IDLE, uint32_t st = 0, uint32_t inv = 0) 
        : state(s), start(st), interval(inv) {}
};

class AlarmManager {
public:
    void begin();
    void update(float tempIn, float volt12, float voltBat);
    void recover(AlarmType t);
    void recoverAll();
    bool isActive(AlarmType t) const { return _a[t].state == ALARM_ACTIVE; }
private:
    AlarmInstance _a[ALM_COUNT];
    void _check(AlarmType t, bool cond);
    void _fire(AlarmType t);
};

extern AlarmManager alarmMgr;
