#pragma once
#include <Arduino.h>
#include <vector>
#include "../types/Event.h"
#include "../types/SystemEnums.h"

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    
    // ================================
    // ESSENTIAL OPERATIONS
    // ================================
    
    // Core event handling (must implement)
    virtual void handleEvent(const Event& event) = 0;
    virtual bool canHandle(EventType type) = 0;
    virtual String getHandlerName() = 0;
    virtual int getPriority() = 0; // Higher = more important
    
    // ================================
    // OPTIONAL FEATURES
    // ================================
    
    // Event filtering
    virtual bool shouldHandle(const Event& event) { return canHandle(event.type); }
    virtual std::vector<EventType> getSupportedEvents() { return {}; }
    
    // Handler lifecycle
    virtual bool initialize() { return true; }
    virtual void shutdown() { /* optional */ }
    virtual bool isReady() { return true; }
    
    // Handler state
    virtual bool isEnabled() { return true; }
    virtual void setEnabled(bool enabled) { /* optional */ }
};

