/*
 * storage/HistoricalDataStorage.cpp
 * Implementation of historical sensor data storage system
 */

#include "storage/HistoricalDataStorage.h"
// #include <Preferences.h>  // Disabled - using RAM-only storage
#include <algorithm>

// Note: Flash persistence disabled - using RAM-only storage for faster operation
// static Preferences preferences;  // Not used in RAM-only mode

HistoricalDataStorage::HistoricalDataStorage(const String& type, size_t max_recs)
    : max_records(max_recs)
    , current_records(0)
    , write_index(0)
    , read_index(0)
    , storage_full(false)
    , storage_type(type)
    , initialized(false) {
    
    // Limit max records based on available memory
    if (max_records > MAX_RECORDS_FLASH) {
        max_records = MAX_RECORDS_FLASH;
        Serial.printf("⚠️  Storage: Limited max records to %zu due to memory constraints\n", max_records);
    }
    
    // Reserve space for record buffer
    record_buffer.reserve(max_records);
}

// ================================
// INITIALIZATION & SETUP
// ================================

bool HistoricalDataStorage::initialize() {
    Serial.printf("💾 Initializing %s storage (max: %zu records)...\n", 
                 storage_type.c_str(), max_records);
    
    // Note: Preferences/flash storage disabled - using RAM-only mode
    // This provides faster operation but data is lost on power cycle
    
    // Clear buffer and reset indices
    record_buffer.clear();
    current_records = 0;
    write_index = 0;
    read_index = 0;
    storage_full = false;
    
    // Note: Flash loading disabled - starting with empty RAM buffer
    // All data stored in RAM only for faster operation
    
    initialized = true;
    
    Serial.printf("✅ RAM-only storage initialized: %zu/%zu capacity (data lost on reboot)\n",
                 current_records, max_records);
    
    return true;
}

bool HistoricalDataStorage::format() {
    Serial.println("🗑️  Formatting storage...");
    
    // Clear all data
    record_buffer.clear();
    current_records = 0;
    write_index = 0;
    read_index = 0;
    storage_full = false;
    
    // Note: No flash storage to clear - RAM only mode
    
    Serial.println("✅ Storage formatted successfully");
    return true;
}

void HistoricalDataStorage::reset() {
    format();
}

// ================================
// CORE STORAGE OPERATIONS
// ================================

bool HistoricalDataStorage::storeReading(const SensorRecord& record) {
    if (!initialized) {
        Serial.println("❌ Storage not initialized");
        return false;
    }
    
    if (!validateRecord(record)) {
        Serial.println("❌ Invalid sensor record");
        return false;
    }
    
    // Handle circular buffer logic
    if (current_records < max_records) {
        // Still have space, just append
        record_buffer.push_back(record);
        current_records++;
        write_index = current_records;
    } else {
        // Buffer full, implement circular buffer
        if (!storage_full) {
            storage_full = true;
            Serial.println("⚠️  Storage buffer full, switching to circular buffer mode");
        }
        
        // Overwrite oldest record
        write_index = write_index % max_records;
        record_buffer[write_index] = record;
        write_index = (write_index + 1) % max_records;
        
        // Update read index to skip overwritten record
        read_index = write_index;
    }
    
    // Note: Flash persistence disabled - data stored in RAM only
    // Data will be lost on power cycle but provides faster operation
    
    return true;
}

bool HistoricalDataStorage::storeReading(unsigned long uptime, 
                                        const CO2SensorData* co2_data,
                                        const VOCSensorData* voc_data) {
    
    SensorRecord record(uptime, co2_data, voc_data);
    return storeReading(record);
}

// ================================
// HISTORICAL DATA QUERIES
// ================================

std::vector<SensorRecord> HistoricalDataStorage::queryByTimeRange(const TimeRange& range, const TimeSync& timeSync) {
    if (!timeSync.has_time) {
        Serial.println("❌ Cannot query by time range: Time not synchronized");
        return {};
    }
    
    // Convert timestamps to uptime range
    unsigned long start_uptime = timeSync.timestampToUptime(range.start_time);
    unsigned long end_uptime = timeSync.timestampToUptime(range.end_time);
    
    Serial.printf("🔍 Time conversion: start_ts=%llu -> uptime=%lu, end_ts=%llu -> uptime=%lu\n",
                 range.start_time, start_uptime, range.end_time, end_uptime);
    Serial.printf("🔍 TimeSync: has_time=%s, offset=%llu, current_uptime=%lu\n",
                 timeSync.has_time ? "true" : "false", timeSync.time_offset, millis());
    
    // Handle case where requested time is before device boot
    if (start_uptime == 0) {
        // Start time is before device boot, adjust to earliest available data
        start_uptime = 1; // Start from beginning of device uptime
        Serial.printf("⚠️ Adjusting start time: requested timestamp before device boot\n");
    }
    
    if (end_uptime == 0) {
        // End time is before device boot - no data available
        Serial.println("❌ End time is before device boot - no data available");
        return {};
    }
    
    // Cap end time to current uptime if it's in the future
    unsigned long current_uptime = millis();
    if (end_uptime > current_uptime) {
        end_uptime = current_uptime;
        Serial.printf("⚠️ Adjusting end time: requested timestamp in future (capped to current uptime: %lu)\n", current_uptime);
    }
    
    return queryByUptimeRange(start_uptime, end_uptime);
}

std::vector<SensorRecord> HistoricalDataStorage::queryByUptimeRange(unsigned long start_uptime, unsigned long end_uptime) {
    std::vector<SensorRecord> results;
    
    if (!initialized || current_records == 0) {
        return results;
    }
    
    // Search through all records
    size_t search_start = storage_full ? read_index : 0;
    size_t search_count = current_records;
    
    for (size_t i = 0; i < search_count; i++) {
        size_t idx = (search_start + i) % max_records;
        const SensorRecord& record = record_buffer[idx];
        
        if (record.uptime >= start_uptime && record.uptime <= end_uptime && record.isValid()) {
            results.push_back(record);
        }
    }
    
    // Sort by uptime (should already be sorted, but ensure it)
    std::sort(results.begin(), results.end(), 
              [](const SensorRecord& a, const SensorRecord& b) {
                  return a.uptime < b.uptime;
              });
    
    Serial.printf("📊 Query found %zu records in uptime range %lu-%lu\n", 
                 results.size(), start_uptime, end_uptime);
    
    return results;
}

std::vector<SensorRecord> HistoricalDataStorage::queryLatest(size_t count) {
    std::vector<SensorRecord> results;
    
    if (!initialized || current_records == 0) {
        return results;
    }
    
    // Limit count to available records
    size_t actual_count = min(count, current_records);
    
    // Get latest records (work backwards from write index)
    for (size_t i = 0; i < actual_count; i++) {
        size_t idx;
        if (storage_full) {
            // Circular buffer: work backwards from write_index
            idx = (write_index - 1 - i + max_records) % max_records;
        } else {
            // Linear buffer: work backwards from end
            idx = current_records - 1 - i;
        }
        
        if (idx < record_buffer.size() && record_buffer[idx].isValid()) {
            results.push_back(record_buffer[idx]);
        }
    }
    
    // Reverse to get chronological order (oldest first)
    std::reverse(results.begin(), results.end());
    
    Serial.printf("📊 Query returned %zu latest records\n", results.size());
    return results;
}

HistoricalDataStorage::QueryResult HistoricalDataStorage::queryByTimeRangePaged(
    const TimeRange& range, const TimeSync& timeSync, 
    size_t page_size, size_t page_index) {
    
    QueryResult result;
    result.has_more = false;
    result.total_available = 0;
    
    // Get all records in range first
    std::vector<SensorRecord> all_records = queryByTimeRange(range, timeSync);
    result.total_available = all_records.size();
    
    if (all_records.empty()) {
        return result;
    }
    
    // Apply sampling if we have too many records
    if (all_records.size() > range.max_points) {
        all_records = sampleRecords(all_records, range.max_points);
    }
    
    // Calculate pagination
    size_t start_idx = page_index * page_size;
    size_t end_idx = min(start_idx + page_size, all_records.size());
    
    if (start_idx < all_records.size()) {
        result.records.assign(all_records.begin() + start_idx, all_records.begin() + end_idx);
        result.has_more = end_idx < all_records.size();
    }
    
    return result;
}

// ================================
// STORAGE INFORMATION
// ================================

StorageInfo HistoricalDataStorage::getStorageInfo(const TimeSync& timeSync) const {
    StorageInfo info;
    info.storage_type = storage_type + "_ram_only";  // Indicate RAM-only storage
    info.total_capacity_mb = (max_records * RECORD_SIZE) / (1024.0 * 1024.0);
    info.used_capacity_mb = (current_records * RECORD_SIZE) / (1024.0 * 1024.0);
    info.free_capacity_mb = info.total_capacity_mb - info.used_capacity_mb;
    info.total_records = current_records;
    info.oldest_record_time = 0;
    info.newest_record_time = 0;
    
    // Get time range of stored data
    if (current_records > 0) {
        unsigned long oldest_uptime, newest_uptime;
        if (getDataTimeRange(oldest_uptime, newest_uptime)) {
            if (timeSync.has_time) {
                // Convert to real timestamps if time is synced
                info.oldest_record_time = timeSync.uptimeToTimestamp(oldest_uptime);
                info.newest_record_time = timeSync.uptimeToTimestamp(newest_uptime);
            } else {
                // Use uptime values directly (will be mapped later when time syncs)
                info.oldest_record_time = oldest_uptime;
                info.newest_record_time = newest_uptime;
            }
        }
    }
    
    // Estimate remaining days (assuming 10-second intervals)
    size_t remaining_records = max_records - current_records;
    if (!storage_full && remaining_records > 0) {
        info.estimated_days_remaining = (remaining_records * 10.0) / (24.0 * 3600.0); // 10s intervals
    } else {
        info.estimated_days_remaining = 0; // Circular buffer mode
    }
    
    return info;
}

bool HistoricalDataStorage::getDataTimeRange(unsigned long& oldest_uptime, unsigned long& newest_uptime) const {
    if (current_records == 0) {
        return false;
    }
    
    if (storage_full) {
        // In circular buffer mode, oldest is at read_index, newest is at write_index-1
        size_t oldest_idx = read_index;
        size_t newest_idx = (write_index - 1 + max_records) % max_records;
        
        oldest_uptime = record_buffer[oldest_idx].uptime;
        newest_uptime = record_buffer[newest_idx].uptime;
    } else {
        // In linear mode, oldest is at index 0, newest is at current_records-1
        oldest_uptime = record_buffer[0].uptime;
        newest_uptime = record_buffer[current_records - 1].uptime;
    }
    
    return true;
}

// ================================
// MAINTENANCE OPERATIONS
// ================================

bool HistoricalDataStorage::clearOldData(unsigned long before_uptime) {
    if (!initialized || current_records == 0) {
        return true;
    }
    
    size_t removed_count = 0;
    std::vector<SensorRecord> filtered_records;
    
    // Collect records newer than before_uptime
    size_t search_start = storage_full ? read_index : 0;
    
    for (size_t i = 0; i < current_records; i++) {
        size_t idx = (search_start + i) % max_records;
        const SensorRecord& record = record_buffer[idx];
        
        if (record.uptime >= before_uptime) {
            filtered_records.push_back(record);
        } else {
            removed_count++;
        }
    }
    
    // Replace buffer with filtered records
    record_buffer = filtered_records;
    current_records = filtered_records.size();
    write_index = current_records;
    read_index = 0;
    storage_full = false;
    
    // Ensure we don't exceed max_records
    if (current_records > max_records) {
        record_buffer.resize(max_records);
        current_records = max_records;
        storage_full = true;
    }
    
    Serial.printf("🗑️  Removed %zu old records (before uptime %lu)\n", removed_count, before_uptime);
    
    if (storage_type == "flash") {
        saveToFlash();
    }
    
    return true;
}

bool HistoricalDataStorage::clearOldData(const TimeSync& timeSync, uint32_t max_age_hours) {
    if (!timeSync.has_time) {
        Serial.println("⚠️  Cannot clear old data: Time not synchronized");
        return false;
    }
    
    unsigned long cutoff_timestamp = timeSync.getCurrentTimestamp() - (max_age_hours * 3600000UL);
    unsigned long cutoff_uptime = timeSync.timestampToUptime(cutoff_timestamp);
    
    return clearOldData(cutoff_uptime);
}

size_t HistoricalDataStorage::getEstimatedDaysRemaining(uint32_t records_per_day) const {
    if (storage_full || records_per_day == 0) {
        return 0; // Circular buffer or invalid input
    }
    
    size_t remaining_records = max_records - current_records;
    return remaining_records / records_per_day;
}

// ================================
// IDataStorage INTERFACE
// ================================

bool HistoricalDataStorage::clear() {
    return format();
}

uint32_t HistoricalDataStorage::getUsedSpace() {
    return current_records * RECORD_SIZE;
}

uint32_t HistoricalDataStorage::getFreeSpace() {
    if (storage_full) return 0;
    return (max_records - current_records) * RECORD_SIZE;
}

String HistoricalDataStorage::exportData(uint32_t count) {
    std::vector<SensorRecord> records = queryLatest(count);
    
    JsonDocument doc;
    doc["storage_type"] = storage_type;
    doc["total_records"] = current_records;
    doc["exported_count"] = records.size();
    doc["export_timestamp"] = millis();
    
    JsonArray data_array = doc["data"].to<JsonArray>();
    
    for (const SensorRecord& record : records) {
        JsonObject record_obj = data_array.add<JsonObject>();
        record_obj["uptime"] = record.uptime;
        record_obj["co2"] = record.co2;
        record_obj["temperature"] = record.temperature;
        record_obj["humidity"] = record.humidity;
        record_obj["pressure"] = record.pressure;
        record_obj["voc"] = record.voc;
        record_obj["validity_flags"] = record.validity_flags;
        record_obj["alert_level"] = record.alert_level;
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

// ================================
// INTERNAL HELPERS
// ================================

bool HistoricalDataStorage::validateRecord(const SensorRecord& record) const {
    // Basic validation
    return record.uptime > 0 && 
           record.validity_flags != 0 && 
           record.isValid();
}

void HistoricalDataStorage::updateIndices() {
    if (storage_full) {
        write_index = (write_index + 1) % max_records;
        read_index = write_index; // Oldest record is always next to be overwritten
    } else {
        write_index = current_records;
        read_index = 0;
    }
}

size_t HistoricalDataStorage::getNextWriteIndex() const {
    if (current_records < max_records) {
        return current_records;
    } else {
        return write_index % max_records;
    }
}

bool HistoricalDataStorage::shouldOverwrite() const {
    return current_records >= max_records;
}


String HistoricalDataStorage::getStorageKey(size_t index) const {
    return "rec_" + String(index);
}

std::vector<SensorRecord> HistoricalDataStorage::sampleRecords(const std::vector<SensorRecord>& records, size_t max_points) const {
    if (records.size() <= max_points) {
        return records;
    }
    
    std::vector<SensorRecord> sampled;
    sampled.reserve(max_points);
    
    // Use simple uniform sampling
    double step = static_cast<double>(records.size()) / max_points;
    
    for (size_t i = 0; i < max_points; i++) {
        size_t idx = static_cast<size_t>(i * step);
        if (idx < records.size()) {
            sampled.push_back(records[idx]);
        }
    }
    
    Serial.printf("📊 Sampled %zu records from %zu (max: %zu)\n", 
                 sampled.size(), records.size(), max_points);
    
    return sampled;
}

// ================================
// STORAGE FACTORY
// ================================

std::unique_ptr<HistoricalDataStorage> StorageFactory::createStorage(StorageType type) {
    switch (type) {
        case FLASH_STORAGE:
            return std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("flash", 600));
            
        case SD_CARD_STORAGE:
            if (isSDCardAvailable()) {
                return std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("sd_card", 500000)); // 500k records
            } else {
                Serial.println("⚠️  SD card not available, falling back to flash");
                return std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("flash", 600));
            }
            
        case AUTO_DETECT:
        default:
            if (isSDCardAvailable()) {
                Serial.println("📱 Auto-detected SD card storage");
                return std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("sd_card", 500000));
            } else {
                Serial.println("💾 Using internal flash storage");
                return std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("flash", 600));
            }
    }
}

bool StorageFactory::isSDCardAvailable() {
    // TODO: Implement SD card detection
    return false; // For now, only use flash storage
}

size_t StorageFactory::getAvailableFlashSpace() {
    return ESP.getFreeHeap();
}
