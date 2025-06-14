#ifndef SENSOR_H
#define SENSOR_H

#include <SparkFun_SCD30_Arduino_Library.h>

class SensorCO2 {
public:
    SensorCO2();
    bool begin();
    bool readData(uint16_t& co2, float& temperature, float& humidity);
    void setMeasurementInterval(uint16_t interval);
    void enableAutoSelfCalibration(bool enable);

private:
    SCD30 scd30;
};

#endif // SENSOR_H