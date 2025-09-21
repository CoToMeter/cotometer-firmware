#include "communication/BluetoothComm.h"

BluetoothComm::BluetoothComm()
    : deviceName("CoToMeter")
    , deviceId("ESP32_001") 
    , initialized(false)
    , advertising(false)
    , connected(false)
    , streaming(false)
    , bytesTransmitted(0)
    , bytesReceived(0)
    , connectionStartTime(0)
    , lastStatusSent(0)
    , statusUpdateInterval(30000) // 30 seconds
    , samplingRate(5)
{
    // Set defaults
    firmwareVersion = "2.0.0";
    hardwareVersion = "2.1";  
    deviceType = "ESP32_HOME";
    availableSensors.push_back("CO2");
    availableSensors.push_back("TEMPERATURE");
    availableSensors.push_back("HUMIDITY");
    availableSensors.push_back("VOC");
    availableSensors.push_back("PRESSURE");
}

// ================================
// ESSENTIAL OPERATIONS
// ================================

bool BluetoothComm::initialize() {
    Serial.println("📡 Initializing Bluetooth communication...");
    
    // Create unique device name
    String macAddress = WiFi.macAddress();
    deviceName = "CoToMeter 😺";
    deviceId = "ESP32_" + macAddress.substring(9);
    deviceId.replace(":", "");
    
    if (!SerialBT.begin(deviceName)) {
        lastError = "Bluetooth initialization failed";
        Serial.println("❌ " + lastError);
        return false;
    }
    
    initialized = true;
    advertising = true;
    
    Serial.printf("✅ Bluetooth initialized: %s (%s)\n", 
                 deviceName.c_str(), deviceId.c_str());
    Serial.println("📱 Ready for mobile app connection");
    
    return true;
}

bool BluetoothComm::isConnected() {
    bool currentlyConnected = SerialBT.hasClient();
    
    if (currentlyConnected != connected) {
        connected = currentlyConnected;
        onConnectionChange();
    }
    
    return connected;
}

void BluetoothComm::disconnect() {
    if (connected) {
        SerialBT.disconnect();
        connected = false;
        streaming = false;
        Serial.println("📱 Bluetooth disconnected");
    }
}

bool BluetoothComm::isReady() {
    return initialized;
}

// ================================
// DATA TRANSMISSION - ✅ FIXED
// ================================

bool BluetoothComm::sendData(const String& data) {
    if (!isConnected()) {
        return false;
    }
    
    SerialBT.println(data);
    bytesTransmitted += data.length() + 1;
    
    Serial.printf("📤 BT Sent: %s\n", data.c_str());
    return true;
}

// ✅ FIX: Use SensorDataBase and handle polymorphism without RTTI
bool BluetoothComm::sendSensorData(const SensorDataBase& data) {
    if (!isConnected() || !streaming) {
        return false;
    }
    
    // ✅ FIX: Use JsonDocument instead of DynamicJsonDocument
    JsonDocument doc;
    doc["type"] = "sensor_data";
    doc["timestamp"] = millis();
    doc["device_id"] = deviceId;
    
    // Create readings object based on sensor type
    JsonObject readings = doc["readings"].to<JsonObject>();
    
    // ✅ FIX: Handle different sensor types without dynamic_cast
    SensorType sensorType = data.getType();
    
    if (sensorType == SensorType::CO2_TEMP_HUMIDITY) {
        // ✅ FIX: Use static_cast instead of dynamic_cast (we know the type)
        const CO2SensorData* co2Data = static_cast<const CO2SensorData*>(&data);
        
        JsonObject co2Reading = readings["co2"].to<JsonObject>();
        co2Reading["value"] = co2Data->co2;
        co2Reading["unit"] = "ppm";
        co2Reading["accuracy"] = 0.95;
        co2Reading["status"] = co2Data->isDataValid() ? "valid" : "invalid";
        
        JsonObject tempReading = readings["temperature"].to<JsonObject>();
        tempReading["value"] = co2Data->temperature;
        tempReading["unit"] = "celsius";
        tempReading["accuracy"] = 0.98;
        tempReading["status"] = "valid";
        
        JsonObject humReading = readings["humidity"].to<JsonObject>();
        humReading["value"] = co2Data->humidity;
        humReading["unit"] = "percent";
        humReading["accuracy"] = 0.92;
        humReading["status"] = "valid";
        
    } else if (sensorType == SensorType::VOC_GAS) {
        const VOCSensorData* vocData = static_cast<const VOCSensorData*>(&data);
        
        JsonObject vocReading = readings["voc"].to<JsonObject>();
        vocReading["value"] = vocData->vocEstimate;
        vocReading["unit"] = "ppb";
        vocReading["accuracy"] = 0.85;
        vocReading["status"] = vocData->gasValid ? "valid" : "invalid";
        
        JsonObject tempReading = readings["temperature"].to<JsonObject>();
        tempReading["value"] = vocData->temperature;
        tempReading["unit"] = "celsius";
        tempReading["accuracy"] = 0.98;
        tempReading["status"] = "valid";
        
        JsonObject humReading = readings["humidity"].to<JsonObject>();
        humReading["value"] = vocData->humidity;
        humReading["unit"] = "percent";
        humReading["accuracy"] = 0.92;
        humReading["status"] = "valid";
        
        JsonObject pressReading = readings["pressure"].to<JsonObject>();
        pressReading["value"] = vocData->pressure / 100.0;
        pressReading["unit"] = "hPa";
        pressReading["accuracy"] = 0.99;
        pressReading["status"] = "valid";
        
    } else if (sensorType == SensorType::PARTICULATE_MATTER) {
        const PMSensorData* pmData = static_cast<const PMSensorData*>(&data);
        
        JsonObject pm25Reading = readings["pm2_5"].to<JsonObject>();
        pm25Reading["value"] = pmData->pm2_5_atmospheric;
        pm25Reading["unit"] = "μg/m³";
        pm25Reading["accuracy"] = 0.90;
        pm25Reading["status"] = pmData->isDataValid() ? "valid" : "invalid";
        
        JsonObject pm10Reading = readings["pm10"].to<JsonObject>();
        pm10Reading["value"] = pmData->pm10_atmospheric;
        pm10Reading["unit"] = "μg/m³";
        pm10Reading["accuracy"] = 0.90;
        pm10Reading["status"] = pmData->isDataValid() ? "valid" : "invalid";
    }
    
    return sendJsonMessage("sensor_data", doc);
}

String BluetoothComm::receiveData() {
    if (!SerialBT.available()) {
        return "";
    }
    
    String receivedData = SerialBT.readStringUntil('\n');
    receivedData.trim();
    
    if (receivedData.length() > 0) {
        bytesReceived += receivedData.length();
        Serial.printf("📥 BT Received: %s\n", receivedData.c_str());
        
        if (dataCallback) {
            dataCallback(receivedData);
        }
    }
    
    return receivedData;
}

bool BluetoothComm::hasDataAvailable() {
    return SerialBT.available() > 0;
}

// ================================
// OPTIONAL FEATURES
// ================================

bool BluetoothComm::startAdvertising() {
    if (!initialized) {
        return initialize();
    }
    advertising = true;
    return true;
}

bool BluetoothComm::stopAdvertising() {
    advertising = false;
    SerialBT.end();
    initialized = false;
    return true;
}

bool BluetoothComm::isAdvertising() {
    return advertising && initialized;
}

void BluetoothComm::setDataCallback(DataCallback callback) {
    dataCallback = callback;
}

void BluetoothComm::setStatusCallback(StatusCallback callback) {
    statusCallback = callback;
}

void BluetoothComm::setDeviceName(const String& name) {
    deviceName = name;
    if (initialized) {
        SerialBT.end();
        delay(100);
        SerialBT.begin(deviceName);
    }
}

String BluetoothComm::getDeviceName() {
    return deviceName;
}

int BluetoothComm::getSignalStrength() {
    return isConnected() ? -45 : -100;
}

String BluetoothComm::getLastError() {
    return lastError;
}

void BluetoothComm::sleep() {
    if (initialized) {
        SerialBT.end();
        initialized = false;
        advertising = false;
    }
}

void BluetoothComm::wakeup() {
    if (!initialized) {
        initialize();
    }
}

// ================================
// PROTOCOL EXTENSIONS
// ================================

void BluetoothComm::setDeviceInfo(const String& firmware, const String& hardware, 
                                 const String& type, const std::vector<String>& sensors) {
    firmwareVersion = firmware;
    hardwareVersion = hardware;
    deviceType = type;
    availableSensors = sensors;
}

void BluetoothComm::setSamplingRate(int rate) {
    samplingRate = rate;
    Serial.printf("📊 Bluetooth: Sampling rate set to %d seconds\n", rate);
}

void BluetoothComm::update() {
    isConnected();
    
    if (!connected) return;
    
    handleIncomingCommands();
    
    uint32_t now = millis();
    if (now - lastStatusSent >= statusUpdateInterval) {
        sendDeviceStatus();
        lastStatusSent = now;
    }
}

void BluetoothComm::handleIncomingCommands() {
    while (hasDataAvailable()) {
        String command = receiveData();
        if (command.length() > 0) {
            parseAndHandleCommand(command);
        }
    }
}

// ================================
// PROTOCOL MESSAGE IMPLEMENTATION - ✅ FIXED
// ================================

bool BluetoothComm::sendDeviceInfo() {
    if (!isConnected()) return false;
    
    JsonDocument doc;  // ✅ FIX: Use JsonDocument
    doc["device_id"] = deviceId;
    doc["device_name"] = deviceName;
    doc["device_type"] = deviceType;
    doc["firmware_version"] = firmwareVersion;
    doc["hardware_version"] = hardwareVersion;
    doc["sampling_rate"] = samplingRate;
    doc["battery_powered"] = true;
    doc["mac_address"] = WiFi.macAddress();
    
    JsonArray sensorsArray = doc["available_sensors"].to<JsonArray>();
    for (size_t i = 0; i < availableSensors.size(); i++) {
        sensorsArray.add(availableSensors[i]);
    }
    
    Serial.println("📋 Sending device info via Bluetooth");
    return sendJsonMessage("device_info", doc);
}

bool BluetoothComm::sendDeviceStatus() {
    if (!isConnected()) return false;
    
    JsonDocument doc;  // ✅ FIX: Use JsonDocument
    doc["timestamp"] = millis();
    doc["device_id"] = deviceId;
    doc["battery_level"] = 85;
    doc["battery_voltage"] = 3.7;
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["uptime_seconds"] = millis() / 1000;
    doc["free_memory"] = ESP.getFreeHeap();
    
    JsonObject sensorStatus = doc["sensor_status"].to<JsonObject>();
    sensorStatus["co2"] = "ready";
    sensorStatus["temperature"] = "ready";
    sensorStatus["humidity"] = "ready";
    sensorStatus["voc"] = "ready";
    sensorStatus["pressure"] = "ready";
    
    return sendJsonMessage("device_status", doc);
}

bool BluetoothComm::sendErrorMessage(const String& errorCode, const String& message, 
                                    const String& severity, const String& sensor) {
    if (!isConnected()) return false;
    
    JsonDocument doc;  // ✅ FIX: Use JsonDocument
    doc["timestamp"] = millis();
    doc["device_id"] = deviceId;
    doc["error_code"] = errorCode;
    doc["error_message"] = message;
    doc["severity"] = severity;
    if (sensor.length() > 0) {
        doc["sensor"] = sensor;
    }
    
    Serial.printf("🚨 Bluetooth error: %s - %s\n", errorCode.c_str(), message.c_str());
    return sendJsonMessage("error", doc);
}

// ================================
// PRIVATE HELPERS - ✅ FIXED
// ================================

void BluetoothComm::onConnectionChange() {
    if (connected) {
        connectionStartTime = millis();
        Serial.println("📱 Mobile app connected via Bluetooth!");
        
        delay(500);
        sendDeviceInfo();
        
        streaming = true;
        
        if (statusCallback) {
            statusCallback(true);
        }
        
    } else {
        streaming = false;
        Serial.println("📱 Mobile app disconnected");
        
        if (statusCallback) {
            statusCallback(false);
        }
    }
}

bool BluetoothComm::sendJsonMessage(const String& type, JsonDocument& doc) {
    String jsonString;
    serializeJson(doc, jsonString);
    return sendData(jsonString);
}

void BluetoothComm::parseAndHandleCommand(const String& command) {
    JsonDocument doc;  // ✅ FIX: Use JsonDocument
    DeserializationError error = deserializeJson(doc, command);
    
    if (error) {
        sendErrorMessage("JSON_PARSE_ERROR", "Invalid JSON format", "error");
        return;
    }
    
    String type = doc["type"].as<String>();
    
    if (type == "connection_ack") {
        Serial.println("✅ Connection acknowledged");
    } else if (type == "set_sampling_rate") {
        handleSetSamplingRate(doc);
    } else if (type == "calibrate_sensor") {
        handleCalibrateSensor(doc);
    } else if (type == "get_device_info") {
        handleGetDeviceInfo(doc);
    } else if (type == "start_streaming") {
        handleStartStreaming(doc);
    } else if (type == "stop_streaming") {
        handleStopStreaming(doc);
    } else if (type == "restart_device") {
        handleRestartDevice(doc);
    } else {
        sendErrorMessage("UNKNOWN_COMMAND", "Command not recognized: " + type, "warning");
    }
}

// ✅ FIX: Command handlers use JsonDocument
void BluetoothComm::handleSetSamplingRate(JsonDocument& cmd) {
    int rate = cmd["rate"].as<int>();
    if (rate >= 1 && rate <= 300) {
        setSamplingRate(rate);
        Serial.printf("✅ Sampling rate changed to %d seconds\n", rate);
    } else {
        sendErrorMessage("INVALID_RATE", "Rate must be 1-300 seconds", "error");
    }
}

void BluetoothComm::handleCalibrateSensor(JsonDocument& cmd) {
    String sensor = cmd["sensor"].as<String>();
    Serial.printf("🔧 Calibration requested for: %s\n", sensor.c_str());
}

void BluetoothComm::handleGetDeviceInfo(JsonDocument& cmd) {
    sendDeviceInfo();
}

void BluetoothComm::handleStartStreaming(JsonDocument& cmd) {
    streaming = true;
    Serial.println("📊 Streaming started via Bluetooth command");
}

void BluetoothComm::handleStopStreaming(JsonDocument& cmd) {
    streaming = false;
    Serial.println("⏸️  Streaming stopped via Bluetooth command");
}

void BluetoothComm::handleRestartDevice(JsonDocument& cmd) {
    Serial.println("🔄 Restart requested via Bluetooth");
    delay(1000);
    ESP.restart();
}

String BluetoothComm::getConnectionStats() {
    String stats = "📊 BT: ";
    stats += connected ? "Connected" : "Disconnected";
    stats += ", Streaming: " + String(streaming ? "Yes" : "No");
    stats += ", Sent: " + String(bytesTransmitted) + "B";
    return stats;
}
