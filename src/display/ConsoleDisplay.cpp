#include "display/ConsoleDisplay.h"

bool ConsoleDisplay::initialize() {
    Serial.println("🖥️ Initializing Console Display...");
    printCatHeader();
    Serial.println("✅ Console Display initialized");
    return true;
}

void ConsoleDisplay::showSensorData(const SensorData& data) {
    // printSeparator();
    // Serial.println("     🐱 COTOMETER READINGS 🐱     ");
    // printSeparator();
    
    // Show uptime
    uint32_t uptime = millis() / 1000;
    Serial.printf("⏰ Uptime: %02d:%02d:%02d\n", 
                 (int)(uptime/3600), 
                 (int)((uptime%3600)/60), 
                 (int)(uptime%60));
    
    // Serial.println();
    
    // CO2 with status
    Serial.printf("🌬️  CO2:         %4.0f ppm ", data.co2);
    AlertLevel level = data.getAlertLevel();
    Serial.print(getCatMood(level));
    
    // Temperature
    Serial.printf("🌡️  Temperature: %4.1f°C", data.temperature);
    if (data.temperature >= 20 && data.temperature <= 24) {
        Serial.printf(" 😸 (Comfortable)");
    } else if (data.temperature < 18) {
        Serial.printf(" 🥶 (Too cold)");
    } else if (data.temperature > 26) {
        Serial.printf(" 🥵 (Too hot)");
    } else {
        Serial.printf(" 😐 (OK)");
    }
    
    // Humidity
    Serial.printf(" 💧  Humidity:    %4.1f%% ", data.humidity);
    if (data.humidity >= 40 && data.humidity <= 60) {
        Serial.printf("😸 (Optimal)");
    } else if (data.humidity < 30) {
        Serial.printf("🏜️ (Too dry)");
    } else if (data.humidity > 70) {
        Serial.printf("🌊 (Too humid)");
    } else {
        Serial.printf("😐 (OK)");
    }
    
    // Serial.println();
    Serial.print("🏠 Air Quality: " + data.getAirQualityText());
    Serial.print("🐱 Cat Mood: " + getCatMood(level));
    
    // printSeparator();
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
        case AlertLevel::GOOD: return "😸 Very Happy!";
        case AlertLevel::FAIR: return "😺 Content";
        case AlertLevel::POOR: return "😿 Concerned";
        case AlertLevel::BAD: return "🙀 Stressed!";
        default: return "😐 Neutral";
    }
}
