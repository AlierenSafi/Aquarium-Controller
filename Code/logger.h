#pragma once
#include "config.h"

class Logger {
public:
    void    begin();
    void    log(EventCode code, uint8_t data = 0);
    void    printAll();
    void    printLast(uint8_t n);
    void    clear();
    uint8_t count() const { return _count; }
private:
    LogEntry _buf[LOG_SIZE];
    uint8_t  _head  = 0;
    uint8_t  _count = 0;
    void     _persist(uint8_t slot);
};

extern Logger logger;
