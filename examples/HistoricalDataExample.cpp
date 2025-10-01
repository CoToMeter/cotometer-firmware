/*
 * examples/HistoricalDataExample.cpp
 * Example demonstrating historical data functionality with time synchronization
 * 
 * This example shows how to:
 * 1. Set up Bluetooth communication with time synchronization
 * 2. Store sensor readings with timestamps
 * 3. Query historical data by time range
 * 4. Handle the complete communication protocol
 */

#include <Arduino.h>
#include "communication/BluetoothComm.h"
#include "storage/HistoricalDataStorage.h"
#include "types/SensorData.h"
#include "types/TimeSync.h"

// Global objects
BluetoothComm bluetoothComm;
std::unique_ptr<HistoricalDataStorage> storage;

// Test sensor data
CO2SensorData co2TestData("SCD41_TEST");
VOCSensorData vocTestData("BME688_TEST");

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("🚀 CoToMeter Historical Data Example");
    Serial.println("=====================================\n");
    
    // Initialize Bluetooth communication
    Serial.println("📡 Initializing Bluetooth...");
    if (!bluetoothComm.initialize()) {
        Serial.println("❌ Failed to initialize Bluetooth");
        return;
    }
    
    // Enable historical data storage
    Serial.println("💾 Enabling historical data storage...");
    if (!bluetoothComm.enableHistoricalData(1000)) { // 1000 records for demo
        Serial.println("❌ Failed to enable historical data");
        return;
    }
    
    Serial.println("✅ Setup complete!\n");
    Serial.println("📱 Connect your Android app to test the protocol");
    Serial.println("⏰ The app will sync time and you can request historical data\n");
    
    // Generate some test data
    generateTestData();
}

void loop() {
    // Handle incoming Bluetooth commands
    bluetoothComm.update();
    
    // Simulate sensor readings every 10 seconds
    static uint32_t lastReading = 0;
    uint32_t now = millis();
    
    if (now - lastReading >= 10000) {
        // Generate fake sensor data
        updateTestSensorData();
        
        // Store the reading
        bluetoothComm.storeCurrentReading(&co2TestData, &vocTestData);
        
        // Send real-time data if connected and streaming
        if (bluetoothComm.isConnected() && bluetoothComm.isStreaming()) {
            bluetoothComm.sendSensorData(co2TestData);
            bluetoothComm.sendSensorData(vocTestData);
        }
        
        // Print status
        Serial.printf("📊 Reading stored: CO2=%.1f ppm, Temp=%.1f°C, VOC=%.1f ppb\n",
                     co2TestData.co2, co2TestData.temperature, vocTestData.vocEstimate);
        Serial.println(bluetoothComm.getConnectionStats());
        
        lastReading = now;
    }
    
    delay(100);
}

void generateTestData() {
    Serial.println("🧪 Generating test historical data...");
    
    // Generate 50 historical readings with 30-second intervals
    for (int i = 0; i < 50; i++) {
        uint32_t test_uptime = millis() - (50 - i) * 30000; // 30 seconds apart
        
        // Create test readings with variation
        co2TestData.uptime = test_uptime;
        co2TestData.co2 = 400 + (i * 10) + random(-50, 50);
        co2TestData.temperature = 22.0 + random(-30, 30) / 10.0;
        co2TestData.humidity = 45.0 + random(-100, 100) / 10.0;
        co2TestData.updateTimestamp();
        co2TestData.setValid(true);
        
        vocTestData.uptime = test_uptime;
        vocTestData.temperature = co2TestData.temperature;
        vocTestData.humidity = co2TestData.humidity;
        vocTestData.pressure = 101325 + random(-1000, 1000);
        vocTestData.vocEstimate = 50 + random(-30, 80);
        vocTestData.gasValid = true;
        vocTestData.heaterStable = true;
        vocTestData.updateTimestamp();
        vocTestData.setValid(true);
        
        // Store the test reading
        bluetoothComm.storeCurrentReading(&co2TestData, &vocTestData);
        
        // Small delay to prevent watchdog timeout
        if (i % 10 == 0) {
            delay(10);
        }
    }
    
    Serial.printf("✅ Generated 50 test readings\n\n");
}

void updateTestSensorData() {
    uint32_t now = millis();
    
    // Update CO2 data with realistic values
    co2TestData.uptime = now;
    co2TestData.co2 = 400 + sin(now / 60000.0) * 200 + random(-20, 20); // Slow variation
    co2TestData.temperature = 22.0 + sin(now / 120000.0) * 3 + random(-5, 5) / 10.0;
    co2TestData.humidity = 45.0 + sin(now / 180000.0) * 10 + random(-20, 20) / 10.0;
    co2TestData.updateTimestamp();
    co2TestData.setValid(true);
    
    // Update VOC data
    vocTestData.uptime = now;
    vocTestData.temperature = co2TestData.temperature + random(-2, 2) / 10.0;
    vocTestData.humidity = co2TestData.humidity + random(-3, 3) / 10.0;
    vocTestData.pressure = 101325 + sin(now / 300000.0) * 500 + random(-100, 100);
    vocTestData.vocEstimate = 50 + sin(now / 90000.0) * 30 + random(-10, 10);
    vocTestData.gasValid = true;
    vocTestData.heaterStable = true;
    vocTestData.updateTimestamp();
    vocTestData.setValid(true);
}

/*
 * EXAMPLE ANDROID APP COMMANDS
 * =============================
 * 
 * Send these JSON commands via Bluetooth to test the protocol:
 * 
 * 1. Check time sync status:
 * {"type": "time_sync_request", "request_id": "test_001"}
 * 
 * 2. Set current time (replace with actual timestamp):
 * {"type": "time_sync_set", "request_id": "test_002", "current_time": 1695123456789, "timezone_offset": "+0300"}
 * 
 * 3. Request storage info:
 * {"type": "storage_info_request", "request_id": "test_003"}
 * 
 * 4. Request historical data (last hour):
 * {"type": "history_request", "request_id": "test_004", "start_time": 1695120000000, "end_time": 1695123600000, "max_points": 100}
 * 
 * 5. Control real-time streaming:
 * {"type": "realtime_control", "action": "start", "interval_ms": 10000}
 * {"type": "realtime_control", "action": "stop"}
 * 
 * EXPECTED RESPONSES
 * ==================
 * 
 * Time sync status response:
 * {
 *   "type": "time_sync_status",
 *   "request_id": "test_001",
 *   "has_time": false,
 *   "current_uptime": 123456,
 *   "last_sync_uptime": 0,
 *   "time_offset": 0
 * }
 * 
 * Time sync acknowledgment:
 * {
 *   "type": "time_sync_ack",
 *   "request_id": "test_002",
 *   "success": true,
 *   "sync_uptime": 123456,
 *   "calculated_offset": 1695000000333,
 *   "message": "Time synchronized successfully"
 * }
 * 
 * Storage info response:
 * {
 *   "type": "storage_info",
 *   "request_id": "test_003",
 *   "storage_type": "flash",
 *   "total_capacity_mb": 3.5,
 *   "used_capacity_mb": 0.1,
 *   "free_capacity_mb": 3.4,
 *   "total_records": 50,
 *   "oldest_record_time": 1695120000000,
 *   "newest_record_time": 1695123456000,
 *   "estimated_days_remaining": 4.2
 * }
 * 
 * Historical data response:
 * {
 *   "type": "history_response",
 *   "request_id": "test_004",
 *   "total_points": 50,
 *   "chunk_index": 0,
 *   "total_chunks": 1,
 *   "data": [
 *     {
 *       "timestamp": 1695120001000,
 *       "uptime": 120001,
 *       "readings": {
 *         "co2": {"value": 450.2, "unit": "ppm", "status": "valid"},
 *         "temperature": {"value": 23.5, "unit": "°C", "status": "valid"},
 *         "humidity": {"value": 45.2, "unit": "%", "status": "valid"}
 *       }
 *     }
 *   ]
 * }
 */
