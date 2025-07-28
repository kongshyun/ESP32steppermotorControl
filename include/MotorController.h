#pragma once
#include <Arduino.h>

// Test Mode Configuration - Comment out this line when motor is connected
// #define TEST_MODE

class MotorController {
private:
    // TB6600 Driver Pins - Updated to match actual connections
    static const int ENABLE_PIN = 18;  // GPIO18 - TB6600 ENA+
    static const int DIR_PIN = 17;     // GPIO17 - TB6600 DIR+
    static const int STEP_PIN = 16;    // GPIO16 - TB6600 PUL+
    
    // Motor Parameters - Optimized for Nema23 5756 + TB6600
    static const int STEPS_PER_REVOLUTION = 200;  // Nema23 standard (1.8° per step)
    static const int MICROSTEPS = 16;  // TB6600 optimal setting for smooth operation
    static const int TOTAL_STEPS_PER_REV = STEPS_PER_REVOLUTION * MICROSTEPS;  // 3200 steps/rev
    
    // Speed limits for Nema23 5756 + TB6600 (based on torque curve)
    static const int MIN_RPM = 1;      // Minimum practical RPM
    static const int MAX_RPM = 1000;   // Maximum safe RPM for this setup
    static const int OPTIMAL_RPM_LOW = 50;   // Optimal low-speed range start
    static const int OPTIMAL_RPM_HIGH = 300; // Optimal high-speed range end
    
    // Speed level definitions (20 levels)
    static const int SPEED_LEVELS = 20;
    static const int SPEED_DELAY_TABLE[SPEED_LEVELS];
    
    // Motor State
    bool isRunning;
    bool isPaused;
    String currentStatus;
    String pausedStatus;
    int currentRPM;
    int targetRotations;
    int completedRotations;
    unsigned long targetDuration;
    unsigned long startTime;
    unsigned long pausedTime;
    unsigned long totalPausedDuration;
    unsigned long stepInterval;
    unsigned long lastStepTime;
    long totalSteps;
    long currentSteps;
    bool isTimeMode;
    
    
    // Test mode simulation
    unsigned long lastSimulationUpdate;
    unsigned long simulationUpdateInterval;
    
    // Motor load simulation
    float simulatedLoad;  // 0.0 to 100.0%
    unsigned long lastLoadReport;
    static const unsigned long LOAD_REPORT_INTERVAL = 1000;  // Report every 1000ms (1 second)
    
    // Step generation
    void generateStep();
    void updateStepInterval(int rpm);
    void simulateProgress();
    float calculateSimulatedLoad();
    int validateRPM(int rpm);  // Validate and limit RPM to safe range
    
public:
    MotorController();
    void begin();
    void executeRotation(int rpm, int rotations, bool clockwise = true);
    void executeTime(int rpm, int duration, bool clockwise = true);
    void executeRotationWithSpeed(int speedLevel, int rotations, bool clockwise = true);
    void executeTimeWithSpeed(int speedLevel, int duration, bool clockwise = true);
    int speedLevelToRPM(int speedLevel);
    void stop();
    void pause();
    void resume();
    void update();  // Call this in main loop
    String getStatus();
    bool isMotorRunning();
    bool isMotorPaused();
    bool isTestMode() { 
        #ifdef TEST_MODE
            return true;
        #else
            return false;
        #endif
    }
};