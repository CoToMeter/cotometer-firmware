#include "CCS811Sensor.h"
#include <Arduino.h>

// Constructor with default I2C address
CCS811Sensor::CCS811Sensor(uint8_t i2cAddress) : ccs811(i2cAddress) {
    // Constructor body (empty)
}

// Initialize the CCS811 sensor
bool CCS811Sensor::begin(TwoWire &wirePort) {
    if (!ccs811.begin(wirePort)) {
        Serial.println("CCS811 initialization failed. Check connections.");
        return false;
    }

    // Wait for the sensor to boot up (recommended by SparkFun)
    delay(1000);

    // Start the CCS811 application firmware
    if (!ccs811.appValid()) {
        Serial.println("CCS811 app firmware not valid. Attempting to restart...");
        if (!ccs811.begin(wirePort)) { // Re-initialize if needed
            Serial.println("Failed to start CCS811 app firmware.");
            return false;
        }
    }

    // Set Drive Mode to 1 measurement per second
    // Replace 'ModeConstantPower1' with the correct enum or constant from the new library
    if (ccs811.setDriveMode(1) != CCS811::CCS811_Status_e::CCS811_Stat_SUCCESS) {
        Serial.println("Failed to set CCS811 Drive Mode.");
        return false;
    }

    Serial.println("CCS811 initialized successfully.");
    return true;
}

// Read eCO2 and TVOC data
bool CCS811Sensor::readData(uint16_t &eCO2, float &TVOC) {
    if (ccs811.dataAvailable()) {
        if (ccs811.readAlgorithmResults()) {
            Serial.println("Error reading CCS811 algorithm results.");
            return false;
        }

        eCO2 = ccs811.getCO2();
        TVOC = ccs811.getTVOC();

        return true;
    }

    return false; // No new data available
}

// Set environmental data for improved accuracy
void CCS811Sensor::setEnvironmentalData(float temperature, float humidity) {
    ccs811.setEnvironmentalData(temperature, humidity);
}

// Get human-readable status string
const char* CCS811Sensor::getStatusString() {
    return ccs811.statusString();
}