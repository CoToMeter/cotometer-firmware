/*
 * types/SensorData.h
 * Clean base class and derived classes for sensor data
 */

#pragma once
#include <Arduino.h>
#include <vector>
#include <memory>
#include "SystemEnums.h"

// ================================
// BASE SENSOR DATA CLASS
// ================================

class SensorDataBase {
protected:
    SensorType sensorType;
    uint32_t timestamp;
    bool valid;
    String sensorId;
    
public:
    SensorDataBase(SensorType type, const String& id = "") 
        : sensorType(type), timestamp(0), valid(false), sensorId(id) {}
    
    virtual ~SensorDataBase() = default;
    
    // Pure virtual methods - must be implemented by derived classes
    virtual String toJson() const = 0;
    virtual String toString() const = 0;
    virtual bool isDataValid() const = 0;
    virtual AlertLevel getAlertLevel() const = 0;
    
    // Common interface methods
    SensorType getType() const { return sensorType; }
    uint32_t getTimestamp() const { return timestamp; }
    bool isValid() const { return valid && isDataValid(); }
    String getSensorId() const { return sensorId; }
    
    void updateTimestamp() { timestamp = millis(); }
    void setValid(bool v) { valid = v; }
    void setSensorId(const String& id) { sensorId = id; }
    
    // Age checking
    uint32_t getAgeMs() const { return millis() - timestamp; }
    bool isExpired(uint32_t maxAgeMs) const { return getAgeMs() > maxAgeMs; }
};

// ================================
// CO2 SENSOR DATA (SCD41)
// ================================

class CO2SensorData : public SensorDataBase {
public:
    float co2;          // ppm
    float temperature;  // °C
    float humidity;     // %
    
    CO2SensorData(const String& id = "SCD41") 
        : SensorDataBase(SensorType::CO2_TEMP_HUMIDITY, id)
        , co2(0.0), temperature(0.0), humidity(0.0) {}
    
    // Interface implementation
    String toJson() const override {
        return "{\"type\":\"co2\","
               "\"sensor_id\":\"" + sensorId + "\","
               "\"timestamp\":" + String(timestamp) + ","
               "\"co2\":" + String(co2, 1) + ","
               "\"temperature\":" + String(temperature, 1) + ","
               "\"humidity\":" + String(humidity, 1) + ","
               "\"valid\":" + (valid ? "true" : "false") + "}";
    }
    
    String toString() const override {
        return "CO2: " + String(co2, 1) + "ppm, " +
               "Temp: " + String(temperature, 1) + "°C, " +
               "Humidity: " + String(humidity, 1) + "%";
    }
    
    bool isDataValid() const override {
        return co2 >= 400 && co2 <= 5000 &&
               temperature >= -40 && temperature <= 125 &&
               humidity >= 0 && humidity <= 100;
    }
    
    AlertLevel getAlertLevel() const override {
        if (!isValid()) return AlertLevel::NONE;
        if (co2 > 2000) return AlertLevel::CRITICAL;
        if (co2 > 1200) return AlertLevel::WARNING;
        if (co2 > 800) return AlertLevel::INFO;
        return AlertLevel::NONE;
    }
    
    // CO2-specific methods
    String getAirQualityText() const {
        switch (getAlertLevel()) {
            case AlertLevel::NONE: return "Excellent";
            case AlertLevel::INFO: return "Good";
            case AlertLevel::WARNING: return "Poor";
            case AlertLevel::CRITICAL: return "Bad";
            default: return "Unknown";
        }
    }
    
    bool isComfortableTemperature() const {
        return temperature >= 20.0 && temperature <= 25.0;
    }
    
    bool isComfortableHumidity() const {
        return humidity >= 40.0 && humidity <= 60.0;
    }
};

// ================================
// VOC SENSOR DATA (BME688)
// ================================

class VOCSensorData : public SensorDataBase {
public:
    float temperature;      // °C
    float humidity;         // %
    float pressure;         // Pa
    float gasResistance;    // Ohms
    float vocEstimate;      // ppb (estimated)
    float vocIndex;         // 0-500 (BSEC index if available)
    bool heaterStable;      // Gas heater stability
    bool gasValid;          // Gas measurement validity
    
    VOCSensorData(const String& id = "BME688") 
        : SensorDataBase(SensorType::VOC_GAS, id)
        , temperature(0.0), humidity(0.0), pressure(0.0)
        , gasResistance(0.0), vocEstimate(0.0), vocIndex(0.0)
        , heaterStable(false), gasValid(false) {}
    
    // Interface implementation
    String toJson() const override {
        return "{\"type\":\"voc\","
               "\"sensor_id\":\"" + sensorId + "\","
               "\"timestamp\":" + String(timestamp) + ","
               "\"temperature\":" + String(temperature, 1) + ","
               "\"humidity\":" + String(humidity, 1) + ","
               "\"pressure\":" + String(pressure, 0) + ","
               "\"gas_resistance\":" + String(gasResistance, 0) + ","
               "\"voc_estimate\":" + String(vocEstimate, 1) + ","
               "\"voc_index\":" + String(vocIndex, 1) + ","
               "\"heater_stable\":" + (heaterStable ? "true" : "false") + ","
               "\"gas_valid\":" + (gasValid ? "true" : "false") + ","
               "\"valid\":" + (valid ? "true" : "false") + "}";
    }
    
    String toString() const override {
        return "Temp: " + String(temperature, 1) + "°C, " +
               "Humidity: " + String(humidity, 1) + "%, " +
               "Pressure: " + String(pressure * 0.00750062, 1) + "mmHg, " +
               "VOC: " + String(vocEstimate, 1) + "ppb, " +
               "Gas: " + String(gasResistance, 0) + "Ω";
    }
        
    bool isDataValid() const override {
        return temperature >= -40 && temperature <= 85 &&
               humidity >= 0 && humidity <= 100 &&
               pressure >= 30000 && pressure <= 110000 &&
               gasResistance > 0;
    }
    
    AlertLevel getAlertLevel() const override {
        if (!isValid() || !gasValid) return AlertLevel::NONE;
        if (vocEstimate > 200) return AlertLevel::CRITICAL;
        if (vocEstimate > 100) return AlertLevel::WARNING;
        if (vocEstimate > 50) return AlertLevel::INFO;
        return AlertLevel::NONE;
    }
    
    // VOC-specific methods
    float getPressureHPa() const { return pressure / 100.0; }
    
    float getAltitudeEstimate(float seaLevelPressure = 101325.0) const {
        return 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
    }
    
    String getVOCQualityText() const {
        switch (getAlertLevel()) {
            case AlertLevel::NONE: return "Clean";
            case AlertLevel::INFO: return "Acceptable";
            case AlertLevel::WARNING: return "Contaminated";
            case AlertLevel::CRITICAL: return "Heavily Polluted";
            default: return "Unknown";
        }
    }
};

// ================================
// PARTICULATE MATTER DATA (PMS7003)
// ================================

class PMSensorData : public SensorDataBase {
public:
    float pm1_0_atmospheric;     // μg/m³ (atmospheric)
    float pm2_5_atmospheric;     // μg/m³ (atmospheric) 
    float pm10_atmospheric;      // μg/m³ (atmospheric)
    float pm1_0_standard;        // μg/m³ (standard)
    float pm2_5_standard;        // μg/m³ (standard)
    float pm10_standard;         // μg/m³ (standard)
    
    // Particle counts (per 0.1L)
    uint16_t particles_03um;
    uint16_t particles_05um; 
    uint16_t particles_10um;
    uint16_t particles_25um;
    uint16_t particles_50um;
    uint16_t particles_100um;
    
    PMSensorData(const String& id = "PMS7003") 
        : SensorDataBase(SensorType::PARTICULATE_MATTER, id)
        , pm1_0_atmospheric(0.0), pm2_5_atmospheric(0.0), pm10_atmospheric(0.0)
        , pm1_0_standard(0.0), pm2_5_standard(0.0), pm10_standard(0.0)
        , particles_03um(0), particles_05um(0), particles_10um(0)
        , particles_25um(0), particles_50um(0), particles_100um(0) {}
    
    // Interface implementation
    String toJson() const override {
        return "{\"type\":\"pm\","
               "\"sensor_id\":\"" + sensorId + "\","
               "\"timestamp\":" + String(timestamp) + ","
               "\"pm1_0\":" + String(pm1_0_atmospheric, 1) + ","
               "\"pm2_5\":" + String(pm2_5_atmospheric, 1) + ","
               "\"pm10\":" + String(pm10_atmospheric, 1) + ","
               "\"particles_03um\":" + String(particles_03um) + ","
               "\"particles_25um\":" + String(particles_25um) + ","
               "\"valid\":" + (valid ? "true" : "false") + "}";
    }
    
    String toString() const override {
        return "PM1.0: " + String(pm1_0_atmospheric, 1) + "μg/m³, " +
               "PM2.5: " + String(pm2_5_atmospheric, 1) + "μg/m³, " +
               "PM10: " + String(pm10_atmospheric, 1) + "μg/m³";
    }
    
    bool isDataValid() const override {
        return pm1_0_atmospheric >= 0 && pm1_0_atmospheric <= 1000 &&
               pm2_5_atmospheric >= 0 && pm2_5_atmospheric <= 1000 &&
               pm10_atmospheric >= 0 && pm10_atmospheric <= 1000 &&
               pm2_5_atmospheric >= pm1_0_atmospheric && // PM2.5 >= PM1.0
               pm10_atmospheric >= pm2_5_atmospheric;    // PM10 >= PM2.5
    }
    
    AlertLevel getAlertLevel() const override {
        if (!isValid()) return AlertLevel::NONE;
        
        // Using WHO/EPA PM2.5 standards
        if (pm2_5_atmospheric > 55.4) return AlertLevel::CRITICAL;   // Unhealthy
        if (pm2_5_atmospheric > 35.4) return AlertLevel::WARNING;    // Unhealthy for sensitive
        if (pm2_5_atmospheric > 12.0) return AlertLevel::INFO;       // Moderate
        return AlertLevel::NONE;                                     // Good
    }
    
    // PM-specific methods
    int getAQIPM25() const {
        // EPA AQI calculation for PM2.5
        float pm = pm2_5_atmospheric;
        if (pm <= 12.0) return (50.0/12.0) * pm;
        if (pm <= 35.4) return 51 + ((100-51)/(35.4-12.1)) * (pm-12.1);
        if (pm <= 55.4) return 101 + ((150-101)/(55.4-35.5)) * (pm-35.5);
        if (pm <= 150.4) return 151 + ((200-151)/(150.4-55.5)) * (pm-55.5);
        if (pm <= 250.4) return 201 + ((300-201)/(250.4-150.5)) * (pm-150.5);
        return 301 + ((500-301)/(500.4-250.5)) * (pm-250.5);
    }
    
    String getPMQualityText() const {
        switch (getAlertLevel()) {
            case AlertLevel::NONE: return "Clean";
            case AlertLevel::INFO: return "Moderate";
            case AlertLevel::WARNING: return "Unhealthy for Sensitive";
            case AlertLevel::CRITICAL: return "Unhealthy";
            default: return "Unknown";
        }
    }
};

// ================================
// COMBINED SENSOR DATA MANAGER
// ================================

// class CombinedSensorData {
// private:
//     std::vector<std::unique_ptr<SensorDataBase>> sensorData;
//     uint32_t lastUpdate;
    
// public:
//     CombinedSensorData() : lastUpdate(0) {}
    
//     // Add sensor data
//     void addCO2Data(const CO2SensorData& data) {
//         auto co2Data = std::make_unique<CO2SensorData>(data);
//         addOrUpdateSensor(std::move(co2Data));
//     }
    
//     void addVOCData(const VOCSensorData& data) {
//         auto vocData = std::make_unique<VOCSensorData>(data);
//         addOrUpdateSensor(std::move(vocData));
//     }
    
//     void addPMData(const PMSensorData& data) {
//         auto pmData = std::make_unique<PMSensorData>(data);
//         addOrUpdateSensor(std::move(pmData));
//     }
    
//     // Get sensor data by type
//     CO2SensorData* getCO2Data() const {
//         return static_cast<CO2SensorData*>(getSensorData(SensorType::CO2_TEMP_HUMIDITY));
//     }
    
//     VOCSensorData* getVOCData() const {
//         return static_cast<VOCSensorData*>(getSensorData(SensorType::VOC_GAS));
//     }
    
//     PMSensorData* getPMData() const {
//         return static_cast<PMSensorData*>(getSensorData(SensorType::PARTICULATE_MATTER));
//     }
    
//     // Overall air quality assessment
//     AlertLevel getOverallAlertLevel() const {
//         AlertLevel maxLevel = AlertLevel::NONE;
        
//         for (const auto& sensor : sensorData) {
//             if (sensor->isValid()) {
//                 AlertLevel level = sensor->getAlertLevel();
//                 if (level > maxLevel) {
//                     maxLevel = level;
//                 }
//             }
//         }
        
//         return maxLevel;
//     }
    
//     // Data availability checks
//     bool hasCO2Data() const { return getCO2Data() && getCO2Data()->isValid(); }
//     bool hasVOCData() const { return getVOCData() && getVOCData()->isValid(); }
//     bool hasPMData() const { return getPMData() && getPMData()->isValid(); }
//     bool hasTemperatureData() const { return hasCO2Data() || hasVOCData(); }
//     bool hasHumidityData() const { return hasCO2Data() || hasVOCData(); }
//     bool hasPressureData() const { return hasVOCData(); }
    
//     // Get preferred values (with fallback logic)
//     float getPreferredTemperature() const {
//         if (hasCO2Data()) return getCO2Data()->temperature;
//         if (hasVOCData()) return getVOCData()->temperature;
//         return 0.0;
//     }
    
//     float getPreferredHumidity() const {
//         if (hasCO2Data()) return getCO2Data()->humidity;
//         if (hasVOCData()) return getVOCData()->humidity;
//         return 0.0;
//     }
    
//     // Export all data
//     String toJson() const {
//         String json = "{\"sensors\":[";
//         bool first = true;
        
//         for (const auto& sensor : sensorData) {
//             if (!first) json += ",";
//             json += sensor->toJson();
//             first = false;
//         }
        
//         json += "],\"overall_alert_level\":" + String((int)getOverallAlertLevel());
//         json += ",\"timestamp\":" + String(millis()) + "}";
//         return json;
//     }
    
//     // Summary for display
//     String getSummary() const {
//         String summary;
//         if (hasCO2Data()) summary += getCO2Data()->toString() + "\n";
//         if (hasVOCData()) summary += getVOCData()->toString() + "\n";
//         if (hasPMData()) summary += getPMData()->toString() + "\n";
//         return summary;
//     }
    
//     // Get cat mood based on overall air quality
//     String getCatMood() const {
//         switch (getOverallAlertLevel()) {
//             case AlertLevel::NONE: return "😸 Very Happy!";
//             case AlertLevel::INFO: return "😺 Content";
//             case AlertLevel::WARNING: return "😿 Concerned";
//             case AlertLevel::CRITICAL: return "🙀 Stressed!";
//             default: return "😐 Neutral";
//         }
//     }
    
// private:
//     void addOrUpdateSensor(std::unique_ptr<SensorDataBase> newData) {
//         // Remove existing sensor of same type
//         sensorData.erase(
//             std::remove_if(sensorData.begin(), sensorData.end(),
//                 [&](const std::unique_ptr<SensorDataBase>& existing) {
//                     return existing->getType() == newData->getType();
//                 }),
//             sensorData.end()
//         );
        
//         // Add new data
//         sensorData.push_back(std::move(newData));
//         lastUpdate = millis();
//     }
    
//     SensorDataBase* getSensorData(SensorType type) const {
//         for (const auto& sensor : sensorData) {
//             if (sensor->getType() == type) {
//                 return sensor.get();
//             }
//         }
//         return nullptr;
//     }
// };