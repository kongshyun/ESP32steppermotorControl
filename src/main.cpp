
#include <Arduino.h>
#include "SerialManager.h"
#include "MotorController.h"

const int led1Pin = 2;   // Hi 명령 수신 시 (내장 LED)
const int led2Pin = 4;   // RPM ROT 명령 시
const int led3Pin = 5;   // RPM TIME 명령 시  
const int led4Pin = 15;  // STOP 명령 시 깜빡이기

bool led4Blinking = false;
unsigned long led4BlinkTime = 0;
bool led4State = false;

// LED 깜빡이기 for running status
bool runningLedBlink = false;
unsigned long runningLedBlinkTime = 0;
bool runningLedState = false;

SerialManager serialManager;
MotorController motorController;

void setup() {
  serialManager.begin();
  motorController.begin();
  
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);
  
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  digitalWrite(led3Pin, LOW);
  digitalWrite(led4Pin, LOW);
  
}

void loop() {
  serialManager.sendStartupReady();
  
  // Update motor controller (handles step generation)
  motorController.update();
  
  // LED 깜빡이기 처리 - 테스트 모드에서 모터 동작 시뮬레이션
  if (motorController.isTestMode() && motorController.isMotorRunning()) {
    // Running 상태일 때 해당 LED 깜빡이기
    if (millis() - runningLedBlinkTime > 200) {  // 빠른 깜빡임
      runningLedState = !runningLedState;
      // 현재 상태에 따라 LED2 또는 LED3 깜빡이기
      String status = motorController.getStatus();
      if (status.indexOf("ROTATING") != -1) {
        digitalWrite(led2Pin, runningLedState);
      } else if (status.indexOf("TIME_MODE") != -1) {
        digitalWrite(led3Pin, runningLedState);
      }
      runningLedBlinkTime = millis();
    }
  } else if (motorController.isTestMode() && !motorController.isMotorRunning()) {
    // 모터가 정지했을 때 모든 LED 끄기 (LED4 제외)
    String status = motorController.getStatus();
    if (status.indexOf("DONE") != -1 || status.indexOf("STOPPED") != -1) {
      if (!led4Blinking) {  // STOP 명령으로 깜빡이지 않을 때만
        digitalWrite(led2Pin, LOW);
        digitalWrite(led3Pin, LOW);
      }
    }
  }
  
  // LED4 깜빡이기 처리 (STOP 명령)
  if (led4Blinking) {
    if (millis() - led4BlinkTime > 500) {
      led4State = !led4State;
      digitalWrite(led4Pin, led4State);
      led4BlinkTime = millis();
    }
  }
  
  if (serialManager.hasCommand()) {
    String input = serialManager.readCommand();
    
    if (input == "HELLO") {
      serialManager.sendResponse("READY");
    }
    else if (input == "HI") {
      digitalWrite(led1Pin, HIGH);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      serialManager.sendResponse("Hi_RECEIVED");
    }
    else if (input.startsWith("SPEED:") && input.indexOf(" ROT:") != -1) {
      int speedIndex = input.indexOf("SPEED:") + 6;
      int rotIndex = input.indexOf(" ROT:") + 5;
      int dirIndex = input.indexOf(" DIR:");
      
      int speedLevel = input.substring(speedIndex, input.indexOf(" ROT:")).toInt();
      int rotations = 0;
      bool clockwise = true; // default to CW
      
      if (dirIndex != -1) {
        // DIR parameter exists
        rotations = input.substring(rotIndex, dirIndex).toInt();
        String direction = input.substring(dirIndex + 5);
        direction.trim();
        clockwise = (direction == "CW");
      } else {
        // No DIR parameter, use original parsing
        rotations = input.substring(rotIndex).toInt();
      }
      
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, HIGH);
      digitalWrite(led3Pin, LOW);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      
      motorController.executeRotationWithSpeed(speedLevel, rotations, clockwise);
    }
    else if (input.startsWith("SPEED:") && input.indexOf(" TIME:") != -1) {
      int speedIndex = input.indexOf("SPEED:") + 6;
      int timeIndex = input.indexOf(" TIME:") + 6;
      int dirIndex = input.indexOf(" DIR:");
      
      int speedLevel = input.substring(speedIndex, input.indexOf(" TIME:")).toInt();
      int duration = 0;
      bool clockwise = true; // default to CW
      
      if (dirIndex != -1) {
        // DIR parameter exists
        duration = input.substring(timeIndex, dirIndex).toInt();
        String direction = input.substring(dirIndex + 5);
        direction.trim();
        clockwise = (direction == "CW");
      } else {
        // No DIR parameter, use original parsing
        duration = input.substring(timeIndex).toInt();
      }
      
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, HIGH);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      
      motorController.executeTimeWithSpeed(speedLevel, duration, clockwise);
    }
    else if (input.startsWith("RPM:") && input.indexOf(" ROT:") != -1) {
      int rpmIndex = input.indexOf("RPM:") + 4;
      int rotIndex = input.indexOf(" ROT:") + 5;
      int dirIndex = input.indexOf(" DIR:");
      
      int rpm = input.substring(rpmIndex, input.indexOf(" ROT:")).toInt();
      int rotations = 0;
      bool clockwise = true; // default to CW
      
      if (dirIndex != -1) {
        // DIR parameter exists
        rotations = input.substring(rotIndex, dirIndex).toInt();
        String direction = input.substring(dirIndex + 5);
        direction.trim();
        clockwise = (direction == "CW");
      } else {
        // No DIR parameter, use original parsing
        rotations = input.substring(rotIndex).toInt();
      }
      
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, HIGH);
      digitalWrite(led3Pin, LOW);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      
      motorController.executeRotation(rpm, rotations, clockwise);
    }
    else if (input.startsWith("RPM:") && input.indexOf(" TIME:") != -1) {
      int rpmIndex = input.indexOf("RPM:") + 4;
      int timeIndex = input.indexOf(" TIME:") + 6;
      int dirIndex = input.indexOf(" DIR:");
      
      int rpm = input.substring(rpmIndex, input.indexOf(" TIME:")).toInt();
      int duration = 0;
      bool clockwise = true; // default to CW
      
      if (dirIndex != -1) {
        // DIR parameter exists
        duration = input.substring(timeIndex, dirIndex).toInt();
        String direction = input.substring(dirIndex + 5);
        direction.trim();
        clockwise = (direction == "CW");
      } else {
        // No DIR parameter, use original parsing
        duration = input.substring(timeIndex).toInt();
      }
      
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, HIGH);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      
      motorController.executeTime(rpm, duration, clockwise);
    }
    else if (input == "STOP") {
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      led4Blinking = true;
      led4BlinkTime = millis();
      
      motorController.pause();
      Serial.println("PAUSED");
    }
    else if (input == "STOPPED") {
      // Pause the motor if it's running
      if (motorController.isMotorRunning() && !motorController.isMotorPaused()) {
        motorController.pause();
        Serial.println("PAUSED");
      }
    }
    else if (input == "RELOAD") {
      // Resume the motor if it's paused
      if (motorController.isMotorRunning() && motorController.isMotorPaused()) {
        motorController.resume();
        Serial.println("RESUMED");
      }
    }
    else if (input == "CLOSE") {
      // Complete termination
      digitalWrite(led1Pin, LOW);
      digitalWrite(led2Pin, LOW);
      digitalWrite(led3Pin, LOW);
      led4Blinking = false;
      digitalWrite(led4Pin, LOW);
      
      motorController.stop();
      Serial.println("CLOSED");
    }
    else if (input == "STATUS") {
      serialManager.sendResponse(motorController.getStatus());
    }
  }
}
