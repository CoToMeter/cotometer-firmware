/*
 * interfaces/ICommunication.h
 * Interface for all communication implementations
 */

#pragma once

#include <Arduino.h>
#include <functional>

class ICommunication {
public:
    using DataCallback = std::function<void(const String&)>;
    using StatusCallback = std::function<void(bool)>;
    
    virtual ~ICommunication() = default;
    
    // Core communication operations
    virtual bool initialize() = 0;
    virtual bool isConnected() = 0;
    virtual void disconnect() = 0;
    virtual bool isReady() = 0;
    
    // Data transmission
    virtual bool sendData(const String& data) = 0;
    virtual bool sendCommand(const String& command) = 0;
    virtual String receiveData() = 0;
    virtual bool hasDataAvailable() = 0;
    
    // Callback management
    virtual void setDataCallback(DataCallback callback) = 0;
    virtual void setStatusCallback(StatusCallback callback) = 0;
    virtual void removeCallbacks() = 0;
    
    // Connection management
    virtual bool startAdvertising() = 0;
    virtual bool stopAdvertising() = 0;
    virtual bool isAdvertising() = 0;
    
    // Configuration
    virtual void setDeviceName(const String& name) = 0;
    virtual String getDeviceName() = 0;
    virtual void setConnectionParameters(const String& params) = 0;
    
    // Status and diagnostics
    virtual int getSignalStrength() = 0;
    virtual uint32_t getBytesTransmitted() = 0;
    virtual uint32_t getBytesReceived() = 0;
    virtual String getLastError() = 0;
    virtual void clearError() = 0;
    
    // Power management
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
    virtual bool isSleeping() = 0;
};