#pragma once
#include "../interfaces/ICommunication.h"
#include "../types/SensorData.h"
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <vector>

class BluetoothComm : public ICommunication {
private:
    BluetoothSerial SerialBT;
    String deviceName;
    String deviceId;
    String lastError;
    bool initialized;
    bool advertising;
    bool connected;
    bool streaming;
    
    // Callbacks
    DataCallback dataCallback;
    StatusCallback statusCallback;
    
    // Device info for protocol
    String firmwareVersion;
    String hardwareVersion;
    String deviceType;
    std::vector<String> availableSensors;
    int samplingRate;
    
    // Statistics
    uint32_t bytesTransmitted;
    uint32_t bytesReceived;
    uint32_t connectionStartTime;
    uint32_t lastStatusSent;
    
    // Timing
    uint32_t statusUpdateInterval;
    
public:
    BluetoothComm();
    virtual ~BluetoothComm() = default;
    
    // ================================
    // ICommunication Interface Implementation
    // ================================
    
    // Essential operations (must implement)
    bool initialize() override;
    bool isConnected() override;
    void disconnect() override;
    bool isReady() override;
    
    // Data transmission (must implement) - ✅ FIX: Use SensorDataBase
    bool sendData(const String& data) override;
    bool sendSensorData(const SensorDataBase& data) override;  // ✅ FIXED
    String receiveData() override;
    bool hasDataAvailable() override;
    
    // Optional features
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
    
    // ================================
    // Bluetooth Protocol Extensions
    // ================================
    
    // Device information setup
    void setDeviceInfo(const String& firmware, const String& hardware, 
                      const String& type, const std::vector<String>& sensors);
    void setSamplingRate(int rate);
    
    // Protocol message sending
    bool sendDeviceInfo();
    bool sendDeviceStatus();
    bool sendErrorMessage(const String& errorCode, const String& message, 
                         const String& severity = "error", const String& sensor = "");
    
    // Command handling (call from main loop)
    void handleIncomingCommands();
    void update(); // Handle periodic status updates
    
    // Status
    bool isStreaming() const { return streaming; }
    String getConnectionStats();
    
private:
    // Connection management
    void onConnectionChange();
    void sendConnectionAck();
    
    // JSON helpers
    bool sendJsonMessage(const String& type, JsonDocument& payload);  // ✅ FIX: JsonDocument
    void parseAndHandleCommand(const String& command);
    
    // Command handlers
    void handleSetSamplingRate(JsonDocument& cmd);  // ✅ FIX: JsonDocument
    void handleCalibrateSensor(JsonDocument& cmd);
    void handleGetDeviceInfo(JsonDocument& cmd);
    void handleStartStreaming(JsonDocument& cmd);
    void handleStopStreaming(JsonDocument& cmd);
    void handleRestartDevice(JsonDocument& cmd);
};
