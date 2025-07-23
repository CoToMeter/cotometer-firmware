/*
 * CoToMeterController.h
 * Main system controller - orchestrates all components
 */

#pragma once

#include "interfaces/ISensor.h"
#include "interfaces/IDisplay.h"
#include "interfaces/ICommunication.h"
#include "interfaces/IPowerManager.h"
#include "interfaces/IEventHandler.h"
#include "interfaces/IDataStorage.h"
#include "interfaces/IAnalytics.h"

#include "types/SensorData.h"
#include "types/DeviceConfig.h"
#include "types/Event.h"
#include "types/SystemEnums.h"

#include "events/EventDispatcher.h"
#include "managers/CalibrationManager.h"
#include "managers/ScreenManager.h"
#include "managers/DiagnosticsManager.h"
#include "managers/OTAManager.h"

#include <memory>
#include <vector>

class CoToMeterController : public IEventHandler {
private:
    // ================================
    // CORE COMPONENTS
    // ================================
    
    // Hardware abstraction
    std::vector<std::unique_ptr<ISensor>> sensors;
    std::unique_ptr<IDisplay> display;
    std::unique_ptr<ICommunication> communication;
    std::unique_ptr<IPowerManager> powerManager;
    std::unique_ptr<IDataStorage> storage;
    std::unique_ptr<IAnalytics> analytics;
    
    // System managers
    std::unique_ptr<EventDispatcher> eventDispatcher;
    std::unique_ptr<CalibrationManager> calibrationManager;
    std::unique_ptr<ScreenManager> screenManager;
    std::unique_ptr<DiagnosticsManager> diagnosticsManager;
    std::unique_ptr<OTAManager> otaManager;
    
    // ================================
    // SYSTEM STATE
    // ================================
    
    SystemState currentState;
    SystemState previousState;
    DeviceConfig config;
    SensorData currentData;
    SensorData previousData;
    
    // State transition tracking
    uint32_t stateChangeTime;
    uint32_t stateTimeouts[static_cast<int>(SystemState::SHUTTING_DOWN) + 1];
    bool stateTransitionPending;
    SystemState pendingState;
    
    // ================================
    // TIMING & SCHEDULING
    // ================================
    
    // Measurement timing
    uint32_t lastMeasurement;
    uint32_t measurementInterval;
    bool forceMeasurement;
    
    // Activity tracking
    uint32_t lastActivity;
    uint32_t lastUserInteraction;
    uint32_t lastCommunication;
    
    // System heartbeat
    uint32_t lastHeartbeat;
    uint32_t heartbeatInterval;
    
    // Calibration scheduling
    uint32_t lastCalibration;
    uint32_t calibrationDueTime;
    bool calibrationRequired;
    
    // ================================
    // INPUT HANDLING
    // ================================
    
    // Button state
    const gpio_num_t BUTTON_PIN = GPIO_NUM_0;
    volatile bool buttonPressed;
    volatile uint32_t buttonPressTime;
    volatile uint32_t buttonReleaseTime;
    bool buttonLongPressDetected;
    
    // Button timing constants
    static const uint32_t BUTTON_DEBOUNCE_TIME = 50;      // ms
    static const uint32_t BUTTON_LONG_PRESS_TIME = 1000;  // ms
    static const uint32_t BUTTON_DOUBLE_PRESS_TIME = 300; // ms
    
    // ================================
    // DATA MANAGEMENT
    // ================================
    
    // Data validation
    SensorData lastValidData;
    uint8_t consecutiveInvalidReadings;
    static const uint8_t MAX_INVALID_READINGS = 5;
    
    // Data smoothing
    std::unique_ptr<IFilter> co2Filter;
    std::unique_ptr<IFilter> pmFilter;
    std::unique_ptr<IFilter> vocFilter;
    std::unique_ptr<IFilter> tempFilter;
    std::unique_ptr<IFilter> humidityFilter;
    
    // Alert management
    AlertLevel currentAlertLevel;
    uint32_t lastAlertTime;
    String activeAlerts;
    bool alertAcknowledged;
    
    // ================================
    // PERFORMANCE MONITORING
    // ================================
    
    // System performance
    uint32_t loopExecutionTime;
    uint32_t maxLoopTime;
    uint32_t avgLoopTime;
    uint32_t loopCounter;
    
    // Memory monitoring
    uint32_t freeHeapAtStart;
    uint32_t minFreeHeap;
    bool memoryWarning;
    
    // Error tracking
    String lastError;
    uint32_t errorCount;
    uint32_t criticalErrorCount;
    
    // ================================
    // INITIALIZATION FLAGS
    // ================================
    
    bool sensorsInitialized;
    bool displayInitialized;
    bool communicationInitialized;
    bool storageInitialized;
    bool systemFullyInitialized;
    
public:
    // ================================
    // CONSTRUCTOR & DESTRUCTOR
    // ================================
    
    CoToMeterController();
    ~CoToMeterController() override;
    
    // Prevent copy construction and assignment
    CoToMeterController(const CoToMeterController&) = delete;
    CoToMeterController& operator=(const CoToMeterController&) = delete;
    
    // ================================
    // SYSTEM LIFECYCLE
    // ================================
    
    // Initialization
    bool initialize();
    bool initializeHardware();
    bool initializeSensors();
    bool initializeDisplay();
    bool initializeCommunication();
    bool initializeStorage();
    bool initializeManagers();
    bool loadConfiguration();
    bool performSelfTest();
    
    // Main execution
    void loop();
    
    // Shutdown
    void shutdown();
    void emergencyShutdown();
    bool isShutdownRequested() const;
    
    // ================================
    // STATE MANAGEMENT
    // ================================
    
    // State transitions
    void setState(SystemState newState);
    SystemState getState() const;
    SystemState getPreviousState() const;
    bool isStateTransitionValid(SystemState from, SystemState to) const;
    String getStateDescription() const;
    uint32_t getTimeInCurrentState() const;
    
    // State queries
    bool isInitializing() const;
    bool isRunning() const;
    bool isSleeping() const;
    bool isCalibrating() const;
    bool hasError() const;
    bool isUpdating() const;
    
    // State-specific operations
    void handleInitializingState();
    void handleRunningState();
    void handleSleepingState();
    void handleCalibratingState();
    void handleErrorState();
    void handleUpdatingState();
    
    // ================================
    // EVENT HANDLING (IEventHandler)
    // ================================
    
    void handleEvent(const Event& event) override;
    bool canHandle(EventType type) override;
    String getHandlerName() override;
    int getPriority() override;
    bool shouldHandle(const Event& event) override;
    std::vector<EventType> getSupportedEvents() override;
    bool initialize() override; // From IEventHandler
    void shutdown() override;   // From IEventHandler
    bool isReady() override;    // From IEventHandler
    
    // Specific event handlers
    void handleButtonEvent(const Event& event);
    void handleSensorEvent(const Event& event);
    void handleCommunicationEvent(const Event& event);
    void handleTimerEvent(const Event& event);
    void handleThresholdEvent(const Event& event);
    void handleBatteryEvent(const Event& event);
    void handleErrorEvent(const Event& event);
    void handleCalibrationEvent(const Event& event);
    void handleUpdateEvent(const Event& event);
    
    // ================================
    // SENSOR MANAGEMENT
    // ================================
    
    // Sensor operations
    bool addSensor(std::unique_ptr<ISensor> sensor);
    bool removeSensor(SensorType type);
    ISensor* getSensor(SensorType type);
    std::vector<ISensor*> getAllSensors();
    size_t getSensorCount() const;
    
    // Data acquisition
    bool performMeasurement();
    bool readAllSensors();
    bool validateSensorData(const SensorData& data);
    void processSensorData(const SensorData& data);
    void applySensorFilters();
    
    // Sensor status
    bool areSensorsReady() const;
    std::vector<SensorType> getFailedSensors() const;
    String getSensorStatus() const;
    
    // ================================
    // CONFIGURATION MANAGEMENT
    // ================================
    
    // Configuration access
    const DeviceConfig& getCurrentConfig() const;
    bool updateConfig(const DeviceConfig& newConfig);
    bool resetConfigToDefaults();
    bool loadConfigFromStorage();
    bool saveConfigToStorage();
    
    // Configuration validation
    bool isConfigValid(const DeviceConfig& config) const;
    std::vector<String> validateConfig(const DeviceConfig& config) const;
    void sanitizeConfig(DeviceConfig& config) const;
    
    // Configuration templates
    bool applyConfigTemplate(const String& templateName);
    std::vector<String> getAvailableTemplates() const;
    String getCurrentTemplate() const;
    
    // ================================
    // DATA ACCESS & MANAGEMENT
    // ================================
    
    // Current data
    const SensorData& getCurrentData() const;
    const SensorData& getPreviousData() const;
    bool hasValidData() const;
    uint32_t getDataAge() const;
    
    // Historical data
    std::vector<SensorData> getHistory(uint32_t count = 100) const;
    std::vector<SensorData> getHistoryForPeriod(uint32_t startTime, uint32_t endTime) const;
    bool clearHistory();
    
    // Data export
    String exportData(const String& format) const;
    bool exportToFile(const String& filename, const String& format) const;
    
    // Data analytics
    String getDataSummary() const;
    String getAnalyticsReport() const;
    int getCurrentAirQualityIndex() const;
    std::vector<String> getRecommendations() const;
    
    // ================================
    // ALERT & THRESHOLD MANAGEMENT
    // ================================
    
    // Threshold checking
    void checkAllThresholds();
    bool checkCO2Threshold(float value);
    bool checkPMThreshold(float pm25, float pm10);
    bool checkVOCThreshold(float value);
    bool checkTemperatureThreshold(float value);
    bool checkHumidityThreshold(float value);
    bool checkBatteryThreshold(uint8_t percentage);
    
    // Alert management
    void triggerAlert(const String& parameter, float value, float threshold);
    void acknowledgeAlert();
    void clearAllAlerts();
    bool hasActiveAlerts() const;
    String getActiveAlerts() const;
    AlertLevel getCurrentAlertLevel() const;
    
    // Alert configuration
    void setAlertEnabled(const String& parameter, bool enabled);
    bool isAlertEnabled(const String& parameter) const;
    void setThreshold(const String& parameter, float threshold);
    float getThreshold(const String& parameter) const;
    
    // ================================
    // DISPLAY MANAGEMENT
    // ================================
    
    // Display operations
    void updateDisplay();
    void showScreen(ScreenType screen);
    ScreenType getCurrentScreen() const;
    void cycleScreen();
    void showMessage(const String& message, uint32_t durationMs = 3000);
    void showError(const String& error);
    
    // Display configuration
    void setDisplayBrightness(uint8_t brightness);
    uint8_t getDisplayBrightness() const;
    void setDisplayTimeout(uint32_t timeoutMs);
    uint32_t getDisplayTimeout() const;
    
    // Display status
    bool isDisplayReady() const;
    bool isDisplaySleeping() const;
    void wakeDisplay();
    void sleepDisplay();
    
    // ================================
    // COMMUNICATION MANAGEMENT
    // ================================
    
    // Connection management
    bool isConnected() const;
    bool startAdvertising();
    bool stopAdvertising();
    void disconnect();
    
    // Data transmission
    bool sendCurrentData();
    bool sendHistoricalData(uint32_t count);
    bool sendConfiguration();
    bool sendSystemStatus();
    
    // Communication configuration
    void setDeviceName(const String& name);
    String getDeviceName() const;
    void setConnectionCallback(std::function<void(bool)> callback);
    void setDataCallback(std::function<void(const String&)> callback);
    
    // Communication status
    int getSignalStrength() const;
    uint32_t getBytesTransmitted() const;
    uint32_t getBytesReceived() const;
    bool isCommunicationReady() const;
    
    // ================================
    // POWER MANAGEMENT
    // ================================
    
    // Power control
    void enterSleepMode();
    void enterDeepSleep(uint64_t sleepTimeUs);
    void wakeUp();
    void enableLowPowerMode(bool enabled);
    
    // Power monitoring
    float getBatteryVoltage() const;
    uint8_t getBatteryPercentage() const;
    bool isLowBattery() const;
    bool isCriticalBattery() const;
    bool isCharging() const;
    uint32_t getEstimatedBatteryLife() const; // minutes
    
    // Component power control
    void enableSensorPower(bool enabled);
    void enableDisplayPower(bool enabled);
    void enableCommunicationPower(bool enabled);
    
    // Power configuration
    void setPowerMode(PowerMode mode);
    PowerMode getPowerMode() const;
    void setSleepTimeout(uint32_t timeoutMs);
    uint32_t getSleepTimeout() const;
    
    // ================================
    // CALIBRATION MANAGEMENT
    // ================================
    
    // Calibration operations
    bool startCalibration(SensorType sensorType = SensorType::UNKNOWN);
    bool cancelCalibration();
    bool isCalibrationInProgress() const;
    float getCalibrationProgress() const; // 0.0 - 1.0
    
    // Calibration status
    bool isCalibrationRequired(SensorType sensorType) const;
    uint32_t getTimeSinceLastCalibration(SensorType sensorType) const;
    uint32_t getNextCalibrationTime(SensorType sensorType) const;
    
    // Calibration configuration
    void setAutoCalibrationEnabled(bool enabled);
    bool isAutoCalibrationEnabled() const;
    void setCalibrationInterval(uint16_t hours);
    uint16_t getCalibrationInterval() const;
    
    // ================================
    // DIAGNOSTICS & MONITORING
    // ================================
    
    // System health
    DiagnosticsManager::SystemHealth getSystemHealth() const;
    bool performSelfTest();
    bool performExtendedTest();
    String generateDiagnosticsReport() const;
    
    // Performance monitoring
    uint32_t getLoopExecutionTime() const;
    uint32_t getMaxLoopTime() const;
    uint32_t getAverageLoopTime() const;
    uint32_t getFreeMemory() const;
    uint32_t getMinFreeMemory() const;
    float getCPUUsage() const;
    
    // System information
    String getFirmwareVersion() const;
    String getHardwareRevision() const;
    uint32_t getUptime() const;
    uint32_t getTotalMeasurements() const;
    uint32_t getErrorCount() const;
    
    // Error management
    void logError(const String& error);
    String getLastError() const;
    std::vector<String> getErrorHistory() const;
    void clearErrorHistory();
    
    // ================================
    // SYSTEM CONTROL
    // ================================
    
    // System operations
    void restart();
    void factoryReset();
    void enterMaintenanceMode();
    void exitMaintenanceMode();
    bool isMaintenanceMode() const;
    
    // Update management
    bool checkForUpdates();
    bool startUpdate();
    bool isUpdateAvailable() const;
    bool isUpdateInProgress() const;
    float getUpdateProgress() const; // 0.0 - 1.0
    
    // ================================
    // UTILITY METHODS
    // ================================
    
    // Time utilities
    uint32_t getCurrentTime() const;
    void syncTime(uint32_t unixTime);
    String getFormattedTime() const;
    String getFormattedUptime() const;
    
    // Data formatting
    String formatSensorData(const SensorData& data) const;
    String formatSystemStatus() const;
    String formatConfiguration() const;
    
    // Validation utilities
    bool validateSensorReading(SensorType type, float value) const;
    bool isSystemHealthy() const;
    bool areAllComponentsReady() const;
    
private:
    // ================================
    // INTERNAL METHODS
    // ================================
    
    // Initialization helpers
    void setupGPIO();
    void setupInterrupts();
    void setupTimers();
    void setupWatchdog();
    void loadFactoryDefaults();
    
    // State management helpers
    void updateStateTimeout();
    bool isStateTimeout() const;
    void handleStateTimeout();
    void logStateChange(SystemState from, SystemState to);
    
    // Measurement helpers
    bool isMeasurementDue() const;
    void scheduleMeasurement(uint32_t delayMs = 0);
    void processMeasurementResults();
    void updateDataFilters(const SensorData& data);
    
    // Button handling
    static void IRAM_ATTR buttonISR();
    void processButtonPress();
    void detectButtonPattern();
    bool isButtonPressed() const;
    uint32_t getButtonPressDuration() const;
    
    // Data validation helpers
    bool isDataValueValid(float value, float min, float max) const;
    bool hasDataChanged(const SensorData& current, const SensorData& previous, float threshold) const;
    void markDataAsInvalid(const String& reason);
    
    // Alert helpers
    void processAlerts();
    void updateAlertLevel();
    bool shouldTriggerAlert(const String& parameter, float value, float threshold) const;
    void formatAlertMessage(const String& parameter, float value, float threshold, String& message) const;
    
    // Communication helpers
    void handleIncomingData(const String& data);
    void handleConnectionStatusChange(bool connected);
    String createDataPacket() const;
    String createStatusPacket() const;
    
    // Power management helpers
    void updatePowerStatus();
    bool shouldEnterSleepMode() const;
    void prepareSleepMode();
    void restoreFromSleep();
    void handleLowBattery();
    void handleCriticalBattery();
    
    // Performance monitoring helpers
    void updatePerformanceCounters();
    void checkMemoryUsage();
    void updateLoopTiming(uint32_t executionTime);
    
    // Error handling helpers
    void handleCriticalError(const String& error);
    void handleRecoverableError(const String& error);
    void attemptErrorRecovery();
    bool isErrorRecoverable(const String& error) const;
    
    // Calibration helpers
    void checkCalibrationSchedule();
    void handleCalibrationComplete(SensorType type, bool success);
    void updateCalibrationStatus();
    
    // Display helpers
    void updateDisplayContent();
    void handleDisplayTimeout();
    void refreshDisplayData();
    
    // Configuration helpers
    void applyConfiguration(const DeviceConfig& config);
    void notifyConfigurationChange();
    bool isConfigurationChange(const DeviceConfig& newConfig) const;
    
    // Utility helpers
    void logActivity(const String& activity);
    void updateHeartbeat();
    void processMaintenanceTasks();
    String generateSystemId() const;
    
    // Static utilities
    static String stateToString(SystemState state);
    static SystemState stringToState(const String& str);
    static bool isValidStateTransition(SystemState from, SystemState to);
};