#pragma once
#include "../interfaces/ICommunication.h"
#include <BluetoothSerial.h>

class BLEComm : public ICommunication {
private:
    BluetoothSerial SerialBT;
    String deviceName;
    String lastError;
    bool initialized;
    bool advertising;
    
    DataCallback dataCallback;
    StatusCallback statusCallback;
    
    uint32_t bytesTransmitted;
    uint32_t bytesReceived;
    
public:
    BLEComm();
    virtual ~BLEComm() = default;
    
    // Essential operations (ICommunication)
    bool initialize() override;
    bool isConnected() override;
    void disconnect() override;
    bool isReady() override;
    
    bool sendData(const String& data) override;
    bool sendSensorData(const SensorData& data) override;
    String receiveData() override;
    bool hasDataAvailable() override;
    
    // Optional features (ICommunication)
    bool startAdvertising() override;
    bool stopAdvertising() override;
    bool isAdvertising() override;
    
    void setDataCallback(DataCallback callback) override;
    void setStatusCallback(StatusCallback callback) override;
    
    void setDeviceName(const String& name) override;
    String getDeviceName() override;
    
    int getSignalStrength() override;
    String getLastError() override;
    
    void sleep() override;
    void wakeup() override;
};

/*
 * communication/BLEComm.cpp
 * Implementation
 */

BLEComm::BLEComm() 
    : deviceName("CoToMeter")
    , lastError("")
    , initialized(false)
    , advertising(false)
    , bytesTransmitted(0)
    , bytesReceived(0)
{
}

bool BLEComm::initialize() {
    Serial.println("📡 Initializing Bluetooth...");
    
    if (!SerialBT.begin(deviceName)) {
        lastError = "Bluetooth initialization failed";
        Serial.println("❌ " + lastError);
        return false;
    }
    
    initialized = true;
    advertising = true;
    
    Serial.println("✅ Bluetooth initialized as: " + deviceName);
    Serial.println("📱 Ready to pair with mobile app");
    
    return true;
}

bool BLEComm::isConnected() {
    return initialized && SerialBT.hasClient();
}

void BLEComm::disconnect() {
    if (isConnected()) {
        SerialBT.disconnect();
        Serial.println("📡 Bluetooth disconnected");
        
        if (statusCallback) {
            statusCallback(false);
        }
    }
}

bool BLEComm::isReady() {
    return initialized;
}

bool BLEComm::sendData(const String& data) {
    if (!isConnected()) {
        lastError = "Not connected";
        return false;
    }
    
    SerialBT.println(data);
    bytesTransmitted += data.length();
    
    Serial.println("📤 Sent: " + data);
    return true;
}

bool BLEComm::sendSensorData(const SensorData& data) {
    return sendData(data.toJson());
}

String BLEComm::receiveData() {
    if (!SerialBT.available()) {
        return "";
    }
    
    String receivedData = SerialBT.readStringUntil('\n');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
        bytesReceived += receivedData.length();
        Serial.println("📥 Received: " + receivedData);
        
        if (dataCallback) {
            dataCallback(receivedData);
        }
    }
    
    return receivedData;
}

bool BLEComm::hasDataAvailable() {
    return SerialBT.available() > 0;
}

bool BLEComm::startAdvertising() {
    if (!initialized) {
        return initialize();
    }
    
    advertising = true;
    Serial.println("📡 Bluetooth advertising started");
    return true;
}

bool BLEComm::stopAdvertising() {
    advertising = false;
    SerialBT.end();
    initialized = false;
    Serial.println("📡 Bluetooth advertising stopped");
    return true;
}

bool BLEComm::isAdvertising() {
    return advertising && initialized;
}

void BLEComm::setDataCallback(DataCallback callback) {
    dataCallback = callback;
}

void BLEComm::setStatusCallback(StatusCallback callback) {
    statusCallback = callback;
}

void BLEComm::setDeviceName(const String& name) {
    deviceName = name;
    
    // If already initialized, restart with new name
    if (initialized) {
        SerialBT.end();
        delay(100);
        SerialBT.begin(deviceName);
        Serial.println("📡 Bluetooth device name changed to: " + deviceName);
    }
}

String BLEComm::getDeviceName() {
    return deviceName;
}

int BLEComm::getSignalStrength() {
    // BluetoothSerial doesn't provide RSSI directly
    // Return a mock value or implement if needed
    return isConnected() ? -45 : -100; // dBm
}

String BLEComm::getLastError() {
    return lastError;
}

void BLEComm::sleep() {
    if (initialized) {
        SerialBT.end();
        initialized = false;
        advertising = false;
        Serial.println("📡 Bluetooth sleeping");
    }
}

void BLEComm::wakeup() {
    if (!initialized) {
        initialize();
    }
}