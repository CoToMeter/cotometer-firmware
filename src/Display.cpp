// #include "Display.h"

// Display::Display(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t busy)
//     : display(GxEPD2_290(cs, dc, rst, busy)) {}

// bool Display::begin() {
//     Serial.println("Initializing E-Ink Display...");
//     display.init();

//     Serial.println("E-Ink Display Initialized Successfully.");
//     display.setRotation(1); // Adjust rotation if needed
//     display.setFont(&FreeMonoBold9pt7b);
//     display.setTextColor(GxEPD_BLACK);
//     return true;
// }

// void Display::showInitializing() {
//     Serial.println("Displaying Initial Messages on E-Ink Display...");
//     display.firstPage();
//     do {
//         display.fillScreen(GxEPD_WHITE);
//         display.setCursor(10, 30);
//         display.println("CoToMeter");
//         display.println("Initializing...");
//     } while (display.nextPage());
//     Serial.println("Initial Messages Displayed.");
// }

// void Display::showReadings(uint16_t co2, float temperature, float humidity) {
//     Serial.println("Updating E-Ink Display with Sensor Data...");
//     display.firstPage();
//     do {
//         display.fillScreen(GxEPD_WHITE); // Clear the display buffer

//         display.setCursor(10, 30);
//         display.println("CoToMeter Readings");
//         display.println("---------------------");
//         display.setCursor(10, 60);
//         display.print("CO2: ");
//         display.print(co2);
//         display.println(" ppm");
//         display.print("Temp: ");
//         display.print(temperature);
//         display.println(" C");
//         display.print("Humidity: ");
//         display.print(humidity);
//         display.println(" %RH");
//     } while (display.nextPage());
//     Serial.println("E-Ink Display Updated.");
// }

// void Display::clear() {
//     display.firstPage();
//     do {
//         display.fillScreen(GxEPD_WHITE);
//     } while (display.nextPage());
//     Serial.println("E-Ink Display Cleared.");
// }