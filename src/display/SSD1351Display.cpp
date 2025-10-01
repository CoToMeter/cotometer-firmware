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
    
    // Show header with timestamp
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("🐱 CoToMeter ");
    display.print(millis() / 1000);
    display.println("s");
    
    // Draw separator line
    display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);
    
    // Get alert level for cat mood
    AlertLevel level = data.getAlertLevel();
    
    // Draw cat face in top right corner
    drawCatFace(level, 95, 25);
    
    // Display readings based on sensor type
    display.setTextSize(1);
    int yPos = 15;
    
    if (data.getType() == SensorType::CO2_TEMP_HUMIDITY) {
        const CO2SensorData* co2Data = static_cast<const CO2SensorData*>(&data);
        
        // CO2 reading (prominent)
        display.setTextColor(getCO2Color(co2Data->co2));
        display.setTextSize(1);
        display.setCursor(0, yPos);
        display.printf("CO2: %4.0f ppm", co2Data->co2);
        yPos += 12;
        
        // SCD41 Temperature
        display.setTextColor(ORANGE);
        display.setTextSize(1);
        display.setCursor(0, yPos);
        display.printf("T1: %4.1fC (SCD41)", co2Data->temperature);
        yPos += 10;
        
        // SCD41 Humidity  
        display.setTextColor(CYAN);
        display.setCursor(0, yPos);
        display.printf("H1: %4.1f%% (SCD41)", co2Data->humidity);
        yPos += 12;
        
        // Placeholder for VOC (if we had combined data)
        display.setTextColor(YELLOW);
        display.setCursor(0, yPos);
        display.print("VOC: Waiting...");
        yPos += 10;
        display.setTextColor(ORANGE);
        display.setCursor(0, yPos);
        display.print("T2: Waiting...");
        yPos += 10;
        display.setTextColor(CYAN);
        display.setCursor(0, yPos);
        display.print("H2: Waiting...");
        
    } else if (data.getType() == SensorType::VOC_GAS) {
        const VOCSensorData* vocData = static_cast<const VOCSensorData*>(&data);
        
        // Placeholder for CO2
        display.setTextColor(WHITE);
        display.setCursor(0, yPos);
        display.print("CO2: Waiting...");
        yPos += 12;
        
        // BME688 Temperature
        display.setTextColor(ORANGE);
        display.setCursor(0, yPos);
        display.printf("T2: %4.1fC (BME688)", vocData->temperature);
        yPos += 10;
        
        // BME688 Humidity
        display.setTextColor(CYAN);
        display.setCursor(0, yPos);
        display.printf("H2: %4.1f%% (BME688)", vocData->humidity);
        yPos += 12;
        
        // VOC reading
        display.setTextColor(getVOCColor(vocData->vocEstimate));
        display.setCursor(0, yPos);
        display.printf("VOC: %5.1f ppb", vocData->vocEstimate);
        yPos += 10;
        
        // Pressure
        display.setTextColor(BLUE);
        display.setCursor(0, yPos);
        display.printf("P: %4.0fhPa", vocData->pressure / 100.0);
        yPos += 10;
        
        // Gas sensor status
        display.setTextColor(vocData->heaterStable ? GREEN : RED);
        display.setCursor(0, yPos);
        display.print(vocData->heaterStable ? "Gas: Stable" : "Gas: Warming");
    }
    
    // Show overall status at bottom
    display.setTextColor(getAlertColor(level));
    display.setCursor(0, 118);
    switch (level) {
        case AlertLevel::NONE: display.print("Air: Excellent"); break;
        case AlertLevel::INFO: display.print("Air: Good"); break;
        case AlertLevel::WARNING: display.print("Air: Poor"); break;
        case AlertLevel::CRITICAL: display.print("Air: Bad!"); break;
        default: display.print("Air: Unknown"); break;
    }
}

// New method to show combined data from both sensors
void SSD1351Display::showCombinedSensorData(const CO2SensorData* co2Data, const VOCSensorData* vocData) {
    display.fillScreen(BLACK);
    
    // Header
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("🐱 CoToMeter ");
    display.print(millis() / 1000);
    display.println("s");
    
    // Draw separator line
    display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);
    
    // Determine overall alert level
    AlertLevel maxLevel = AlertLevel::NONE;
    if (co2Data && co2Data->isValid()) {
        AlertLevel co2Level = co2Data->getAlertLevel();
        if (co2Level > maxLevel) maxLevel = co2Level;
    }
    if (vocData && vocData->isValid()) {
        AlertLevel vocLevel = vocData->getAlertLevel();
        if (vocLevel > maxLevel) maxLevel = vocLevel;
    }
    
    // Draw cat face
    drawCatFace(maxLevel, 95, 25);
    
    int yPos = 15;
    
    // CO2 from SCD41
    if (co2Data && co2Data->isValid()) {
        display.setTextColor(getCO2Color(co2Data->co2));
        display.setTextSize(1);
        display.setCursor(0, yPos);
        display.printf("CO2: %4.0f ppm", co2Data->co2);
    } else {
        display.setTextColor(RED);
        display.setCursor(0, yPos);
        display.print("CO2: No data");
    }
    yPos += 12;
    
    // VOC from BME688
    if (vocData && vocData->isValid()) {
        display.setTextColor(getVOCColor(vocData->vocEstimate));
        display.setCursor(0, yPos);
        display.printf("VOC: %5.1f ppb", vocData->vocEstimate);
    } else {
        display.setTextColor(RED);
        display.setCursor(0, yPos);
        display.print("VOC: No data");
    }
    yPos += 12;
    
    // Temperature comparison
    display.setTextColor(ORANGE);
    display.setCursor(0, yPos);
    if (co2Data && co2Data->isValid()) {
        display.printf("T1: %4.1fC", co2Data->temperature);
    } else {
        display.print("T1: --.-C");
    }
    yPos += 10;
    
    display.setCursor(0, yPos);
    if (vocData && vocData->isValid()) {
        display.printf("T2: %4.1fC", vocData->temperature);
    } else {
        display.print("T2: --.-C");
    }
    yPos += 12;
    
    // Humidity comparison
    display.setTextColor(CYAN);
    display.setCursor(0, yPos);
    if (co2Data && co2Data->isValid()) {
        display.printf("H1: %4.1f%%", co2Data->humidity);
    } else {
        display.print("H1: --.-%");
    }
    yPos += 10;
    
    display.setCursor(0, yPos);
    if (vocData && vocData->isValid()) {
        display.printf("H2: %4.1f%%", vocData->humidity);
    } else {
        display.print("H2: --.-%");
    }
    yPos += 12;
    
    // Pressure from BME688
    if (vocData && vocData->isValid()) {
        display.setTextColor(BLUE);
        display.setCursor(0, yPos);
        display.printf("P: %4.0fhPa", vocData->pressure / 100.0);
    }
    
    // Overall air quality at bottom
    display.setTextColor(getAlertColor(maxLevel));
    display.setCursor(0, 118);
    switch (maxLevel) {
        case AlertLevel::NONE: display.print("Air: Excellent"); break;
        case AlertLevel::INFO: display.print("Air: Good"); break;
        case AlertLevel::WARNING: display.print("Air: Poor"); break;
        case AlertLevel::CRITICAL: display.print("Air: Bad!"); break;
        default: display.print("Air: Unknown"); break;
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
    drawCatFace(AlertLevel::CRITICAL, 64, 50);
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

uint16_t SSD1351Display::getCO2Color(float co2) {
    if (co2 < 600) return GREEN;        // Excellent
    if (co2 < 800) return YELLOW;       // Good  
    if (co2 < 1200) return ORANGE;      // Fair
    return RED;                         // Poor
}

uint16_t SSD1351Display::getVOCColor(float voc) {
    if (voc < 50) return GREEN;         // Clean
    if (voc < 100) return YELLOW;       // Acceptable
    if (voc < 200) return ORANGE;       // Moderate
    return RED;                         // High
}

void SSD1351Display::drawCatFace(AlertLevel level, int centerX, int centerY) {
    uint16_t color = getAlertColor(level);
    int radius = 12;
    
    // Cat head (circle)
    display.drawCircle(centerX, centerY, radius, color);
    
    // Ears (triangles)
    display.drawLine(centerX - 8, centerY - 8, centerX - 3, centerY - 15, color);
    display.drawLine(centerX - 3, centerY - 15, centerX + 2, centerY - 8, color);
    display.drawLine(centerX + 2, centerY - 8, centerX + 8, centerY - 15, color);
    display.drawLine(centerX + 8, centerY - 15, centerX + 13, centerY - 8, color);
    
    // Eyes
    if (level == AlertLevel::CRITICAL) {
        // X eyes for critical
        display.drawLine(centerX - 4, centerY - 3, centerX - 1, centerY, color);
        display.drawLine(centerX - 1, centerY - 3, centerX - 4, centerY, color);
        display.drawLine(centerX + 1, centerY - 3, centerX + 4, centerY, color);
        display.drawLine(centerX + 4, centerY - 3, centerX + 1, centerY, color);
    } else {
        // Normal eyes
        display.fillCircle(centerX - 3, centerY - 2, 1, color);
        display.fillCircle(centerX + 3, centerY - 2, 1, color);
    }
    
    // Mouth based on alert level
    if (level == AlertLevel::NONE) {
        // Happy mouth (smile)
        display.drawCircle(centerX, centerY + 2, 4, color);
        display.drawLine(centerX - 3, centerY + 1, centerX + 3, centerY + 1, BLACK); // Cover top half
    } else if (level >= AlertLevel::WARNING) {
        // Sad mouth (frown)
        display.drawCircle(centerX, centerY + 7, 4, color);
        display.drawLine(centerX - 3, centerY + 6, centerX + 3, centerY + 6, BLACK); // Cover bottom half
    } else {
        // Neutral mouth
        display.drawLine(centerX - 3, centerY + 3, centerX + 3, centerY + 3, color);
    }
    
    // Whiskers
    display.drawLine(centerX - 15, centerY, centerX - 9, centerY, color);
    display.drawLine(centerX + 9, centerY, centerX + 15, centerY, color);
}