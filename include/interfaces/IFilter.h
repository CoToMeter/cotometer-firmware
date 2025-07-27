#pragma once
#include <Arduino.h>

class IFilter {
public:
    virtual ~IFilter() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Data filtering (must implement)
    virtual float filter(float newValue) = 0;
    virtual void reset() = 0;
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Filter state
    virtual bool isReady() { return true; }
    virtual float getCurrentValue() { return 0.0; }
    virtual String getFilterType() { return "unknown"; }
    
    // Configuration
    virtual void setParameter(const String& param, float value) { /* optional */ }
    virtual float getParameter(const String& param) { return 0.0; }
};