// File: include/Constants.h
#pragma once
#include <Arduino.h>

namespace Constants {
    // Event system constants
    const size_t MAX_EVENT_QUEUE_SIZE = 50;
    
    // Sensor constants
    const uint32_t SENSOR_READ_TIMEOUT_MS = 5000;
    const uint32_t SENSOR_WARMUP_TIME_MS = 3000;
    
    // Communication constants
    const uint32_t BLE_CONNECTION_TIMEOUT_MS = 30000;
    const uint32_t DATA_TRANSMISSION_INTERVAL_MS = 5000;
    
    // Power management
    const uint8_t BATTERY_LOW_THRESHOLD = 20;      // %
    const uint8_t BATTERY_CRITICAL_THRESHOLD = 10; // %
    
    // Display constants
    const uint32_t DISPLAY_TIMEOUT_MS = 30000;     // 30 seconds
    const uint8_t DISPLAY_BRIGHTNESS_DEFAULT = 128; // 0-255
    
    // Measurement intervals
    const uint32_t MEASUREMENT_INTERVAL_FAST_MS = 5000;    // 5 seconds
    const uint32_t MEASUREMENT_INTERVAL_NORMAL_MS = 10000; // 10 seconds
    const uint32_t MEASUREMENT_INTERVAL_SLOW_MS = 30000;   // 30 seconds
    
    // Calibration
    const uint32_t AUTO_CALIBRATION_INTERVAL_HOURS = 24;
    const uint16_t CO2_CALIBRATION_TARGET_PPM = 400;
    
    // GPIO pins (ESP32-S3)
    namespace GPIO {
        const int SDA_PIN = 21;
        const int SCL_PIN = 22;
        const int SPI_SCK_PIN = 18;
        const int SPI_MISO_PIN = 19;
        const int SPI_MOSI_PIN = 23;
        const int BME688_CS_PIN = 4;
        const int BUTTON_PIN = 0;
        const int BATTERY_ADC_PIN = 36;
        const int SENSOR_POWER_PIN = 2;
    }
    
    // I2C addresses
    namespace I2C {
        const uint8_t SCD41_ADDRESS = 0x62;
        const uint8_t BME688_ADDRESS_1 = 0x76;
        const uint8_t BME688_ADDRESS_2 = 0x77;
    }
    
    // Thresholds
    namespace Thresholds {
        const uint16_t CO2_GOOD_MAX = 600;        // ppm
        const uint16_t CO2_FAIR_MAX = 800;        // ppm
        const uint16_t CO2_POOR_MAX = 1200;       // ppm
        const uint16_t CO2_BAD_MAX = 2000;        // ppm
        
        const float VOC_GOOD_MAX = 50.0;          // ppb
        const float VOC_FAIR_MAX = 100.0;         // ppb
        const float VOC_POOR_MAX = 200.0;         // ppb
        
        const float TEMP_COMFORT_MIN = 20.0;      // °C
        const float TEMP_COMFORT_MAX = 24.0;      // °C
        
        const float HUMIDITY_COMFORT_MIN = 40.0;  // %
        const float HUMIDITY_COMFORT_MAX = 60.0;  // %
    }
    
    // Firmware info
    const String FIRMWARE_VERSION = "2.0.0";
    const String DEVICE_NAME = "CoToMeter";
    const String MANUFACTURER = "CoTo Industries";
}