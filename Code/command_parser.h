#pragma once

class CommandParser {
public:
    void process(char* line);
private:
    void _help();
    void _status();
    void _get(char* arg);
    void _set(char* key, char* val);
    void _relay(char* arg);
    void _fan(char* arg);
    void _logCmd(char* arg);
    void _alarmCmd(char* arg);
    void _eepromCmd(char* arg);
    void _ledsCmd(char* arg);
    void _rgbCmd(char* arg);
    void _diag();
    static void _out(const char* s);
};

extern CommandParser cmdParser;
