#pragma once
#include <Arduino.h>
#include <stdint.h>

/* ── Pins ── */
#define PIN_DS18B20_IN   33
#define PIN_DS18B20_OUT  27
#define PIN_DS18B20_VCC  31
#define PIN_FAN          44
#define PIN_BUZZER       25
#define PIN_IR           35
#define PIN_BT_STATE     22
#define PIN_LED_RED      29   /* PA7 */
#define PIN_LED_GREEN    30   /* PC7 */
#define PIN_RGB_RED      10   /* RGB Strip Red */
#define PIN_RGB_GREEN    9    /* RGB Strip Green */
#define PIN_RGB_BLUE     8    /* RGB Strip Blue */
#define PIN_RTC_SQW      18
#define ADC_12V          A0
#define ADC_5V           A1
#define ADC_3V3          A2
#define ADC_BAT          A3

// RELAY_PINS now defined in globals.cpp (static removed)
extern const uint8_t RELAY_PINS[8];

/* ── FreeRTOS (Not used - FreeRTOS-free version) ── */
// #define STACK_SERIAL   200
// #define STACK_SENSORS   80
// #define STACK_DISPLAY   80
// #define STACK_SYSTEM    60
// #define PRI_SERIAL       3
// #define PRI_SENSORS      2
// #define PRI_DISPLAY      4
// #define PRI_SYSTEM       5

/* ── EEPROM map ── */
enum EEAddr : uint16_t {
    EE_HEADER    = 0x0000,   /* 32B  */
    EE_SETTINGS  = 0x0020,   /* 64B  */
    EE_SCENARIOS = 0x0060,   /* 480B = 10×48 */
    EE_SCHEDULES = 0x0250,   /* 400B = 20×20 */
    EE_ALARM_CFG = 0x03F0,   /* 32B  */
    EE_LOG       = 0x0410,   /* 256B = 2+40×6 */
    EE_NTP       = 0x0510,   /* 16B  */
};
#define EE_MAGIC 0xAA

/* ── Structs ── */
struct __attribute__((packed)) LogEntry {
    uint8_t code, hour, minute;
    int8_t  temp;
    uint8_t data, day;
}; /* 6B */
static_assert(sizeof(LogEntry) == 6, "");
#define LOG_SIZE 40

struct AlarmCfg {
    float   tempHigh, tempLow, volt12Low, voltBatLow;
    uint8_t enabled;
    uint8_t _p[7];
}; /* 24B */
// static_assert(sizeof(AlarmCfg) == 24, "AlarmCfg size mismatch");

struct SysSettings {
    float   targetTemp, hysteresis;
    uint8_t tempUnit;   /* 0=C 1=F */
    uint8_t _p[3];
}; /* 12B */
// static_assert(sizeof(SysSettings) == 12, "SysSettings size mismatch");

struct Scenario {
    char    name[16];
    uint8_t relayMask, fanSpeed; /* fanSpeed 0-100 */
    float   targetTemp;
    uint8_t active;
    // Padding will be calculated automatically
};
// static_assert(sizeof(Scenario) == 28, "Scenario size mismatch");

struct Schedule {
    uint8_t scenIdx, dayMask, hour, minute, active;
    uint8_t _p[3];
}; /* 8B */
// static_assert(sizeof(Schedule) == 8, "Schedule size mismatch");

/* ── Enums ── */
enum EventCode : uint8_t {
    EVT_NONE=0, EVT_BOOT, EVT_TEMP_HIGH, EVT_TEMP_LOW,
    EVT_VOLT_LOW, EVT_BATT_LOW, EVT_RELAY_CHG,
    EVT_SCENARIO, EVT_ALARM_RECOVER, EVT_SCHEDULE,
};
enum AlarmType : uint8_t {
    ALM_TEMP_HIGH=0, ALM_TEMP_LOW, ALM_VOLT_LOW, ALM_BATT_LOW, ALM_COUNT
};
enum AlarmState : uint8_t {
    ALARM_IDLE=0, ALARM_PENDING, ALARM_ACTIVE,
};

/* ── Constants ── */
// V12_RATIO removed - using resistor values in voltage.cpp
#define VREF        5.0f
#define ADC_MAXF    1023.0f
#define ADC_SAMPLES 10
