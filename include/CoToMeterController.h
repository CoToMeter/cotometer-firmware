#pragma once
#include "interfaces/ISensor.h"
#include "interfaces/IDisplay.h"
#include "communication/BluetoothComm.h"
#include "types/SensorData.h"
#include <memory>
#include <vector>

class CoToMeterController {
private:
    std::vector<std::unique_ptr<ISensor>> sensors;
    std::unique_ptr<IDisplay> display;
    std::unique_ptr<ICommunication> communication;

    uint32_t lastMeasurement;
    uint32_t measurementInterval;
    
    // Sensor data storage
    CO2SensorData* co2Data;
    VOCSensorData* vocData;
    
    // Helper methods
    void printCombinedData();
    void checkAlerts();
    String getCombinedCatMood();
    
public:
    CoToMeterController();
    ~CoToMeterController() = default;
    
    bool initialize();
    void loop();
    
    // Data access
    CO2SensorData* getCO2Data() { return co2Data; }
    VOCSensorData* getVOCData() { return vocData; }
    
    ICommunication* getCommunication() { return communication.get(); }
    bool isCommunicationConnected() { 
        return communication && communication->isConnected(); 
    }


    // Delete copy constructor and assignment
    CoToMeterController(const CoToMeterController&) = delete;
    CoToMeterController& operator=(const CoToMeterController&) = delete;
};
