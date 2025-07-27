// /*
//  * Complete SCD41 Test Program
//  * For the real Sensirion I2C SCD4x library v1.1.0
//  * 
//  * Use this code in src/main.cpp to test your setup
//  */

// #include <Arduino.h>
// #include <Wire.h>
// #include <SensirionI2cScd4x.h>

// SensirionI2cScd4x scd4x_test;
// void scanI2CDevices();

// void setup() {
//     Serial.begin(9600);
//     delay(2000);
    
//     Serial.println("🐱 CoToMeter - SCD41 Connection Test");
//     Serial.println("=====================================");
//     Serial.println("Using: Sensirion I2C SCD4x Library v1.1.0");
//     Serial.println();
    
//     // Initialize I2C
//     Wire.begin(21, 22); // SDA=21, SCL=22
//     Wire.setClock(100000); // 100kHz
    
//     Serial.println("📡 I2C initialized on pins SDA=21, SCL=22");
    
//     // Scan for I2C devices
//     Serial.println("\n🔍 Scanning for I2C devices...");
//     scanI2CDevices();
    
//     // Initialize SCD41
//     Serial.println("\n🔧 Initializing SCD41 sensor...");
//     scd4x_test.begin(Wire, SCD41_I2C_ADDR_62);
    
//     // Stop any running measurement
//     int16_t error = scd4x_test.stopPeriodicMeasurement();
//     if (error) {
//         Serial.printf("⚠️  Warning stopping measurement: %d\n", error);
//     }
//     delay(1000);
    
//     // Get sensor serial number
//     uint64_t serialNumber;
//     error = scd4x_test.getSerialNumber(serialNumber);
    
//     if (error) {
//         Serial.println("❌ ERROR: Cannot communicate with SCD41!");
//         Serial.printf("💡 Error code: %d\n", error);
//         Serial.println("🔧 Check your connections:");
//         Serial.println("   - VDD to 3.3V");
//         Serial.println("   - GND to GND");
//         Serial.println("   - SDA to GPIO 21");
//         Serial.println("   - SCL to GPIO 22");
//         Serial.println("   - SEL to GND (for 0x62 address)");
//         return;
//     }
    
//     Serial.println("✅ SCD41 sensor found!");
//     Serial.printf("📟 Serial Number: 0x%016llX\n", serialNumber);
    
//     // Check sensor variant
//     uint16_t variant;
//     error = scd4x_test.getSensorVariantRaw(variant);
//     if (error == 0) {
//         Serial.printf("🔍 Sensor variant raw: 0x%04X\n", variant);
//         switch (variant & SCD4X_SENSOR_VARIANT_MASK) {
//             case SCD4X_SENSOR_VARIANT_SCD40:
//                 Serial.println("📡 Detected: SCD40");
//                 break;
//             case SCD4X_SENSOR_VARIANT_SCD41:
//                 Serial.println("📡 Detected: SCD41");
//                 break;
//             case SCD4X_SENSOR_VARIANT_SCD42:
//                 Serial.println("📡 Detected: SCD42");
//                 break;
//             case SCD4X_SENSOR_VARIANT_SCD43:
//                 Serial.println("📡 Detected: SCD43");
//                 break;
//             default:
//                 Serial.printf("📡 Detected: Unknown variant (0x%04X)\n", variant);
//                 break;
//         }
//     }
    
//     // Perform self-test
//     Serial.println("\n🧪 Performing self-test...");
//     uint16_t sensorStatus;
//     error = scd4x_test.performSelfTest(sensorStatus);
//     if (error) {
//         Serial.printf("❌ Self-test command failed: %d\n", error);
//     } else if (sensorStatus == 0) {
//         Serial.println("✅ Self-test passed - sensor is functioning correctly");
//     } else {
//         Serial.printf("⚠️  Self-test warning - sensor status: 0x%04X\n", sensorStatus);
//     }
    
//     // Enable automatic self-calibration
//     error = scd4x_test.setAutomaticSelfCalibrationEnabled(1);
//     if (error) {
//         Serial.printf("⚠️  Warning: Could not enable auto-calibration: %d\n", error);
//     } else {
//         Serial.println("🔄 Automatic self-calibration enabled");
//     }
    
//     // Start measurement
//     error = scd4x_test.startPeriodicMeasurement();
//     if (error) {
//         Serial.println("❌ ERROR: Failed to start measurement");
//         Serial.printf("💡 Error code: %d\n", error);
//         return;
//     }
    
//     Serial.println("🔄 Measurement started successfully!");
//     Serial.println("⏳ Waiting for first reading (5 seconds)...\n");
// }

// void loop() {
//     // Check if data is ready
//     bool dataReady = false;
//     int16_t error = scd4x_test.getDataReadyStatus(dataReady);
    
//     if (error) {
//         Serial.printf("❌ Error checking data ready flag: %d\n", error);
//         delay(5000);
//         return;
//     }
    
//     if (!dataReady) {
//         Serial.print(".");
//         delay(1000);
//         return;
//     }
    
//     // Read measurement
//     uint16_t co2;
//     float temperature;
//     float humidity;
    
//     error = scd4x_test.readMeasurement(co2, temperature, humidity);
//     if (error) {
//         Serial.printf("❌ Error reading measurement: %d\n", error);
//         delay(5000);
//         return;
//     }
    
//     // Display results with cat-themed output
//     Serial.println("\n╔═══════════════════════════════╗");
//     Serial.println("║     🐱 SCD41 READINGS 🐱     ║");
//     Serial.println("╠═══════════════════════════════╣");
//     Serial.printf("║ 🌬️  CO2:         %4d ppm   ║\n", co2);
//     Serial.printf("║ 🌡️  Temperature: %5.1f°C   ║\n", temperature);
//     Serial.printf("║ 💧  Humidity:    %5.1f%%    ║\n", humidity);
    
//     // Air quality assessment with cat moods
//     String quality;
//     String catMood;
//     String recommendation;
    
//     if (co2 <= 600) {
//         quality = "Excellent";
//         catMood = "😸 Purr-fect!";
//         recommendation = "Keep up the good work!";
//     } else if (co2 <= 800) {
//         quality = "Good";
//         catMood = "😺 Happy";
//         recommendation = "Air quality is good";
//     } else if (co2 <= 1200) {
//         quality = "Fair";
//         catMood = "😐 Neutral";
//         recommendation = "Consider some ventilation";
//     } else if (co2 <= 2000) {
//         quality = "Poor";
//         catMood = "😿 Concerned";
//         recommendation = "Open windows or improve airflow";
//     } else {
//         quality = "Bad";
//         catMood = "🙀 Stressed!";
//         recommendation = "URGENT: Need fresh air!";
//     }
    
//     Serial.println("╠═══════════════════════════════╣");
//     Serial.printf("║ 🏠 Air Quality: %-13s ║\n", quality.c_str());
//     Serial.printf("║ 🐱 Cat Mood: %-16s ║\n", catMood.c_str());
//     Serial.println("╠═══════════════════════════════╣");
    
//     // Temperature comfort assessment
//     String tempComfort;
//     if (temperature >= 20 && temperature <= 24) {
//         tempComfort = "😸 Comfortable";
//     } else if (temperature < 18) {
//         tempComfort = "🥶 Too cold";
//     } else if (temperature > 26) {
//         tempComfort = "🥵 Too hot";
//     } else {
//         tempComfort = "😐 OK";
//     }
    
//     // Humidity comfort assessment
//     String humidityComfort;
//     if (humidity >= 40 && humidity <= 60) {
//         humidityComfort = "😸 Optimal";
//     } else if (humidity < 30) {
//         humidityComfort = "🏜️ Too dry";
//     } else if (humidity > 70) {
//         humidityComfort = "🌊 Too humid";
//     } else {
//         humidityComfort = "😐 OK";
//     }
    
//     Serial.printf("║ 🌡️  Temp Comfort: %-11s ║\n", tempComfort.c_str());
//     Serial.printf("║ 💧  Humidity: %-15s ║\n", humidityComfort.c_str());
//     Serial.println("╚═══════════════════════════════╝");
    
//     Serial.printf("💡 Recommendation: %s\n", recommendation.c_str());
    
//     // Validate readings and show warnings
//     if (co2 < 400 || co2 > 5000) {
//         Serial.println("⚠️  WARNING: CO2 reading out of expected range!");
//     }
    
//     if (temperature < -10 || temperature > 60) {
//         Serial.println("⚠️  WARNING: Temperature reading unusual!");
//     }
    
//     if (humidity < 0 || humidity > 100) {
//         Serial.println("⚠️  WARNING: Humidity reading out of range!");
//     }
    
//     // Show uptime and memory usage
//     uint32_t uptime = millis() / 1000;
//     Serial.printf("⏰ Uptime: %02d:%02d:%02d | ", 
//                  (int)(uptime/3600), 
//                  (int)((uptime%3600)/60), 
//                  (int)(uptime%60));
//     Serial.printf("🧠 Free heap: %d bytes\n", ESP.getFreeHeap());
    
//     Serial.println();
    
//     delay(5000); // Wait 5 seconds before next reading
// }

// void scanI2CDevices() {
//     uint8_t deviceCount = 0;
    
//     for (uint8_t address = 1; address < 127; address++) {
//         Wire.beginTransmission(address);
//         uint8_t error = Wire.endTransmission();
        
//         if (error == 0) {
//             Serial.printf("✅ Device found at address 0x%02X", address);
            
//             // Identify common devices
//             switch (address) {
//                 case 0x62:
//                     Serial.println(" (SCD41 CO2 Sensor) 🎯");
//                     break;
//                 case 0x76:
//                 case 0x77:
//                     Serial.println(" (BME688 or similar sensor)");
//                     break;
//                 case 0x68:
//                     Serial.println(" (MPU6050 or RTC)");
//                     break;
//                 case 0x3C:
//                 case 0x3D:
//                     Serial.println(" (OLED Display)");
//                     break;
//                 default:
//                     Serial.println();
//                     break;
//             }
//             deviceCount++;
//         }
//     }
    
//     if (deviceCount == 0) {
//         Serial.println("❌ No I2C devices found!");
//         Serial.println("🔧 Check your wiring and power supply");
//     } else {
//         Serial.printf("📊 Total devices found: %d\n", deviceCount);
//     }
// }


#include <Arduino.h>
#include "CoToMeterController.h"

CoToMeterController controller;

void setup() {
    if (!controller.initialize()) {
        Serial.println("❌ Controller initialization failed!");
        while (1) delay(1000); // Halt on failure
    }
}

void loop() {
    controller.loop();
}


