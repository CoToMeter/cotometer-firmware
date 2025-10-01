# 📱 CoToMeter Android API Specification

## ESP32 ↔ Android Bluetooth Communication Protocol

This document provides the complete API specification for Android apps to communicate with the ESP32 CoToMeter device via Bluetooth.

---

## 🔄 **Communication Flow Overview**

```
📱 Android App                    📡 ESP32 Device
     │                               │
     ├─[1] Connect via Bluetooth ────→│
     │                               ├─[2] Auto-send device_info
     │←─── Device Info ───────────────┤
     │                               │
     ├─[3] Check Time Sync ──────────→│
     │                               ├─[4] Return sync status
     │←─── Time Sync Status ──────────┤
     │                               │
     ├─[5] Set Current Time ─────────→│ (if needed)
     │                               ├─[6] Confirm time sync
     │←─── Time Sync Ack ─────────────┤
     │                               │
     ├─[7] Request Storage Info ─────→│
     │                               ├─[8] Return storage details
     │←─── Storage Info ──────────────┤
     │                               │
     ├─[9] Request Historical Data ──→│
     │                               ├─[10] Query & send data
     │←─── Historical Data ───────────┤
     │                               │
     ├─[11] Control Real-time ───────→│
     │←─── Live Sensor Data ──────────┤ (continuous)
```

---

## 📡 **1. Connection & Device Discovery**

### **1.1 Bluetooth Device Identification**
```
Device Name: "CoToMeter 😺"
Service UUID: Standard Bluetooth Serial (SPP)
MAC Address Pattern: 88:13:BF:xx:xx:xx
```

### **1.2 Connection Sequence**
1. Scan for Bluetooth devices
2. Filter by name prefix "CoToMeter"
3. Connect to device
4. Wait for automatic `device_info` message
5. Begin API communication

---

## 📨 **2. Message Format**

### **2.1 General Structure**
All messages are JSON strings terminated with `\n`:

```json
{
  "type": "message_type",
  "request_id": "unique_identifier", 
  "timestamp": 1695123456789,
  ...additional_fields
}
```

### **2.2 Message Types**
- **Commands (App → ESP32)**: `time_sync_request`, `time_sync_set`, `history_request`, etc.
- **Responses (ESP32 → App)**: `device_info`, `time_sync_status`, `history_response`, etc.
- **Real-time Data**: `sensor_data` (continuous stream)
- **Errors**: `error` (when commands fail)

---

## 🎯 **3. API Commands (App → ESP32)**

### **3.1 Time Synchronization**

#### **Check Time Sync Status**
```json
{
  "type": "time_sync_request",
  "request_id": "app_sync_check_001"
}
```

#### **Set Current Time**
```json
{
  "type": "time_sync_set",
  "request_id": "app_sync_set_001",
  "current_time": 1695123456789,
  "timezone_offset": "+0300"
}
```

**Parameters:**
- `current_time`: Unix timestamp in milliseconds
- `timezone_offset`: Format `±HHMM` (e.g., `+0300`, `-0500`)

### **3.2 Storage Management**

#### **Get Storage Information**
```json
{
  "type": "storage_info_request",
  "request_id": "app_storage_001"
}
```

#### **Request Historical Data**
```json
{
  "type": "history_request",
  "request_id": "app_history_001",
  "start_time": 1695120000000,
  "end_time": 1695123600000,
  "max_points": 1000,
  "sensors": ["co2", "temperature", "humidity", "voc", "pressure"]
}
```

**Parameters:**
- `start_time`: Unix timestamp (ms) - start of time range
- `end_time`: Unix timestamp (ms) - end of time range  
- `max_points`: Maximum data points to return (1-10000)
- `sensors`: Array of sensor types to include (optional)

### **3.3 Real-time Data Control**

#### **Start Real-time Streaming**
```json
{
  "type": "realtime_control",
  "action": "start",
  "interval_ms": 10000
}
```

#### **Stop Real-time Streaming**
```json
{
  "type": "realtime_control", 
  "action": "stop"
}
```

#### **Pause Real-time Streaming**
```json
{
  "type": "realtime_control",
  "action": "pause"
}
```

### **3.4 Device Configuration**

#### **Set Sampling Rate**
```json
{
  "type": "set_sampling_rate",
  "rate": 10
}
```

#### **Get Device Info**
```json
{
  "type": "get_device_info",
  "request_id": "app_info_001"
}
```

#### **Restart Device**
```json
{
  "type": "restart_device",
  "request_id": "app_restart_001"
}
```

---

## 📬 **4. API Responses (ESP32 → App)**

### **4.1 Device Information**
```json
{
  "type": "device_info",
  "device_id": "ESP32_69D270",
  "device_name": "CoToMeter 😺",
  "device_type": "ESP32_HOME",
  "firmware_version": "2.0.0",
  "hardware_version": "2.1",
  "mac_address": "88:13:BF:69:D2:70",
  "available_sensors": ["CO2", "TEMPERATURE", "HUMIDITY", "VOC", "PRESSURE"],
  "sampling_rate": 10,
  "battery_powered": true,
  "storage_type": "flash_ram_only",
  "storage_capacity_mb": 0.023
}
```

### **4.2 Time Synchronization**

#### **Time Sync Status**
```json
{
  "type": "time_sync_status",
  "request_id": "app_sync_check_001",
  "has_time": false,
  "current_uptime": 123456,
  "last_sync_uptime": 0,
  "time_offset": 0,
  "timezone_offset": "+0000",
  "sync_age_minutes": 0,
  "is_stale": true
}
```

#### **Time Sync Acknowledgment**
```json
{
  "type": "time_sync_ack",
  "request_id": "app_sync_set_001", 
  "success": true,
  "sync_uptime": 123456,
  "calculated_offset": 1695000000333,
  "timezone_offset": "+0300",
  "message": "Time synchronized successfully"
}
```

### **4.3 Storage Information**
```json
{
  "type": "storage_info",
  "request_id": "app_storage_001",
  "storage_type": "flash_ram_only",
  "total_capacity_mb": 0.023,
  "used_capacity_mb": 0.012,
  "free_capacity_mb": 0.011,
  "total_records": 315,
  "oldest_record_time": 1695120000000,
  "newest_record_time": 1695123456789,
  "estimated_days_remaining": 0.0,
  "time_synced": true,
  "time_sync_age_minutes": 5,
  "message": "Data stored in RAM only - lost on reboot"
}
```

### **4.4 Historical Data Response**
```json
{
  "type": "history_response",
  "request_id": "app_history_001",
  "total_points": 250,
  "chunk_index": 0,
  "total_chunks": 1,
  "data": [
    {
      "timestamp": 1695120001000,
      "uptime": 120001,
      "readings": {
        "co2": {
          "value": 450.2,
          "unit": "ppm", 
          "status": "valid"
        },
        "temperature": {
          "value": 23.5,
          "unit": "°C",
          "status": "valid"
        },
        "humidity": {
          "value": 45.2,
          "unit": "%",
          "status": "valid"
        },
        "voc": {
          "value": 27.8,
          "unit": "ppb",
          "status": "valid"
        },
        "pressure": {
          "value": 1013.2,
          "unit": "hPa",
          "status": "valid"
        }
      }
    }
  ]
}
```

### **4.5 Real-time Sensor Data**
```json
{
  "type": "sensor_data",
  "timestamp": 1695123456789,
  "device_id": "ESP32_69D270",
  "readings": {
    "co2": {
      "value": 455.1,
      "unit": "ppm",
      "accuracy": 0.95,
      "status": "valid"
    },
    "temperature": {
      "value": 22.8,
      "unit": "celsius", 
      "accuracy": 0.98,
      "status": "valid"
    },
    "humidity": {
      "value": 47.3,
      "unit": "percent",
      "accuracy": 0.92,
      "status": "valid"
    },
    "voc": {
      "value": 28.5,
      "unit": "ppb",
      "accuracy": 0.85,
      "status": "valid"
    },
    "pressure": {
      "value": 1014.7,
      "unit": "hPa",
      "accuracy": 0.99,
      "status": "valid"
    }
  }
}
```

### **4.6 Error Responses**
```json
{
  "type": "error",
  "request_id": "app_history_001",
  "timestamp": 1695123456789,
  "device_id": "ESP32_69D270",
  "error_code": "INVALID_RANGE",
  "error_message": "Start time too far in the past (max 1 year)",
  "severity": "error",
  "details": {
    "earliest_data": 1695120000000,
    "latest_data": 1695123456789
  }
}
```

**Common Error Codes:**
- `STORAGE_ERROR`: Historical data not enabled
- `TIME_NOT_SYNCED`: Time sync required
- `INVALID_RANGE`: Invalid time range parameters
- `NO_DATA`: No data in requested range
- `JSON_PARSE_ERROR`: Invalid JSON format
- `UNKNOWN_COMMAND`: Command not recognized

---

## 🔧 **5. Android Implementation Guide**

### **5.1 Connection Setup**
```kotlin
class CoToMeterBluetoothManager {
    private var bluetoothSocket: BluetoothSocket? = null
    private var isConnected = false
    
    suspend fun connectToDevice(device: BluetoothDevice): Result<Boolean> {
        try {
            val uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB") // SPP
            bluetoothSocket = device.createRfcommSocketToServiceRecord(uuid)
            bluetoothSocket?.connect()
            isConnected = true
            
            // Wait for device_info message
            val deviceInfo = waitForMessage("device_info", timeout = 5000)
            return Result.success(true)
        } catch (e: Exception) {
            return Result.failure(e)
        }
    }
}
```

### **5.2 Message Sending**
```kotlin
suspend fun sendCommand(command: JsonObject): Result<String> {
    if (!isConnected) return Result.failure(Exception("Not connected"))
    
    try {
        val message = command.toString() + "\n"
        bluetoothSocket?.outputStream?.write(message.toByteArray())
        return Result.success("Sent")
    } catch (e: Exception) {
        return Result.failure(e)
    }
}
```

### **5.3 Message Receiving**
```kotlin
private fun startMessageListener() {
    CoroutineScope(Dispatchers.IO).launch {
        val buffer = ByteArray(1024)
        val inputStream = bluetoothSocket?.inputStream
        
        while (isConnected) {
            try {
                val bytes = inputStream?.read(buffer)
                if (bytes != null && bytes > 0) {
                    val message = String(buffer, 0, bytes)
                    parseMessage(message)
                }
            } catch (e: Exception) {
                // Handle connection error
                break
            }
        }
    }
}

private fun parseMessage(message: String) {
    try {
        val json = JsonParser.parseString(message).asJsonObject
        val type = json.get("type").asString
        
        when (type) {
            "device_info" -> handleDeviceInfo(json)
            "sensor_data" -> handleSensorData(json)  
            "time_sync_status" -> handleTimeSyncStatus(json)
            "history_response" -> handleHistoryResponse(json)
            "storage_info" -> handleStorageInfo(json)
            "error" -> handleError(json)
        }
    } catch (e: Exception) {
        Log.e("CoToMeter", "Failed to parse message: $message", e)
    }
}
```

### **5.4 Time Synchronization Helper**
```kotlin
class TimeSyncManager {
    suspend fun synchronizeTime(bluetoothManager: CoToMeterBluetoothManager): Result<Boolean> {
        // 1. Check current sync status
        val statusRequest = JsonObject().apply {
            addProperty("type", "time_sync_request")
            addProperty("request_id", generateRequestId())
        }
        bluetoothManager.sendCommand(statusRequest)
        
        // 2. Wait for status response
        val statusResponse = waitForResponse("time_sync_status")
        val hasTime = statusResponse?.get("has_time")?.asBoolean ?: false
        
        if (!hasTime) {
            // 3. Send current time
            val timeRequest = JsonObject().apply {
                addProperty("type", "time_sync_set")
                addProperty("request_id", generateRequestId())
                addProperty("current_time", System.currentTimeMillis())
                addProperty("timezone_offset", getCurrentTimezoneOffset())
            }
            bluetoothManager.sendCommand(timeRequest)
            
            // 4. Wait for acknowledgment
            val ackResponse = waitForResponse("time_sync_ack")
            return Result.success(ackResponse?.get("success")?.asBoolean ?: false)
        }
        
        return Result.success(true)
    }
    
    private fun getCurrentTimezoneOffset(): String {
        val timezone = TimeZone.getDefault()
        val offset = timezone.rawOffset
        val hours = offset / (1000 * 60 * 60)
        val minutes = (offset % (1000 * 60 * 60)) / (1000 * 60)
        return String.format("%+03d%02d", hours, minutes)
    }
}
```

### **5.5 Historical Data Manager**
```kotlin
class HistoricalDataManager {
    suspend fun requestHistoricalData(
        bluetoothManager: CoToMeterBluetoothManager,
        startTime: Long,
        endTime: Long,
        maxPoints: Int = 1000
    ): Result<List<SensorReading>> {
        
        val request = JsonObject().apply {
            addProperty("type", "history_request")
            addProperty("request_id", generateRequestId())
            addProperty("start_time", startTime)
            addProperty("end_time", endTime)
            addProperty("max_points", maxPoints)
            add("sensors", JsonArray().apply {
                add("co2")
                add("temperature") 
                add("humidity")
                add("voc")
                add("pressure")
            })
        }
        
        bluetoothManager.sendCommand(request)
        
        // Handle chunked responses
        val allData = mutableListOf<SensorReading>()
        var expectedChunks = 1
        var receivedChunks = 0
        
        while (receivedChunks < expectedChunks) {
            val response = waitForResponse("history_response")
            if (response != null) {
                expectedChunks = response.get("total_chunks").asInt
                val data = response.getAsJsonArray("data")
                
                data.forEach { element ->
                    val reading = parseSensorReading(element.asJsonObject)
                    allData.add(reading)
                }
                
                receivedChunks++
            }
        }
        
        return Result.success(allData)
    }
}
```

### **5.6 Real-time Data Stream**
```kotlin
class RealTimeDataStream {
    private val sensorDataFlow = MutableSharedFlow<SensorReading>()
    
    suspend fun startStream(bluetoothManager: CoToMeterBluetoothManager): Flow<SensorReading> {
        val request = JsonObject().apply {
            addProperty("type", "realtime_control")
            addProperty("action", "start")
            addProperty("interval_ms", 10000)
        }
        bluetoothManager.sendCommand(request)
        
        return sensorDataFlow.asSharedFlow()
    }
    
    suspend fun stopStream(bluetoothManager: CoToMeterBluetoothManager) {
        val request = JsonObject().apply {
            addProperty("type", "realtime_control")
            addProperty("action", "stop")
        }
        bluetoothManager.sendCommand(request)
    }
    
    fun handleSensorData(json: JsonObject) {
        val reading = parseSensorReading(json)
        sensorDataFlow.tryEmit(reading)
    }
}
```

---

## 🔧 **6. Usage Examples**

### **6.1 Complete Connection Flow**
```kotlin
// 1. Connect to device
val result = bluetoothManager.connectToDevice(device)
if (result.isFailure) return

// 2. Synchronize time
val timeSync = timeSyncManager.synchronizeTime(bluetoothManager)
if (timeSync.isFailure) return

// 3. Get storage info
val storageInfo = getStorageInfo(bluetoothManager)

// 4. Request historical data (last hour)
val endTime = System.currentTimeMillis()
val startTime = endTime - (60 * 60 * 1000) // 1 hour ago
val historyData = historicalDataManager.requestHistoricalData(
    bluetoothManager, startTime, endTime, 100
)

// 5. Start real-time stream
realTimeStream.startStream(bluetoothManager)
    .collect { sensorReading ->
        updateUI(sensorReading)
    }
```

### **6.2 Error Handling**
```kotlin
private fun handleError(json: JsonObject) {
    val errorCode = json.get("error_code").asString
    val errorMessage = json.get("error_message").asString
    val severity = json.get("severity").asString
    
    when (errorCode) {
        "TIME_NOT_SYNCED" -> {
            // Trigger time synchronization
            timeSyncManager.synchronizeTime(bluetoothManager)
        }
        "STORAGE_ERROR" -> {
            // Historical data not available
            showMessage("Historical data not available")
        }
        "INVALID_RANGE" -> {
            // Adjust time range and retry
            val details = json.getAsJsonObject("details")
            val earliestData = details.get("earliest_data").asLong
            val latestData = details.get("latest_data").asLong
            // Retry with valid range
        }
        "NO_DATA" -> {
            // No data in requested range
            showMessage("No data available for selected time range")
        }
    }
}
```

---

## 📊 **7. Data Models**

### **7.1 Sensor Reading**
```kotlin
data class SensorReading(
    val timestamp: Long,
    val uptime: Long?,
    val co2: SensorValue?,
    val temperature: SensorValue?,
    val humidity: SensorValue?, 
    val voc: SensorValue?,
    val pressure: SensorValue?
)

data class SensorValue(
    val value: Double,
    val unit: String,
    val status: String,
    val accuracy: Double? = null
)
```

### **7.2 Device Info**
```kotlin
data class DeviceInfo(
    val deviceId: String,
    val deviceName: String,
    val deviceType: String,
    val firmwareVersion: String,
    val hardwareVersion: String,
    val macAddress: String,
    val availableSensors: List<String>,
    val samplingRate: Int,
    val batteryPowered: Boolean,
    val storageType: String,
    val storageCapacityMb: Double
)
```

### **7.3 Storage Info**
```kotlin
data class StorageInfo(
    val storageType: String,
    val totalCapacityMb: Double,
    val usedCapacityMb: Double,
    val freeCapacityMb: Double,
    val totalRecords: Int,
    val oldestRecordTime: Long,
    val newestRecordTime: Long,
    val estimatedDaysRemaining: Double,
    val timeSynced: Boolean,
    val timeSyncAgeMinutes: Int?,
    val message: String?
)
```

---

## ⚡ **8. Best Practices**

### **8.1 Connection Management**
- Always check connection status before sending commands
- Implement connection retry with exponential backoff
- Handle connection loss gracefully
- Close connections properly when done

### **8.2 Time Synchronization**
- Always sync time before requesting historical data
- Check if time sync is stale (>24 hours) and re-sync
- Handle timezone changes properly
- Use device's actual timezone, not UTC

### **8.3 Data Requests**
- Limit historical data requests to reasonable time ranges
- Use appropriate `max_points` to avoid large transfers
- Handle chunked responses for large datasets
- Implement timeout for data requests

### **8.4 Real-time Streaming**
- Only start streaming when actively displaying data
- Stop streaming when app goes to background
- Handle data buffer overflow in high-frequency streams
- Implement data averaging for smooth UI updates

### **8.5 Error Recovery**
- Implement retry logic for failed commands
- Parse error details to provide helpful user messages
- Automatically handle recoverable errors (time sync, etc.)
- Log errors for debugging but don't crash the app

---

## 🔍 **9. Troubleshooting**

### **Common Issues & Solutions**

| **Issue** | **Cause** | **Solution** |
|-----------|-----------|--------------|
| Connection fails | Device not discoverable | Check if ESP32 is advertising |
| No device_info received | Connection timeout | Wait longer or reconnect |
| TIME_NOT_SYNCED error | Historical data before sync | Call time sync first |
| STORAGE_ERROR | Historical data disabled | Enable historical data on ESP32 |
| NO_DATA response | Invalid time range | Check available data range |
| JSON parse errors | Corrupted messages | Implement message validation |
| Real-time stream stops | Bluetooth disconnection | Reconnect and restart stream |

### **Debug Commands**
```kotlin
// Check device status
val deviceInfo = JsonObject().apply {
    addProperty("type", "get_device_info")
    addProperty("request_id", "debug_001")
}

// Force time sync
val timeSync = JsonObject().apply {
    addProperty("type", "time_sync_set") 
    addProperty("request_id", "debug_002")
    addProperty("current_time", System.currentTimeMillis())
    addProperty("timezone_offset", "+0000")
}

// Get storage status
val storageCheck = JsonObject().apply {
    addProperty("type", "storage_info_request")
    addProperty("request_id", "debug_003")
}
```

---

This API specification provides everything needed to build a robust Android app that communicates seamlessly with your ESP32 CoToMeter device! 🚀
