/*
 * storage/HistoricalDataStorage.h
 * Historical sensor data storage system for ESP32
 * Supports both internal flash and external SD card storage
 */

#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "../types/SensorData.h"
#include "../types/TimeSync.h"
#include "../interfaces/IDataStorage.h"

/**
 * Compact sensor record for efficient storage
 * Fixed size structure for optimal flash storage
 */
// no need to use PM sensor data
struct __attribute__((packed)) SensorRecord {
    unsigned long uptime;        // ESP32 uptime in ms (4 bytes)
    float co2;                   // CO2 in ppm (4 bytes)
    float temperature;           // Temperature in °C (4 bytes)
    float humidity;              // Humidity in % (4 bytes)
    float pressure;              // Pressure in hPa (4 bytes)
    float voc;                   // VOC index/estimate (4 bytes)
    uint8_t validity_flags;      // Bit flags for data validity (1 byte)
    uint8_t alert_level;         // Overall alert level (1 byte)
    uint8_t reserved[2];         // Reserved for future use (2 bytes)
    // Total: 38 bytes per record
    
    // Validity flag bits
    static const uint8_t FLAG_CO2_VALID = 0x01;
    static const uint8_t FLAG_TEMP_VALID = 0x02;
    static const uint8_t FLAG_HUMIDITY_VALID = 0x04;
    static const uint8_t FLAG_PRESSURE_VALID = 0x08;
    static const uint8_t FLAG_VOC_VALID = 0x10;
    static const uint8_t FLAG_OVERALL_VALID = 0x80;
    
    /**
     * Default constructor - initialize with safe values
     */
    SensorRecord() : uptime(0), co2(0), temperature(0), humidity(0), 
                    pressure(0), voc(0), 
                    validity_flags(0), alert_level(0) {
        reserved[0] = reserved[1] = 0;
    }
    
    /**
     * Constructor from sensor data objects
     */
    SensorRecord(unsigned long record_uptime, 
                const CO2SensorData* co2_data = nullptr,
                const VOCSensorData* voc_data = nullptr) 
        : uptime(record_uptime), validity_flags(0), alert_level(0) {
        
        reserved[0] = reserved[1] = 0;
        
        // Initialize all values to zero
        co2 = temperature = humidity = pressure = voc = 0.0;
        
        // Fill from CO2 sensor data
        if (co2_data && co2_data->isValid()) {
            co2 = co2_data->co2;
            temperature = co2_data->temperature;
            humidity = co2_data->humidity;
            validity_flags |= FLAG_CO2_VALID | FLAG_TEMP_VALID | FLAG_HUMIDITY_VALID;
            alert_level = max(alert_level, (uint8_t)co2_data->getAlertLevel());
        }
        
        // Fill from VOC sensor data (override temp/humidity if available)
        if (voc_data && voc_data->isValid()) {
            if (!co2_data || !co2_data->isValid()) { // Use VOC temp/humidity if CO2 not available
                temperature = voc_data->temperature;
                humidity = voc_data->humidity;
                validity_flags |= FLAG_TEMP_VALID | FLAG_HUMIDITY_VALID;
            }
            pressure = voc_data->pressure / 100.0; // Convert Pa to hPa
            voc = voc_data->vocEstimate;
            validity_flags |= FLAG_PRESSURE_VALID | FLAG_VOC_VALID;
            alert_level = max(alert_level, (uint8_t)voc_data->getAlertLevel());
        }
        
        // Set overall validity if any sensor data is valid
        if (validity_flags != 0) {
            validity_flags |= FLAG_OVERALL_VALID;
        }
    }
    
    /**
     * Convert to JSON for transmission
     */
    String toJson(const TimeSync& timeSync) const {
        JsonDocument doc;
        
        // Calculate timestamp from uptime
        unsigned long timestamp = timeSync.uptimeToTimestamp(uptime);
        doc["timestamp"] = timestamp;
        doc["uptime"] = uptime;
        
        // Create readings object
        JsonObject readings = doc["readings"].to<JsonObject>();
        
        if (validity_flags & FLAG_CO2_VALID) {
            JsonObject co2Reading = readings["co2"].to<JsonObject>();
            co2Reading["value"] = round(co2 * 10) / 10.0; // 1 decimal place
            co2Reading["unit"] = "ppm";
            co2Reading["status"] = "valid";
        }
        
        if (validity_flags & FLAG_TEMP_VALID) {
            JsonObject tempReading = readings["temperature"].to<JsonObject>();
            tempReading["value"] = round(temperature * 10) / 10.0;
            tempReading["unit"] = "°C";
            tempReading["status"] = "valid";
        }
        
        if (validity_flags & FLAG_HUMIDITY_VALID) {
            JsonObject humReading = readings["humidity"].to<JsonObject>();
            humReading["value"] = round(humidity * 10) / 10.0;
            humReading["unit"] = "%";
            humReading["status"] = "valid";
        }
        
        if (validity_flags & FLAG_PRESSURE_VALID) {
            JsonObject pressReading = readings["pressure"].to<JsonObject>();
            pressReading["value"] = round(pressure * 10) / 10.0;
            pressReading["unit"] = "hPa";
            pressReading["status"] = "valid";
        }
        
        if (validity_flags & FLAG_VOC_VALID) {
            JsonObject vocReading = readings["voc"].to<JsonObject>();
            vocReading["value"] = round(voc * 10) / 10.0;
            vocReading["unit"] = "ppb";
            vocReading["status"] = "valid";
        }
            
        String result;
        serializeJson(doc, result);
        return result;
    }
    
    /**
     * Check if record is valid (has any sensor data)
     */
    bool isValid() const {
        return (validity_flags & FLAG_OVERALL_VALID) != 0;
    }
    
    /**
     * Get overall alert level
     */
    AlertLevel getAlertLevel() const {
        return static_cast<AlertLevel>(alert_level);
    }
};

/**
 * Storage information structure
 */
struct StorageInfo {
    String storage_type;            // "flash" or "sd_card"
    float total_capacity_mb;        // Total capacity in MB
    float used_capacity_mb;         // Used capacity in MB
    float free_capacity_mb;         // Free capacity in MB
    uint32_t total_records;         // Total number of records
    unsigned long oldest_record_time;  // Oldest record timestamp
    unsigned long newest_record_time;  // Newest record timestamp
    float estimated_days_remaining; // Estimated days of storage remaining
    
    /**
     * Convert to JSON for transmission
     */
    String toJson() const {
        JsonDocument doc;
        doc["storage_type"] = storage_type;
        doc["total_capacity_mb"] = round(total_capacity_mb * 10) / 10.0;
        doc["used_capacity_mb"] = round(used_capacity_mb * 10) / 10.0;
        doc["free_capacity_mb"] = round(free_capacity_mb * 10) / 10.0;
        doc["total_records"] = total_records;
        doc["oldest_record_time"] = oldest_record_time;
        doc["newest_record_time"] = newest_record_time;
        doc["estimated_days_remaining"] = round(estimated_days_remaining * 10) / 10.0;
        
        String result;
        serializeJson(doc, result);
        return result;
    }
};

/**
 * Historical data storage manager
 * Handles circular buffer storage of sensor records
 */
class HistoricalDataStorage : public IDataStorage {
private:
    // Storage configuration
    static const size_t RECORD_SIZE = sizeof(SensorRecord);
    static const size_t MAX_RECORDS_FLASH = 600;     // ~24KB for default NVS partition (38-byte records)
    static const size_t CHUNK_SIZE = 50;             // Records per transmission chunk
    static const uint32_t STORAGE_MAGIC = 0x436F546D; // "CoTm" magic number
    
    // Storage state
    size_t max_records;
    size_t current_records;
    size_t write_index;
    size_t read_index;
    bool storage_full;
    String storage_type;
    
    // Record buffer for flash storage
    std::vector<SensorRecord> record_buffer;
    
    // Storage validation
    bool initialized;
    
public:
    HistoricalDataStorage(const String& type = "flash", size_t max_recs = MAX_RECORDS_FLASH);
    virtual ~HistoricalDataStorage() = default;
    
    // ================================
    // INITIALIZATION & SETUP
    // ================================
    
    bool initialize();
    bool format();
    void reset();
    
    // ================================
    // CORE STORAGE OPERATIONS
    // ================================
    
    bool storeReading(const SensorRecord& record);
    bool storeReading(unsigned long uptime, 
                     const CO2SensorData* co2_data = nullptr,
                     const VOCSensorData* voc_data = nullptr);
    
    // ================================
    // HISTORICAL DATA QUERIES
    // ================================
    
    std::vector<SensorRecord> queryByTimeRange(const TimeRange& range, const TimeSync& timeSync);
    std::vector<SensorRecord> queryByUptimeRange(unsigned long start_uptime, unsigned long end_uptime);
    std::vector<SensorRecord> queryLatest(size_t count = 100);
    
    // Query with pagination support
    struct QueryResult {
        std::vector<SensorRecord> records;
        size_t total_available;
        bool has_more;
    };
    
    QueryResult queryByTimeRangePaged(const TimeRange& range, const TimeSync& timeSync, 
                                     size_t page_size = CHUNK_SIZE, size_t page_index = 0);
    
    // ================================
    // STORAGE INFORMATION
    // ================================
    
    StorageInfo getStorageInfo(const TimeSync& timeSync) const;
    size_t getRecordCount() const { return current_records; }
    size_t getMaxRecords() const { return max_records; }
    bool isFull() const { return storage_full; }
    bool isEmpty() const { return current_records == 0; }
    
    // Get time range of stored data
    bool getDataTimeRange(unsigned long& oldest_uptime, unsigned long& newest_uptime) const;
    
    // ================================
    // MAINTENANCE OPERATIONS
    // ================================
    
    bool clearOldData(unsigned long before_uptime);
    bool clearOldData(const TimeSync& timeSync, uint32_t max_age_hours = 168); // 7 days default
    size_t getEstimatedDaysRemaining(uint32_t records_per_day = 8640) const; // Assuming 10s interval
    
    // ================================
    // IDataStorage INTERFACE
    // ================================
    
    bool saveConfig(const DeviceConfig& config) override { return false; } // Not implemented
    bool loadConfig(DeviceConfig& config) override { return false; }       // Not implemented  
    bool hasConfig() override { return false; }                            // Not implemented
    
    bool saveData(const SensorDataBase& data) override { return false; }       // Legacy interface
    bool loadData(SensorDataBase& data, uint32_t index = 0) override { return false; } // Legacy interface
    uint32_t getDataCount() override { return current_records; }
    
    bool clear() override;
    uint32_t getUsedSpace() override;
    uint32_t getFreeSpace() override;
    String exportData(uint32_t count = 10) override;
    String getStorageType() override { return storage_type; }
    bool isReady() override { return initialized; }
    
private:
    // ================================
    // INTERNAL HELPERS
    // ================================
    
    bool validateRecord(const SensorRecord& record) const;
    void updateIndices();
    size_t getNextWriteIndex() const;
    bool shouldOverwrite() const;
    
    // Storage persistence (for ESP32 flash)
    bool loadFromFlash();
    bool saveToFlash();
    String getStorageKey(size_t index) const;
    
    // Data sampling for large queries
    std::vector<SensorRecord> sampleRecords(const std::vector<SensorRecord>& records, 
                                           size_t max_points) const;
};

// ================================
// STORAGE FACTORY
// ================================

/**
 * Factory for creating storage instances
 */
class StorageFactory {
public:
    enum StorageType {
        FLASH_STORAGE,
        SD_CARD_STORAGE,
        AUTO_DETECT
    };
    
    static std::unique_ptr<HistoricalDataStorage> createStorage(StorageType type = AUTO_DETECT);
    static bool isSDCardAvailable();
    static size_t getAvailableFlashSpace();
};
