#include "display/SSD1351Display.h"

SSD1351Display::SSD1351Display() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN) {
}

bool SSD1351Display::initialize() {
    Serial.println("🖥️ Initializing SSD1351 OLED display...");
    
    // Initialize SPI
    SPI.begin(SCLK_PIN, -1, MOSI_PIN, CS_PIN);
    Serial.printf("📡 SPI initialized: SCK=%d, MOSI=%d, CS=%d\n", SCLK_PIN, MOSI_PIN, CS_PIN);
    
    // Initialize display - begin() returns void, so we just call it
    display.begin();
    
    // Test if display is working by clearing screen
    display.fillScreen(BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.println("CoToMeter");
    display.setCursor(30, 65);
    display.println("Starting...");
    
    Serial.println("✅ SSD1351 display ready");
    return true;  // Assume success since begin() doesn't return status
}

void SSD1351Display::showSensorData(const SensorDataBase& data) {
    display.fillScreen(BLACK);
    
    // Show sensor type and timestamp
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("CoToMeter - ");
    display.print(millis() / 1000);
    display.println("s");
    
    // Draw cat face based on alert level
    AlertLevel level = data.getAlertLevel();
    drawCatFace(level);
    
    // Show sensor readings
    display.setTextSize(1);
    display.setCursor(0, 70);
    
    // Handle different sensor types
    if (data.getType() == SensorType::CO2_TEMP_HUMIDITY) {
        const CO2SensorData* co2Data = static_cast<const CO2SensorData*>(&data);
        
        display.setTextColor(CYAN);
        display.printf("CO2: %.0f ppm\n", co2Data->co2);
        
        display.setTextColor(ORANGE);
        display.printf("Temp: %.1fC\n", co2Data->temperature);
        
        display.setTextColor(BLUE);
        display.printf("Hum: %.1f%%\n", co2Data->humidity);
        
    } else if (data.getType() == SensorType::VOC_GAS) {
        const VOCSensorData* vocData = static_cast<const VOCSensorData*>(&data);
        
        display.setTextColor(GREEN);
        display.printf("VOC: %.1f ppb\n", vocData->vocEstimate);
        
        display.setTextColor(ORANGE);
        display.printf("Temp: %.1fC\n", vocData->temperature);
        
        display.setTextColor(YELLOW);
        display.printf("Press:%.0fhPa\n", vocData->pressure / 100.0);
    }
    
    // Show alert status
    display.setTextColor(getAlertColor(level));
    display.setCursor(0, 115);
    switch (level) {
        case AlertLevel::NONE: display.print("Great!"); break;
        case AlertLevel::INFO: display.print("Good"); break;
        case AlertLevel::WARNING: display.print("Warning"); break;
        case AlertLevel::CRITICAL: display.print("Alert!"); break;
        default: display.print("Unknown"); break;
    }
}

void SSD1351Display::showMessage(const String& message) {
    display.fillScreen(BLACK);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Message:");
    display.setTextColor(CYAN);
    display.setCursor(0, 20);
    display.println(message);
}

void SSD1351Display::showError(const String& error) {
    display.fillScreen(BLACK);
    display.setTextColor(RED);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ERROR:");
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.println(error);
    
    // Draw sad cat
    display.drawCircle(64, 50, 20, RED);  // Head
    display.fillCircle(58, 45, 2, RED);   // Left eye
    display.fillCircle(70, 45, 2, RED);   // Right eye
    display.drawCircle(64, 52, 5, RED);   // Sad mouth
}

uint16_t SSD1351Display::getAlertColor(AlertLevel level) {
    switch (level) {
        case AlertLevel::NONE: return GREEN;
        case AlertLevel::INFO: return YELLOW;
        case AlertLevel::WARNING: return ORANGE;
        case AlertLevel::CRITICAL: return RED;
        default: return WHITE;
    }
}

void SSD1351Display::drawCatFace(AlertLevel level) {
    uint16_t color = getAlertColor(level);
    
    // Cat head (circle)
    display.drawCircle(64, 35, 15, color);
    
    // Ears (triangles)
    display.drawLine(54, 25, 59, 15, color);
    display.drawLine(59, 15, 64, 25, color);
    display.drawLine(64, 25, 69, 15, color);
    display.drawLine(69, 15, 74, 25, color);
    
    // Eyes
    if (level == AlertLevel::CRITICAL) {
        // X eyes for critical
        display.drawLine(58, 30, 62, 34, color);
        display.drawLine(62, 30, 58, 34, color);
        display.drawLine(66, 30, 70, 34, color);
        display.drawLine(70, 30, 66, 34, color);
    } else {
        // Normal eyes
        display.fillCircle(59, 32, 2, color);
        display.fillCircle(69, 32, 2, color);
    }
    
    // Mouth based on alert level
    if (level == AlertLevel::NONE) {
        // Happy mouth
        display.drawCircle(64, 35, 5, color);
        display.drawLine(60, 38, 68, 38, BLACK); // Cover top half
    } else if (level >= AlertLevel::WARNING) {
        // Sad mouth
        display.drawCircle(64, 42, 5, color);
        display.drawLine(60, 40, 68, 40, BLACK); // Cover bottom half
    } else {
        // Neutral mouth
        display.drawLine(60, 38, 68, 38, color);
    }
}
