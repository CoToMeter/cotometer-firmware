#pragma once
#include <Arduino.h>
#include "../types/SensorData.h"
#include "../types/DeviceConfig.h"

class IDataStorage {
public:
    virtual ~IDataStorage() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Configuration storage (must implement)
    virtual bool saveConfig(const DeviceConfig& config) = 0;
    virtual bool loadConfig(DeviceConfig& config) = 0;
    virtual bool hasConfig() = 0;
    
    // Data storage (must implement) 
    virtual bool saveData(const SensorData& data) = 0;
    virtual bool loadData(SensorData& data, uint32_t index = 0) = 0;
    virtual uint32_t getDataCount() = 0;
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Storage management
    virtual bool clear() { return false; }
    virtual bool format() { return false; }
    virtual uint32_t getUsedSpace() { return 0; }
    virtual uint32_t getFreeSpace() { return 1000000; } // 1MB default
    
    // Data export
    virtual String exportData(uint32_t count = 10) { return "{}"; }
    virtual bool importData(const String& jsonData) { return false; }
    
    // Storage info
    virtual String getStorageType() { return "preferences"; }
    virtual bool isReady() { return true; }
};

