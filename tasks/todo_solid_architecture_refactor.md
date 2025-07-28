# ESP32 Motor Control - Simplified Architecture Refactor

## Problem Analysis

Current architecture issues:
1. **Monolithic main.cpp**: All functionality mixed in single file
2. **Missing motor control**: Future stepper motor commands not implemented
3. **No separation of concerns**: Serial communication and motor control mixed

## Requirements from system_guide.yaml
- Support "ROTATION rpm,rot" and "TIME rpm,duration" commands
- Periodic status transmission to Qt
- LED indication for connection status (simple receive confirmation)
- UART communication at 115200 baud

## Simplified Architecture

### 1. **main.cpp**
- Setup and main loop
- LED control for receive confirmation
- Coordinate SerialManager and MotorController

### 2. **SerialManager**
- Handle UART communication
- Parse incoming commands
- Send responses and status updates

### 3. **MotorController**
- Handle stepper motor operations
- Support ROTATION and TIME commands
- Provide motor status

## Todo Items

### Phase 1: Create Classes
- [x] Create SerialManager class
- [x] Create MotorController class

### Phase 2: Refactor main.cpp
- [x] Integrate SerialManager
- [x] Integrate MotorController
- [x] Keep LED as simple receive confirmation

### Phase 3: Add Motor Commands
- [x] Add ROTATION rpm,rot command support
- [x] Add TIME rpm,duration command support
- [x] Add STATUS command for motor state

### Phase 4: Testing
- [ ] Test HELLO/READY functionality
- [ ] Test LED ON/OFF commands
- [ ] Test motor commands
- [ ] Validate startup behavior

## Review

### Changes Made
1. **SerialManager Class**: Handles UART communication and startup ready messages
2. **MotorController Class**: Manages motor operations with mock implementation
3. **main.cpp Refactor**: Clean separation of concerns with LED kept simple
4. **New Commands**: Added ROTATION, TIME, and STATUS commands

### Architecture Benefits
- Clean separation: serial communication vs motor control
- LED remains simple receive confirmation
- Easy to extend with real motor hardware
- Maintains all existing functionality

## Files to Create/Modify
- `include/SerialManager.h`
- `include/MotorController.h`
- `src/main.cpp` (refactor)