#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

SCD30 airSensor;

// Pin definitions
#define SDA_PIN 21
#define SCL_PIN 22
#define READY_PIN 19  // Optional: SCD30 data ready pin

// Measurement intervals
#define MEASUREMENT_INTERVAL 2  // seconds
#define SERIAL_BAUD 9600

// Variables for data tracking
unsigned long lastMeasurement = 0;
bool sensorInitialized = false;

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(1000);  // Give serial time to initialize
    
    Serial.println();
    Serial.println("🐱==================================🐱");
    Serial.println("      CoToMeter Starting Up!        ");
    Serial.println("🐱==================================🐱");
    
    // Initialize I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    // Wire.setClock(50000);  // 50kHz for better reliability
    
    Serial.print("🔍 Searching for SCD30 sensor...");
    
    // Try to initialize SCD30
    if (airSensor.begin() == false) {
        Serial.println(" ❌ FAILED");
        Serial.println("💀 SCD30 not detected!");
        Serial.println("🔧 Check wiring:");
        Serial.println("   VDD -> 3.3V or 5V");
        Serial.println("   GND -> GND");
        Serial.println("   SCL -> GPIO22");
        Serial.println("   SDA -> GPIO21");
        
        while(1) {
            delay(1000);
            Serial.print(".");
        }
    }
    
    Serial.println(" ✅ SUCCESS");
    Serial.println("🎉 SCD30 sensor connected!");
    
    // Configure sensor settings
    Serial.println("⚙️  Configuring sensor...");
    
    // Enable automatic self calibration
    if (airSensor.setAutoSelfCalibration(true)) {
        Serial.println("✅ Auto self-calibration: ENABLED");
    } else {
        Serial.println("❌ Auto self-calibration: FAILED");
    }
    
    // Set measurement interval
    if (airSensor.setMeasurementInterval(MEASUREMENT_INTERVAL)) {
        Serial.printf("✅ Measurement interval: %d seconds\n", MEASUREMENT_INTERVAL);
    } else {
        Serial.println("❌ Setting measurement interval: FAILED");
    }
    
    // Set altitude compensation (Lviv is ~296m above sea level)
    if (airSensor.setAltitudeCompensation(296)) {
        Serial.println("✅ Altitude compensation: 296m (Lviv)");
    } else {
        Serial.println("❌ Altitude compensation: FAILED");
    }
    
    // Get sensor info
    Serial.println("\n📋 Sensor Information:");
    // Serial.printf("   Firmware version: v%d.%d\n", 
    //               airSensor.getFirmwareVersion() >> 8, 
    //               airSensor.getFirmwareVersion() & 0xFF);
    
    Serial.println("\n🚀 CoToMeter ready to measure!");
    Serial.println("📊 Starting measurements...\n");
    
    sensorInitialized = true;
    lastMeasurement = millis();
}

void loop() {
    if (!sensorInitialized) {
        delay(1000);
        return;
    }
    
    // Check if new data is available
    if (airSensor.dataAvailable()) {
        // Read all sensor values
        float co2 = airSensor.getCO2();
        float temperature = airSensor.getTemperature();
        float humidity = airSensor.getHumidity();
        
        // Get current time
        unsigned long currentTime = millis();
        unsigned long uptime = currentTime / 1000;  // Convert to seconds
        
        // Print header with timestamp
        Serial.println("🐱===========================🐱");
        Serial.printf("⏰ Uptime: %02d:%02d:%02d\n", 
                     (int)(uptime/3600), 
                     (int)((uptime%3600)/60), 
                     (int)(uptime%60));
        Serial.println("📊 CoToMeter Readings:");
        
        // CO2 reading with status
        Serial.printf("🌬️  CO2: %.0f ppm", co2);
        if (co2 < 400) {
            Serial.println(" ⚠️ (Too low - check sensor)");
        } else if (co2 <= 600) {
            Serial.println(" 😸 (Excellent)");
        } else if (co2 <= 1000) {
            Serial.println(" 😺 (Good)");
        } else if (co2 <= 1500) {
            Serial.println(" 😿 (Poor - ventilate!)");
        } else if (co2 <= 2000) {
            Serial.println(" 🙀 (Bad - open windows!)");
        } else {
            Serial.println(" 💀 (Dangerous - immediate action!)");
        }
        
        // Temperature reading
        Serial.printf("🌡️  Temperature: %.1f°C", temperature);
        if (temperature >= 20 && temperature <= 24) {
            Serial.println(" 😸 (Comfortable)");
        } else if (temperature < 18) {
            Serial.println(" 🥶 (Too cold)");
        } else if (temperature > 26) {
            Serial.println(" 🥵 (Too hot)");
        } else {
            Serial.println(" 😐 (Acceptable)");
        }
        
        // Humidity reading
        Serial.printf("💧 Humidity: %.1f%%", humidity);
        if (humidity >= 40 && humidity <= 60) {
            Serial.println(" 😸 (Optimal)");
        } else if (humidity < 30) {
            Serial.println(" 🏜️ (Too dry)");
        } else if (humidity > 70) {
            Serial.println(" 🌊 (Too humid)");
        } else {
            Serial.println(" 😐 (Acceptable)");
        }
        
        // Cat mood based on overall air quality
        Serial.print("🐱 Cat Mood: ");
        if (co2 <= 600 && temperature >= 20 && temperature <= 24 && humidity >= 40 && humidity <= 60) {
            Serial.println("😸 Very Happy!");
        } else if (co2 <= 1000 && temperature >= 18 && temperature <= 26) {
            Serial.println("😺 Content");
        } else if (co2 <= 1500) {
            Serial.println("😿 Concerned");
        } else {
            Serial.println("🙀 Stressed!");
        }
        
        Serial.println("🐱===========================🐱\n");
        
        lastMeasurement = currentTime;
    }
    
    // Check if sensor is not responding
    if (millis() - lastMeasurement > 30000) {  // 30 seconds timeout
        Serial.println("⚠️ No data from sensor for 30 seconds...");
        Serial.println("🔄 Checking sensor status...");
        
        if (!airSensor.begin()) {
            Serial.println("❌ Sensor connection lost!");
            sensorInitialized = false;
        }
        
        lastMeasurement = millis();
    }
    
    // Small delay to prevent overwhelming the serial output
    delay(1000);
}

// Helper function to format uptime
String formatUptime(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    
    char buffer[20];
    sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, secs);
    return String(buffer);
}