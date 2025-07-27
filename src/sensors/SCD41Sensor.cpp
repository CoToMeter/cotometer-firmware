/*
 * sensors/SCD41Sensor.cpp
 * Implementation using the real Sensirion I2C SCD4x library v1.1.0
 */

#include "sensors/SCD41Sensor.h"
#include <SensirionI2cScd4x.h>

// Create sensor instance
SensirionI2cScd4x scd4x;

SCD41Sensor::SCD41Sensor() : initialized(false) {
    currentData = SensorData();
    lastError = "";
}

bool SCD41Sensor::initialize() {
    Serial.println("🔧 Initializing SCD41 sensor...");
    
    // Initialize I2C communication
    Wire.begin(21, 22); // SDA=21, SCL=22 for ESP32-S3
    Wire.setClock(100000); // 100kHz for reliability
    
    // Initialize sensor with I2C address 0x62
    scd4x.begin(Wire, SCD41_I2C_ADDR_62);
    
    // Stop potentially running measurement
    int16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.printf("⚠️  Warning stopping measurement: %d\n", error);
    }
    delay(1000);
    
    // Get sensor serial number for verification
    uint64_t serialNumber;
    error = scd4x.getSerialNumber(serialNumber);
    if (error) {
        lastError = "Failed to get serial number. Check connections!";
        Serial.println("❌ SCD41 initialization failed: " + lastError);
        Serial.printf("💡 Error code: %d\n", error);
        return false;
    }
    
    Serial.printf("✅ SCD41 Serial: 0x%016llX\n", serialNumber);
    
    // Check sensor variant
    uint16_t variant;
    error = scd4x.getSensorVariantRaw(variant);
    if (error == 0) {
        switch (variant & SCD4X_SENSOR_VARIANT_MASK) {
            case SCD4X_SENSOR_VARIANT_SCD40:
                Serial.println("📡 Detected: SCD40");
                break;
            case SCD4X_SENSOR_VARIANT_SCD41:
                Serial.println("📡 Detected: SCD41");
                break;
            case SCD4X_SENSOR_VARIANT_SCD42:
                Serial.println("📡 Detected: SCD42");
                break;
            case SCD4X_SENSOR_VARIANT_SCD43:
                Serial.println("📡 Detected: SCD43");
                break;
            default:
                Serial.printf("📡 Detected: Unknown variant (0x%04X)\n", variant);
                break;
        }
    }
    
    // Enable automatic self-calibration by default
    error = scd4x.setAutomaticSelfCalibrationEnabled(1);
    if (error) {
        Serial.printf("⚠️  Warning: Could not enable auto-calibration: %d\n", error);
    } else {
        Serial.println("🔄 Automatic self-calibration enabled");
    }
    
    // Start periodic measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        lastError = "Failed to start measurement";
        Serial.println("❌ SCD41 measurement start failed: " + lastError);
        Serial.printf("💡 Error code: %d\n", error);
        return false;
    }
    
    initialized = true;
    Serial.println("✅ SCD41 sensor initialized successfully");
    Serial.println("🔄 Waiting for first measurement (5 seconds)...");
    
    return true;
}

bool SCD41Sensor::readData() {
    if (!initialized) {
        lastError = "Sensor not initialized";
        return false;
    }
    
    // Check if data is ready
    bool dataReady = false;
    int16_t error = scd4x.getDataReadyStatus(dataReady);
    if (error) {
        lastError = "Error checking data ready flag";
        Serial.printf("❌ Data ready check error: %d\n", error);
        return false;
    }
    
    if (!dataReady) {
        lastError = "Data not ready yet";
        return false;
    }
    
    // Read measurement
    uint16_t co2;
    float temperature;
    float humidity;
    
    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        lastError = "Error reading measurement";
        Serial.printf("❌ Read measurement error: %d\n", error);
        return false;
    }
    
    // Validate readings
    if (co2 == 0) {
        lastError = "Invalid CO2 reading (0 ppm)";
        return false;
    }
    
    // Update sensor data
    currentData.co2 = co2;
    currentData.temperature = temperature;
    currentData.humidity = humidity;
    currentData.updateTimestamp();
    currentData.valid = true;
    
    lastError = "";
    
    // Debug output
    // Serial.printf("📊 SCD41 Reading - CO2: %d ppm, Temp: %.1f°C, Humidity: %.1f%%\n", 
    //               co2, temperature, humidity);
    
    return true;
}

SensorData SCD41Sensor::getCurrentData() {
    return currentData;
}

bool SCD41Sensor::isReady() {
    if (!initialized) {
        return false;
    }
    
    bool dataReady = false;
    int16_t error = scd4x.getDataReadyStatus(dataReady);
    return (error == 0) && dataReady;
}

String SCD41Sensor::getLastError() {
    return lastError;
}

bool SCD41Sensor::performForcedRecalibration(uint16_t targetCO2) {
    if (!initialized) {
        lastError = "Sensor not initialized";
        return false;
    }
    
    Serial.printf("🔧 Performing forced recalibration to %d ppm...\n", targetCO2);
    
    // Stop measurement for calibration
    int16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        lastError = "Failed to stop measurement for calibration";
        return false;
    }
    delay(500);
    
    // Perform calibration
    uint16_t frcCorrection;
    error = scd4x.performForcedRecalibration(targetCO2, frcCorrection);
    if (error) {
        lastError = "Forced recalibration failed";
        Serial.printf("❌ Calibration error: %d\n", error);
        // Restart measurement even if calibration failed
        scd4x.startPeriodicMeasurement();
        return false;
    }
    
    // Check if calibration was successful (0xFFFF indicates failure)
    if (frcCorrection == 0xFFFF) {
        lastError = "Calibration failed - sensor not ready";
        Serial.println("❌ Calibration failed - sensor was not operated long enough");
        scd4x.startPeriodicMeasurement();
        return false;
    }
    
    // Convert correction value (subtract 0x8000 to get signed value)
    int16_t correctionPpm = frcCorrection - 0x8000;
    Serial.printf("✅ Calibration successful. Correction: %d ppm\n", correctionPpm);
    
    // Restart measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        lastError = "Failed to restart measurement after calibration";
        return false;
    }
    
    return true;
}

bool SCD41Sensor::setAutomaticSelfCalibration(bool enabled) {
    if (!initialized) {
        lastError = "Sensor not initialized";
        return false;
    }
    
    // Stop measurement to change settings
    int16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        lastError = "Failed to stop measurement";
        return false;
    }
    delay(500);
    
    error = scd4x.setAutomaticSelfCalibrationEnabled(enabled ? 1 : 0);
    if (error) {
        lastError = enabled ? "Failed to enable auto-calibration" : "Failed to disable auto-calibration";
        Serial.printf("❌ Error setting auto-calibration: %d\n", error);
        scd4x.startPeriodicMeasurement(); // Restart measurement
        return false;
    }
    
    Serial.println(enabled ? "🔄 Automatic self-calibration enabled" : "⏸️ Automatic self-calibration disabled");
    
    // Restart measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        lastError = "Failed to restart measurement";
        return false;
    }
    
    return true;
}

bool SCD41Sensor::getAutomaticSelfCalibration() {
    if (!initialized) {
        return false;
    }
    
    // Stop measurement to read settings
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    uint16_t ascEnabled;
    int16_t error = scd4x.getAutomaticSelfCalibrationEnabled(ascEnabled);
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    if (error) {
        return false;
    }
    
    return ascEnabled != 0;
}

bool SCD41Sensor::setSensorAltitude(uint16_t altitude) {
    if (!initialized) {
        return false;
    }
    
    // Stop measurement to change settings
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.setSensorAltitude(altitude);
    bool success = (error == 0);
    
    if (success) {
        Serial.printf("✅ Sensor altitude set to %d meters\n", altitude);
    } else {
        Serial.printf("❌ Failed to set altitude: %d\n", error);
    }
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return success;
}

bool SCD41Sensor::setTemperatureOffset(float offset) {
    if (!initialized) {
        return false;
    }
    
    // Stop measurement to change settings
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.setTemperatureOffset(offset);
    bool success = (error == 0);
    
    if (success) {
        Serial.printf("✅ Temperature offset set to %.1f°C\n", offset);
    } else {
        Serial.printf("❌ Failed to set temperature offset: %d\n", error);
    }
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return success;
}

bool SCD41Sensor::setAmbientPressure(uint32_t pressure) {
    if (!initialized) {
        return false;
    }
    
    int16_t error = scd4x.setAmbientPressure(pressure);
    bool success = (error == 0);
    
    if (success) {
        Serial.printf("✅ Ambient pressure set to %lu Pa\n", pressure);
    } else {
        Serial.printf("❌ Failed to set ambient pressure: %d\n", error);
    }
    
    return success;
}

bool SCD41Sensor::getSensorSerialNumber(uint64_t& serialNumber) {
    if (!initialized) {
        return false;
    }
    
    // Stop measurement to read info
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.getSerialNumber(serialNumber);
    bool success = (error == 0);
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return success;
}

bool SCD41Sensor::performSelfTest() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("🧪 Performing sensor self-test...");
    
    // Stop measurement for self-test
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    uint16_t sensorStatus;
    int16_t error = scd4x.performSelfTest(sensorStatus);
    
    if (error) {
        Serial.printf("❌ Self-test command failed: %d\n", error);
        scd4x.startPeriodicMeasurement();
        return false;
    }
    
    bool testPassed = (sensorStatus == 0);
    if (testPassed) {
        Serial.println("✅ Self-test passed - sensor is functioning correctly");
    } else {
        Serial.printf("❌ Self-test failed - sensor status: 0x%04X\n", sensorStatus);
    }
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return testPassed;
}

bool SCD41Sensor::getSensorVariant(uint16_t& variant) {
    if (!initialized) {
        return false;
    }
    
    // Stop measurement to read info
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.getSensorVariantRaw(variant);
    bool success = (error == 0);
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return success;
}

bool SCD41Sensor::powerDown() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("💤 Putting sensor to sleep...");
    
    // Stop measurement first
    int16_t error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.printf("❌ Failed to stop measurement: %d\n", error);
        return false;
    }
    delay(500);
    
    // Power down (SCD41 only)
    error = scd4x.powerDown();
    if (error) {
        Serial.printf("❌ Failed to power down: %d\n", error);
        return false;
    }
    
    Serial.println("✅ Sensor powered down");
    return true;
}

bool SCD41Sensor::wakeUp() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("⏰ Waking up sensor...");
    
    // Wake up sensor (SCD41 only)
    int16_t error = scd4x.wakeUp();
    if (error) {
        Serial.printf("❌ Failed to wake up: %d\n", error);
        return false;
    }
    
    delay(1000); // Give sensor time to wake up
    
    // Restart measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.printf("❌ Failed to restart measurement: %d\n", error);
        return false;
    }
    
    Serial.println("✅ Sensor awakened and measurement restarted");
    return true;
}

bool SCD41Sensor::measureSingleShot(uint16_t& co2, float& temp, float& humidity) {
    if (!initialized) {
        return false;
    }
    
    // Use the convenience method that handles timing
    int16_t error = scd4x.measureAndReadSingleShot(co2, temp, humidity);
    if (error) {
        lastError = "Single shot measurement failed";
        Serial.printf("❌ Single shot error: %d\n", error);
        return false;
    }
    
    Serial.printf("📊 Single shot - CO2: %d ppm, Temp: %.1f°C, Humidity: %.1f%%\n", 
                  co2, temp, humidity);
    return true;
}

bool SCD41Sensor::persistSettings() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("💾 Persisting settings to EEPROM...");
    
    // Stop measurement to persist settings
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.persistSettings();
    bool success = (error == 0);
    
    if (success) {
        Serial.println("✅ Settings saved to EEPROM");
    } else {
        Serial.printf("❌ Failed to persist settings: %d\n", error);
    }
    
    // Restart measurement
    scd4x.startPeriodicMeasurement();
    
    return success;
}

bool SCD41Sensor::performFactoryReset() {
    if (!initialized) {
        return false;
    }
    
    Serial.println("🏭 Performing factory reset...");
    
    // Stop measurement for factory reset
    scd4x.stopPeriodicMeasurement();
    delay(500);
    
    int16_t error = scd4x.performFactoryReset();
    bool success = (error == 0);
    
    if (success) {
        Serial.println("✅ Factory reset completed");
        Serial.println("🔄 Reinitializing sensor...");
        
        // Reinitialize after factory reset
        delay(1000);
        initialized = false;
        return initialize();
    } else {
        Serial.printf("❌ Factory reset failed: %d\n", error);
        scd4x.startPeriodicMeasurement();
    }
    
    return success;
}