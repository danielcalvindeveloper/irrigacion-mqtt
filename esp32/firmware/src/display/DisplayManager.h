#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================================
// DisplayManager - Gestor de pantalla OLED I2C
// ============================================================================
class DisplayManager {
private:
    Adafruit_SSD1306* oledDisplay;
    bool initialized;
    
    int screenWidth;
    int screenHeight;
    uint8_t i2cAddress;
    
    int lastWifiRssi;
    bool lastWifiConnected;
    bool lastMqttConnected;
    
    // Para alternar fecha/hora
    unsigned long lastDateTimeToggle;
    bool showingDate;
    
public:
    DisplayManager();
    ~DisplayManager();
    
    bool init(int sda, int scl, uint8_t address, int width, int height);
    bool isInitialized();
    
    void clear();
    void display();
    
    void showMessage(const char* message, int textSize = 1);
    void showTwoLines(const char* line1, const char* line2);
    void showStatus(const char* title, const char* status);
    
    void drawText(const char* text, int x, int y, int size = 1);
    void drawCenteredText(const char* text, int y, int size = 1);
    
    // Iconos de estado
    void drawWiFiIcon(int rssi, bool connected);
    void drawMqttIcon(bool connected);
    void updateStatusIcons(int wifiRssi, bool wifiConnected, bool mqttConnected);
    
    // Fecha y hora en centro superior
    void updateDateTimeDisplay(int dayOfWeek, int dayOfMonth, int hour, int minute);
    
    // Línea de estado inferior
    void showStatusLine(const char* message);
    void clearStatusLine();
    
    // Indicadores de zonas
    void drawZoneIndicators(bool zone1, bool zone2, bool zone3, bool zone4);
    void updateZoneIndicators(bool zone1, bool zone2, bool zone3, bool zone4);
    
    void setBrightness(uint8_t brightness);
    void powerOff();
    void powerOn();
};

#endif
