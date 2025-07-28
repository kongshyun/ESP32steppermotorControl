#pragma once
#include <Arduino.h>

class SerialManager {
private:
    bool readySent;
    unsigned long readyStartTime;
    
public:
    SerialManager();
    void begin();
    bool hasCommand();
    String readCommand();
    void sendResponse(const String& response);
    void sendStartupReady();
    void sendStatus(const String& status);
};