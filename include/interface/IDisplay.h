/*
 * interfaces/IDisplay.h
 * Interface for all display implementations
 */

#pragma once

#include <Arduino.h>
#include "../types/SystemEnums.h"

class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Core display operations
    virtual bool initialize() = 0;
    virtual void clear() = 0;
    virtual void update() = 0;
    virtual bool isReady() = 0;
    
    // Screen management
    virtual void showScreen(ScreenType screen) = 0;
    virtual ScreenType getCurrentScreen() = 0;
    virtual void showMessage(const String& message) = 0;
    virtual void showError(const String& error) = 0;
    
    // Display settings
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual uint8_t getBrightness() = 0;
    virtual void setContrast(uint8_t contrast) = 0;
    virtual void setRotation(uint8_t rotation) = 0;
    
    // Power management
    virtual void sleep() = 0;
    virtual void wakeup() = 0;
    virtual bool isSleeping() = 0;
    
    // Drawing primitives (optional)
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    
    // Text operations
    virtual void setTextSize(uint8_t size) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const String& text) = 0;
    virtual void println(const String& text) = 0;
    
    // Display information
    virtual int16_t getWidth() = 0;
    virtual int16_t getHeight() = 0;
    virtual String getDisplayType() = 0;
};