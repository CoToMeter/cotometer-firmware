/*
 * sensors/BME688Sensor.h
 * BME688 VOC/Gas sensor implementation using official Bosch BME68x library
 */

#pragma once
#include "../interfaces/ISensor.h"
#include "../types/SensorData.h"
#include <Wire.h>
#include <bme68xLibrary.h>

class BME688Sensor : public ISensor {
private:
    Bme68x bme688;
    VOCSensorData currentData;  // Only VOC data structure
    String lastError;
    bool initialized;
    uint8_t i2cAddress;
    uint8_t csPin;
    
    // BME688-specific data
    float temperature;
    float humidity;
    float pressure;
    float gasResistance;
    float vocEstimate;
    
    // Sensor configuration
    bool gasHeaterEnabled;
    uint16_t heaterTemp;
    uint16_t heaterDuration;
    
    // Helper methods
    bool configureBasicSettings();
    bool configureGasHeater();
    bool validateReadings(const bme68xData& data);
    float calculateVOCEstimate(float gasResistance, float temperature, float humidity);
    
public:
    BME688Sensor(uint8_t address = 0x76, uint8_t csPin = 4);  // Added CS pin parameter
    ~BME688Sensor() = default;
    
    // ISensor interface implementation
    bool initialize() override;
    bool readData() override;
    SensorDataBase* getCurrentData() override;  // Return VOC data directly
    bool isReady() override;
    String getLastError() override;
    
    // BME688-specific methods  
    bool setI2CAddress(uint8_t address);
    uint8_t getI2CAddress() const;
    bool setCSPin(uint8_t pin);
    uint8_t getCSPin() const;
    
    // Gas sensor configuration
    bool enableGasSensor(bool enable = true);
    bool setHeaterProfile(uint16_t temperature, uint16_t duration);
    bool setHeaterProfileAdvanced(uint16_t* temperatures, uint16_t* durations, uint8_t profileLength);
    
    // Advanced sensor settings
    bool setOversampling(uint8_t osTemp = BME68X_OS_2X, uint8_t osHum = BME68X_OS_1X, uint8_t osPres = BME68X_OS_16X);
    bool setFilter(uint8_t filterCoeff = BME68X_FILTER_SIZE_3);
    bool setOperationMode(uint8_t mode = BME68X_FORCED_MODE);
    
    // Individual data getters
    float getTemperature() const;
    float getHumidity() const;
    float getPressure() const;
    float getGasResistance() const;
    float getVOCEstimate() const;
    
    // Sensor diagnostics
    bool performSelfTest();
    uint32_t getUniqueId();
    String getSensorVariant();
    bool isGasDataValid() const;
    bool isHeaterStable() const;
    
    // Calibration and configuration
    bool setAmbientTemperature(int8_t temp);
    bool softReset();
    String getConfigurationString() const;
    
    // Power management
    bool enterSleepMode();
    bool wakeUp();
    
    // Data validation helpers
    bool isTemperatureValid() const;
    bool isHumidityValid() const;
    bool isPressureValid() const;
    bool isGasResistanceValid() const;
};

// Helper functions for BME688 integration
// namespace BME688Utils {
//     String operationModeToString(uint8_t mode);
//     String oversamplingToString(uint8_t os);
//     String filterToString(uint8_t filter);
//     float gasResistanceToAirQuality(float gasResistance);
//     AlertLevel vocToAlertLevel(float vocEstimate);
// }