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
    , batteryPowered(true)
    , storageCapacityMB(3.5)
    , historicalDataEnabled(false)
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
            Serial.printf("🔍 Processing command: %s\n", command.c_str());
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
    doc["battery_powered"] = batteryPowered;
    doc["mac_address"] = WiFi.macAddress();
    doc["storage_type"] = historicalDataEnabled ? 
                         (historicalStorage ? historicalStorage->getStorageType() : "flash") : "none";
    doc["storage_capacity_mb"] = storageCapacityMB;
    
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
    doc["type"] = "device_status";  // ✅ ADD: Missing type field
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

// ================================
// TIME SYNCHRONIZATION FUNCTIONS
// ================================

bool BluetoothComm::sendTimeSyncStatus(const String& request_id) {
    if (!isConnected()) return false;
    
    JsonDocument doc;
    doc["type"] = "time_sync_status";
    if (request_id.length() > 0) {
        doc["request_id"] = request_id;
    }
    doc["has_time"] = timeSync.has_time;
    doc["current_uptime"] = millis();
    
    if (timeSync.has_time) {
        doc["current_timestamp"] = timeSync.getCurrentTimestamp();
        doc["sync_age_minutes"] = timeSync.getSyncAgeMinutes();
    }
    
    Serial.printf("⏰ Sending time sync status: has_time=%s, uptime=%lu\n", 
                 timeSync.has_time ? "true" : "false", millis());
    return sendJsonMessage("time_sync_status", doc);
}

bool BluetoothComm::sendTimeSyncAck(const String& request_id, bool success, const String& message) {
    if (!isConnected()) return false;
    
    JsonDocument doc;
    doc["type"] = "time_sync_ack";
    doc["request_id"] = request_id;
    doc["success"] = success;
    doc["current_uptime"] = millis();
    
    if (message.length() > 0) {
        doc["message"] = message;
    }
    
    if (success && timeSync.has_time) {
        doc["current_timestamp"] = timeSync.getCurrentTimestamp();
    }
    
    Serial.printf("⏰ Sending time sync ack: success=%s\n", success ? "true" : "false");
    return sendJsonMessage("time_sync_ack", doc);
}

bool BluetoothComm::synchronizeTime(uint64_t current_timestamp, const String& timezone_offset) {
    bool success = timeSync.synchronizeTime(current_timestamp, timezone_offset);
    
    if (success) {
        Serial.printf("⏰ Time synchronized! Current time: %llu\n", timeSync.getCurrentTimestamp());
        
        // Enable historical data storage now that we have time sync
        if (!historicalDataEnabled && historicalStorage) {
            Serial.println("📊 Enabling historical data storage after time sync...");
            historicalDataEnabled = true;
        }
    } else {
        Serial.println("⚠️ Time synchronization failed");
    }
    
    return success;
}

// ================================
// HISTORICAL DATA FUNCTIONS
// ================================

bool BluetoothComm::sendHistoricalData(const String& request_id, const TimeRange& range, 
                                     size_t chunk_size) {
    if (!isConnected()) return false;
    
    if (!historicalStorage) {
        return sendErrorMessage("STORAGE_ERROR", "Historical data not enabled", "error", "", request_id);
    }
    
    // Create TimeRange struct for query
    TimeRange query_range;
    query_range.start_time = range.start_time;
    query_range.end_time = range.end_time;
    query_range.max_points = range.max_points;
    
    // Query historical data
    std::vector<SensorRecord> records = historicalStorage->queryByTimeRange(query_range, timeSync);
    
    JsonDocument doc;
    doc["t"] = "historical_data";  // t = type
    doc["r"] = request_id;         // r = request_id  
    doc["n"] = records.size();     // n = total_records
    doc["s"] = timeSync.has_time;  // s = time_synced
    
    // 🚀 ULTRA COMPACT FORMAT: Single letter field names + 2 decimal precision
    // Each record: {t: timestamp, c: co2, T: temp, h: humidity, p: pressure, v: voc}
    JsonArray dataArray = doc["d"].to<JsonArray>();  // d = data
    
    for (const SensorRecord& record : records) {
        JsonObject dataPoint = dataArray.add<JsonObject>();
        
        // t = timestamp
        if (timeSync.has_time) {
            dataPoint["t"] = timeSync.uptimeToTimestamp(record.uptime);
        } else {
            dataPoint["t"] = record.uptime;
        }
        
        // c = CO2 (integer is fine)
        if (record.validity_flags & SensorRecord::FLAG_CO2_VALID) {
            dataPoint["c"] = (int)round(record.co2);
        }
        
        // T = Temperature (2 decimal places)
        if (record.validity_flags & SensorRecord::FLAG_TEMP_VALID) {
            dataPoint["T"] = round(record.temperature * 100) / 100.0;
        }
        
        // h = Humidity (2 decimal places)
        if (record.validity_flags & SensorRecord::FLAG_HUMIDITY_VALID) {
            dataPoint["h"] = round(record.humidity * 100) / 100.0;
        }
        
        // p = Pressure (2 decimal places)
        if (record.validity_flags & SensorRecord::FLAG_PRESSURE_VALID) {
            dataPoint["p"] = round(record.pressure * 100) / 100.0;
        }
        
        // v = VOC (2 decimal places)
        if (record.validity_flags & SensorRecord::FLAG_VOC_VALID) {
            dataPoint["v"] = round(record.voc * 100) / 100.0;
        }
    }
    
    Serial.printf("🚀 Sending %zu historical records in ULTRA COMPACT format\n", records.size());
    Serial.println("📝 Format: {t:type, r:request_id, n:count, s:time_synced, d:[{t:timestamp, c:co2, T:temp, h:humidity, p:pressure, v:voc}]}");
    return sendJsonMessage("historical_data", doc);
}

bool BluetoothComm::sendStorageInfo(const String& request_id) {
    if (!isConnected()) return false;
    
    JsonDocument doc;
    doc["t"] = "storage_info";  // t = type
    if (request_id.length() > 0) {
        doc["r"] = request_id;  // r = request_id
    }
    
    if (historicalStorage) {
        doc["e"] = true;  // e = enabled
        doc["c"] = historicalStorage->getRecordCount();  // c = record_count
        doc["m"] = historicalStorage->getMaxRecords();   // m = max_records
        doc["f"] = historicalStorage->isFull();          // f = is_full
        doc["z"] = historicalStorage->isEmpty();         // z = is_empty (using z to avoid conflict)
        doc["y"] = historicalStorage->getStorageType();  // y = storage_type
        doc["s"] = timeSync.has_time;                    // s = time_synced
        
        // Get time range if data exists
        unsigned long oldest_uptime, newest_uptime;
        if (historicalStorage->getDataTimeRange(oldest_uptime, newest_uptime)) {
            if (timeSync.has_time) {
                doc["o"] = timeSync.uptimeToTimestamp(oldest_uptime);  // o = earliest_timestamp
                doc["l"] = timeSync.uptimeToTimestamp(newest_uptime);  // l = latest_timestamp
            } else {
                doc["o"] = oldest_uptime;   // o = earliest_uptime
                doc["l"] = newest_uptime;   // l = latest_uptime
                doc["g"] = "Time not synced";  // g = message
            }
        }
    } else {
        doc["e"] = false;  // e = enabled
        doc["g"] = "Storage not initialized";  // g = message
    }
    
    Serial.println("💾 Sending storage info in compact format");
    return sendJsonMessage("storage_info", doc);
}

// ================================
// HISTORICAL DATA MANAGEMENT
// ================================

bool BluetoothComm::enableHistoricalData(size_t max_records) {
    if (!historicalStorage) {
        historicalStorage = std::unique_ptr<HistoricalDataStorage>(new HistoricalDataStorage("ram_only", max_records));
        if (!historicalStorage->initialize()) {
            Serial.println("❌ Failed to initialize historical data storage");
            historicalStorage.reset();
            return false;
        }
    }
    
    historicalDataEnabled = true;
    Serial.printf("✅ Historical data enabled with %zu max records\n", max_records);
    return true;
}

bool BluetoothComm::disableHistoricalData() {
    if (historicalStorage) {
        historicalStorage.reset();
    }
    historicalDataEnabled = false;
    Serial.println("📊 Historical data disabled");
    return true;
}

bool BluetoothComm::storeCurrentReading(const CO2SensorData* co2_data,
                                       const VOCSensorData* voc_data) {
    if (!historicalDataEnabled || !historicalStorage) {
        return false;
    }
    
    return historicalStorage->storeReading(millis(), co2_data, voc_data);
}

void BluetoothComm::parseAndHandleCommand(const String& command) {
    JsonDocument doc;  // ✅ FIX: Use JsonDocument
    DeserializationError error = deserializeJson(doc, command);
    
    if (error) {
        sendErrorMessage("JSON_PARSE_ERROR", "Invalid JSON format", "error");
        return;
    }
    
    String type = doc["type"].as<String>();
    Serial.printf("🔍 Parsing command type: '%s'\n", type.c_str());
    
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
    } else if (type == "time_sync_request") {
        handleTimeSyncRequest(doc);
    } else if (type == "time_sync_set") {
        handleTimeSyncSet(doc);
    } else if (type == "history_request") {
        handleHistoryRequest(doc);
    } else if (type == "realtime_control") {
        handleRealtimeControl(doc);
    } else if (type == "storage_info_request") {
        handleStorageInfoRequest(doc);
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
    stats += ", Time: " + timeSync.getStatusString();
    if (historicalDataEnabled && historicalStorage) {
        stats += ", Records: " + String(historicalStorage->getRecordCount());
    }
    return stats;
}

// ================================
// NEW COMMAND HANDLERS IMPLEMENTATION
// ================================

void BluetoothComm::handleTimeSyncRequest(JsonDocument& cmd) {
    String request_id = cmd["request_id"].as<String>();
    
    Serial.println("⏰ Time sync requested");
    Serial.printf("🔍 Request ID: '%s'\n", request_id.c_str());
    sendTimeSyncStatus(request_id);
}

void BluetoothComm::handleTimeSyncSet(JsonDocument& cmd) {
    String request_id = cmd["request_id"].as<String>();
    uint64_t current_time = cmd["current_time"].as<uint64_t>();
    String timezone_offset = cmd["timezone_offset"].as<String>();
    
    if (timezone_offset.length() == 0) {
        timezone_offset = "+0000";
    }
    
    Serial.printf("⏰ Setting time: %llu, timezone: %s\n", current_time, timezone_offset.c_str());
    
    bool success = synchronizeTime(current_time, timezone_offset);
    sendTimeSyncAck(request_id, success);
}

void BluetoothComm::handleHistoryRequest(JsonDocument& cmd) {
    String request_id = cmd["request_id"].as<String>();
    
    TimeRange range;
    range.start_time = cmd["start_time"].as<uint64_t>();
    range.end_time = cmd["end_time"].as<uint64_t>();
    range.max_points = cmd["max_points"].as<int>();
    
    if (range.max_points == 0) {
        range.max_points = 1000; // Default
    }
    
    Serial.printf("📊 History request: %llu-%llu, max_points=%u\n", 
                 range.start_time, range.end_time, range.max_points);
    
    sendHistoricalData(request_id, range);
}

void BluetoothComm::handleRealtimeControl(JsonDocument& cmd) {
    String action = cmd["action"].as<String>();
    int interval_ms = cmd["interval_ms"].as<int>();
    
    if (action == "start") {
        streaming = true;
        if (interval_ms > 0) {
            // TODO: Set sampling interval
        }
        Serial.println("📊 Real-time streaming started");
    } else if (action == "stop") {
        streaming = false;
        Serial.println("📊 Real-time streaming stopped");
    } else if (action == "pause") {
        streaming = false;
        Serial.println("📊 Real-time streaming paused");
    }
}

void BluetoothComm::handleStorageInfoRequest(JsonDocument& cmd) {
    String request_id = cmd["request_id"].as<String>();
    
    Serial.println("💾 Storage info requested");
    sendStorageInfo(request_id);
}

bool BluetoothComm::sendErrorMessage(const String& errorCode, const String& message, 
                                    const String& severity, const String& sensor,
                                    const String& request_id, JsonDocument* details) {
    if (!isConnected()) return false;
    
    JsonDocument doc;
    doc["type"] = "error";
    if (request_id.length() > 0) {
        doc["request_id"] = request_id;
    }
    doc["timestamp"] = millis();
    doc["device_id"] = deviceId;
    doc["error_code"] = errorCode;
    doc["error_message"] = message;
    doc["severity"] = severity;
    if (sensor.length() > 0) {
        doc["sensor"] = sensor;
    }
    
    if (details) {
        doc["details"] = *details;
    }
    
    Serial.printf("🚨 Bluetooth error: %s - %s\n", errorCode.c_str(), message.c_str());
    return sendJsonMessage("error", doc);
}