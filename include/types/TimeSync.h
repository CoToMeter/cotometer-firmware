/*
 * types/TimeSync.h
 * Time synchronization data structures for ESP32 ↔ Android communication
 */

#pragma once
#include <Arduino.h>

/**
 * Time synchronization management for ESP32 devices without RTC
 * Maps Unix timestamps to ESP32 uptime for historical data retrieval
 */
struct TimeSync {
    bool has_time = false;              // Whether time has been synchronized
    unsigned long sync_uptime = 0;      // ESP32 uptime when sync occurred (ms)
    uint64_t time_offset = 0;           // Unix timestamp - uptime offset
    unsigned long last_sync_uptime = 0; // Last successful sync uptime
    String timezone_offset = "+0000";   // Timezone offset string (e.g., "+0300")
    
    /**
     * Calculate current Unix timestamp from ESP32 uptime
     * @return Current Unix timestamp in milliseconds, or 0 if not synced
     */
    uint64_t getCurrentTimestamp() const {
        if (!has_time) return 0;
        return millis() + time_offset;
    }
    
    /**
     * Convert Unix timestamp to ESP32 uptime
     * @param timestamp Unix timestamp in milliseconds
     * @return ESP32 uptime in milliseconds, or 0 if not synced
     */
    unsigned long timestampToUptime(uint64_t timestamp) const {
        if (!has_time || timestamp < time_offset) return 0;
        return (unsigned long)(timestamp - time_offset);
    }
    
    /**
     * Convert ESP32 uptime to Unix timestamp
     * @param uptime ESP32 uptime in milliseconds
     * @return Unix timestamp in milliseconds, or 0 if not synced
     */
    uint64_t uptimeToTimestamp(unsigned long uptime) const {
        if (!has_time) return 0;
        return uptime + time_offset;
    }
    
    /**
     * Set time synchronization with current app timestamp
     * @param current_timestamp Current Unix timestamp from app (ms)
     * @param timezone_str Timezone offset string (e.g., "+0300")
     * @return true if sync successful
     */
    bool synchronizeTime(uint64_t current_timestamp, const String& timezone_str = "+0000") {
        unsigned long current_uptime = millis();
        
        // Basic validation
        if (current_timestamp < 1600000000000ULL) { // Before 2020
            return false;
        }
        
        // Calculate offset  
        time_offset = current_timestamp - current_uptime;
        sync_uptime = current_uptime;
        last_sync_uptime = current_uptime;
        timezone_offset = timezone_str;
        has_time = true;
        
        Serial.printf("⏰ Time synchronized: Offset=%llu, Uptime=%lu, Timestamp=%llu\n", 
                     time_offset, current_uptime, current_timestamp);
        
        return true;
    }
    
    /**
     * Check if time sync is stale and needs refresh
     * @param max_age_hours Maximum age in hours before considering stale
     * @return true if sync is stale
     */
    bool isSyncStale(uint8_t max_age_hours = 24) const {
        if (!has_time) return true;
        
        unsigned long age_ms = millis() - last_sync_uptime;
        unsigned long max_age_ms = max_age_hours * 3600000UL; // hours to ms
        
        return age_ms > max_age_ms;
    }
    
    /**
     * Get age of current time sync in minutes
     * @return Age in minutes, or 0 if not synced
     */
    uint32_t getSyncAgeMinutes() const {
        if (!has_time) return 0;
        return (millis() - last_sync_uptime) / 60000;
    }
    
    /**
     * Reset time synchronization (clear all data)
     */
    void reset() {
        has_time = false;
        sync_uptime = 0;
        time_offset = 0;
        last_sync_uptime = 0;
        timezone_offset = "+0000";
        
        Serial.println("⏰ Time sync reset");
    }
    
    /**
     * Get human-readable sync status
     * @return Status string
     */
    String getStatusString() const {
        if (!has_time) {
            return "No time sync";
        }
        
        uint32_t age_min = getSyncAgeMinutes();
        String status = "Synced ";
        
        if (age_min < 60) {
            status += String(age_min) + "min ago";
        } else {
            status += String(age_min / 60) + "h " + String(age_min % 60) + "min ago";
        }
        
        if (isSyncStale()) {
            status += " (STALE)";
        }
        
        return status;
    }
};

/**
 * Time range specification for historical data queries
 */
struct TimeRange {
    uint64_t start_time = 0;            // Start timestamp (Unix ms)
    uint64_t end_time = 0;              // End timestamp (Unix ms)
    uint16_t max_points = 1000;         // Maximum data points to return
    
    /**
     * Validate time range
     * @return true if range is valid
     */
    bool isValid() const {
        return start_time > 0 && 
               end_time > start_time && 
               max_points > 0 && 
               max_points <= 10000; // Reasonable limit
    }
    
    /**
     * Get duration of time range in seconds
     * @return Duration in seconds
     */
    uint32_t getDurationSeconds() const {
        if (!isValid()) return 0;
        return (end_time - start_time) / 1000;
    }
    
    /**
     * Get suggested sampling interval for max_points
     * @return Suggested interval in seconds
     */
    uint32_t getSuggestedInterval() const {
        uint32_t duration = getDurationSeconds();
        if (duration == 0 || max_points == 0) return 10; // Default 10s
        
        return max(1U, duration / max_points);
    }
    
    /**
     * Check if timestamp falls within this range
     * @param timestamp Unix timestamp to check
     * @return true if timestamp is in range
     */
    bool contains(uint64_t timestamp) const {
        return timestamp >= start_time && timestamp <= end_time;
    }
};
