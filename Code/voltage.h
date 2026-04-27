#pragma once
#include <Arduino.h>

/* Voltaj olcum siniflari */
enum voltages {SOURCE, MAIN_REGULATOR, ESP_REGULATOR, RTC_BAT};

class VoltageChannel {
private:
    uint8_t pin;
    uint8_t measureSensitivity;
    double r1, r2;

    double toVoltage() {
        double sum = 0;
        double vv;
        for (int i = 0; i < measureSensitivity; i++) {
            vv = analogRead(pin) * (5.0 / 1023.0);
            sum += vv;
        }
        vv = sum / measureSensitivity;
        return vv;
    }

public:
    VoltageChannel(uint8_t ppin, double rr1 = 0, double rr2 = 0, uint8_t sens = 10) {
        pin = ppin;
        r1 = rr1;
        r2 = rr2;
        measureSensitivity = sens;
        // pinMode(pin, INPUT); // Kaldırıldı - analog pinler varsayılan olarak INPUT
    }

    double read() {
        double vs = toVoltage();
        if ((r1 != 0) && (r2 != 0))
            vs = vs * ((r1 / r2) + 1);
        return vs;
    }
};

/* Global voltaj kanallari */
extern VoltageChannel inputVoltage;   // A0 - 12V giris
extern VoltageChannel ardVoltage;     // A1 - Arduino 5V
extern VoltageChannel espVoltage;     // A2 - ESP 3.3V
extern VoltageChannel rtcVoltage;     // A3 - RTC batarya

/* Geriye uyumlu fonksiyon */
double voltageRead(int d);

/* Eski VoltageMonitor sinifi - geriye uyumluluk icin */
class VoltageMonitor {
public:
    void begin();
    float read12V();
    float read5V();
    float read3V3();
    float readBat();
};

extern VoltageMonitor voltmon;
