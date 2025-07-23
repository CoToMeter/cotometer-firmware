#pragma once
#include <WString.h>

enum class SensorType {
    CO2_TEMP_HUMIDITY,
    PARTICULATE_MATTER,
    VOC_GAS,
    COMBINED
};

class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool initialize() = 0;
    virtual bool readData() = 0;
    virtual bool isReady() = 0;
    virtual String getDataJson() = 0;
    virtual SensorType getType() = 0;
    virtual float getValue(const String& parameter) = 0;
};
