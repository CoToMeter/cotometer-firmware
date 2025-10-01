#pragma once
#include <Arduino.h>
#include <functional>
#include "../types/SensorData.h"

class ICommunication {
public:
    using DataCallback = std::function<void(const String&)>;
    using StatusCallback = std::function<void(bool)>;
    
    virtual ~ICommunication() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Core operations (must implement)
    virtual bool initialize() = 0;
    virtual bool isConnected() = 0;
    virtual void disconnect() = 0;
    virtual bool isReady() = 0;
    
    // Data transmission (must implement)
    virtual bool sendData(const String& data) = 0;
    virtual bool sendSensorData(const SensorDataBase& data) = 0;
    virtual String receiveData() = 0;
    virtual bool hasDataAvailable() = 0;
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Connection management
    virtual bool startAdvertising() { return false; }
    virtual bool stopAdvertising() { return false; }
    virtual bool isAdvertising() { return false; }
    
    // Callbacks
    virtual void setDataCallback(DataCallback callback) { /* optional */ }
    virtual void setStatusCallback(StatusCallback callback) { /* optional */ }
    
    // Configuration
    virtual void setDeviceName(const String& name) { /* optional */ }
    virtual String getDeviceName() { return "CoToMeter"; }
    
    // Status
    virtual int getSignalStrength() { return -50; } // dBm
    virtual String getLastError() { return ""; }
    
    // Power management
    virtual void sleep() { /* optional */ }
    virtual void wakeup() { /* optional */ }
};

