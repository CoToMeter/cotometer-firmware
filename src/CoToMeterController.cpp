#include "CoToMeterController.h"
#include "sensors/SCD41Sensor.h"
#include "display/ConsoleDisplay.h"
#include <Wire.h>

// Static member for ISR
CoToMeterController* CoToMeterController::instance = nullptr;

CoToMeterController::CoToMeterController() 
    : lastMeasurement(0)
    , measurementInterval(15000) // 30 seconds
    // , buttonPressed(false)
{
    instance = this; // Set static instance for ISR
}

bool CoToMeterController::initialize() {
    Serial.begin(9600);
    delay(1000);
    
    Serial.println("🚀 CoToMeter v2.0 Starting...");
    
    // Initialize I2C
    Wire.begin(21, 22); // SDA=21, SCL=22
    Wire.setClock(100000);
    
    // Initialize button
    // pinMode(BUTTON_PIN, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
        
    // Create components (C++11 compatible)
    sensor.reset(new SCD41Sensor());
    display.reset(new ConsoleDisplay());
    
    // Initialize components
    if (!display->initialize()) {
        Serial.println("❌ Display initialization failed");
        return false;
    }
    
    if (!sensor->initialize()) {
        display->showError("Sensor initialization failed: " + sensor->getLastError());
        return false;
    }
    
    display->showMessage("CoToMeter initialized successfully! 🐱");
    Serial.println("✅ CoToMeter ready to measure!");
    
    return true;
}

void CoToMeterController::loop() {
    uint32_t currentTime = millis();
    
    // Handle button press
    // if (buttonPressed) {
    //     buttonPressed = false;
    //     handleButtonPress();
    // }
    
    // Take periodic measurements
    if (currentTime - lastMeasurement >= measurementInterval) {
        if (sensor->readData()) {
            SensorData data = sensor->getCurrentData();
            display->showSensorData(data);
        } else {
            display->showError("Failed to read sensor: " + sensor->getLastError());
        }
        
        lastMeasurement = currentTime;
    }
    
    delay(100);
}

// void CoToMeterController::handleButtonPress() {
//     display->showMessage("Button pressed! Taking immediate measurement...");
    
//     if (sensor->readData()) {
//         SensorData data = sensor->getCurrentData();
//         display->showSensorData(data);
//     } else {
//         display->showError("Failed to read sensor: " + sensor->getLastError());
//     }
// }

// void IRAM_ATTR CoToMeterController::buttonISR() {
//     if (instance) {
//         instance->buttonPressed = true;
//     }
// }
