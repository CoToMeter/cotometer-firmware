#pragma once
#include <Arduino.h>
#include "../types/SensorData.h"

class IAnalytics {
public:
    virtual ~IAnalytics() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Air Quality Index (must implement)
    virtual int calculateAQI(const SensorData& data) = 0;
    virtual String getAQIDescription(int aqi) = 0;
    virtual AlertLevel getAlertLevel(const SensorData& data) = 0;
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Data analysis
    virtual bool isDataValid(const SensorData& data) { return data.valid; }
    virtual float getDataTrend(const String& parameter) { return 0.0; } // +/- trend
    virtual String getRecommendations(const SensorData& data) { return ""; }
    
    // Statistics
    virtual float getAverage(const String& parameter) { return 0.0; }
    virtual float getMin(const String& parameter) { return 0.0; }
    virtual float getMax(const String& parameter) { return 0.0; }
    
    // Analytics info
    virtual String getAnalyticsVersion() { return "1.0"; }
    virtual bool isReady() { return true; }
};
