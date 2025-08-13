#include <Arduino.h>
#include "CoToMeterController.h"

CoToMeterController controller;

void setup() {
    Serial.begin(115200);
    delay(2000); // Give time for Serial to initialize
    
    Serial.println();
    Serial.println("╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                🐱 COTOMETER v2.0 🐱                   ║");
    Serial.println("║              Dual Sensor Air Monitor                  ║");
    Serial.println("║                                                       ║");
    Serial.println("║  Sensors: SCD41 (I2C) + BME688 (SPI)                ║");
    Serial.println("║  Measures: CO2, VOC, Temperature, Humidity, Pressure  ║");
    Serial.println("╚═══════════════════════════════════════════════════════╝");
    Serial.println();
    
    if (!controller.initialize()) {
        Serial.println("❌ Controller initialization failed!");
        Serial.println("💡 Check your sensor connections:");
        Serial.println("   SCD41:  SDA=21, SCL=22, VDD=3.3V, GND=GND");
        Serial.println("   BME688: SCK=18, MISO=19, MOSI=23, CS=4, VDD=3.3V, GND=GND");
        Serial.println("🔄 System will restart in 10 seconds...");
        delay(10000);
        ESP.restart();
    }
    
    Serial.println("🎉 System ready! Starting continuous monitoring...");
}

void loop() {
    controller.loop();
}

// Optional: Add system monitoring
void printSystemInfo() {
    Serial.println("\n📊 System Information:");
    Serial.printf("   Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("   Chip Revision: %d\n", ESP.getChipRevision());
    Serial.printf("   CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   Flash Size: %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("   Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.println();
}