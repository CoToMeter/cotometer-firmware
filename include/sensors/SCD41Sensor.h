/*
 * sensors/SCD41Sensor.h
 * Updated to use CO2SensorData structure
 */

#pragma once
#include "../interfaces/ISensor.h"
#include "../types/SensorData.h"
#include <Wire.h>

class SCD41Sensor : public ISensor {
private:
    CO2SensorData currentData;
    String lastError;
    bool initialized;
    
public:
    SCD41Sensor();
    ~SCD41Sensor() = default;
    
    // ISensor interface implementation
    bool initialize() override;
    bool readData() override;
    SensorDataBase* getCurrentData() override; 
    bool isReady() override;
    String getLastError() override;
    
    // SCD41-specific methods
    CO2SensorData getCO2Data();  

    bool performForcedRecalibration(uint16_t targetCO2 = 400);
    bool setAutomaticSelfCalibration(bool enabled);
    bool getAutomaticSelfCalibration();
    bool setSensorAltitude(uint16_t altitude);
    bool setTemperatureOffset(float offset);
    bool setAmbientPressure(uint32_t pressure);
    bool getSensorSerialNumber(uint64_t& serialNumber);
    bool performSelfTest();
    bool getSensorVariant(uint16_t& variant);
    bool powerDown();
    bool wakeUp();
    bool measureSingleShot(uint16_t& co2, float& temp, float& humidity);
    bool persistSettings();
    bool performFactoryReset();
};