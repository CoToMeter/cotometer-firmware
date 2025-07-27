#pragma once
#include "interfaces/ISensor.h"
#include "interfaces/IDisplay.h"
#include "types/SensorData.h"
#include <memory>

class CoToMeterController {
private:
    std::unique_ptr<ISensor> sensor;
    std::unique_ptr<IDisplay> display;
    
    uint32_t lastMeasurement;
    uint32_t measurementInterval;
    
    // Button handling
    // const gpio_num_t BUTTON_PIN = GPIO_NUM_0;
    // volatile bool buttonPressed;
    // static void IRAM_ATTR buttonISR();
    static CoToMeterController* instance; // For ISR callback
    
public:
    CoToMeterController();
    ~CoToMeterController() = default;
    
    bool initialize();
    void loop();
    void handleButtonPress();
    
    // Delete copy constructor and assignment
    CoToMeterController(const CoToMeterController&) = delete;
    CoToMeterController& operator=(const CoToMeterController&) = delete;
};
