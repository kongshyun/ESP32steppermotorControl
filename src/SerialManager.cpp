#include "SerialManager.h"

SerialManager::SerialManager() : readySent(false), readyStartTime(0) {}

void SerialManager::begin() {
    Serial.begin(115200);
    readyStartTime = millis();
}

bool SerialManager::hasCommand() {
    return Serial.available();
}

String SerialManager::readCommand() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        return input;
    }
    return "";
}

void SerialManager::sendResponse(const String& response) {
    Serial.println(response);
}

void SerialManager::sendStartupReady() {
    if (!readySent && millis() - readyStartTime < 3000) {
        Serial.println("ESP32 READY");
        delay(200);
    } else {
        readySent = true;
    }
}

void SerialManager::sendStatus(const String& status) {
    Serial.println(status);
}