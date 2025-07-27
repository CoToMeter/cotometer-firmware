#pragma once
#include <Arduino.h>

enum class AlertLevel {
    GOOD = 0,
    FAIR = 1, 
    POOR = 2,
    BAD = 3
};

struct SensorData {
    float co2 = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    uint32_t timestamp = 0;
    bool valid = false;
    
    SensorData() = default;
    
    void updateTimestamp() {
        timestamp = millis();
    }
    
    AlertLevel getAlertLevel() const {
        if (!valid) return AlertLevel::GOOD;
        if (co2 > 2000) return AlertLevel::BAD;
        if (co2 > 1200) return AlertLevel::POOR;
        if (co2 > 800) return AlertLevel::FAIR;
        return AlertLevel::GOOD;
    }
    
    String getAirQualityText() const {
        switch (getAlertLevel()) {
            case AlertLevel::GOOD: return "Good";
            case AlertLevel::FAIR: return "Fair";
            case AlertLevel::POOR: return "Poor";
            case AlertLevel::BAD: return "Bad";
            default: return "Unknown";
        }
    }
    
    String toJson() const {
        return "{\"timestamp\":" + String(timestamp) + 
               ",\"co2\":" + String(co2, 1) + 
               ",\"temperature\":" + String(temperature, 1) + 
               ",\"humidity\":" + String(humidity, 1) + 
               ",\"valid\":" + (valid ? "true" : "false") + "}";
    }
};

