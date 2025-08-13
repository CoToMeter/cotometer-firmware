#pragma once
#include "../types/SensorData.h"

class ISensor {
public:
    virtual ~ISensor() = default;
    
    // Essential operations only
    virtual bool initialize() = 0;
    virtual bool readData() = 0;
    virtual SensorDataBase* getCurrentData() = 0;  
    virtual bool isReady() = 0;
    virtual String getLastError() = 0;
};

