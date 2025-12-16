#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "../config/Config.h"

// ============================================================================
// RelayController - Control de relés con timers automáticos
// ============================================================================
// Gestiona el estado de hasta 8 relés (zonas de riego) con temporizadores
// automáticos que apagan los relés al finalizar el tiempo configurado.

class RelayController {
private:
    // Estado de cada zona (true = activa, false = inactiva)
    bool zoneState[MAX_ZONES];
    
    // Timer de cada zona en segundos (0 = sin timer activo)
    int zoneTimer[MAX_ZONES];
    
    // Último update de timers
    unsigned long lastUpdate;

public:
    // Constructor
    RelayController();
    
    // Inicializar pines de relés
    void init();
    
    // Encender zona con duración específica (segundos)
    void turnOn(int zona, int duracionSeg);
    
    // Apagar zona inmediatamente
    void turnOff(int zona);
    
    // Actualizar timers (llamar cada segundo en loop)
    void loop();
    
    // Consultar estado de zona
    bool isActive(int zona);
    
    // Obtener tiempo restante de zona (segundos)
    int getRemainingTime(int zona);
    
    // Apagar todas las zonas (emergencia)
    void emergencyStop();
    
    // Validar número de zona (1-8)
    bool isValidZone(int zona);
};

#endif // RELAY_CONTROLLER_H
