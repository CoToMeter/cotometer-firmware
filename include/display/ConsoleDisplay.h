#pragma once
#include "../interfaces/IDisplay.h"

class ConsoleDisplay : public IDisplay {
private:
    void printCatHeader();
    void printSeparator();
    String getCatMood(AlertLevel level);
    
public:
    ConsoleDisplay() = default;
    ~ConsoleDisplay() = default;
    
    bool initialize() override;
    void showSensorData(const SensorDataBase& data) override;
    void showMessage(const String& message) override;
    void showError(const String& error) override;
};
