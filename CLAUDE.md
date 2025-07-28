# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-based motor control system that receives UART commands to control stepper motors. The system is designed to receive commands from a Qt application and control Nema23 stepper motors via TB6600 drivers.

### Hardware Components
- ESP32 development board (esp32dev)
- Nema23 5756 stepper motor
- TB6600 driver
- USB to UART connection
- 24V DC SMPS power supply

### Current Implementation Status
The current code (`src/main.cpp`) implements basic UART communication with LED control for testing purposes, as the actual stepper motor hardware is not yet connected.

## Development Commands

### Build and Upload
```bash
# Build the project
pio run

# Upload to ESP32
pio run --target upload

# Build and upload in one command
pio run --target upload

# Monitor serial output
pio device monitor

# Build, upload, and monitor
pio run --target upload && pio device monitor
```

### Development Environment
- Framework: Arduino
- Platform: Espressif32
- Board: esp32dev
- Monitor speed: 115200 baud
- Dependencies: ESP32Servo library (^3.0.8)

## Code Architecture

### Core Components
1. **Serial Communication**: Handles UART communication at 115200 baud
2. **LED Control**: PWM-based LED brightness control for testing (Pin 18)
3. **Command Processing**: Processes incoming serial commands ("HELLO", "ON", "OFF")

### Command Protocol
- `HELLO` → responds with `READY`
- `ON [brightness]` → turns LED on with specified brightness (0-255)
- `OFF` → turns LED off
- System sends "ESP32 READY" messages for first 3 seconds after startup

### Future Motor Control Commands (from system guide)
- `ROTATION rpm,rot` → control motor rotation
- `TIME rpm,duration` → control motor for specific duration

## Development Workflow

Per the system workflow configuration, follow these steps:
1. Read codebase and create a plan in `tasks/todo_{TASK_TITLE}.md`
2. Get plan verification before starting work
3. Work on todo items, marking complete as you go
4. Provide high-level explanations of changes
5. Keep changes simple and minimal
6. Add review section to todo file when complete
7. Move completed todo to `tasks/completed/` directory

## File Structure
- `src/main.cpp` - Main application code
- `platformio.ini` - PlatformIO configuration
- `prompt/` - System documentation and workflow guides
- `include/`, `lib/`, `test/` - Standard PlatformIO directories