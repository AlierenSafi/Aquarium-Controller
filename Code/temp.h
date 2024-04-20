#ifndef __TEMP_H
#define __TEMP_H

#define DS18B20_IN_VCC 31
#define DS18B20_OUT_PIN 27
#define DS18B20_IN_PIN 33

#include <OneWire.h>  // OneWire kütüphanesini ekliyoruz.
enum dereceCinsi { C,
                   F
                 };
enum sensors { OUT,
               IN
             };

class sicaklik {
    int DS18B20_Pin;
  public:
    sicaklik(int pp) {
      DS18B20_Pin = pp;
    }
    OneWire sensor() {
      OneWire ds(DS18B20_Pin);
      return ds;
    }
    float oku(int d) {
      float ss = getTemp(sensor(), d);
      return ss;
    }
    float getTemp(OneWire sensor, int d) {
      //returns the temperature from one DS18S20 in DEG Celsius
      byte data[12];
      byte addr[8];
      if (!sensor.search(addr)) {
        //no more sensors on chain, reset search
        sensor.reset_search();
        return -1000;
      }
      if (OneWire::crc8(addr, 7) != addr[7]) {
        //Serial.println("CRC is not valid!");
        return -1000;
      }
      if (addr[0] != 0x10 && addr[0] != 0x28) {
        //Serial.print("Device is not recognized");
        return -1000;
      }
      sensor.reset();
      sensor.select(addr);
      sensor.write(0x44, 1);  // start conversion, with parasite power on at the end
      byte present = sensor.reset();
      present = present;
      sensor.select(addr);
      sensor.write(0xBE);  // Read Scratchpad

      for (int i = 0; i < 9; i++) {  // we need 9 bytes
        data[i] = sensor.read();
      }
      sensor.reset_search();
      byte MSB = data[1];
      byte LSB = data[0];
      float tempRead = ((MSB << 8) | LSB);  //using two's compliment
      float TemperatureSum = tempRead / 16;

      if (d == 0) {
        return TemperatureSum;
      }
      if (d == 1) {
        return ((TemperatureSum * 1.8) + 32);
      } else {
        return -1000;
      }
    }
};

sicaklik disari(DS18B20_OUT_PIN);
sicaklik iceri(DS18B20_IN_PIN);

float Temp(int d, int f) {
  float v = -100;
  if (d == 0) {
    v = disari.oku(f);
  } else if (d == 1) {
    pinMode(DS18B20_IN_VCC, OUTPUT);
    digitalWrite(DS18B20_IN_VCC, HIGH);
    delay(100);
    v = iceri.oku(f);
    delay(50);
    digitalWrite(DS18B20_IN_VCC, LOW);
  }
  return v;
}

#endif
