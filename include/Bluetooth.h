#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include <Arduino.h>

class Bluetooth {
public:
    // Constructor accepting the device name
    Bluetooth(const char* deviceName);
    
    // Initialize Bluetooth
    void begin();
    
    // Send a message via Bluetooth
    void sendMessage(const String& message);
    
    // Receive a message via Bluetooth
    bool receiveMessage(String& message);
    
    // Check if a Bluetooth client is connected
    bool isConnected();

private:
    BluetoothSerial SerialBT;   // BluetoothSerial object
    const char* _deviceName;    // Device name
};

#endif // BLUETOOTH_H