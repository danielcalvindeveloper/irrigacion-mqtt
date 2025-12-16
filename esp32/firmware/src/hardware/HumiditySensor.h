#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <Arduino.h>
#include "../config/Config.h"

// ============================================================================
// HumiditySensor - Lectura de sensores capacitivos de humedad
// ============================================================================
// Lee valores ADC de sensores capacitivos y los convierte a porcentaje de
// humedad usando calibración configurada en Config.h

class HumiditySensor {
private:
    int pin;           // Pin ADC del sensor
    int zona;          // Número de zona asociada
    int lastReading;   // Última lectura raw
    int lastPercent;   // Último porcentaje calculado
    
    // Valores de calibración (pueden ser personalizados por sensor)
    int wetValue;
    int dryValue;

public:
    // Constructor
    HumiditySensor();
    
    // Inicializar sensor con pin y zona
    void init(int adcPin, int zoneNumber);
    
    // Leer valor ADC raw
    int readRaw();
    
    // Leer y convertir a porcentaje (0-100%)
    int readPercent();
    
    // Calibrar sensor con valores específicos
    void calibrate(int wet, int dry);
    
    // Obtener última lectura sin leer sensor
    int getLastRaw();
    int getLastPercent();
    
    // Verificar si sensor está conectado (detecta valores fuera de rango)
    bool isConnected();
    
    // Obtener zona asociada
    int getZone();
};

#endif // HUMIDITY_SENSOR_H
