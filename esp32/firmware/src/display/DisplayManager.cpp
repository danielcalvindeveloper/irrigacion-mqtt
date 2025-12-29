#include "DisplayManager.h"
#include "../utils/Logger.h"

// ============================================================================
// Constructor y Destructor
// ============================================================================
DisplayManager::DisplayManager() {
    oledDisplay = nullptr;
    initialized = false;
    screenWidth = 128;
    screenHeight = 64;
    i2cAddress = 0x3C;
    lastWifiRssi = -100;
    lastWifiConnected = false;
    lastMqttConnected = false;
    lastDateTimeToggle = 0;
    showingDate = true;
}

DisplayManager::~DisplayManager() {
    if (oledDisplay != nullptr) {
        delete oledDisplay;
    }
}

// ============================================================================
// Inicialización
// ============================================================================
bool DisplayManager::init(int sda, int scl, uint8_t address, int width, int height) {
    Logger::info("Inicializando DisplayManager...");
    
    screenWidth = width;
    screenHeight = height;
    i2cAddress = address;
    
    Wire.begin(sda, scl);
    
    oledDisplay = new Adafruit_SSD1306(screenWidth, screenHeight, &Wire, -1);
    
    if (!oledDisplay->begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        Logger::logf(LOG_LEVEL_ERROR, "Fallo init SSD1306 en direccion 0x%02X", i2cAddress);
        initialized = false;
        return false;
    }
    
    Logger::logf(LOG_LEVEL_INFO, "Display SSD1306 inicializado correctamente (0x%02X)", i2cAddress);
    
    oledDisplay->clearDisplay();
    oledDisplay->setTextColor(SSD1306_WHITE);
    oledDisplay->setTextWrap(true);
    oledDisplay->display();
    
    initialized = true;
    return true;
}

bool DisplayManager::isInitialized() {
    return initialized;
}

// ============================================================================
// Control básico
// ============================================================================
void DisplayManager::clear() {
    if (!initialized) return;
    oledDisplay->clearDisplay();
}

void DisplayManager::display() {
    if (!initialized) return;
    oledDisplay->display();
}

// ============================================================================
// Funciones de texto
// ============================================================================
void DisplayManager::showMessage(const char* message, int textSize) {
    if (!initialized) return;
    
    oledDisplay->clearDisplay();
    oledDisplay->setTextSize(textSize);
    oledDisplay->setCursor(0, 0);
    oledDisplay->println(message);
    oledDisplay->display();
}

void DisplayManager::showTwoLines(const char* line1, const char* line2) {
    if (!initialized) return;
    
    oledDisplay->clearDisplay();
    
    oledDisplay->setTextSize(2);
    oledDisplay->setCursor(0, 10);
    oledDisplay->println(line1);
    
    oledDisplay->setTextSize(1);
    oledDisplay->setCursor(0, 40);
    oledDisplay->println(line2);
    
    oledDisplay->display();
}

void DisplayManager::showStatus(const char* title, const char* status) {
    if (!initialized) return;
    
    oledDisplay->clearDisplay();
    
    oledDisplay->setTextSize(1);
    oledDisplay->setCursor(0, 0);
    oledDisplay->println(title);
    
    oledDisplay->drawLine(0, 10, screenWidth, 10, SSD1306_WHITE);
    
    oledDisplay->setTextSize(2);
    oledDisplay->setCursor(0, 20);
    oledDisplay->println(status);
    
    oledDisplay->display();
}

void DisplayManager::drawText(const char* text, int x, int y, int size) {
    if (!initialized) return;
    
    oledDisplay->setTextSize(size);
    oledDisplay->setCursor(x, y);
    oledDisplay->print(text);
}

void DisplayManager::drawCenteredText(const char* text, int y, int size) {
    if (!initialized) return;
    
    oledDisplay->setTextSize(size);
    int16_t x1, y1;
    uint16_t w, h;
    oledDisplay->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    int x = (screenWidth - w) / 2;
    oledDisplay->setCursor(x, y);
    oledDisplay->print(text);
}

// ============================================================================
// Iconos de estado
// ============================================================================
void DisplayManager::drawWiFiIcon(int rssi, bool connected) {
    if (!initialized) return;
    
    // Posición esquina superior derecha
    int x = screenWidth - 16;
    int y = 1;
    
    if (!connected) {
        // WiFi desconectado - solo contorno
        oledDisplay->drawPixel(x + 7, y + 10, SSD1306_WHITE);
        oledDisplay->drawLine(x + 4, y + 7, x + 10, y + 7, SSD1306_WHITE);
        oledDisplay->drawLine(x + 2, y + 4, x + 12, y + 4, SSD1306_WHITE);
        oledDisplay->drawLine(x, y + 1, x + 14, y + 1, SSD1306_WHITE);
    } else {
        // WiFi conectado - con nivel de señal
        // Punto central
        oledDisplay->fillCircle(x + 7, y + 10, 1, SSD1306_WHITE);
        
        // Calcular nivel de señal (1-4 barras)
        int level = 0;
        if (rssi > -50) level = 4;
        else if (rssi > -60) level = 3;
        else if (rssi > -70) level = 2;
        else if (rssi > -80) level = 1;
        else level = 1;
        
        // Barra 1 (más cercana)
        if (level >= 1) {
            oledDisplay->drawLine(x + 4, y + 7, x + 10, y + 7, SSD1306_WHITE);
            oledDisplay->drawLine(x + 5, y + 8, x + 9, y + 8, SSD1306_WHITE);
        } else {
            oledDisplay->drawLine(x + 4, y + 7, x + 10, y + 7, SSD1306_WHITE);
        }
        
        // Barra 2
        if (level >= 2) {
            oledDisplay->drawLine(x + 2, y + 4, x + 12, y + 4, SSD1306_WHITE);
            oledDisplay->drawLine(x + 3, y + 5, x + 11, y + 5, SSD1306_WHITE);
        } else {
            oledDisplay->drawLine(x + 2, y + 4, x + 12, y + 4, SSD1306_WHITE);
        }
        
        // Barra 3
        if (level >= 3) {
            oledDisplay->drawLine(x, y + 1, x + 14, y + 1, SSD1306_WHITE);
            oledDisplay->drawLine(x + 1, y + 2, x + 13, y + 2, SSD1306_WHITE);
        } else {
            oledDisplay->drawLine(x, y + 1, x + 14, y + 1, SSD1306_WHITE);
        }
        
        // Barra 4 (más lejana, señal excelente)
        if (level >= 4) {
            for (int i = 0; i < 3; i++) {
                oledDisplay->drawPixel(x - 2 + i, y, SSD1306_WHITE);
                oledDisplay->drawPixel(x + 14 - i, y, SSD1306_WHITE);
            }
        }
    }
}

void DisplayManager::drawMqttIcon(bool connected) {
    if (!initialized) return;
    
    // Posición esquina superior izquierda
    int x = 2;
    int y = 2;
    
    if (!connected) {
        // MQTT desconectado - M con contorno
        oledDisplay->drawLine(x, y, x, y + 9, SSD1306_WHITE);
        oledDisplay->drawLine(x, y, x + 3, y + 3, SSD1306_WHITE);
        oledDisplay->drawLine(x + 3, y + 3, x + 6, y, SSD1306_WHITE);
        oledDisplay->drawLine(x + 6, y, x + 6, y + 9, SSD1306_WHITE);
        
        // X pequeña indicando desconexión
        oledDisplay->drawLine(x + 8, y + 4, x + 11, y + 7, SSD1306_WHITE);
        oledDisplay->drawLine(x + 11, y + 4, x + 8, y + 7, SSD1306_WHITE);
    } else {
        // MQTT conectado - M rellena
        oledDisplay->drawLine(x, y, x, y + 9, SSD1306_WHITE);
        oledDisplay->drawLine(x + 1, y, x + 1, y + 9, SSD1306_WHITE);
        oledDisplay->drawLine(x, y, x + 3, y + 3, SSD1306_WHITE);
        oledDisplay->drawLine(x + 1, y + 1, x + 3, y + 3, SSD1306_WHITE);
        oledDisplay->drawLine(x + 3, y + 3, x + 6, y, SSD1306_WHITE);
        oledDisplay->drawLine(x + 4, y + 3, x + 6, y + 1, SSD1306_WHITE);
        oledDisplay->drawLine(x + 6, y, x + 6, y + 9, SSD1306_WHITE);
        oledDisplay->drawLine(x + 5, y, x + 5, y + 9, SSD1306_WHITE);
        
        // Checkmark indicando conexión
        oledDisplay->drawLine(x + 8, y + 5, x + 9, y + 7, SSD1306_WHITE);
        oledDisplay->drawLine(x + 9, y + 7, x + 12, y + 3, SSD1306_WHITE);
    }
}

void DisplayManager::updateStatusIcons(int wifiRssi, bool wifiConnected, bool mqttConnected) {
    if (!initialized) return;
    
    lastWifiRssi = wifiRssi;
    lastWifiConnected = wifiConnected;
    lastMqttConnected = mqttConnected;
    
    drawWiFiIcon(wifiRssi, wifiConnected);
    drawMqttIcon(mqttConnected);
}

// ============================================================================
// Fecha y hora en centro superior
// ============================================================================
void DisplayManager::updateDateTimeDisplay(int dayOfWeek, int dayOfMonth, int hour, int minute) {
    if (!initialized) return;
    
    unsigned long now = millis();
    
    // Alternar cada 5 segundos
    if (now - lastDateTimeToggle >= 5000) {
        lastDateTimeToggle = now;
        showingDate = !showingDate;
    }
    
    // Limpiar área central superior (entre los iconos)
    // Iconos WiFi (izq): x=0-15, MQTT (der): x=113-128
    // Área disponible: x=20 a x=108 (88 píxeles de ancho)
    oledDisplay->fillRect(20, 0, 88, 10, SSD1306_BLACK);
    
    char buffer[12];
    
    if (showingDate) {
        // Mostrar día de la semana (3 letras) + número de día
        const char* dias[] = {"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"};
        snprintf(buffer, sizeof(buffer), "%s %02d", dias[dayOfWeek % 7], dayOfMonth);
    } else {
        // Mostrar hora:minuto
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    }
    
    // Centrar el texto en el área disponible
    oledDisplay->setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    oledDisplay->getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    
    // Calcular posición centrada en el área entre iconos
    int x = 20 + (88 - w) / 2;  // Centro del área disponible
    
    oledDisplay->setCursor(x, 1);
    oledDisplay->print(buffer);
}

// ============================================================================
// Línea de estado inferior
// ============================================================================
void DisplayManager::showStatusLine(const char* message) {
    if (!initialized) return;
    
    // Limpiar la línea inferior (últimos 12 píxeles)
    oledDisplay->fillRect(0, screenHeight - 12, screenWidth, 12, SSD1306_BLACK);
    
    // Dibujar una línea separadora
    oledDisplay->drawLine(0, screenHeight - 13, screenWidth, screenHeight - 13, SSD1306_WHITE);
    
    // Mostrar el mensaje centrado
    oledDisplay->setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    oledDisplay->getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
    int x = (screenWidth - w) / 2;
    if (x < 0) x = 0;
    
    oledDisplay->setCursor(x, screenHeight - 9);
    oledDisplay->print(message);
}

void DisplayManager::clearStatusLine() {
    if (!initialized) return;
    
    // Limpiar la línea inferior y la separadora
    oledDisplay->fillRect(0, screenHeight - 13, screenWidth, 13, SSD1306_BLACK);
}

// ============================================================================
// Indicadores de zonas
// ============================================================================
void DisplayManager::drawZoneIndicators(bool zone1, bool zone2, bool zone3, bool zone4) {
    if (!initialized) return;
    
    // Calcular posiciones centradas (área entre los iconos superiores y línea inferior)
    // Área disponible: Y desde 15 hasta screenHeight-15 (aprox 34 píxeles de altura)
    int startY = 18;
    int boxSize = 20;
    int spacing = 6;
    int totalWidth = (boxSize * 4) + (spacing * 3);
    int startX = (screenWidth - totalWidth) / 2;
    
    // Limpiar área central
    oledDisplay->fillRect(0, startY - 2, screenWidth, boxSize + 8, SSD1306_BLACK);
    
    // Zona 1
    int x1 = startX;
    if (zone1) {
        oledDisplay->fillRect(x1, startY, boxSize, boxSize, SSD1306_WHITE);
    } else {
        oledDisplay->drawRect(x1, startY, boxSize, boxSize, SSD1306_WHITE);
    }
    // Número de zona
    oledDisplay->setTextSize(1);
    oledDisplay->setCursor(x1 + 7, startY + boxSize + 2);
    oledDisplay->print("1");
    
    // Zona 2
    int x2 = x1 + boxSize + spacing;
    if (zone2) {
        oledDisplay->fillRect(x2, startY, boxSize, boxSize, SSD1306_WHITE);
    } else {
        oledDisplay->drawRect(x2, startY, boxSize, boxSize, SSD1306_WHITE);
    }
    oledDisplay->setCursor(x2 + 7, startY + boxSize + 2);
    oledDisplay->print("2");
    
    // Zona 3
    int x3 = x2 + boxSize + spacing;
    if (zone3) {
        oledDisplay->fillRect(x3, startY, boxSize, boxSize, SSD1306_WHITE);
    } else {
        oledDisplay->drawRect(x3, startY, boxSize, boxSize, SSD1306_WHITE);
    }
    oledDisplay->setCursor(x3 + 7, startY + boxSize + 2);
    oledDisplay->print("3");
    
    // Zona 4
    int x4 = x3 + boxSize + spacing;
    if (zone4) {
        oledDisplay->fillRect(x4, startY, boxSize, boxSize, SSD1306_WHITE);
    } else {
        oledDisplay->drawRect(x4, startY, boxSize, boxSize, SSD1306_WHITE);
    }
    oledDisplay->setCursor(x4 + 7, startY + boxSize + 2);
    oledDisplay->print("4");
}

void DisplayManager::updateZoneIndicators(bool zone1, bool zone2, bool zone3, bool zone4) {
    if (!initialized) return;
    drawZoneIndicators(zone1, zone2, zone3, zone4);
}

// ============================================================================
// Control de brillo y poder
// ============================================================================
void DisplayManager::setBrightness(uint8_t brightness) {
    if (!initialized) return;
    oledDisplay->dim(brightness < 128);
}

void DisplayManager::powerOff() {
    if (!initialized) return;
    oledDisplay->ssd1306_command(SSD1306_DISPLAYOFF);
}

void DisplayManager::powerOn() {
    if (!initialized) return;
    oledDisplay->ssd1306_command(SSD1306_DISPLAYON);
}
