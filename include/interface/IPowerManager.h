/*
 * interfaces/IPowerManager.h
 * Interface for power management
 */

#pragma once

#include <Arduino.h>

class IPowerManager {
public:
    enum class PowerMode {
        NORMAL,
        LOW_POWER,
        ULTRA_LOW_POWER,
        DEEP_SLEEP
    };
    
    struct BatteryInfo {
        float voltage;
        uint8_t percentage;
        bool isCharging;
        bool isLowBattery;
        uint32_t estimatedTimeLeft; // minutes
    };
    
    virtual ~IPowerManager() = default;
    
    // Sleep modes
    virtual void enterDeepSleep(uint64_t sleepTimeUs) = 0;
    virtual void enterLightSleep() = 0;
    virtual void wakeup() = 0;
    virtual bool isSleeping() = 0;
    
    // Battery monitoring
    virtual float getBatteryVoltage() = 0;
    virtual uint8_t getBatteryPercentage() = 0;
    virtual BatteryInfo getBatteryInfo() = 0;
    virtual bool isLowBattery() = 0;
    virtual bool isCharging() = 0;
    
    // Power control
    virtual void enableSensorPower() = 0;
    virtual void disableSensorPower() = 0;
    virtual bool isSensorPowerEnabled() = 0;
    
    virtual void enableDisplayPower() = 0;
    virtual void disableDisplayPower() = 0;
    virtual bool isDisplayPowerEnabled() = 0;
    
    virtual void enableCommunicationPower() = 0;
    virtual void disableCommunicationPower() = 0;
    virtual bool isCommunicationPowerEnabled() = 0;
    
    // Power modes
    virtual void setPowerMode(PowerMode mode) = 0;
    virtual PowerMode getPowerMode() = 0;
    virtual void setLowPowerMode(bool enabled) = 0;
    
    // Power monitoring
    virtual float getCurrentConsumption() = 0; // mA
    virtual uint32_t getUptimeMs() = 0;
    virtual uint32_t getEstimatedRuntimeMs() = 0;
    
    // Callbacks
    virtual void setLowBatteryCallback(std::function<void()> callback) = 0;
    virtual void setChargingCallback(std::function<void(bool)> callback) = 0;
    
    // Configuration
    virtual void setLowBatteryThreshold(uint8_t percentage) = 0;
    virtual void setCriticalBatteryThreshold(uint8_t percentage) = 0;
    virtual void setAutoSleepTimeout(uint32_t timeoutMs) = 0;
};