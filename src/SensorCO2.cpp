#include "SensorCO2.h"

SensorCO2::SensorCO2() {}

bool SensorCO2::begin() {
    if (scd30.begin()) {
        Serial.println("SCD30 Sensor Found and Initialized Successfully!");
        return true;
    } else {
        Serial.println("SCD30 Sensor Not Found. Please Check Connections.");
        return false;
    }
}

bool SensorCO2::readData(uint16_t& co2, float& temperature, float& humidity) {
    if (scd30.dataAvailable()) {
        if (scd30.readMeasurement()) {
            co2 = scd30.getCO2();
            temperature = scd30.getTemperature();
            humidity = scd30.getHumidity();
            return true;
        } else {
            Serial.println("Error reading data from SCD30 Sensor.");
        }
    }
    return false;
}

void SensorCO2::setMeasurementInterval(uint16_t interval) {
    scd30.setMeasurementInterval(interval);
    Serial.println("SCD30 Measurement Interval Set to " + String(interval) + " seconds");
}

void SensorCO2::enableAutoSelfCalibration(bool enable) {
    scd30.setAutoSelfCalibration(enable);
    Serial.println("SCD30 Auto Self-Calibration " + String(enable ? "Enabled" : "Disabled"));
}