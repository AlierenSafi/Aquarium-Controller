#ifndef __VOLTAGE_H
#define __VOLTAGE_H

enum voltages {SOURCE, MAIN_REGULATOR, ESP_REGULATOR, RTC_BAT};

class voltage {
    int pin;
    int measureSensivity = 10;
    double r1, r2;

    double toVoltage(int p) {
      double sum = 0;
      double vv;
      for (int i = 0; i < measureSensivity; i++) {
        vv = analogRead(p) * (5.0 / 1023.0);
        sum += vv;
      }
      vv = sum / measureSensivity;
      sum = 0;
      return vv;
    }

  public:
    voltage(int ppin, double rr1 = 0, double rr2 = 0) {
      pin = ppin;
      r1 = rr1;
      r2 = rr2;
      pinMode(pin, INPUT);
    }
    double read() {
      double vs = toVoltage(pin);
      if ((r1 != 0) && (r2 != 0))
        vs = vs * ((r1 / r2) + 1);
      return vs;
    }
};

//voltage inputVoltage(A0, 10, 4.7); // Sinir 15.63 V
//voltage espVoltage(A2, 3.3, 10); // Sinir 6.65 V
//voltage ardVoltage(A1, 1, 4.7); // Sinir 6.06 V
//voltage rtcVoltage(A3,4.7,15); // Sinir 6.56 V

voltage inputVoltage(A0, 10, 1); // Sinir 15.63 V
voltage ardVoltage(A1, 10, 4.7); // Sinir 6.06 V
voltage espVoltage(A2, 3.3, 4.7); // Sinir 6.65 V
voltage rtcVoltage(A3, 4.7, 15); // Sinir 6.56 V

double voltageRead(int d) {
  double v = -1;
  if (d == 0) {
    v = inputVoltage.read();
  }
  else if (d == 1) {
    v = ardVoltage.read();
  }
  else if (d == 2) {
    v = espVoltage.read();
  }
  else if (d == 3) {
    v = rtcVoltage.read();
  }
  return v;
}


#endif
