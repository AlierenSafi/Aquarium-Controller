#ifndef __MY_TIMER_H
#define __MY_TIMER_H

class myTimer {
    unsigned long currentTime = 0, TO = 0;
  public:
    void (*isrCallback)();
    myTimer(void (*isr)() , unsigned long timeout) {
      isrCallback = isr;
      TO = timeout;
      currentTime = millis();
    }
    void check() {
      unsigned long cc = millis();
      if (cc - currentTime >= TO) {
        currentTime = cc;
        this->isrCallback();
      }
    }
};
#endif
