#include "CoToMeterController.h"
#include "sensors/SCD41Sensor.h"
#include "sensors/BME688Sensor.h"
#include "display/SSD1351Display.h" 
#include <Wire.h>
#include <SPI.h>

CoToMeterController::CoToMeterController() 
    : lastMeasurement(0)
    , measurementInterval(10000) // 10 seconds
    , co2Data(nullptr)
    , vocData(nullptr)
{
}

bool CoToMeterController::initialize() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("🚀 CoToMeter v2.0 Starting...");
    Serial.println("🔧 Initializing dual sensor system with SSD1351 OLED...");
    
    // Initialize I2C for SCD41
    Wire.begin(21, 22); // SDA=21, SCL=22
    Wire.setClock(100000);
    Serial.println("📡 I2C initialized for SCD41 (SDA=21, SCL=22)");
    
    // Initialize SPI for BME688 AND SSD1351 Display (shared bus)
    SPI.begin(18, 19, 23, 4); // SCK=18, MISO=19, MOSI=23, CS=4 for BME688
    Serial.println("📡 SPI initialized for BME688 + SSD1351 Display");
    
    // Initialize SSD1351 OLED display FIRST
    display.reset(new SSD1351Display());
    if (!display->initialize()) {
        Serial.println("❌ SSD1351 display initialization failed");
        Serial.println("💡 Check OLED wiring: CS=5, DC=16, RST=17, SCK=18, MOSI=23");
        return false;
    }
    
    // Show startup message on OLED
    display->showMessage("🐱 CoToMeter\n\nInitializing\nsensors...");
    
    // Create and initialize SCD41 sensor (I2C)
    Serial.println("\n🌬️ Initializing SCD41 CO2 sensor via I2C...");
    auto scd41Sensor = std::unique_ptr<ISensor>(new SCD41Sensor());
    if (!scd41Sensor->initialize()) {
        Serial.println("❌ SCD41 initialization failed: " + scd41Sensor->getLastError());
        display->showError("SCD41 Failed\n" + scd41Sensor->getLastError());
        return false;
    }
    sensors.push_back(std::move(scd41Sensor));
    Serial.println("✅ SCD41 sensor initialized successfully");
    
    // Update display
    display->showMessage("SCD41 ✅\n\nInitializing\nBME688...");
    delay(1000);
    
    // Create and initialize BME688 sensor (SPI)
    Serial.println("\n🌡️ Initializing BME688 VOC sensor via SPI...");
    auto bme688Sensor = std::unique_ptr<ISensor>(new BME688Sensor(0x76, 4)); // CS=4
    if (!bme688Sensor->initialize()) {
        Serial.println("❌ BME688 initialization failed: " + bme688Sensor->getLastError());
        display->showError("BME688 Failed\n" + bme688Sensor->getLastError());
        return false;
    }
    sensors.push_back(std::move(bme688Sensor));
    Serial.println("✅ BME688 sensor initialized successfully");
    
    // Show success on OLED
    display->showMessage("🐱 CoToMeter\n\nSCD41 ✅\nBME688 ✅\n\nStarting...");
    
    Serial.println("\n✅ CoToMeter initialized with SSD1351 OLED!");
    Serial.println("📊 Starting measurements in 3 seconds...");
    delay(3000);
    
    return true;
}

void CoToMeterController::loop() {
    uint32_t currentTime = millis();
    
    // Take periodic measurements
    if (currentTime - lastMeasurement >= measurementInterval) {
        Serial.println("\n" + String("=").substring(0, 50));
        Serial.println("📊 Taking measurements from both sensors...");
        
        bool hasNewData = false;
        
        // Read from all sensors
        for (auto& sensor : sensors) {
            if (sensor->readData()) {
                SensorDataBase* data = sensor->getCurrentData();
                
                // Store data by type
                if (data->getType() == SensorType::CO2_TEMP_HUMIDITY) {
                    co2Data = static_cast<CO2SensorData*>(data);
                    Serial.println("✅ SCD41 data updated");
                    hasNewData = true;
                }
                else if (data->getType() == SensorType::VOC_GAS) {
                    vocData = static_cast<VOCSensorData*>(data);
                    Serial.println("✅ BME688 data updated");
                    hasNewData = true;
                }
            } else {
                Serial.println("⚠️ Failed to read from sensor: " + sensor->getLastError());
            }
        }
        
        // Display combined data and run analysis
        if (hasNewData) {
            printCombinedData();
            checkAlerts();
            
            // Show combined data on OLED using the new method
            SSD1351Display* oledDisplay = static_cast<SSD1351Display*>(display.get());
            if (oledDisplay) {
                oledDisplay->showCombinedSensorData(
                    (co2Data && co2Data->isValid()) ? co2Data : nullptr,
                    (vocData && vocData->isValid()) ? vocData : nullptr
                );
            }
        } else {
            display->showError("No sensor data\navailable");
        }
        
        lastMeasurement = currentTime;
    }
    
    // Quick update cycle for display refresh
    delay(100);
}

void CoToMeterController::printCombinedData() {
    Serial.println("\n╔═══════════════════════════════════════════════════════╗");
    Serial.println("║                🐱 COTOMETER READINGS 🐱               ║");
    Serial.println("╠═══════════════════════════════════════════════════════╣");
    
    // CO2 Data from SCD41
    if (co2Data && co2Data->isValid()) {
        Serial.printf("║ 🌬️  CO2:         %4.0f ppm                        ║\n", co2Data->co2);
        Serial.printf("║ 🌡️  Temperature: %5.1f°C (SCD41)                ║\n", co2Data->temperature);
        Serial.printf("║ 💧  Humidity:    %5.1f%% (SCD41)                 ║\n", co2Data->humidity);
        Serial.println("╠═══════════════════════════════════════════════════════╣");
    } else {
        Serial.println("║ 🌬️  CO2:         No data available                  ║");
        Serial.println("╠═══════════════════════════════════════════════════════╣");
    }
    
    // VOC Data from BME688
    if (vocData && vocData->isValid()) {
        Serial.printf("║ 🧪  VOC Estimate: %5.1f ppb                        ║\n", vocData->vocEstimate);
        Serial.printf("║ 🔥  Gas Resist:   %8.0f Ω                       ║\n", vocData->gasResistance);
        Serial.printf("║ 🌡️  Temperature: %5.1f°C (BME688)               ║\n", vocData->temperature);
        Serial.printf("║ 💧  Humidity:    %5.1f%% (BME688)                ║\n", vocData->humidity);
        Serial.printf("║ 🌪️  Pressure:    %6.1f hPa                       ║\n", vocData->pressure / 100.0);
        Serial.printf("║ 🔥  Heater:      %s                              ║\n", 
                     vocData->heaterStable ? "Stable  " : "Unstable");
        Serial.println("╠═══════════════════════════════════════════════════════╣");
    } else {
        Serial.println("║ 🧪  VOC:          No data available                  ║");
        Serial.println("╠═══════════════════════════════════════════════════════╣");
    }
    
    // Temperature comparison
    if (co2Data && co2Data->isValid() && vocData && vocData->isValid()) {
        float tempDiff = abs(co2Data->temperature - vocData->temperature);
        Serial.printf("║ 📊  Temp Diff:   %5.1f°C between sensors         ║\n", tempDiff);
        if (tempDiff > 2.0) {
            Serial.println("║ ⚠️   Warning: Large temperature difference!        ║");
        }
        
        float humDiff = abs(co2Data->humidity - vocData->humidity);
        Serial.printf("║ 📊  Humidity Diff: %5.1f%% between sensors        ║\n", humDiff);
        if (humDiff > 5.0) {
            Serial.println("║ ⚠️   Warning: Large humidity difference!           ║");
        }
        Serial.println("╠═══════════════════════════════════════════════════════╣");
    }
    
    // Combined assessment
    String catMood = getCombinedCatMood();
    Serial.printf("║ 🐱  Cat Mood:    %-28s  ║\n", catMood.c_str());
    
    // Air quality recommendations
    String recommendation = "Monitor levels";
    if (co2Data && co2Data->isValid()) {
        if (co2Data->co2 > 1500) {
            recommendation = "URGENT: Ventilate now!";
        } else if (co2Data->co2 > 1000) {
            recommendation = "Open windows for fresh air";
        } else if (co2Data->co2 < 600) {
            recommendation = "Excellent air quality!";
        }
    }
    if (vocData && vocData->isValid() && vocData->vocEstimate > 200) {
        recommendation = "Check for VOC sources!";
    }
    Serial.printf("║ 💡  Advice:      %-28s  ║\n", recommendation.c_str());
    
    // System status
    uint32_t uptime = millis() / 1000;
    Serial.printf("║ ⏰  Uptime:      %02d:%02d:%02d                        ║\n", 
                 (int)(uptime/3600), (int)((uptime%3600)/60), (int)(uptime%60));
    Serial.printf("║ 🧠  Free Memory: %d bytes                        ║\n", ESP.getFreeHeap());
    
    Serial.println("╚═══════════════════════════════════════════════════════╝");
}

void CoToMeterController::checkAlerts() {
    std::vector<String> alerts;
    
    // Check CO2 levels
    if (co2Data && co2Data->isValid()) {
        if (co2Data->co2 > 2000) {
            alerts.push_back("🚨 CRITICAL: CO2 extremely high (" + String(co2Data->co2, 0) + " ppm)");
        } else if (co2Data->co2 > 1200) {
            alerts.push_back("⚠️ WARNING: CO2 elevated (" + String(co2Data->co2, 0) + " ppm)");
        }
    }
    
    // Check VOC levels
    if (vocData && vocData->isValid()) {
        if (vocData->vocEstimate > 200) {
            alerts.push_back("🚨 CRITICAL: VOC levels high (" + String(vocData->vocEstimate, 1) + " ppb)");
        } else if (vocData->vocEstimate > 100) {
            alerts.push_back("⚠️ WARNING: VOC detected (" + String(vocData->vocEstimate, 1) + " ppb)");
        }
    }
    
    // Check temperature comfort (using both sensors)
    if (co2Data && co2Data->isValid()) {
        if (co2Data->temperature < 18) {
            alerts.push_back("🥶 INFO: Temperature too cold (SCD41: " + String(co2Data->temperature, 1) + "°C)");
        } else if (co2Data->temperature > 26) {
            alerts.push_back("🥵 INFO: Temperature too hot (SCD41: " + String(co2Data->temperature, 1) + "°C)");
        }
    }
    
    // Check sensor agreement
    if (co2Data && co2Data->isValid() && vocData && vocData->isValid()) {
        float tempDiff = abs(co2Data->temperature - vocData->temperature);
        float humDiff = abs(co2Data->humidity - vocData->humidity);
        
        if (tempDiff > 3.0) {
            alerts.push_back("⚠️ WARNING: Large temperature difference between sensors (" + String(tempDiff, 1) + "°C)");
        }
        if (humDiff > 10.0) {
            alerts.push_back("⚠️ WARNING: Large humidity difference between sensors (" + String(humDiff, 1) + "%)");
        }
    }
    
    // Check BME688 heater stability
    if (vocData && vocData->isValid() && !vocData->heaterStable) {
        alerts.push_back("ℹ️ INFO: BME688 gas heater warming up - VOC readings may be inaccurate");
    }
    
    // Display alerts on Serial and OLED if critical
    if (!alerts.empty()) {
        Serial.println("\n🚨 ALERTS:");
        for (const String& alert : alerts) {
            Serial.println("   " + alert);
        }
        Serial.println();
        
        // Show critical alerts on OLED
        bool hasCritical = false;
        for (const String& alert : alerts) {
            if (alert.indexOf("CRITICAL") >= 0) {
                hasCritical = true;
                break;
            }
        }
        
        if (hasCritical) {
            display->showError("CRITICAL ALERT!\nCheck levels");
            delay(2000); // Show alert for 2 seconds
        }
    }
}

String CoToMeterController::getCombinedCatMood() {
    AlertLevel maxAlert = AlertLevel::NONE;
    
    // Get highest alert level from both sensors
    if (co2Data && co2Data->isValid()) {
        AlertLevel co2Alert = co2Data->getAlertLevel();
        if (co2Alert > maxAlert) maxAlert = co2Alert;
    }
    
    if (vocData && vocData->isValid()) {
        AlertLevel vocAlert = vocData->getAlertLevel();
        if (vocAlert > maxAlert) maxAlert = vocAlert;
    }
    
    // Return cat mood based on highest alert
    switch (maxAlert) {
        case AlertLevel::NONE: return "😸 Very Happy!";
        case AlertLevel::INFO: return "😺 Content";
        case AlertLevel::WARNING: return "😿 Concerned";
        case AlertLevel::CRITICAL: return "🙀 Stressed!";
        default: return "😐 Neutral";
    }
}