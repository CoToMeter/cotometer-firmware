#include "Bluetooth.h"

// Constructor implementation
Bluetooth::Bluetooth(const char* deviceName) : _deviceName(deviceName) {}

// Initialize Bluetooth
void Bluetooth::begin() {
    Serial.println("Initializing Bluetooth...");
    if (!SerialBT.begin(_deviceName)) { // Start Bluetooth with the device name
        Serial.println("An error occurred initializing Bluetooth");
    } else {
        Serial.println("Bluetooth initialized successfully");
        Serial.println("Device Name: " + String(_deviceName));
        Serial.println("You can now pair your Android device with ESP32");
    }
}

// Send a message via Bluetooth
void Bluetooth::sendMessage(const String& message) {
    if (SerialBT.hasClient()) {
        SerialBT.println(message);
        Serial.println("Sent via Bluetooth: " + message);
    } else {
        Serial.println("No Bluetooth client connected. Unable to send message.");
    }
}

// Receive a message via Bluetooth
bool Bluetooth::receiveMessage(String& message) {
    if (SerialBT.available()) {
        message = SerialBT.readStringUntil('\n');
        message.trim(); // Remove any trailing whitespace or newline characters
        Serial.println("Received via Bluetooth: " + message);
        return true;
    }
    return false;
}

// Check if a Bluetooth client is connected
bool Bluetooth::isConnected() {
    return SerialBT.hasClient();
}