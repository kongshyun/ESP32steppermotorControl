#include "MotorController.h"

// Speed level to step delay table (microseconds)
// Level 1 (slowest) to Level 20 (fastest)
const int MotorController::SPEED_DELAY_TABLE[SPEED_LEVELS] = {
    10000,  // Level 1:  Slowest (6 RPM)
    8000,   // Level 2:  (7.5 RPM)
    6500,   // Level 3:  (9.2 RPM)
    5000,   // Level 4:  (12 RPM)
    4000,   // Level 5:  (15 RPM)
    3200,   // Level 6:  (18.75 RPM)
    2600,   // Level 7:  (23 RPM)
    2100,   // Level 8:  (28.6 RPM)
    1700,   // Level 9:  (35.3 RPM)
    1400,   // Level 10: (42.9 RPM)
    1150,   // Level 11: (52.2 RPM)
    950,    // Level 12: (63.2 RPM)
    800,    // Level 13: (75 RPM)
    680,    // Level 14: (88.2 RPM)
    580,    // Level 15: (103.4 RPM)
    500,    // Level 16: (120 RPM)
    430,    // Level 17: (139.5 RPM)
    370,    // Level 18: (162.2 RPM)
    320,    // Level 19: (187.5 RPM)
    280     // Level 20: Fastest (214.3 RPM)
};

MotorController::MotorController() : 
    isRunning(false), 
    isPaused(false),
    currentStatus("IDLE"),
    pausedStatus(""),
    currentRPM(0),
    targetRotations(0),
    completedRotations(0),
    targetDuration(0),
    startTime(0),
    pausedTime(0),
    totalPausedDuration(0),
    stepInterval(0),
    lastStepTime(0),
    totalSteps(0),
    currentSteps(0),
    isTimeMode(false),
    lastSimulationUpdate(0),
    simulationUpdateInterval(1000),
    simulatedLoad(0.0),
    lastLoadReport(0) {}

void MotorController::begin() {
    #ifndef TEST_MODE
        // Initialize TB6600 driver pins for real motor
        pinMode(STEP_PIN, OUTPUT);
        pinMode(DIR_PIN, OUTPUT);
        pinMode(ENABLE_PIN, OUTPUT);
        
        // Set initial states for TB6600 driver
        digitalWrite(STEP_PIN, LOW);       // Step signal idle state
        digitalWrite(DIR_PIN, LOW);        // Forward direction (CW)
        digitalWrite(ENABLE_PIN, HIGH);    // Disable driver initially (active low)
        
        // Add small delay for TB6600 initialization
        delay(100);
        
        currentStatus = "READY";
        Serial.println("Motor Controller initialized - TB6600 + Nema23 5756");
        Serial.println("Microsteps: 1/16, Steps per revolution: 3200");
    #else
        currentStatus = "TEST_MODE_READY";
        Serial.println("Motor Controller initialized in TEST mode");
    #endif
}

void MotorController::updateStepInterval(int rpm) {
    // Calculate microseconds per step
    // (60 seconds * 1,000,000 microseconds) / (rpm * steps_per_revolution)
    stepInterval = (60L * 1000000L) / (rpm * TOTAL_STEPS_PER_REV);
    
    // In test mode, calculate simulation update interval (1 rotation per second for visualization)
    #ifdef TEST_MODE
        simulationUpdateInterval = 60000 / rpm;  // milliseconds per rotation
    #endif
}

float MotorController::calculateSimulatedLoad() {
    // Return random load value between 10% and 50%
    return (float)(random(100, 500)) / 10.0;  // 10.0 to 50.0
}

int MotorController::validateRPM(int rpm) {
    if (rpm < MIN_RPM) {
        Serial.println("Warning: RPM too low, setting to minimum: " + String(MIN_RPM));
        return MIN_RPM;
    }
    
    if (rpm > MAX_RPM) {
        Serial.println("Warning: RPM too high, setting to maximum: " + String(MAX_RPM));
        return MAX_RPM;
    }
    
    // Provide feedback for optimal range
    if (rpm >= OPTIMAL_RPM_LOW && rpm <= OPTIMAL_RPM_HIGH) {
        Serial.println("RPM " + String(rpm) + " is in optimal range");
    } else {
        Serial.println("RPM " + String(rpm) + " is outside optimal range (" + 
                      String(OPTIMAL_RPM_LOW) + "-" + String(OPTIMAL_RPM_HIGH) + ")");
    }
    
    return rpm;
}

void MotorController::generateStep() {
    #ifndef TEST_MODE
        // TB6600 step pulse generation
        digitalWrite(STEP_PIN, HIGH);
        delayMicroseconds(5);   // TB6600 requires minimum 2.5us pulse, use 5us for safety
        digitalWrite(STEP_PIN, LOW);
        delayMicroseconds(5);   // Additional delay for clean signal
    #endif
}

void MotorController::simulateProgress() {
    // Simulate rotation progress in test mode
    unsigned long currentTime = millis();
    
    if (currentTime - lastSimulationUpdate >= simulationUpdateInterval) {
        completedRotations++;
        lastSimulationUpdate = currentTime;
        
        // Send progress update
        Serial.println("TURN:" + String(completedRotations));
        
        // Check if target reached
        if (!isTimeMode && completedRotations >= targetRotations) {
            isRunning = false;
            currentStatus = "DONE";
            Serial.println("DONE");
        }
    }
    
    // For time mode, check duration (excluding paused time)
    if (isTimeMode && (currentTime - startTime - totalPausedDuration >= targetDuration)) {
        isRunning = false;
        currentStatus = "DONE";
        Serial.println("DONE");
    }
}

void MotorController::executeRotation(int rpm, int rotations, bool clockwise) {
    if (isRunning) {
        return;  // Already running
    }
    
    // Validate and limit RPM to safe range
    currentRPM = validateRPM(rpm);
    targetRotations = rotations;
    completedRotations = 0;
    totalSteps = (long)rotations * TOTAL_STEPS_PER_REV;
    currentSteps = 0;
    isTimeMode = false;
    isPaused = false;
    totalPausedDuration = 0;
    
    updateStepInterval(rpm);
    
    isRunning = true;
    lastStepTime = micros();
    lastSimulationUpdate = millis();
    startTime = millis();
    currentStatus = "ROTATING";
    
    // Enable TB6600 driver (active low)
    #ifndef TEST_MODE
        digitalWrite(DIR_PIN, clockwise ? LOW : HIGH);  // Set direction: LOW = CW, HIGH = CCW
        digitalWrite(ENABLE_PIN, LOW);
        delay(10);  // Small delay for driver to stabilize
    #endif
    
    Serial.println("Starting rotation: " + String(rpm) + " RPM, " + String(rotations) + " rotations, Direction: " + String(clockwise ? "CW" : "CCW"));
}

void MotorController::executeTime(int rpm, int duration, bool clockwise) {
    if (isRunning) {
        return;  // Already running
    }
    
    // Validate and limit RPM to safe range
    currentRPM = validateRPM(rpm);
    targetDuration = (unsigned long)duration * 1000;  // Convert to milliseconds
    startTime = millis();
    currentSteps = 0;
    completedRotations = 0;
    isTimeMode = true;
    isPaused = false;
    totalPausedDuration = 0;
    
    updateStepInterval(rpm);
    
    isRunning = true;
    lastStepTime = micros();
    lastSimulationUpdate = millis();
    currentStatus = "TIME_MODE";
    
    // Enable TB6600 driver (active low)
    #ifndef TEST_MODE
        digitalWrite(DIR_PIN, clockwise ? LOW : HIGH);  // Set direction: LOW = CW, HIGH = CCW
        digitalWrite(ENABLE_PIN, LOW);
        delay(10);  // Small delay for driver to stabilize
    #endif
    
    Serial.println("Starting time mode: " + String(rpm) + " RPM for " + String(duration) + " seconds, Direction: " + String(clockwise ? "CW" : "CCW"));
}

void MotorController::stop() {
    isRunning = false;
    isPaused = false;
    currentStatus = "STOPPED";
    pausedStatus = "";
    totalPausedDuration = 0;
    
    // Disable TB6600 driver (active low, so HIGH disables)
    #ifndef TEST_MODE
        digitalWrite(ENABLE_PIN, HIGH);
        delay(5);  // Small delay for clean shutdown
    #endif
    
    Serial.println("Motor stopped - TB6600 driver disabled");
}

void MotorController::pause() {
    if (!isRunning || isPaused) {
        return;  // Not running or already paused
    }
    
    isPaused = true;
    pausedTime = millis();
    pausedStatus = currentStatus;  // Save current status
    currentStatus = "PAUSED";
    
    // Keep TB6600 driver enabled but stop stepping
    Serial.println("Motor paused");
}

void MotorController::resume() {
    if (!isRunning || !isPaused) {
        return;  // Not running or not paused
    }
    
    isPaused = false;
    
    // Calculate how long we were paused and add to total paused time
    unsigned long pauseDuration = millis() - pausedTime;
    totalPausedDuration += pauseDuration;
    
    // Restore the previous status
    currentStatus = pausedStatus;
    pausedStatus = "";
    
    // Reset timing for smooth resumption
    lastStepTime = micros();
    lastSimulationUpdate = millis();
    
    Serial.println("Motor resumed");
}

void MotorController::update() {
    if (!isRunning || isPaused) {
        return;
    }
    
    // Update simulated load and report periodically
    unsigned long currentTime = millis();
    if (currentTime - lastLoadReport >= LOAD_REPORT_INTERVAL) {
        simulatedLoad = calculateSimulatedLoad();
        Serial.println("LOAD:" + String(simulatedLoad, 1) + "%");
        lastLoadReport = currentTime;
    }
    
    #ifdef TEST_MODE
        // In test mode, simulate progress without actual motor control
        simulateProgress();
    #else
        // Real motor control
        unsigned long currentMicros = micros();
        
        // Check if it's time for the next step
        if (currentMicros - lastStepTime >= stepInterval) {
            generateStep();
            currentSteps++;
            lastStepTime = currentMicros;
            
            // Update completed rotations
            if (currentSteps % TOTAL_STEPS_PER_REV == 0) {
                completedRotations = currentSteps / TOTAL_STEPS_PER_REV;
                
                // Send progress update via Serial
                Serial.println("TURN:" + String(completedRotations));
            }
            
            if (isTimeMode) {
                // Check if time duration has elapsed (excluding paused time)
                if (millis() - startTime - totalPausedDuration >= targetDuration) {
                    isRunning = false;
                    currentStatus = "DONE";
                    digitalWrite(ENABLE_PIN, HIGH);
                    Serial.println("DONE");
                }
            } else {
                // Rotation mode - check if target reached
                if (currentSteps >= totalSteps) {
                    isRunning = false;
                    currentStatus = "DONE";
                    digitalWrite(ENABLE_PIN, HIGH);
                    Serial.println("DONE");
                }
            }
        }
    #endif
}

String MotorController::getStatus() {
    if (isRunning) {
        String loadInfo = " LOAD:" + String(simulatedLoad, 1) + "%";
        if (isTimeMode) {
            unsigned long elapsed = millis() - startTime - totalPausedDuration;
            return "TIME_MODE RPM:" + String(currentRPM) + 
                   " ELAPSED:" + String(elapsed/1000) + "s" +
                   " ROTATIONS:" + String(completedRotations) +
                   loadInfo +
                   #ifdef TEST_MODE
                   " [TEST]";
                   #else
                   "";
                   #endif
        } else {
            return "ROTATING RPM:" + String(currentRPM) + 
                   " COMPLETED:" + String(completedRotations) + 
                   "/" + String(targetRotations) +
                   loadInfo +
                   #ifdef TEST_MODE
                   " [TEST]";
                   #else
                   "";
                   #endif
        }
    }
    return currentStatus + 
    #ifdef TEST_MODE
        " [TEST]";
    #else
        "";
    #endif
}

bool MotorController::isMotorRunning() {
    return isRunning;
}

bool MotorController::isMotorPaused() {
    return isPaused;
}

int MotorController::speedLevelToRPM(int speedLevel) {
    // Validate speed level
    if (speedLevel < 1) speedLevel = 1;
    if (speedLevel > SPEED_LEVELS) speedLevel = SPEED_LEVELS;
    
    // Convert delay to RPM
    // RPM = (60 * 1,000,000) / (delay * steps_per_revolution)
    int delay = SPEED_DELAY_TABLE[speedLevel - 1];
    int rpm = (60L * 1000000L) / (delay * TOTAL_STEPS_PER_REV);
    
    return rpm;
}

void MotorController::executeRotationWithSpeed(int speedLevel, int rotations, bool clockwise) {
    // Validate speed level
    if (speedLevel < 1 || speedLevel > SPEED_LEVELS) {
        Serial.println("Invalid speed level. Must be 1-20");
        return;
    }
    
    // Convert speed level to RPM
    int rpm = speedLevelToRPM(speedLevel);
    
    Serial.println("Speed Level " + String(speedLevel) + " = " + String(rpm) + " RPM");
    
    // Execute with converted RPM
    executeRotation(rpm, rotations, clockwise);
}

void MotorController::executeTimeWithSpeed(int speedLevel, int duration, bool clockwise) {
    // Validate speed level
    if (speedLevel < 1 || speedLevel > SPEED_LEVELS) {
        Serial.println("Invalid speed level. Must be 1-20");
        return;
    }
    
    // Convert speed level to RPM
    int rpm = speedLevelToRPM(speedLevel);
    
    Serial.println("Speed Level " + String(speedLevel) + " = " + String(rpm) + " RPM");
    
    // Execute with converted RPM
    executeTime(rpm, duration, clockwise);
}