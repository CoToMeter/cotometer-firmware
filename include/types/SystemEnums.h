/*
 * types/SystemEnums.h
 * System-wide enumerations - header only
 */

#pragma once
#include <Arduino.h>

// ================================
// SENSOR TYPES
// ================================

enum class SensorType {
    UNKNOWN = 0,
    CO2_TEMP_HUMIDITY = 1,   // SCD41
    PARTICULATE_MATTER = 2,   // PMS7003
    VOC_GAS = 3,             // BME688
    COMBINED = 4             // Multiple sensors
};

// ================================
// DISPLAY TYPES
// ================================

enum class ScreenType {
    BOOT = 0,
    MAIN = 1,
    SETTINGS = 2,
    CALIBRATION = 3,
    STATISTICS = 4,
    DIAGNOSTICS = 5,
    SLEEP = 6,
    ERROR = 7,
    UPDATE = 8,
    FACTORY_RESET = 9
};

// ================================
// SYSTEM STATES
// ================================

enum class SystemState {
    UNDEFINED = 0,
    INITIALIZING = 1,
    RUNNING = 2,
    SLEEPING = 3,
    CALIBRATING = 4,
    ERROR = 5,
    UPDATING = 6,
    MAINTENANCE = 7,
    FACTORY_RESET = 8,
    SHUTTING_DOWN = 9
};

// ================================
// EVENT TYPES
// ================================

enum class EventType {
    // User input events
    BUTTON_PRESS = 100,
    BUTTON_LONG_PRESS = 101,
    BUTTON_DOUBLE_PRESS = 102,
    
    // Sensor events
    SENSOR_DATA_READY = 200,
    SENSOR_ERROR = 201,
    SENSOR_CALIBRATION_NEEDED = 202,
    SENSOR_DISCONNECTED = 203,
    
    // Communication events
    BLE_CONNECTED = 300,
    BLE_DISCONNECTED = 301,
    BLE_DATA_RECEIVED = 302,
    
    // System events
    THRESHOLD_EXCEEDED = 400,
    LOW_BATTERY = 401,
    CRITICAL_BATTERY = 402,
    
    // Timer events
    TIMER_EXPIRED = 500,
    MEASUREMENT_TIMER = 501,
    
    // Error events
    ERROR_OCCURRED = 600,
    SYSTEM_RESET = 601,
    
    // Calibration events
    CALIBRATION_START = 700,
    CALIBRATION_COMPLETE = 701,
    CALIBRATION_FAILED = 702
};

// ================================
// ALERT LEVELS
// ================================

enum class AlertLevel {
    NONE = 0,
    INFO = 1,
    WARNING = 2,
    CRITICAL = 3,
    EMERGENCY = 4
};

// ================================
// DATA QUALITY LEVELS
// ================================

enum class DataQuality {
    INVALID = 0,
    POOR = 1,
    FAIR = 2,
    GOOD = 3,
    EXCELLENT = 4
};

// ================================
// COMMUNICATION PROTOCOLS
// ================================

enum class CommunicationType {
    NONE = 0,
    BLE = 1,
    WIFI = 2,
    SERIAL_ = 3
};

// ================================
// CALIBRATION TYPES
// ================================

enum class CalibrationType {
    NONE = 0,
    ZERO_POINT = 1,
    SPAN = 2,
    MULTI_POINT = 3,
    FACTORY = 4,
    USER = 5
};

// ================================
// POWER MODES
// ================================

enum class PowerMode {
    NORMAL = 0,
    ECO = 1,
    LOW_POWER = 2,
    ULTRA_LOW_POWER = 3,
    DEEP_SLEEP = 4
};

// ================================
// DISPLAY MODES
// ================================

enum class DisplayMode {
    ALWAYS_ON = 0,
    AUTO_BRIGHTNESS = 1,
    POWER_SAVE = 2,
    SLEEP_MODE = 3
};

// ================================
// UTILITY FUNCTIONS
// ================================

inline String sensorTypeToString(SensorType type) {
    switch (type) {
        case SensorType::CO2_TEMP_HUMIDITY: return "CO2_TEMP_HUMIDITY";
        case SensorType::PARTICULATE_MATTER: return "PARTICULATE_MATTER";
        case SensorType::VOC_GAS: return "VOC_GAS";
        case SensorType::COMBINED: return "COMBINED";
        default: return "UNKNOWN";
    }
}

inline String alertLevelToString(AlertLevel level) {
    switch (level) {
        case AlertLevel::NONE: return "NONE";
        case AlertLevel::INFO: return "INFO";
        case AlertLevel::WARNING: return "WARNING";
        case AlertLevel::CRITICAL: return "CRITICAL";
        case AlertLevel::EMERGENCY: return "EMERGENCY";
        default: return "UNKNOWN";
    }
}