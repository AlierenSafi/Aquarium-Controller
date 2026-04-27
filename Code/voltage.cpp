#include "voltage.h"

/* Voltaj kanallari tanimlari
 * A0 - 12V giris (r1=10k, r2=4.7k) - Sinir ~15.63V
 * A1 - Arduino 5V (r1=10k, r2=4.7k) - Sinir ~6.06V  
 * A2 - ESP 3.3V (r1=3.3k, r2=4.7k) - Sinir ~6.65V
 * A3 - RTC batarya (r1=4.7k, r2=15k) - Sinir ~6.56V
 */
VoltageChannel inputVoltage(A0, 10.0, 4.7);   // 12V giris - Sinir ~15.63V
VoltageChannel ardVoltage(A1, 10.0, 4.7);     // Arduino 5V - Sinir ~6.06V
VoltageChannel espVoltage(A2, 3.3, 4.7);      // ESP 3.3V - Sinir ~6.65V
VoltageChannel rtcVoltage(A3, 4.7, 15.0);     // RTC batarya - Sinir ~6.56V

VoltageMonitor voltmon;

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

/* Eski VoltageMonitor fonksiyonlari - geriye uyumluluk */
void VoltageMonitor::begin() {
    // Pinler VoltageChannel constructor'inda yapilandirildi
}

float VoltageMonitor::read12V() {
    return (float)inputVoltage.read();
}

float VoltageMonitor::read5V() {
    return (float)ardVoltage.read();
}

float VoltageMonitor::read3V3() {
    return (float)espVoltage.read();
}

float VoltageMonitor::readBat() {
    return (float)rtcVoltage.read();
}
