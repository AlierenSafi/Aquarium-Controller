#include <Arduino.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <EEPROM.h>

#include "config.h"
#include "globals.h"
#include "scheduler.h"
#include "mutex.h"

// Module headers
#include "relays.h"
#include "actuator.h"
#include "voltage.h"
#include "rtc_driver.h"
#include "logger.h"
#include "alarm_manager.h"
#include "command_parser.h"
#include "ds18b20.h"
#include "time_utils.h"
#include "rgb_strip.h"

/* ============================================================
 * AQUARIUM CONTROL SYSTEM - FreeRTOS-free Version
 * ============================================================ */

// Timers
Timer timerSensors(600000);   // 10 minutes
Timer timerDisplay(1000);     // 1 second
Timer timerSystem(5000);      // 5 seconds
Timer timerSerial(50);        // 50ms
Timer timerAlarmLed(500);     // 500ms - alarm LED blinking

// DS18B20 sensors
DS18B20 sensIn(PIN_DS18B20_IN);
DS18B20 sensOut(PIN_DS18B20_OUT);  // DS18B20 on TinyRTC - Pin 27

// Serial port buffers - separate for USB and Bluetooth
static char serBufUSB[64];
static uint8_t serPosUSB = 0;
static char serBufBT[64];
static uint8_t serPosBT = 0;

// First boot flag
static bool firstBootSensors = true;

/* ============================================================
 * SETUP
 * ============================================================ */
void setup() {
    // Disable watchdog
    wdt_disable();
    
    // Configure LEDs
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
    
    // Serial ports
    Serial.begin(115200);
    Serial3.begin(115200);  // Bluetooth
    
    delay(1000);
    
    Serial.println("========================================");
    Serial.println("    AQUARIUM CONTROL SYSTEM v1.0");
    Serial.println("========================================");
    
    // Initialize mutex (backward compatibility)
    Serial.print("[INIT] Mutex... ");
    initMutexes();
    Serial.println("OK");
    
    // Start I2C
    Serial.print("[INIT] I2C... ");
    Wire.begin();
    Serial.println("OK");
    
    // I2C scan
    Serial.println("[INIT] I2C Scan:");
    uint8_t i2cFound = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print("       0x");
            Serial.println(addr, HEX);
            i2cFound++;
        }
    }
    if (i2cFound == 0) Serial.println("       No devices!");
    
    // Initialize LCD
    Serial.print("[INIT] LCD... ");
    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Aquarium v1.0");
    Serial.println("OK");
    
    // Initialize RTC
    Serial.print("[INIT] RTC... ");
    if (rtcDrv.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("FAIL");
    }
    
    // DS18B20 power
    Serial.print("[INIT] DS18B20... ");
    pinMode(PIN_DS18B20_VCC, OUTPUT);
    digitalWrite(PIN_DS18B20_VCC, HIGH);
    Serial.println("OK");
    
    // Other modules
    Serial.print("[INIT] Peripherals... ");
    voltmon.begin();
    relayMgr.begin();
    actuators.init(relayMgr);
    alarmMgr.begin();
    logger.begin();
    rgbStrip.begin();  // Initialize RGB LED Strip
    Serial.println("OK");
    
    // EEPROM
    Serial.print("[INIT] EEPROM... ");
    if (EEPROM.read(EE_HEADER) == EE_MAGIC) {
        EEPROM.get(EE_SETTINGS, gSettings);
        EEPROM.get(EE_ALARM_CFG, gAlarmCfg);
        EEPROM.get(EE_SCENARIOS, gScenarios);
        EEPROM.get(EE_SCHEDULES, gSchedules);
        Serial.println("OK (loaded)");
    } else {
        Serial.println("OK (first boot)");
        EEPROM.write(EE_HEADER, EE_MAGIC);
        EEPROM.put(EE_SETTINGS, gSettings);
        EEPROM.put(EE_ALARM_CFG, gAlarmCfg);
        EEPROM.put(EE_SCENARIOS, gScenarios);
        EEPROM.put(EE_SCHEDULES, gSchedules);
    }
    
    logger.log(EVT_BOOT);
    lcd.clear();
    
    Serial.println("========================================");
    Serial.println("SYSTEM READY!");
    Serial.println("Type 'help' for commands.");
    Serial.println("========================================");
    
    // Success LED
    digitalWrite(PIN_LED_GREEN, HIGH);
}

/* ============================================================
 * SENSOR TASK - Non-blocking
 * ============================================================ */
void taskSensors() {
    // Read immediately on first boot
    if (firstBootSensors) {
        firstBootSensors = false;
        
        Serial.println("[SENSORS] First boot - reading sensors...");
        
        // Read sensors (blocking - OK on first boot)
        float temp;
        if (sensIn.read(temp)) {
            gTempIn = temp;
            Serial.print("[SENSORS] TempIn: ");
            Serial.println(temp);
        } else {
            Serial.println("[SENSORS] TempIn: NOT CONNECTED");
        }
        
        if (sensOut.read(temp)) {
            gTempOut = temp;
            Serial.print("[SENSORS] TempOut: ");
            Serial.println(temp);
        } else {
            Serial.println("[SENSORS] TempOut: NOT CONNECTED");
        }
        
        // Fan control
        actuators.autoFan(gTempIn);
        
        // Start timer
        timerSensors.reset();
        return;
    }
    
    // Normal periodic reading (10 minutes)
    if (!timerSensors.check()) return;
    
    // Read sensors
    float temp;
    if (sensIn.read(temp)) {
        gTempIn = temp;
    }
    if (sensOut.read(temp)) {
        gTempOut = temp;
    }
    
    // Fan control
    actuators.autoFan(gTempIn);
    
    // Reset timer
    timerSensors.reset();
}

/* ============================================================
 * DISPLAY TASK - Every second
 * ============================================================ */
void taskDisplay() {
    if (!timerDisplay.check()) return;
    
    char buf[21];
    
    // Line 1: Time and date
    fmtTime(buf, gHour, gMinute, gSecond);
    lcd.setCursor(0,0);
    lcd.print(buf);
    lcd.print(' ');
    fmtDate(buf, gDay, gMonth, gYear);
    lcd.print(buf);
    
    // Line 2: Temperatures
    snprintf(buf, sizeof(buf), "In:%5.1f Out:%5.1f", (double)gTempIn, (double)gTempOut);
    lcd.setCursor(0,1);
    lcd.print(buf);
    
    // Line 3: Relay and Fan
    snprintf(buf, sizeof(buf), "R:0x%02X F:%3u%%", gRelayMask, (unsigned)(gFanDuty * 100 / 255));
    lcd.setCursor(0,2);
    lcd.print(buf);
    
    // Line 4: Status
    bool anyAlarm = alarmMgr.isActive(ALM_TEMP_HIGH) || alarmMgr.isActive(ALM_TEMP_LOW) ||
                    alarmMgr.isActive(ALM_VOLT_LOW) || alarmMgr.isActive(ALM_BATT_LOW);
    if (!anyAlarm) {
        lcd.setCursor(0,3);
        lcd.print("System OK           ");
    } else {
        lcd.setCursor(0,3);
        lcd.print("ALM:");
        if (alarmMgr.isActive(ALM_TEMP_HIGH)) lcd.print("TH ");
        if (alarmMgr.isActive(ALM_TEMP_LOW))  lcd.print("TL ");
        if (alarmMgr.isActive(ALM_VOLT_LOW))  lcd.print("VL ");
        if (alarmMgr.isActive(ALM_BATT_LOW))  lcd.print("BL ");
    }
}

/* ============================================================
 * SYSTEM TASK - Every 5 seconds
 * ============================================================ */
void taskSystem() {
    if (!timerSystem.check()) return;
    
    // Update RTC
    rtcDrv.getDateTime();
    
    // Read voltage and check alarms - with new classes
    float v12 = (float)inputVoltage.read();
    float vBat = (float)rtcVoltage.read();
    alarmMgr.update(gTempIn, v12, vBat);
    
    // Schedule check
    static uint8_t lastTrigIdx = 0xFF, lastTrigMin = 0xFF, lastTrigHour = 0xFF, lastTrigDow = 0xFF;
    
    for (uint8_t i = 0; i < 20; i++) {
        const Schedule& s = gSchedules[i];
        if (!s.active || s.scenIdx >= 10) continue;
        
        if (schedMatch(s.dayMask, gDow, gHour, gMinute, s.hour, s.minute)) {
            if (i == lastTrigIdx && gHour == lastTrigHour && 
                gMinute == lastTrigMin && gDow == lastTrigDow) {
                continue;
            }
            actuators.applyScenario(gScenarios[s.scenIdx]);
            logger.log(EVT_SCHEDULE, i);
            lastTrigIdx = i;
            lastTrigHour = gHour;
            lastTrigMin = gMinute;
            lastTrigDow = gDow;
        }
    }
    
    // Green LED - 100ms blink (heartbeat) - only if LEDs enabled
    if (gLedsEnabled) {
        digitalWrite(PIN_LED_GREEN, HIGH);
        delay(100);
        digitalWrite(PIN_LED_GREEN, LOW);
    }
    
    // Red LED - Alarm status (toggle) - only if LEDs enabled
    if (gLedsEnabled) {
        static bool alarmLedToggle = false;
        bool anyAlarm = alarmMgr.isActive(ALM_TEMP_HIGH) || alarmMgr.isActive(ALM_TEMP_LOW) ||
                        alarmMgr.isActive(ALM_VOLT_LOW) || alarmMgr.isActive(ALM_BATT_LOW);
        if (anyAlarm) {
            alarmLedToggle = !alarmLedToggle;
            digitalWrite(PIN_LED_RED, alarmLedToggle ? HIGH : LOW);
        } else {
            digitalWrite(PIN_LED_RED, LOW);
            alarmLedToggle = false;
        }
    } else {
        // LEDs disabled - both LEDs off
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_RED, LOW);
    }
}

/* ============================================================
 * SERIAL TASK - Every 50ms
 * ============================================================ */
void processChar(char c, char* buf, uint8_t& pos) {
    if (c == '\n' || c == '\r') {
        if (pos > 0) {
            buf[pos] = '\0';
            cmdParser.process(buf);
            pos = 0;
        }
    } else if (pos < 63) {
        buf[pos++] = c;
    }
}

void taskSerial() {
    if (!timerSerial.check()) return;
    
    // USB (Serial)
    while (Serial.available()) {
        gActivePort = 0;  // USB port active
        processChar((char)Serial.read(), serBufUSB, serPosUSB);
    }
    
    // Bluetooth (Serial3)
    while (Serial3.available()) {
        gActivePort = 1;  // Bluetooth port active
        processChar((char)Serial3.read(), serBufBT, serPosBT);
    }
}

/* ============================================================
 * MAIN LOOP
 * ============================================================ */
void loop() {
    // Run all tasks
    taskSensors();
    taskDisplay();
    taskSystem();
    taskSerial();
    
    // Short delay (to reduce CPU load)
    delay(10);
}
