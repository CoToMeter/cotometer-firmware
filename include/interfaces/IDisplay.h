#pragma once
#include "../types/SensorData.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Essential operations only
    virtual bool initialize() = 0;
    virtual void showSensorData(const SensorDataBase& data) = 0;
    virtual void showMessage(const String& message) = 0;
    virtual void showError(const String& error) = 0;
};
