/*
 * test_compilation.cpp
 * Simple compilation test for historical data implementation
 */

#include <Arduino.h>
#include "communication/BluetoothComm.h"
#include "storage/HistoricalDataStorage.h"
#include "types/TimeSync.h"
#include "types/SensorData.h"
#include "CoToMeterController.h"

// Test that all headers compile correctly
void test_compilation() {
    // Test time sync
    TimeSync timeSync;
    timeSync.synchronizeTime(1695123456789UL, "+0300");
    
    // Test storage
    auto storage = StorageFactory::createStorage();
    storage->initialize();
    
    // Test sensor data
    CO2SensorData co2Data("TEST");
    VOCSensorData vocData("TEST");
    
    // Test communication
    BluetoothComm bluetooth;
    bluetooth.initialize();
    bluetooth.enableHistoricalData();
    bluetooth.storeCurrentReading(&co2Data, &vocData, nullptr);
    
    // Test controller
    CoToMeterController controller;
    controller.initialize();
}

void setup() {
    Serial.begin(115200);
    Serial.println("✅ All headers compile successfully!");
}

void loop() {
    delay(1000);
}
