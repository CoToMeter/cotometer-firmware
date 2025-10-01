/*
 * types/Event.h
 * Event system data structures
 */

#pragma once

#include <Arduino.h>
#include "SystemEnums.h"
#include <vector>
#include <memory>

// ================================
// EVENT STRUCTURE
// ================================

struct Event {
    EventType type;             // Event type identifier
    String data;               // Event data payload (JSON string)
    String source;             // Event source identifier
    uint32_t timestamp;        // Event creation time
    int priority;              // Event priority (0-10, higher = more important)
    uint32_t id;               // Unique event ID
    
    // Event lifecycle
    bool handled;              // Has this event been processed?
    uint8_t handlerCount;      // Number of handlers that processed this event
    uint32_t processingTime;   // Time spent processing (microseconds)
    
    // Event context
    SystemState systemState;   // System state when event occurred
    AlertLevel alertLevel;     // Associated alert level
    String context;            // Additional context information
    
    // Constructors
    Event();
    Event(EventType type, const String& data = "");
    Event(EventType type, const String& data, int priority);
    Event(EventType type, const String& data, const String& source, int priority);
    
    // Copy constructor and assignment
    Event(const Event& other);
    Event& operator=(const Event& other);
    
    // Validation
    bool isValid() const;
    bool isExpired(uint32_t maxAge) const; // Check if event is too old
    
    // Utility methods
    void markAsHandled();
    void incrementHandlerCount();
    void addProcessingTime(uint32_t processingMicros);
    void updateTimestamp();
    String getAgeString() const; // Human-readable age
    
    // Data payload helpers
    bool hasData() const;
    bool setData(const String& key, const String& value);
    String getData(const String& key, const String& defaultValue = "") const;
    bool setDataJson(const String& json);
    String getDataJson() const;
    
    // Serialization
    String toString() const;
    String toJson() const;
    bool fromJson(const String& json);
    
    // Static factory methods
    static Event createButtonEvent(const String& buttonId = "main");
    static Event createSensorEvent(SensorType sensorType, const String& sensorData);
    static Event createErrorEvent(const String& errorMessage, const String& source);
    static Event createTimerEvent(const String& timerId);
    static Event createThresholdEvent(const String& parameter, float value, float threshold);
    static Event createBatteryEvent(uint8_t percentage, bool isCharging);
    static Event createCalibrationEvent(SensorType sensorType, bool success);
    static Event createCommunicationEvent(bool connected, CommunicationType commType);
    
    // Comparison operators
    bool operator==(const Event& other) const;
    bool operator!=(const Event& other) const;
    bool operator<(const Event& other) const; // For priority ordering
    
private:
    static uint32_t nextEventId;
    void generateId();
};

// ================================
// EVENT QUEUE
// ================================

class EventQueue {
private:
    std::vector<Event> events;
    size_t maxSize;
    mutable bool sorted;
    
public:
    explicit EventQueue(size_t maxSize = Constants::MAX_EVENT_QUEUE_SIZE);
    ~EventQueue();
    
    // Queue operations
    bool enqueue(const Event& event);
    Event dequeue();
    bool isEmpty() const;
    bool isFull() const;
    size_t size() const;
    size_t capacity() const;
    
    // Priority queue operations
    Event peek() const; // Look at highest priority event without removing
    bool enqueueWithPriority(const Event& event);
    void sortByPriority() const;
    
    // Queue management
    void clear();
    bool remove(uint32_t eventId);
    bool contains(uint32_t eventId) const;
    bool contains(EventType type) const;
    
    // Event filtering
    std::vector<Event> getEventsByType(EventType type) const;
    std::vector<Event> getEventsBySource(const String& source) const;
    std::vector<Event> getEventsByPriority(int minPriority) const;
    std::vector<Event> getEventsOlderThan(uint32_t ageMs) const;
    
    // Queue statistics
    uint32_t getTotalEventsProcessed() const;
    uint32_t getAverageProcessingTime() const;
    Event getOldestEvent() const;
    Event getNewestEvent() const;
    
    // Cleanup operations
    void removeExpiredEvents(uint32_t maxAge);
    void removeHandledEvents();
    void removeEventsByType(EventType type);
    
private:
    uint32_t totalProcessed;
    uint32_t totalProcessingTime;
    void maintainMaxSize();
};

// ================================
// EVENT STATISTICS
// ================================

struct EventStatistics {
    // Count statistics
    uint32_t totalEvents;
    uint32_t handledEvents;
    uint32_t expiredEvents;
    uint32_t droppedEvents; // Due to queue full
    
    // Type distribution
    std::vector<std::pair<EventType, uint32_t>> eventTypeCounts;
    
    // Timing statistics
    uint32_t averageProcessingTime;
    uint32_t maxProcessingTime;
    uint32_t minProcessingTime;
    uint32_t totalProcessingTime;
    
    // Performance metrics
    float eventsPerSecond;
    uint32_t peakQueueSize;
    uint32_t currentQueueSize;
    
    // Error statistics
    uint32_t handlingErrors;
    String lastError;
    uint32_t lastErrorTime;
    
    EventStatistics();
    void reset();
    String toString() const;
    String toJson() const;
};

// ================================
// EVENT FILTER
// ================================

class EventFilter {
public:
    using FilterFunction = std::function<bool(const Event&)>;
    
private:
    std::vector<EventType> allowedTypes;
    std::vector<EventType> blockedTypes;
    std::vector<String> allowedSources;
    std::vector<String> blockedSources;
    int minPriority;
    int maxPriority;
    uint32_t maxAge;
    FilterFunction customFilter;
    
public:
    EventFilter();
    
    // Type filtering
    void allowType(EventType type);
    void blockType(EventType type);
    void allowTypes(const std::vector<EventType>& types);
    void blockTypes(const std::vector<EventType>& types);
    void clearTypeFilters();
    
    // Source filtering
    void allowSource(const String& source);
    void blockSource(const String& source);
    void allowSources(const std::vector<String>& sources);
    void blockSources(const std::vector<String>& sources);
    void clearSourceFilters();
    
    // Priority filtering
    void setPriorityRange(int min, int max);
    void setMinPriority(int priority);
    void setMaxPriority(int priority);
    void clearPriorityFilter();
    
    // Age filtering
    void setMaxAge(uint32_t ageMs);
    void clearAgeFilter();
    
    // Custom filtering
    void setCustomFilter(FilterFunction filter);
    void clearCustomFilter();
    
    // Filter operations
    bool shouldAccept(const Event& event) const;
    std::vector<Event> filterEvents(const std::vector<Event>& events) const;
    
    // Utility
    void reset();
    String toString() const;
};

// ================================
// EVENT BUILDER (Builder Pattern)
// ================================

class EventBuilder {
private:
    Event event;
    
public:
    EventBuilder();
    explicit EventBuilder(EventType type);
    
    // Fluent interface for building events
    EventBuilder& withType(EventType type);
    EventBuilder& withData(const String& data);
    EventBuilder& withSource(const String& source);
    EventBuilder& withPriority(int priority);
    EventBuilder& withAlertLevel(AlertLevel level);
    EventBuilder& withContext(const String& context);
    EventBuilder& withSystemState(SystemState state);
    
    // Data payload builders
    EventBuilder& addData(const String& key, const String& value);
    EventBuilder& addData(const String& key, float value);
    EventBuilder& addData(const String& key, int value);
    EventBuilder& addData(const String& key, bool value);
    
    // Specialized builders
    EventBuilder& asSensorEvent(SensorType sensorType);
    EventBuilder& asButtonEvent(const String& buttonId = "main");
    EventBuilder& asErrorEvent(const String& errorMsg);
    EventBuilder& asTimerEvent(const String& timerId);
    EventBuilder& asThresholdEvent(const String& parameter, float value, float threshold);
    
    // Build the final event
    Event build();
    
    // Validation
    bool isValid() const;
    std::vector<String> getValidationErrors() const;
};

// ================================
// EVENT UTILITIES
// ================================

namespace EventUtils {
    // Event type checking
    bool isUserEvent(EventType type);
    bool isSensorEvent(EventType type);
    bool isSystemEvent(EventType type);
    bool isCommunicationEvent(EventType type);
    bool isTimerEvent(EventType type);
    bool isErrorEvent(EventType type);
    bool isCalibrationEvent(EventType type);
    
    // Event priority helpers
    int getDefaultPriority(EventType type);
    bool isHighPriority(const Event& event);
    bool isLowPriority(const Event& event);
    
    // Event age utilities
    bool isRecentEvent(const Event& event, uint32_t ageMs = 5000);
    bool isOldEvent(const Event& event, uint32_t ageMs = 60000);
    String formatAge(uint32_t ageMs);
    
    // Event data helpers
    bool extractSensorData(const Event& event, float& co2, float& temp, float& humidity);
    bool extractButtonData(const Event& event, String& buttonId, uint32_t& pressTime);
    bool extractErrorData(const Event& event, String& errorCode, String& errorMessage);
    
    // Event formatting
    String formatEventForDisplay(const Event& event);
    String formatEventForLogging(const Event& event);
    String formatEventForTransmission(const Event& event);
    
    // Event validation
    bool validateEventData(const Event& event);
    std::vector<String> getValidationErrors(const Event& event);
    Event sanitizeEvent(const Event& event);
}