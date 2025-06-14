#ifndef CCS811SENSOR_H
#define CCS811SENSOR_H

#include <SparkFunCCS811.h>
#include <Wire.h>

class CCS811Sensor {
public:
    // Constructor with optional I2C address (default is 0x5B)
    CCS811Sensor(uint8_t i2cAddress = 0x5B);

    // Initialize the sensor
    bool begin(TwoWire &wirePort = Wire);

    // Read eCO2 and TVOC data
    bool readData(uint16_t &eCO2, float &TVOC);

    // Set environmental data for improved accuracy
    void setEnvironmentalData(float temperature, float humidity);

    // Get human-readable status string
    const char* getStatusString();

private:
    CCS811 ccs811; // Instance of the CCS811 sensor
};

#endif // CCS811SENSOR_H