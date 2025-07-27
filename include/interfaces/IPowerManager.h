#pragma once
#include <Arduino.h>
#include <functional>

class IPowerManager {
public:
    enum class PowerMode {
        NORMAL = 0,
        ECO = 1,
        DEEP_SLEEP = 2
    };
    
    struct BatteryInfo {
        float voltage;              // Battery voltage (V)
        uint8_t percentage;         // Battery percentage (0-100)
        bool isCharging;           // Is battery charging
        bool isLowBattery;         // Is battery below threshold
    };
    
    virtual ~IPowerManager() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Sleep modes (must implement)
    virtual void enterDeepSleep(uint64_t sleepTimeUs) = 0;
    virtual void wakeup() = 0;
    virtual bool isSleeping() = 0;
    
    // Battery monitoring (must implement)
    virtual float getBatteryVoltage() = 0;
    virtual uint8_t getBatteryPercentage() = 0;
    virtual bool isLowBattery() = 0;
    virtual bool isCharging() = 0;
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Power control
    virtual void enableSensorPower() { /* optional */ }
    virtual void disableSensorPower() { /* optional */ }
    virtual void enableDisplayPower() { /* optional */ }
    virtual void disableDisplayPower() { /* optional */ }
    
    // Power modes
    virtual void setPowerMode(PowerMode mode) { /* optional */ }
    virtual PowerMode getPowerMode() { return PowerMode::NORMAL; }
    
    // Battery info
    virtual BatteryInfo getBatteryInfo() {
        BatteryInfo info;
        info.voltage = getBatteryVoltage();
        info.percentage = getBatteryPercentage();
        info.isCharging = isCharging();
        info.isLowBattery = isLowBattery();
        return info;
    }
    
    // Callbacks
    virtual void setLowBatteryCallback(std::function<void()> callback) { /* optional */ }
    virtual void setChargingCallback(std::function<void(bool)> callback) { /* optional */ }
    
    // Configuration
    virtual void setLowBatteryThreshold(uint8_t percentage) { /* optional */ }
    virtual uint8_t getLowBatteryThreshold() { return 20; }
};
