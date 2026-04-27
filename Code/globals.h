#pragma once
#include "config.h"
#include "rgb_strip.h"
#include <LiquidCrystal_I2C_AvrI2C.h>

extern volatile float    gTempIn, gTempOut;
extern volatile uint8_t  gFanDuty;
extern volatile uint8_t  gHour, gMinute, gSecond;
extern volatile uint8_t  gDay, gMonth, gDow;
extern volatile uint16_t gYear;
extern volatile bool     gTempF;
extern volatile uint8_t  gRelayMask;
extern volatile uint8_t  gActivePort;  // 0: USB, 1: Bluetooth
extern volatile bool     gLedsEnabled; // true: LED'ler aktif, false: LED'ler kapali

extern SysSettings gSettings;
extern AlarmCfg    gAlarmCfg;
extern Scenario    gScenarios[10];
extern Schedule    gSchedules[20];

// LCD nesnesi
extern LiquidCrystal_I2C_AvrI2C lcd;
