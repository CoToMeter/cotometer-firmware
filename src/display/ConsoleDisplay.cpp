#include "display/ConsoleDisplay.h"

bool ConsoleDisplay::initialize() {
    Serial.println("🖥️ Initializing Console Display...");
    printCatHeader();
    Serial.println("✅ Console Display initialized");
    return true;
}

void ConsoleDisplay::showSensorData(const SensorDataBase& data) {
    // Show uptime
    uint32_t uptime = millis() / 1000;
    Serial.printf("⏰ Uptime: %02d:%02d:%02d\n", 
                 (int)(uptime/3600), 
                 (int)((uptime%3600)/60), 
                 (int)(uptime%60));
    
    // Use the sensor's toString method
    Serial.println("📊 " + data.toString());
    
    // Show alert level and cat mood
    AlertLevel level = data.getAlertLevel();
    Serial.println("🐱 Cat Mood: " + getCatMood(level));
    
    Serial.println();
}

void ConsoleDisplay::showMessage(const String& message) {
    Serial.println("📢 " + message);
}

void ConsoleDisplay::showError(const String& error) {
    Serial.println("❌ ERROR: " + error);
}

void ConsoleDisplay::printCatHeader() {
    Serial.println("╔════════════════════════════════╗");
    Serial.println("║        🐱 COTOMETER 🐱         ║");
    Serial.println("║     Cat Air Quality Monitor    ║");
    Serial.println("╚════════════════════════════════╝");
}

void ConsoleDisplay::printSeparator() {
    Serial.println("╠════════════════════════════════╣");
}

String ConsoleDisplay::getCatMood(AlertLevel level) {
    switch (level) {
        case AlertLevel::NONE: return "😸 Very Happy!";
        case AlertLevel::INFO: return "😺 Content";
        case AlertLevel::WARNING: return "😿 Concerned";
        case AlertLevel::CRITICAL: return "🙀 Stressed!";
        default: return "😐 Neutral";
    }
}