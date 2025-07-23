/*
 * types/SensorData.h
 * Data structure for sensor readings
 */

#pragma once

#include <Arduino.h>
#include "SystemEnums.h"

struct SensorData {
    // Environmental data
    float co2;              // CO2 concentration (ppm)
    float temperature;      // Temperature (°C)
    float humidity;         // Relative humidity (%)
    float pm25;            // PM2.5 particles (μg/m³)
    float pm10;            // PM10 particles (μg/m³)
    float voc;             // Volatile Organic Compounds (ppb)
    float pressure;        // Atmospheric pressure (hPa)
    float noise;           // Sound level (dB)
    float light;           // Light intensity (lux)
    
    // Metadata
    uint32_t timestamp;     // Unix timestamp
    bool valid;            // Data validity flag
    AlertLevel alertLevel; // Current alert level
    DataQuality quality;   // Data quality indicator
    
    // Source information
    SensorType sourceType; // Which sensor provided this data
    String sensorId;       // Unique sensor identifier
    uint16_t sequenceNumber; // For data ordering
    
    // Calculated values
    int aqi;               // Air Quality Index
    float heatIndex;       // Heat index (°C)
    float dewPoint;        // Dew point (°C)
    
    // Constructor
    SensorData();
    
    // Copy constructor
    SensorData(const SensorData& other);
    
    // Assignment operator
    SensorData& operator=(const SensorData& other);
    
    // Validation methods
    bool isValid() const;
    bool hasTemperature() const;
    bool hasHumidity() const;
    bool hasCO2() const;
    bool hasPM() const;
    bool hasVOC() const;
    bool hasPressure() const;
    
    // Data quality checks
    bool isTemperatureValid() const;
    bool isHumidityValid() const;
    bool isCO2Valid() const;
    bool isPMValid() const;
    bool isVOCValid() const;
    
    // Utility methods
    void reset();
    void invalidate();
    void updateTimestamp();
    void calculateDerivedValues();
    
    // Serialization
    String toJson() const;
    bool fromJson(const String& json);
    String toCsv() const;
    String toHumanReadable() const;
    
    // Static factory methods
    static SensorData fromTemperatureHumidity(float temp, float hum);
    static SensorData fromCO2(float co2);
    static SensorData fromPM(float pm25, float pm10);
    static SensorData fromVOC(float voc);
    static SensorData invalid();
    
    // Comparison operators
    bool operator==(const SensorData& other) const;
    bool operator!=(const SensorData& other) const;
    
    // Range validation constants
    static const float CO2_MIN;
    static const float CO2_MAX;
    static const float TEMP_MIN;
    static const float TEMP_MAX;
    static const float HUMIDITY_MIN;
    static const float HUMIDITY_MAX;
    static const float PM_MIN;
    static const float PM_MAX;
    static const float VOC_