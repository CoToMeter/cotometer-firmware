
#include <Arduino.h>
#include "CoToMeterController.h"

CoToMeterController controller;

void setup() {
    Serial.begin(115200);
    delay(2000); // Give time for Serial to initialize
    
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                🐱 COTOMETER v2.0 🐱                   ║");
    Serial.println("║              Dual Sensor + OLED Monitor               ║");
    Serial.println("║                                                       ║");
    Serial.println("║  Sensors: SCD41 (I2C) + BME688 (SPI)                ║");
    Serial.println("║  Display: SSD1351 OLED 128x128 (SPI)                 ║");
    Serial.println("║  Measures: CO2, VOC, Temperature, Humidity, Pressure  ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    if (!controller.initialize()) {
        Serial.println("❌ Controller initialization failed!");
        Serial.println("💡 Check your connections:");
        Serial.println("   SCD41:   SDA=21, SCL=22, VDD=3.3V, GND=GND");
        Serial.println("   BME688:  SCK=18, MISO=19, MOSI=23, CS=4, VDD=3.3V, GND=GND");
        Serial.println("   SSD1351: SCK=18, MOSI=23, CS=5, DC=16, RST=17, VDD=3.3V, GND=GND");
        Serial.println("🔄 System will restart in 10 seconds...");
        delay(10000);
        ESP.restart();
    }
    
    Serial.println("🎉 System ready! Starting continuous monitoring...");
    Serial.println("📱 OLED display will show real-time sensor data with cat mood!");
}

void loop() {
    controller.loop();
}
