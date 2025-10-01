/*
 * types/DeviceConfig.h
 * Device configuration structure
 */

#pragma once

#include <Arduino.h>
#include "SystemEnums.h"

struct DeviceConfig {
    // ================================
    // MEASUREMENT SETTINGS
    // ================================
    
    // Thresholds for alerts
    uint16_t co2Threshold;          // CO2 alert threshold (ppm)
    uint16_t pm25Threshold;         // PM2.5 alert threshold (μg/m³)
    uint16_t vocThreshold;          // VOC alert threshold (ppb)
    int8_t tempMinThreshold;        // Minimum temperature (°C)
    int8_t tempMaxThreshold;        // Maximum temperature (°C)
    uint8_t humidityMinThreshold;   // Minimum humidity (%)
    uint8_t humidityMaxThreshold;   // Maximum humidity (%)
    
    // Measurement intervals
    uint16_t measurementInterval;   // Time between measurements (seconds)
    uint16_t calibrationInterval;   // Time between auto-calibrations (hours)
    uint8_t averagingWindow;        // Number of samples for averaging
    
    // ================================
    // DISPLAY SETTINGS
    // ================================
    
    uint8_t displayBrightness;      // Display brightness (0-255)
    uint8_t displayContrast;        // Display contrast (0-255)
    uint8_t displayRotation;        // Display rotation (0-3)
    uint32_t displayTimeout;        // Time before display sleep (ms)
    bool autoSleep;                 // Enable auto-sleep
    DisplayMode displayMode;        // Display power mode
    
    // Screen preferences
    ScreenType defaultScreen;       // Default screen on startup
    bool showAnimations;            // Enable cat animations
    bool showGraphs;                // Show trend graphs
    uint8_t refreshRate;            // Screen refresh rate (Hz)
    
    // ================================
    // ALERT SETTINGS
    // ================================
    
    bool alertsEnabled;             // Master alert enable/disable
    bool soundAlerts;               // Enable sound alerts
    bool visualAlerts;              // Enable visual alerts
    bool vibrationAlerts;           // Enable vibration alerts (if available)
    
    // Alert timing
    uint16_t alertDuration;         // Alert duration (seconds)
    uint8_t alertVolume;            // Alert volume (0-100)
    uint16_t snoozeTime;            // Snooze duration (minutes)
    
    // Alert types per parameter
    bool co2AlertEnabled;
    bool pmAlertEnabled;
    bool vocAlertEnabled;
    bool temperatureAlertEnabled;
    bool humidityAlertEnabled;
    bool batteryAlertEnabled;
    
    // ================================
    // POWER SETTINGS
    // ================================
    
    PowerMode powerMode;            // System power mode
    uint32_t sleepTimeout;          // Time before deep sleep (ms)
    uint32_t autoShutdownTime;      // Auto shutdown time (hours)
    
    // Battery management
    uint8_t lowBatteryThreshold;    // Low battery percentage
    uint8_t criticalBatteryThreshold; // Critical battery percentage
    bool powerSaveMode;             // Enable power save features
    
    // Component power control
    bool sensorAutoSleep;           // Auto sleep sensors when idle
    bool displayAutoSleep;          // Auto sleep display when idle
    bool communicationAutoSleep;    // Auto sleep communication when idle
    
    // ================================
    // COMMUNICATION SETTINGS
    // ================================
    
    // Bluetooth settings
    bool bleEnabled;                // Enable BLE
    String deviceName;              // BLE device name
    String pinCode;                 // BLE pairing PIN (if required)
    uint16_t connectionTimeout;     // BLE connection timeout (seconds)
    bool autoReconnect;             // Auto-reconnect to last device
    
    // WiFi settings (future use)
    bool wifiEnabled;               // Enable WiFi
    String wifiSSID;                // WiFi network name
    String wifiPassword;            // WiFi password
    bool wifiAutoConnect;           // Auto-connect to WiFi
    
    // Data transmission
    uint16_t dataTransmissionInterval; // How often to send data (seconds)
    bool realTimeData;              // Send data in real-time
    bool batchTransmission;         // Send data in batches
    uint8_t maxRetryAttempts;       // Max retry attempts for failed transmissions
    
    // ================================
    // CALIBRATION SETTINGS
    // ================================
    
    // Auto-calibration
    bool autoCalibrationEnabled;    // Enable automatic calibration
    uint16_t autoCalibrationHours;  // Hours between auto-calibrations
    
    // Calibration offsets (applied to readings)
    float co2CalibrationOffset;     // CO2 offset (ppm)
    float tempCalibrationOffset;    // Temperature offset (°C)
    float humidityCalibrationOffset; // Humidity offset (%)
    float pmCalibrationOffset;      // PM offset (μg/m³)
    float vocCalibrationOffset;     // VOC offset (ppb)
    
    // Calibration factors (multipliers)
    float co2CalibrationFactor;     // CO2 scale factor
    float tempCalibrationFactor;    // Temperature scale factor
    float humidityCalibrationFactor; // Humidity scale factor
    float pmCalibrationFactor;      // PM scale factor
    float vocCalibrationFactor;     // VOC scale factor
    
    // ================================
    // DATA STORAGE SETTINGS
    // ================================
    
    uint16_t maxHistoryRecords;     // Maximum stored measurements
    uint8_t dataCompressionLevel;   // Data compression (0-9, 0=none)
    bool autoExportData;            // Auto-export data when full
    String exportFormat;            // Export format (JSON, CSV, etc.)
    
    // Data retention
    uint16_t dataRetentionDays;     // Days to keep data
    bool autoDeleteOldData;         // Auto-delete old data
    
    // ================================
    // SYSTEM SETTINGS
    // ================================
    
    String firmwareVersion;         // Current firmware version
    String hardwareRevision;        // Hardware revision
    uint32_t configVersion;         // Configuration version number
    uint32_t lastModified;          // Last modification timestamp
    
    // System behavior
    bool debugMode;                 // Enable debug logging
    uint8_t logLevel;               // Logging level (0-5)
    bool performanceMonitoring;     // Monitor system performance
    
    // Factory settings
    bool factoryResetProtection;    // Protect against accidental reset
    String factoryResetPin;         // PIN required for factory reset
    
    // ================================
    // USER PREFERENCES
    // ================================
    
    String userName;                // User name
    String location;                // Installation location
    String timezone;                // Timezone string
    bool use24HourFormat;           // Time format preference
    String temperatureUnit;         // Temperature unit (C/F)
    String languageCode;            // Language code (en, uk, etc.)
    
    // Personalization
    uint8_t catPersonality;         // Cat avatar personality (0-5)
    String customAlertSounds;       // Custom alert sound paths
    uint16_t customColors[8];       // Custom color palette
    
    // ================================
    // METHODS
    // ================================
    
    // Constructors
    DeviceConfig();
    DeviceConfig(const DeviceConfig& other);
    
    // Assignment
    DeviceConfig& operator=(const DeviceConfig& other);
    
    // Validation
    bool isValid() const;
    std::vector<String> validate() const; // Returns list of validation errors
    void sanitize(); // Fix invalid values
    
    // Factory presets
    static DeviceConfig getDefaultConfig();
    static DeviceConfig getOfficeConfig();
    static DeviceConfig getHomeConfig();
    static DeviceConfig getBedroomConfig();
    static DeviceConfig getOutdoorConfig();
    static DeviceConfig getLowPowerConfig();
    static DeviceConfig getHighAccuracyConfig();
    
    // Serialization
    String toJson() const;
    bool fromJson(const String& json);
    String toConfigFile() const;
    bool fromConfigFile(const String& configData);
    
    // Utility
    void reset(); // Reset to default values
    void updateTimestamp(); // Update lastModified
    uint32_t calculateChecksum() const; // For integrity checking
    bool hasChanged(const DeviceConfig& other) const;
    
    // Comparison
    bool operator==(const DeviceConfig& other) const;
    bool operator!=(const DeviceConfig& other) const;
    
    // Configuration templates
    void applyTemplate(const String& templateName);
    String getCurrentTemplate() const;
    std::vector<String> getAvailableTemplates() const;
    
    // Import/Export
    bool exportToFile(const String& filename) const;
    bool importFromFile(const String& filename);
    String exportToString() const;
    bool importFromString(const String& data);
    
    // Migration (for version updates)
    bool migrateFromVersion(uint32_t oldVersion);
    bool needsMigration() const;
    
private:
    // Internal validation helpers
    bool isThresholdValid(float value, float min, float max) const;
    bool isPercentageValid(uint8_t value) const;
    bool isTimeoutValid(uint32_t timeout) const;
    void setDefaults();
};