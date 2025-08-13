/*
 * sensors/BME688Sensor.cpp
 * BME688 VOC/Gas sensor implementation using official Bosch BME68x library
 */

#include "sensors/BME688Sensor.h"

BME688Sensor::BME688Sensor(uint8_t address, uint8_t csPin) 
    : initialized(false)
    , i2cAddress(address)
    , csPin(csPin)
    , temperature(0.0)
    , humidity(0.0)
    , pressure(0.0)
    , gasResistance(0.0)
    , vocEstimate(0.0)
    , gasHeaterEnabled(true)
    , heaterTemp(320)      // Default: 320°C
    , heaterDuration(150)  // Default: 150ms
{
    currentData = VOCSensorData("BME688");
    lastError = "";
    
    // DON'T configure CS pin here - do it in initialize()
}

bool BME688Sensor::initialize() {
    Serial.println("🔧 Initializing BME688 sensor via SPI...");
    
    // Configure CS pin for SPI mode (controlled LOW/HIGH)
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);  // CS HIGH = not selected initially
    Serial.printf("🔧 CS pin %d configured for SPI mode\n", csPin);
    
    // Initialize SPI
    SPI.begin(18, 19, 23, csPin); // SCK=18, MISO=19, MOSI=23, CS=csPin
    SPI.setFrequency(1000000);    // 1MHz
    SPI.setDataMode(SPI_MODE0);   // CPOL=0, CPHA=0
    Serial.println("🔧 SPI bus initialized: SCK=18, MISO=19, MOSI=23");
    
    delay(100);
    
    // Initialize BME688 with SPI
    bme688.begin(csPin, SPI);
    
    // Check if sensor is responding
    if (bme688.checkStatus() == BME68X_ERROR) {
        lastError = "BME688 not found via SPI (CS pin " + String(csPin) + ")";
        Serial.println("❌ " + lastError);
        Serial.printf("💡 Check SPI wiring: SCK=18, MISO=19, MOSI=23, CS=%d\n", csPin);
        return false;
    }
    
    Serial.printf("✅ BME688 found via SPI (CS pin %d)\n", csPin);
    
    // Get sensor info
    uint32_t uniqueId = bme688.getUniqueId();
    Serial.printf("📟 BME688 Unique ID: 0x%08X\n", uniqueId);
    
    // Configure basic sensor settings
    if (!configureBasicSettings()) {
        lastError = "Failed to configure basic settings";
        return false;
    }
    
    // Configure gas heater
    if (!configureGasHeater()) {
        lastError = "Failed to configure gas heater";
        return false;
    }
    
    // Set forced mode for on-demand measurements
    bme688.setOpMode(BME68X_FORCED_MODE);
    
    initialized = true;
    Serial.println("✅ BME688 sensor initialized successfully via SPI");
    
    return true;
}

bool BME688Sensor::configureBasicSettings() {
    // Set oversampling rates
    // Temperature: 2x, Pressure: 16x, Humidity: 1x (good balance of accuracy vs speed)
    bme688.setTPH(BME68X_OS_2X, BME68X_OS_16X, BME68X_OS_1X);
    
    // Set IIR filter coefficient (reduces noise)
    bme688.setFilter(BME68X_FILTER_SIZE_3);
    
    // Set ambient temperature for better accuracy
    bme688.setAmbientTemp(25); // Assume 25°C ambient
    
    Serial.println("🔧 Basic settings configured:");
    Serial.println("   - Temperature oversampling: 2x");
    Serial.println("   - Pressure oversampling: 16x");
    Serial.println("   - Humidity oversampling: 1x");
    Serial.println("   - IIR filter: coefficient 4");
    
    return true;
}

bool BME688Sensor::configureGasHeater() {
    if (gasHeaterEnabled) {
        // Set heater temperature and duration for VOC measurements
        bme688.setHeaterProf(heaterTemp, heaterDuration);
        
        Serial.printf("🔥 Gas heater configured: %d°C for %dms\n", heaterTemp, heaterDuration);
    } else {
        Serial.println("🔥 Gas heater disabled");
    }
    
    return true;
}

bool BME688Sensor::readData() {
    if (!initialized) {
        lastError = "Sensor not initialized";
        return false;
    }
    
    // Set forced mode to trigger measurement
    bme688.setOpMode(BME68X_FORCED_MODE);
    
    // Wait for measurement to complete
    uint32_t measDuration = bme688.getMeasDur(BME68X_FORCED_MODE);
    delay(measDuration / 1000); // Convert microseconds to milliseconds
    
    // Check if data is available
    uint8_t nFieldsLeft = bme688.fetchData();
    
    if (nFieldsLeft == 0) {
        lastError = "No data available from BME688";
        return false;
    }
    
    // Get the data
    bme68xData data;
    bme688.getData(data);
    
    // Validate readings
    if (!validateReadings(data)) {
        lastError = "Invalid sensor readings";
        return false;
    }
    
    // Store the readings
    temperature = data.temperature;
    humidity = data.humidity;
    pressure = data.pressure;
    gasResistance = data.gas_resistance;
    
    // Calculate VOC estimate from gas resistance
    vocEstimate = calculateVOCEstimate(gasResistance, temperature, humidity);
    
    // Store readings in VOC sensor data structure
    currentData.temperature = temperature;
    currentData.humidity = humidity;
    currentData.pressure = pressure;
    currentData.gasResistance = gasResistance;
    currentData.vocEstimate = vocEstimate;
    currentData.heaterStable = (data.status & BME68X_HEAT_STAB_MSK) != 0;
    currentData.gasValid = (data.status & BME68X_GASM_VALID_MSK) != 0;
    currentData.updateTimestamp();
    currentData.setValid(true);
    
    lastError = "";
    
    // Debug output
    // Serial.printf("📊 BME688 Reading - Temp: %.1f°C, Humidity: %.1f%%, Gas: %.0fΩ, VOC: %.1fppb\n", 
    //               temperature, humidity, gasResistance, vocEstimate);
    
    return true;
}

bool BME688Sensor::validateReadings(const bme68xData& data) {
    // Check status flags
    if (gasHeaterEnabled && !(data.status & BME68X_GASM_VALID_MSK)) {
        lastError = "Gas measurement not valid";
        return false;
    }
    
    if (gasHeaterEnabled && !(data.status & BME68X_HEAT_STAB_MSK)) {
        Serial.println("⚠️  Warning: Heater not stable");
        // Don't fail, just warn
    }
    
    // Validate temperature range
    if (data.temperature < -40 || data.temperature > 85) {
        lastError = "Temperature out of range: " + String(data.temperature) + "°C";
        return false;
    }
    
    // Validate humidity range
    if (data.humidity < 0 || data.humidity > 100) {
        lastError = "Humidity out of range: " + String(data.humidity) + "%";
        return false;
    }
    
    // Validate pressure range (if measuring pressure)
    if (data.pressure < 30000 || data.pressure > 110000) {
        lastError = "Pressure out of range: " + String(data.pressure) + "Pa";
        return false;
    }
    
    return true;
}

float BME688Sensor::calculateVOCEstimate(float gasRes, float temp, float hum) {
    // Simple VOC estimation based on gas resistance
    // This is a basic algorithm - for production use, consider Bosch BSEC library
    
    if (gasRes <= 0) return 0.0;
    
    // Baseline resistance (clean air) - should be calibrated for your environment
    const float baselineResistance = 50000.0; // 50kΩ typical for clean air
    
    // Calculate ratio
    float ratio = baselineResistance / gasRes;
    
    // Convert to approximate VOC concentration (very rough estimation)
    float voc = 0.0;
    
    if (ratio > 1.0) {
        // Higher resistance = cleaner air, lower VOC
        voc = (ratio - 1.0) * 50.0; // Scale factor
    } else {
        // Lower resistance = more VOCs
        voc = (1.0 - ratio) * 200.0; // Different scale for contamination
    }
    
    // Apply temperature and humidity compensation (simplified)
    voc = voc * (1.0 + (temp - 25.0) * 0.01); // 1% per degree from 25°C
    voc = voc * (1.0 + (hum - 50.0) * 0.005); // 0.5% per % RH from 50%
    
    return max(0.0f, voc); // Ensure non-negative
}

SensorDataBase* BME688Sensor::getCurrentData() {
    return &currentData;
}

bool BME688Sensor::isReady() {
    if (!initialized) return false;
    
    // Check if measurement is complete
    uint8_t opMode = bme688.getOpMode();
    return (opMode == BME68X_SLEEP_MODE); // Ready when not measuring
}

String BME688Sensor::getLastError() {
    return lastError;
}

// BME688-specific getters
float BME688Sensor::getTemperature() const { return temperature; }
float BME688Sensor::getHumidity() const { return humidity; }
float BME688Sensor::getPressure() const { return pressure; }
float BME688Sensor::getGasResistance() const { return gasResistance; }
float BME688Sensor::getVOCEstimate() const { return vocEstimate; }

bool BME688Sensor::setI2CAddress(uint8_t address) {
    if (address != 0x76 && address != 0x77) {
        lastError = "Invalid I2C address. Use 0x76 or 0x77";
        return false;
    }
    i2cAddress = address;
    return true;
}

uint8_t BME688Sensor::getI2CAddress() const {
    return i2cAddress;
}

bool BME688Sensor::setCSPin(uint8_t pin) {
    csPin = pin;
    if (initialized) {
        pinMode(csPin, OUTPUT);
        digitalWrite(csPin, HIGH);  // Keep I2C mode
        Serial.printf("🔧 CS pin changed to %d\n", csPin);
    }
    return true;
}

uint8_t BME688Sensor::getCSPin() const {
    return csPin;
}

bool BME688Sensor::enableGasSensor(bool enable) {
    gasHeaterEnabled = enable;
    return configureGasHeater();
}

bool BME688Sensor::setHeaterProfile(uint16_t temperature, uint16_t duration) {
    heaterTemp = temperature;
    heaterDuration = duration;
    
    if (initialized) {
        bme688.setHeaterProf(heaterTemp, heaterDuration);
        Serial.printf("🔥 Heater profile updated: %d°C for %dms\n", heaterTemp, heaterDuration);
    }
    
    return true;
}

bool BME688Sensor::softReset() {
    if (!initialized) return false;
    
    bme688.softReset();
    delay(10); // Wait for reset
    
    // Reconfigure after reset
    return configureBasicSettings() && configureGasHeater();
}

String BME688Sensor::getSensorVariant() {
    // This would need to be implemented based on variant ID reading
    // For now, return a generic string
    return "BME688";
}

bool BME688Sensor::performSelfTest() {
    // BME688 doesn't have a built-in self-test like SCD41
    // We can do a basic functionality test
    if (!initialized) return false;
    
    // Try to read data
    bool testResult = readData();
    
    if (testResult) {
        Serial.println("✅ BME688 self-test passed");
    } else {
        Serial.println("❌ BME688 self-test failed: " + lastError);
    }
    
    return testResult;
}

uint32_t BME688Sensor::getUniqueId() {
    if (!initialized) return 0;
    return bme688.getUniqueId();
}

// // Helper namespace implementation
// namespace BME688Utils {
//     String operationModeToString(uint8_t mode) {
//         switch (mode) {
//             case BME68X_SLEEP_MODE: return "Sleep";
//             case BME68X_FORCED_MODE: return "Forced";
//             case BME68X_PARALLEL_MODE: return "Parallel";
//             case BME68X_SEQUENTIAL_MODE: return "Sequential";
//             default: return "Unknown";
//         }
//     }
    
//     AlertLevel vocToAlertLevel(float vocEstimate) {
//         if (vocEstimate < 50) return AlertLevel::GOOD;
//         if (vocEstimate < 100) return AlertLevel::FAIR;
//         if (vocEstimate < 200) return AlertLevel::POOR;
//         return AlertLevel::BAD;
//     }
// }