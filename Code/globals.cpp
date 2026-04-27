#include "globals.h"

volatile float    gTempIn=0, gTempOut=0;
volatile uint8_t  gFanDuty=0;
volatile uint8_t  gHour=0, gMinute=0, gSecond=0;
volatile uint8_t  gDay=1, gMonth=1, gDow=0;
volatile uint16_t gYear=2024;
volatile bool     gTempF=false;
volatile uint8_t  gRelayMask=0;
volatile uint8_t  gActivePort=0;  // 0: USB, 1: Bluetooth
volatile bool     gLedsEnabled=true; // LED'ler varsayilan olarak aktif

SysSettings gSettings = {26.0f, 1.0f, 0};
AlarmCfg    gAlarmCfg = {30.0f, 20.0f, 11.0f, 2.5f, 1};
Scenario    gScenarios[10];
Schedule    gSchedules[20];

// LCD nesnesi tanimi
LiquidCrystal_I2C_AvrI2C lcd(0x27, 20, 4);

// Röle pinleri tanimi (config.h'den extern olarak bildirildi)
const uint8_t RELAY_PINS[8] = {36,38,40,42,43,41,39,37};
