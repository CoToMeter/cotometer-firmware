/*
 * sensors/SensorFactory.h
 * Factory for creating sensor instances
 */

#pragma once

#include "../interfaces/ISensor.h"
#include "../types/SystemEnums.h"
#include <memory>
#include <vector>
#include <map>

// ================================
// SENSOR FACTORY
// ================================

class SensorFactory {
public:
    // Factory method parameters
    struct SensorParams {
        // Communication parameters
        HardwareSerial* serial = nullptr;
        uint8_t i2cAddress = 0;
        int sdaPin = -1;
        int sclPin = -1;
        uint32_t baudRate = 9600;
        
        // Power control
        int powerPin = -1;
        bool powerActiveHigh = true;
        
        // Timing
        uint32_t warmupTime = 1000;    // ms
        uint32_t readInterval = 5000;   // ms
        
        // Calibration
        bool autoCalibration = true;
        float calibrationOffset = 0.0;
        float calibrationScale = 1.0;
        
        // Custom parameters (JSON string)
        String customParams;
        
        SensorParams() = default;
    };
    
    // Sensor registration entry
    struct SensorInfo {
        String name;
        String description;
        String manufacturer;
        String version;
        std::vector<String> parameters;  // What this sensor measures
        bool requiresSerial;
        bool requiresI2C;
        bool requiresPowerControl;
        uint32_t defaultI2CAddress;
        String datasheet;
        
        SensorInfo() : requiresSerial(false), requiresI2C(false), 
                      requiresPowerControl(false), defaultI2CAddress(0) {}
    };

private:
    // Registered sensor types
    static std::map<SensorType, SensorInfo> registeredSensors;
    static bool initialized;
    
public:
    // ================================
    // FACTORY METHODS
    // ================================
    
    // Main factory method
    static std::unique_ptr<ISensor> createSensor(SensorType type, const SensorParams& params = SensorParams());
    
    // Convenience methods for specific sensors
    static std::unique_ptr<ISensor> createSCD41Sensor(int sdaPin = 21, int sclPin = 22);
    static std::unique_ptr<ISensor> createPMS7003Sensor(HardwareSerial* serial, int powerPin = -1);
    static std::unique_ptr<ISensor> createBME688Sensor(uint8_t i2cAddress = 0x76);
    
    // Auto-detection methods
    static std::vector<SensorType> detectAvailableSensors();
    static std::unique_ptr<ISensor> createDetectedSensor(SensorType type);
    static bool isSensorPresent(SensorType type, const SensorParams& params = SensorParams());
    
    // ================================
    // SENSOR INFORMATION
    // ================================
    
    // Query available sensors
    static std::vector<SensorType> getAvailableSensorTypes();
    static std::vector<String> getAvailableSensorNames();
    static SensorInfo getSensorInfo(SensorType type);
    static bool isSensorSupported(SensorType type);
    
    // Sensor capabilities
    static std::vector<String> getSensorParameters(SensorType type);
    static bool canMeasureParameter(SensorType type, const String& parameter);
    static std::vector<SensorType> getSensorsForParameter(const String& parameter);
    
    // ================================
    // CONFIGURATION VALIDATION
    // ================================
    
    // Parameter validation
    static bool validateParams(SensorType type, const SensorParams& params);
    static std::vector<String> getParamValidationErrors(SensorType type, const SensorParams& params);
    static SensorParams getDefaultParams(SensorType type);
    static SensorParams sanitizeParams(SensorType type, const SensorParams& params);
    
    // Hardware requirements
    static bool checkHardwareRequirements(SensorType type);
    static std::vector<String> getHardwareRequirements(SensorType type);
    static bool isHardwareAvailable(SensorType type);
    
    // ================================
    // SENSOR TESTING & DIAGNOSTICS
    // ================================
    
    // Sensor testing
    static bool testSensor(SensorType type, const SensorParams& params = SensorParams());
    static String runSensorDiagnostics(SensorType type, const SensorParams& params = SensorParams());
    static bool verifySensorCommunication(SensorType type, const SensorParams& params);
    
    // Sensor health checking
    static bool isSensorHealthy(ISensor* sensor);
    static String getSensorHealthReport(ISensor* sensor);
    static std::vector<String> getSensorWarnings(ISensor* sensor);
    
    // ================================
    // BULK OPERATIONS
    // ================================
    
    // // Multi-sensor creation
    // static std::vector<std::unique_ptr<ISensor>> createSensorSet(const std::vector<SensorType>& types);
    // static std::vector<std::unique_ptr<ISensor>> createAllAvailableSensors();
    // static std::vector<std::unique_ptr<ISensor>> createSensorsForParameters(const std::vector<String>& parameters);
    
    // Sensor management
    static bool initializeAllSensors(std::vector<std::unique_ptr<ISensor>>& sensors);
    static void shutdownAllSensors(std::vector<std::unique_ptr<ISensor>>& sensors);
    static std::vector<SensorType> getFailedSensors(const std::vector<std::unique_ptr<ISensor>>& sensors);
    
    // ================================
    // CONFIGURATION MANAGEMENT
    // ================================
    
    // Configuration templates
    static SensorParams getOfficeConfig(SensorType type);
    static SensorParams getHomeConfig(SensorType type);
    static SensorParams getOutdoorConfig(SensorType type);
    static SensorParams getLowPowerConfig(SensorType type);
    static SensorParams getHighAccuracyConfig(SensorType type);
    
    // Configuration serialization
    static String paramsToJson(const SensorParams& params);
    static SensorParams paramsFromJson(const String& json);
    static String paramsToConfigString(const SensorParams& params);
    static SensorParams paramsFromConfigString(const String& config);
    
    // ================================
    // SENSOR REGISTRATION & PLUGINS
    // ================================
    
    // Sensor registration (for extensibility)
    static bool registerSensor(SensorType type, const SensorInfo& info);
    static bool unregisterSensor(SensorType type);
    static void registerBuiltinSensors();
    
    // Plugin system (future extensibility)
    static bool loadSensorPlugin(const String& pluginPath);
    static std::vector<String> getLoadedPlugins();
    static bool unloadSensorPlugin(const String& pluginName);
    
    // ================================
    // UTILITY METHODS
    // ================================
    
    // String conversions
    static String sensorTypeToString(SensorType type);
    static SensorType stringToSensorType(const String& str);
    static String paramsToString(const SensorParams& params);
    
    // Hardware detection
    static std::vector<uint8_t> scanI2CAddresses();
    static bool isI2CAddressAvailable(uint8_t address);
    static std::vector<HardwareSerial*> getAvailableSerialPorts();
    
    // Error handling
    static String getLastFactoryError();
    static void clearFactoryErrors();
    static bool hasFactoryErrors();
    
private:
    // ================================
    // INTERNAL METHODS
    // ================================
    
    // Initialization
    static void initialize();
    static void registerSCD41();
    static void registerPMS7003();
    static void registerBME688();
    
    // Sensor creation helpers
    static std::unique_ptr<ISensor> createSensorInternal(SensorType type, const SensorParams& params);
    static bool validateSensorType(SensorType type);
    static bool validateCommonParams(const SensorParams& params);
    
    // Hardware helpers
    static bool setupI2C(int sdaPin, int sclPin);
    static bool setupSerial(HardwareSerial* serial, uint32_t baudRate);
    static bool setupPowerControl(int powerPin, bool activeHigh);
    
    // Detection helpers
    static bool detectSCD41(const SensorParams& params);
    static bool detectPMS7003(const SensorParams& params);
    static bool detectBME688(const SensorParams& params);
    
    // Validation helpers
    static bool isValidI2CAddress(uint8_t address);
    static bool isValidGPIOPin(int pin);
    static bool isValidBaudRate(uint32_t baudRate);
    
    // Error management
    static String lastError;
    static void setError(const String& error);
};

// ================================
// SENSOR BUILDER (Builder Pattern)
// ================================

class SensorBuilder {
private:
    SensorType type;
    SensorFactory::SensorParams params;
    
public:
    SensorBuilder();
    explicit SensorBuilder(SensorType sensorType);
    
    // Fluent interface for building sensors
    SensorBuilder& withType(SensorType sensorType);
    SensorBuilder& withSerial(HardwareSerial* serial);
    SensorBuilder& withI2C(uint8_t address, int sdaPin = -1, int sclPin = -1);
    SensorBuilder& withPowerControl(int powerPin, bool activeHigh = true);
    SensorBuilder& withBaudRate(uint32_t baudRate);
    SensorBuilder& withWarmupTime(uint32_t warmupMs);
    SensorBuilder& withReadInterval(uint32_t intervalMs);
    SensorBuilder& withCalibration(bool autoCalibration, float offset = 0.0, float scale = 1.0);
    SensorBuilder& withCustomParams(const String& params);
    
    // Template configurations
    SensorBuilder& asOfficeConfig();
    SensorBuilder& asHomeConfig();
    SensorBuilder& asOutdoorConfig();
    SensorBuilder& asLowPowerConfig();
    SensorBuilder& asHighAccuracyConfig();
    
    // Build the sensor
    std::unique_ptr<ISensor> build();
    
    // Validation
    bool isValid() const;
    std::vector<String> getValidationErrors() const;
    SensorFactory::SensorParams getParams() const;
};

// ================================
// SENSOR REGISTRY
// ================================

class SensorRegistry {
private:
    std::vector<std::unique_ptr<ISensor>> sensors;
    std::map<SensorType, size_t> typeToIndex;
    std::map<String, size_t> nameToIndex;
    
public:
    SensorRegistry();
    ~SensorRegistry();
    
    // Registry operations
    bool addSensor(std::unique_ptr<ISensor> sensor);
    bool removeSensor(SensorType type);
    bool removeSensor(const String& name);
    void clear();
    
    // Sensor access
    ISensor* getSensor(SensorType type);
    ISensor* getSensor(const String& name);
    std::vector<ISensor*> getAllSensors();
    std::vector<ISensor*> getSensorsByParameter(const String& parameter);
    
    // Registry queries
    size_t getCount() const;
    bool hasSensor(SensorType type) const;
    bool hasSensor(const String& name) const;
    std::vector<SensorType> getRegisteredTypes() const;
    std::vector<String> getRegisteredNames() const;
    
    // Bulk operations
    bool initializeAll();
    void shutdownAll();
    std::vector<SensorType> getFailedSensors() const;
    std::vector<ISensor*> getReadySensors();
    
    // Data collection
    bool readAllSensors();
    std::vector<SensorData> collectAllData();
    SensorData collectCombinedData();
    
    // Status and diagnostics
    String getRegistryStatus() const;
    bool performRegistryHealthCheck();
    String generateRegistryReport() const;
};

// ================================
// SENSOR UTILITIES
// ================================

namespace SensorUtils {
    // Data combination utilities
    SensorData combineSensorData(const std::vector<SensorData>& dataList);
    SensorData averageSensorData(const std::vector<SensorData>& dataList);
    bool isSensorDataConsistent(const std::vector<SensorData>& dataList, float tolerance = 0.1);
    
    // Sensor comparison
    bool areSensorsCompatible(SensorType type1, SensorType type2);
    std::vector<String> getCommonParameters(SensorType type1, SensorType type2);
    float compareSensorAccuracy(ISensor* sensor1, ISensor* sensor2, const String& parameter);
    
    // Calibration utilities
    bool isCalibrationNeeded(ISensor* sensor, uint32_t intervalHours = 24);
    float calculateCalibrationDrift(ISensor* sensor, const std::vector<SensorData>& referenceData);
    bool performCrossCalibration(ISensor* primary, ISensor* secondary);
    
    // Validation utilities
    bool isSensorDataValid(const SensorData& data, SensorType expectedType);
    std::vector<String> validateSensorReading(const SensorData& data, SensorType type);
    bool isSensorReadingInRange(float value, SensorType type, const String& parameter);
    
    // Formatting utilities
    String formatSensorValue(float value, const String& parameter);
    String formatSensorData(const SensorData& data, bool includeUnits = true);
    String formatSensorStatus(ISensor* sensor);
}