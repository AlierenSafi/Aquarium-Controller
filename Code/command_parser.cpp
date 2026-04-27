#include "command_parser.h"
#include "globals.h"
#include "config.h"
#include "mutex.h"
#include "relays.h"
#include "actuator.h"
#include "logger.h"
#include "alarm_manager.h"
#include "rtc_driver.h"
#include "voltage.h"
#include <EEPROM.h>
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

CommandParser cmdParser;

static char _ob[64]; /* output scratch - increased from 48 */

void CommandParser::_out(const char* s) {
    // gActivePort: Which port did the message come from?
    // 0 = USB/Serial, 1 = Bluetooth/Serial3
    if (gActivePort == 1) {
        // Respond to Bluetooth
        Serial3.println(s);
    } else {
        // Respond to USB (default)
        Serial.println(s);
    }
}

void CommandParser::_help() {
    _out("");
    _out("========================================");
    _out("  AQUARIUM CONTROL SYSTEM - COMMANDS");
    _out("========================================");
    _out("");
    _out("--- GENERAL COMMANDS ---");
    _out("  help         : Show this help message");
    _out("  status       : Show system status");
    _out("  time         : Show date and time");
    _out("  time DD/MM/YYYY HH:MM:SS : Set date and time");
    _out("  diag         : Hardware diagnostics");
    _out("");
    _out("--- TEMPERATURE COMMANDS ---");
    _out("  get temp              : Show temperature values");
    _out("  set tempunit c|f      : Temperature unit (Celsius/Fahrenheit)");
    _out("  set targettemp 26.0   : Set target temperature");
    _out("");
    _out("--- FAN COMMANDS ---");
    _out("  get fan               : Show fan speed");
    _out("  fan auto              : Automatic fan control");
    _out("  fan speed 50          : Set fan speed to 50% (0-100)");
    _out("");
    _out("--- RELAY COMMANDS ---");
    _out("  get relay             : Show relay status");
    _out("  relay on 1            : Turn on relay 1 (1-8)");
    _out("  relay off 1           : Turn off relay 1");
    _out("  relay toggle 1        : Toggle relay 1");
    _out("  relay all on          : Turn all relays on");
    _out("  relay all off         : Turn all relays off");
    _out("");
    _out("--- ALARM COMMANDS ---");
    _out("  alarm                 : Show alarm thresholds");
    _out("  alarm recover         : Clear all alarms");
    _out("");
    _out("--- LOG COMMANDS ---");
    _out("  log all               : Show all log entries");
    _out("  log last 10           : Show last 10 log entries");
    _out("  log clear             : Clear log entries");
    _out("");
    _out("--- EEPROM COMMANDS ---");
    _out("  eeprom map            : Show EEPROM map");
    _out("  eeprom clear          : Clear EEPROM (Warning!)");
    _out("--- LED COMMANDS ---");
    _out("  leds enable 0|1       : Enable/disable LEDs (0=off, 1=on)");
    _out("  leds status           : Show LED status");
    _out("");
    _out("--- RGB LED STRIP COMMANDS ---");
    _out("  rgb                   : Show current RGB status");
    _out("  rgb on / white        : White light");
    _out("  rgb off               : Turn off LED");
    _out("  rgb red|green|blue    : Basic colors");
    _out("  rgb yellow|cyan|magenta|orange|purple : Other colors");
    _out("  rgb rgb 255 128 0     : RGB values (0-255)");
    _out("  rgb brightness 50     : Set brightness to 50% (0-100)");
    _out("");
    _out("========================================");
    _out("");
}

void CommandParser::process(char* line) {
    /* strip CR/LF */
    for (char* p = line; *p; p++) if (*p=='\r'||*p=='\n') { *p='\0'; break; }
    if (!*line) return;

    char* cmd = strtok(line, " ");
    char* arg = strtok(nullptr, "");
    if (!cmd) return;

    if (!strcmp(cmd,"help")||!strcmp(cmd,"?")) {
        _help();
    } else if (!strcmp(cmd,"status"))  { _status(); }
    else if (!strcmp(cmd,"get"))       { _get(arg); }
    else if (!strcmp(cmd,"set")) {
        char* key = strtok(arg," ");
        char* val = strtok(nullptr,"");
        _set(key,val);
    }
    else if (!strcmp(cmd,"relay"))     { _relay(arg); }
    else if (!strcmp(cmd,"fan"))       { _fan(arg); }
    else if (!strcmp(cmd,"log"))       { _logCmd(arg); }
    else if (!strcmp(cmd,"alarm"))     { _alarmCmd(arg); }
    else if (!strcmp(cmd,"eeprom"))    { _eepromCmd(arg); }
    else if (!strcmp(cmd,"leds"))      { _ledsCmd(arg); }
    else if (!strcmp(cmd,"rgb"))       { _rgbCmd(arg); }
    else if (!strcmp(cmd,"strip"))     { _rgbCmd(arg); }
    else if (!strcmp(cmd,"diag"))      { _diag(); }
    else if (!strcmp(cmd,"time")) {
        if (!arg) {
            // Just show time
            snprintf(_ob,sizeof(_ob),"%02u:%02u:%02u %02u/%02u/%04u",
                     gHour,gMinute,gSecond,gDay,gMonth,gYear);
            _out(_ob);
        } else {
            // Set time: time DD/MM/YYYY HH:MM:SS
            uint8_t d=1, mo=1, h=0, m=0, s=0;
            uint16_t y=2024;
            sscanf(arg,"%hhu/%hhu/%hu %hhu:%hhu:%hhu",&d,&mo,&y,&h,&m,&s);
            rtcDrv.setDateTime(y, mo, d, h, m, s);
            _out("OK:time set");
        }
    } else { _out("ERR:unknown"); }
}

void CommandParser::_status() {
    // Temperature values
    char bufIn[16], bufOut[16];
    dtostrf(gTempIn, 4, 1, bufIn);
    dtostrf(gTempOut, 4, 1, bufOut);
    snprintf(_ob, sizeof(_ob), "Tin=%sC Tout=%sC", bufIn, bufOut);
    _out(_ob);
    
    // Fan and Relay
    snprintf(_ob, sizeof(_ob), "Fan=%u%% Relay=0x%02X",
             (unsigned)(gFanDuty * 100 / 255), gRelayMask);
    _out(_ob);
    
    // Alarm status and details
    bool hasAlarm = false;
    char alarmMsg[64] = "";
    
    if (alarmMgr.isActive(ALM_TEMP_HIGH)) {
        strcat(alarmMsg, "TH ");
        hasAlarm = true;
    }
    if (alarmMgr.isActive(ALM_TEMP_LOW)) {
        strcat(alarmMsg, "TL ");
        hasAlarm = true;
    }
    if (alarmMgr.isActive(ALM_VOLT_LOW)) {
        strcat(alarmMsg, "VL ");
        hasAlarm = true;
    }
    if (alarmMgr.isActive(ALM_BATT_LOW)) {
        strcat(alarmMsg, "BL ");
        hasAlarm = true;
    }
    
    if (hasAlarm) {
        snprintf(_ob, sizeof(_ob), "ALARM: %s", alarmMsg);
        _out(_ob);
        
        // Detailed alarm reasons
        if (alarmMgr.isActive(ALM_TEMP_HIGH)) {
            char buf[16];
            dtostrf(gAlarmCfg.tempHigh, 4, 1, buf);
            snprintf(_ob, sizeof(_ob), "  - High temp! (>%sC)", buf);
            _out(_ob);
        }
        if (alarmMgr.isActive(ALM_TEMP_LOW)) {
            char buf[16];
            dtostrf(gAlarmCfg.tempLow, 4, 1, buf);
            snprintf(_ob, sizeof(_ob), "  - Low temp! (<%sC)", buf);
            _out(_ob);
        }
        if (alarmMgr.isActive(ALM_VOLT_LOW)) {
            char buf[16];
            dtostrf(gAlarmCfg.volt12Low, 4, 1, buf);
            snprintf(_ob, sizeof(_ob), "  - Low voltage! (<%sV)", buf);
            _out(_ob);
        }
        if (alarmMgr.isActive(ALM_BATT_LOW)) {
            char buf[16];
            dtostrf(gAlarmCfg.voltBatLow, 4, 1, buf);
            snprintf(_ob, sizeof(_ob), "  - Low battery! (<%sV)", buf);
            _out(_ob);
        }
    } else {
        _out("Status: NORMAL (No alarms)");
    }
}

void CommandParser::_get(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    if (!strcmp(arg,"temp")) {
        // Arduino AVR snprintf doesn't support %f - use dtostrf
        char bufIn[16], bufOut[16];
        dtostrf(gTempIn, 4, 1, bufIn);
        dtostrf(gTempOut, 4, 1, bufOut);
        snprintf(_ob,sizeof(_ob),"in=%s out=%s unit=%c",
                 bufIn, bufOut, gTempF?'F':'C');
        _out(_ob);
    } else if (!strcmp(arg,"fan")) {
        snprintf(_ob,sizeof(_ob),"fan=%u%%",(unsigned)(gFanDuty*100/255));
        _out(_ob);
    } else if (!strcmp(arg,"relay")) {
        snprintf(_ob,sizeof(_ob),"relay=0x%02X",gRelayMask);
        _out(_ob);
    } else if (!strcmp(arg,"time")) {
        snprintf(_ob,sizeof(_ob),"%02u:%02u:%02u",gHour,gMinute,gSecond);
        _out(_ob);
    } else { _out("ERR:unknown"); }
}

void CommandParser::_set(char* key, char* val) {
    if (!key||!val) { _out("ERR:arg"); return; }
    if (!strcmp(key,"tempunit")) {
        gTempF = (val[0]=='f'||val[0]=='F');
        gSettings.tempUnit = gTempF ? 1 : 0;
        EEPROM.put(EE_SETTINGS, gSettings);
        _out("OK");
    } else if (!strcmp(key,"targettemp")) {
        gSettings.targetTemp = (float)atof(val);
        EEPROM.put(EE_SETTINGS, gSettings);
        _out("OK");
    } else if (!strcmp(key,"time")) {
        /* format: HH:MM:SS */
        uint8_t h=0,m=0,s=0;
        sscanf(val,"%hhu:%hhu:%hhu",&h,&m,&s);
        rtcDrv.setDateTime(gYear,gMonth,gDay,h,m,s);
        _out("OK");
    } else { _out("ERR:unknown"); }
}

void CommandParser::_relay(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    char* sub = strtok(arg," ");
    char* num = strtok(nullptr," ");

    if (!strcmp(sub,"all")) {
        if (num && !strcmp(num,"on"))  relayMgr.allOn();
        else                           relayMgr.allOff();
        _out("OK"); return;
    }
    if (!num) { _out("ERR:idx"); return; }
    uint8_t idx = (uint8_t)(atoi(num) - 1);
    if (idx >= 8) { _out("ERR:range"); return; }
    if      (!strcmp(sub,"on"))     relayMgr.on(idx);
    else if (!strcmp(sub,"off"))    relayMgr.off(idx);
    else if (!strcmp(sub,"toggle")) relayMgr.toggle(idx);
    else { _out("ERR:sub"); return; }
    _out("OK");
}

void CommandParser::_fan(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    char* sub = strtok(arg," ");
    char* val = strtok(nullptr," ");
    if (!strcmp(sub,"auto")) {
        actuators.autoFan(gTempIn);
        _out("OK");
    } else if (!strcmp(sub,"speed")&&val) {
        actuators.setFanPercent((uint8_t)atoi(val));
        _out("OK");
    } else { _out("ERR:arg"); }
}

void CommandParser::_logCmd(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    if (!strcmp(arg,"all"))        { logger.printAll(); }
    else if (!strcmp(arg,"clear")) { logger.clear(); _out("OK"); }
    else {
        char* sub = strtok(arg," ");
        char* n   = strtok(nullptr," ");
        if (!strcmp(sub,"last")&&n) logger.printLast((uint8_t)atoi(n));
        else _out("ERR:arg");
    }
}

void CommandParser::_alarmCmd(char* arg) {
    if (!arg) {
        // Convert float values to string (snprintf %f issue)
        char bufTH[16], bufTL[16], bufVL[16], bufBL[16];
        dtostrf(gAlarmCfg.tempHigh, 4, 1, bufTH);
        dtostrf(gAlarmCfg.tempLow, 4, 1, bufTL);
        dtostrf(gAlarmCfg.volt12Low, 4, 1, bufVL);
        dtostrf(gAlarmCfg.voltBatLow, 4, 1, bufBL);
        
        snprintf(_ob,sizeof(_ob),"TH=%s TL=%s VL=%s BL=%s",
                 bufTH, bufTL, bufVL, bufBL);
        _out(_ob);
        
        // Add description
        _out("TH: Temp High, TL: Temp Low");
        _out("VL: Volt Low, BL: Bat Low");
        return;
    }
    if (!strcmp(arg,"recover")) { alarmMgr.recoverAll(); _out("OK"); }
    else _out("ERR:arg");
}

void CommandParser::_ledsCmd(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    
    char* sub = strtok(arg, " ");
    char* val = strtok(nullptr, " ");
    
    if (!strcmp(sub, "enable") && val) {
        uint8_t enable = (uint8_t)atoi(val);
        gLedsEnabled = (enable != 0);
        _out(gLedsEnabled ? "LEDs enabled" : "LEDs disabled");
    } else if (!strcmp(sub, "status")) {
        snprintf(_ob, sizeof(_ob), "LEDs: %s", gLedsEnabled ? "ON" : "OFF");
        _out(_ob);
    } else {
        _out("ERR:arg (enable 0|1 / status)");
    }
}

void CommandParser::_eepromCmd(char* arg) {
    if (!arg) { _out("ERR:arg"); return; }
    if (!strcmp(arg,"clear")) {
        if (MUTEX_TAKE(xEEPROMMutex)) {
            for (uint16_t i=0;i<0x0800;i++) EEPROM.update(i,0xFF);
            MUTEX_GIVE(xEEPROMMutex);
        }
        _out("OK:cleared");
    } else if (!strcmp(arg,"map")) {
        _out("0000 Header  0020 Settings  0060 Scenarios");
        _out("0250 Schedules  03F0 AlarmCfg  0410 Log  0510 NTP");
    } else { _out("ERR:arg"); }
}

void CommandParser::_diag() {
    snprintf(_ob,sizeof(_ob),"RTC:%s BT:%s",
             rtcDrv.isRunning()?"OK":"FAIL",
             digitalRead(PIN_BT_STATE)?"CONN":"DISC");
    _out(_ob);
    
    // Read raw ADC values
    uint16_t raw12V = analogRead(A0);
    uint16_t raw5V = analogRead(A1);
    uint16_t raw3V3 = analogRead(A2);
    uint16_t rawBat = analogRead(A3);
    
    snprintf(_ob,sizeof(_ob),"Raw ADC: A0=%u A1=%u A2=%u A3=%u", raw12V, raw5V, raw3V3, rawBat);
    _out(_ob);
    
    // Read with new voltage classes
    double v12 = inputVoltage.read();
    double v5 = ardVoltage.read();
    double v33 = espVoltage.read();
    double vb = rtcVoltage.read();
    
    // Convert float values to string
    char buf12[16], buf5[16], buf33[16], bufBat[16];
    dtostrf(v12, 4, 2, buf12);
    dtostrf(v5, 4, 2, buf5);
    dtostrf(v33, 4, 2, buf33);
    dtostrf(vb, 4, 2, bufBat);
    
    snprintf(_ob,sizeof(_ob),"V12=%s V5=%s V33=%s Vbat=%s", buf12, buf5, buf33, bufBat);
    _out(_ob);
    
    snprintf(_ob,sizeof(_ob),"Log:%u/%u Fan:%u%%",
             logger.count(),LOG_SIZE,(unsigned)(gFanDuty*100/255));
    _out(_ob);
}

void CommandParser::_rgbCmd(char* arg) {
    if (!arg) {
        // Show current status
        char buf[48];
        rgbStrip.getColorString(buf, sizeof(buf));
        _out(buf);
        return;
    }
    
    char* cmd = strtok(arg, " ");
    if (!cmd) { _out("ERR:arg"); return; }
    
    // Simple color commands
    if (!strcmp(cmd, "off")) {
        rgbStrip.setOff();
        _out("OK:off");
    }
    else if (!strcmp(cmd, "on") || !strcmp(cmd, "white")) {
        rgbStrip.setWhite();
        _out("OK:white");
    }
    else if (!strcmp(cmd, "red")) {
        rgbStrip.setRedColor();
        _out("OK:red");
    }
    else if (!strcmp(cmd, "green")) {
        rgbStrip.setGreenColor();
        _out("OK:green");
    }
    else if (!strcmp(cmd, "blue")) {
        rgbStrip.setBlueColor();
        _out("OK:blue");
    }
    else if (!strcmp(cmd, "yellow")) {
        rgbStrip.setYellow();
        _out("OK:yellow");
    }
    else if (!strcmp(cmd, "cyan")) {
        rgbStrip.setCyan();
        _out("OK:cyan");
    }
    else if (!strcmp(cmd, "magenta")) {
        rgbStrip.setMagenta();
        _out("OK:magenta");
    }
    else if (!strcmp(cmd, "orange")) {
        rgbStrip.setOrange();
        _out("OK:orange");
    }
    else if (!strcmp(cmd, "purple")) {
        rgbStrip.setPurple();
        _out("OK:purple");
    }
    // RGB values: rgb 255 0 0
    else if (!strcmp(cmd, "rgb")) {
        char* rval = strtok(nullptr, " ");
        char* gval = strtok(nullptr, " ");
        char* bval = strtok(nullptr, " ");
        if (rval && gval && bval) {
            uint8_t r = (uint8_t)atoi(rval);
            uint8_t g = (uint8_t)atoi(gval);
            uint8_t b = (uint8_t)atoi(bval);
            rgbStrip.setRGB(r, g, b);
            _out("OK");
        } else {
            _out("ERR:rgb R G B (0-255)");
        }
    }
    // Brightness: rgb brightness 50
    else if (!strcmp(cmd, "brightness") || !strcmp(cmd, "bright")) {
        char* val = strtok(nullptr, " ");
        if (val) {
            uint8_t pct = (uint8_t)atoi(val);
            if (pct > 100) pct = 100;
            rgbStrip.setBrightness(pct);
            snprintf(_ob, sizeof(_ob), "OK:brightness %u%%", (unsigned)pct);
            _out(_ob);
        } else {
            snprintf(_ob, sizeof(_ob), "Brightness: %u%%", (unsigned)rgbStrip.getBrightness());
            _out(_ob);
        }
    }
    else {
        _out("ERR:off/on/white/red/green/blue/yellow/cyan/magenta/orange/purple/rgb/brightness");
    }
}
