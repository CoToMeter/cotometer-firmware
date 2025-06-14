#include <Arduino.h>
#include <Wire.h>

// Include the module headers
#include "Bluetooth.h"
#include "SensorCO2.h"
// #include "CCS811Sensor.h"
//#include "Display.h"

// Define pin connections for E-Ink Display
#define EPD_CS     5    // Chip Select
#define EPD_DC     33   // Data/Command Control
#define EPD_RST    25   // Reset
#define EPD_BUSY   4    // Busy Indicator

// Define LED pin (optional)
#define STATUS_LED 2

// Instantiate modules
Bluetooth bluetooth("CoToMeter");
SensorCO2 sensor;
//Display display(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY);
// CCS811Sensor ccs811; // Instantiate CCS811Sensor

void setup() {
    // Initialize Serial Monitor
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect (only necessary for native USB)
    }
    delay(5000);
    Serial.println("CoToMeter Initialization Started");

    // Initialize I2C (Wire) on GPIO 21 (SDA) and GPIO 22 (SCL)
    Wire.begin(21, 22);
    Serial.println("I2C Initialized");

    // Initialize the built-in LED pin as an output (optional)
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, LOW); // Turn LED off initially
    Serial.println("Status LED Initialized");

    // Initialize Sensor
    if (sensor.begin()) {
        sensor.setMeasurementInterval(5); // Set to 5 seconds
        sensor.enableAutoSelfCalibration(true);
    } else {
        Serial.println("Sensor initialization failed. Halting...");
        while (1);
    }

    // Initialize CCS811 Sensor
    // if (ccs811.begin()) {
    //     Serial.println("CCS811 Sensor initialized successfully.");
    // } else {
    //     Serial.println("CCS811 Sensor initialization failed. Halting...");
    //     while (1);
    // }

    // Initialize Display
    // if (display.begin()) {
    //     display.showInitializing();
    // } else {
    //     Serial.println("Display initialization failed. Halting...");
    //     while (1);
    // }

    // Initialize Bluetooth
    bluetooth.begin();
}

void loop() {
    // Check if new data is available from SensorCO2
    uint16_t co2 = 0;
    float temperature = 0.0;
    float humidity = 0.0;

    if (sensor.readData(co2, temperature, humidity)) {
        // Log data to Serial Monitor
        Serial.print("SensorCO2 - CO2: ");
        Serial.print(co2);
        Serial.print(" ppm, Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Humidity: ");
        Serial.print(humidity);
        Serial.println(" %RH");

        // Send data via Bluetooth
        String message = "SensorCO2 - CO2: " + String(co2) + " ppm, Temp: " + String(temperature) + " C, Humidity: " + String(humidity) + " %RH";

        // Read data from CCS811
        uint16_t eCO2 = 0;
        float TVOC = 0.0;
        // if (ccs811.readData(eCO2, TVOC)) {
        //     Serial.print("CCS811 - eCO2: ");
        //     Serial.print(eCO2);
        //     Serial.print(" ppm, TVOC: ");
        //     Serial.print(TVOC);
        //     Serial.println(" ppb");

        //     // Append CCS811 data to the Bluetooth message
        //     message += "\nCCS811 - eCO2: " + String(eCO2) + " ppm, TVOC: " + String(TVOC) + " ppb";
        // } else {
        //     Serial.println("CCS811 - No data available or read failed.");
        //     message += "\nCCS811 - No data available.";
        // }

        bluetooth.sendMessage(message);

        // Optional: Blink the LED to indicate successful reading
        digitalWrite(STATUS_LED, HIGH);
        delay(100);
        digitalWrite(STATUS_LED, LOW);
    }

    // Handle Bluetooth received messages
    String receivedMessage;
    if (bluetooth.receiveMessage(receivedMessage)) {
        // Process the received message
        receivedMessage.trim(); // Remove any trailing newline or spaces
        Serial.println("Processing received Bluetooth message: " + receivedMessage);

        if (receivedMessage.equalsIgnoreCase("LED ON")) {
            digitalWrite(STATUS_LED, HIGH);
            bluetooth.sendMessage("LED turned ON");
        }
        else if (receivedMessage.equalsIgnoreCase("LED OFF")) {
            digitalWrite(STATUS_LED, LOW);
            bluetooth.sendMessage("LED turned OFF");
        }
        else {
            bluetooth.sendMessage("Unknown Command");
        }
    }

    // Small delay to prevent overwhelming the loop
    delay(100);
}
