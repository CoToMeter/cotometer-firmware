/*
 * interfaces/IEventHandler.h
 * Interface for event handling
 */

#pragma once

#include "../types/Event.h"
#include "../types/SystemEnums.h"

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    
    // Core event handling
    virtual void handleEvent(const Event& event) = 0;
    virtual bool canHandle(EventType type) = 0;
    
    // Handler information
    virtual String getHandlerName() = 0;
    virtual int getPriority() = 0; // Higher number = higher priority
    
    // Event filtering
    virtual bool shouldHandle(const Event& event) = 0;
    virtual std::vector<EventType> getSupportedEvents() = 0;
    
    // Handler lifecycle
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool isReady() = 0;
};