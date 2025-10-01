#pragma once
#include "../interfaces/IDisplay.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

class SSD1351Display : public IDisplay {
private:
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 128;
    
    // Pin definitions
    static const int CS_PIN = 5;
    static const int DC_PIN = 16;
    static const int RST_PIN = 17;
    static const int SCLK_PIN = 18;
    static const int MOSI_PIN = 23;
    
    // Colors (RGB565)
    static const uint16_t BLACK = 0x0000;
    static const uint16_t WHITE = 0xFFFF;
    static const uint16_t RED = 0xF800;
    static const uint16_t GREEN = 0x07E0;
    static const uint16_t BLUE = 0x001F;
    static const uint16_t YELLOW = 0xFFE0;
    static const uint16_t CYAN = 0x07FF;
    static const uint16_t ORANGE = 0xFD20;
    
    Adafruit_SSD1351 display;
    
    // Helper methods
    uint16_t getAlertColor(AlertLevel level);
    uint16_t getCO2Color(float co2);
    uint16_t getVOCColor(float voc);
    void drawCatFace(AlertLevel level, int centerX = 64, int centerY = 35);
    
public:
    SSD1351Display();
    ~SSD1351Display() = default;
    
    // IDisplay interface
    bool initialize() override;
    void showSensorData(const SensorDataBase& data) override;
    void showMessage(const String& message) override;
    void showError(const String& error) override;
    
    // New method for combined sensor display
    void showCombinedSensorData(const CO2SensorData* co2Data, const VOCSensorData* vocData);
};