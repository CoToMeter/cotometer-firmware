/*
 * types/SystemEnums.h
 * System-wide enumerations and constants
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
    COMBINED = 4,            // Multiple sensors
    PRESSURE = 5,            // Pressure sensor
    LIGHT = 6,               // Light sensor
    NOISE = 7                // Sound level sensor
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
    WIFI_CONNECTED = 303,
    WIFI_DISCONNECTED = 304,
    
    // System events
    THRESHOLD_EXCEEDED = 400,
    LOW_BATTERY = 401,
    CRITICAL_BATTERY = 402,
    CHARGING_STARTED = 403,
    CHARGING_STOPPED = 404,
    
    // Timer events
    TIMER_EXPIRED = 500,
    MEASUREMENT_TIMER = 501,
    DISPLAY_TIMEOUT = 502,
    SLEEP_TIMEOUT = 503,
    
    // Error events
    ERROR_OCCURRED = 600,
    SYSTEM_RESET = 601,
    WATCHDOG_RESET = 602,
    CONFIG_ERROR = 603,
    
    // Calibration events
    CALIBRATION_START = 700,
    CALIBRATION_COMPLETE = 701,
    CALIBRATION_FAILED = 702,
    
    // Update events
    UPDATE_AVAILABLE = 800,
    UPDATE_STARTED = 801,
    UPDATE_PROGRESS = 802,
    UPDATE_COMPLETE = 803,
    UPDATE_FAILED = 804,
    
    // Configuration events
    CONFIG_CHANGED = 900,
    CONFIG_RESET = 901,
    CONFIG_SAVED = 902,
    CONFIG_LOADED = 903
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
// AIR QUALITY INDEX LEVELS
// ================================

enum class AQILevel {
    GOOD = 0,                    // 0-50
    MODERATE = 1,                // 51-100
    UNHEALTHY_SENSITIVE = 2,     // 101-150
    UNHEALTHY = 3,               // 151-200
    VERY_UNHEALTHY = 4,          // 201-300
    HAZARDOUS = 5                // 301+
};

// ================================
// COMMUNICATION PROTOCOLS
// ================================

enum class CommunicationType {
    NONE = 0,
    BLE = 1,
    WIFI = 2,
    LORA = 3,
    SERIAL = 4,
    USB = 5
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
// CONSTANTS
// ================================

namespace Constants {
    // Timing constants (milliseconds)
    const uint32_t MEASUREMENT_INTERVAL_DEFAULT = 60000;  // 1 minute
    const uint32_t DISPLAY_TIMEOUT_DEFAULT = 30000;      // 30 seconds
    const uint32_t SLEEP_TIMEOUT_DEFAULT = 300000;       // 5 minutes
    const uint32_t CALIBRATION_TIMEOUT = 300000;         // 5 minutes
    
    // Sensor thresholds (default values)
    const uint16_t CO2_THRESHOLD_DEFAULT = 1000;         // ppm
    const uint16_t PM25_THRESHOLD_DEFAULT = 35;          // μg/m³
    const uint16_t VOC_THRESHOLD_DEFAULT = 200;          // ppb
    const uint8_t HUMIDITY_MIN = 40;                     // %
    const uint8_t HUMIDITY_MAX = 60;                     // %
    const int8_t TEMP_MIN = 18;                          // °C
    const int8_t TEMP_MAX = 26;                          // °C
    
    // Battery levels (percentage)
    const uint8_t BATTERY_LOW_THRESHOLD = 20;           // %
    const uint8_t BATTERY_CRITICAL_THRESHOLD = 10;      // %
    const float BATTERY_VOLTAGE_MIN = 3.0;              // V
    const float BATTERY_VOLTAGE_MAX = 4.2;              // V
    
    // Communication settings
    const uint16_t BLE_MTU_SIZE = 512;                  // bytes
    const uint32_t BLE_CONNECTION_TIMEOUT = 30000;      // 30 seconds
    const String BLE_DEVICE_NAME_DEFAULT = "CoToMeter";
    
    // Display settings
    const uint8_t DISPLAY_BRIGHTNESS_DEFAULT = 128;     // 0-255
    const uint8_t DISPLAY_CONTRAST_DEFAULT = 128;       // 0-255
    const uint8_t DISPLAY_ROTATION_DEFAULT = 0;         // 0-3
    
    // Data storage
    const size_t MAX_HISTORY_RECORDS = 1440;            // 24 hours @ 1min intervals
    const size_t MAX_ERROR_LOG_ENTRIES = 100;
    const size_t CONFIG_VERSION = 1;
    
    // System limits
    const uint8_t MAX_RETRY_ATTEMPTS = 3;
    const uint32_t WATCHDOG_TIMEOUT = 60000;            // 60 seconds
    const size_t MAX_EVENT_QUEUE_SIZE = 50;
    const uint8_t MAX_CONCURRENT_HANDLERS = 10;
}

// ================================
// UTILITY FUNCTIONS
// ================================

String sensorTypeToString(SensorType type);
String systemStateToString(SystemState state);
String eventTypeToString(EventType type);
String alertLevelToString(AlertLevel level);
String aqiLevelToString(AQILevel level);

SensorType stringToSensorType(const String& str);
SystemState stringToSystemState(const String& str);
EventType stringToEventType(const String& str);
AlertLevel stringToAlertLevel(const String& str);

bool isValidSensorType(int type);
bool isValidSystemState(int state);
bool isValidEventType(int type);
bool isValidAlertLevel(int level);