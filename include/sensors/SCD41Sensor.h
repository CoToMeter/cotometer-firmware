/*
 * sensors/SCD41Sensor.h
 * Updated for the real Sensirion I2C SCD4x library v1.1.0
 */

#pragma once
#include "../interfaces/ISensor.h"
#include <Wire.h>

class SCD41Sensor : public ISensor {
private:
    SensorData currentData;
    String lastError;
    bool initialized;
    
public:
    SCD41Sensor();
    ~SCD41Sensor() = default;
    
    // ISensor interface implementation
    bool initialize() override;
    bool readData() override;
    SensorData getCurrentData() override;
    bool isReady() override;
    String getLastError() override;
    
    // SCD41-specific methods
    bool performForcedRecalibration(uint16_t targetCO2 = 400);
    bool setAutomaticSelfCalibration(bool enabled);
    bool getAutomaticSelfCalibration();
    
    // Advanced sensor configuration
    bool setSensorAltitude(uint16_t altitude);
    bool setTemperatureOffset(float offset);
    bool setAmbientPressure(uint32_t pressure);
    
    // Sensor information and diagnostics
    bool getSensorSerialNumber(uint64_t& serialNumber);
    bool performSelfTest();
    bool getSensorVariant(uint16_t& variant);
    
    // Power management (SCD41 only)
    bool powerDown();
    bool wakeUp();
    
    // Single shot measurements (SCD41 only)
    bool measureSingleShot(uint16_t& co2, float& temp, float& humidity);
    
    // Settings persistence
    bool persistSettings();
    bool performFactoryReset();
};