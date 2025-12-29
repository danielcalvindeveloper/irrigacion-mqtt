#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "../config/Config.h"

// ============================================================================
// RelayController - Control de relés con timers automáticos
// ============================================================================
// Gestiona el estado de hasta 8 relés (zonas de riego) con temporizadores
// automáticos que apagan los relés al finalizar el tiempo configurado.

// Forward declaration para callback
typedef void (*ZoneStateChangedCallback)(int zona, bool estado);
typedef void (*RiegoEventCallback)(int zona, String evento, String origen, int duracion, int versionAgenda);

class RelayController {
private:
    // Estado de cada zona (true = activa, false = inactiva)
    bool zoneState[MAX_ZONES];
    
    // Timer de cada zona en segundos (0 = sin timer activo)
    int zoneTimer[MAX_ZONES];
    
    // Duración programada inicial de cada zona
    int zoneDuracionProgramada[MAX_ZONES];
    
    // Origen del riego: "manual" o "agenda"
    String zoneOrigen[MAX_ZONES];
    
    // Versión de agenda (0 si es manual)
    int zoneVersionAgenda[MAX_ZONES];
    
    // Último update de timers
    unsigned long lastUpdate;
    
    // Callbacks
    ZoneStateChangedCallback stateChangedCallback;
    RiegoEventCallback riegoEventCallback;

public:
    // Constructor
    RelayController();
    
    // Inicializar pines de relés
    void init();
    
    // Encender zona con duración específica (segundos)
    void turnOn(int zona, int duracionSeg, String origen = "manual", int versionAgenda = 0);
    
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
    
    // Registrar callback para cambios de estado
    void setStateChangedCallback(ZoneStateChangedCallback callback);
    
    // Registrar callback para eventos de riego
    void setRiegoEventCallback(RiegoEventCallback callback);
};

#endif // RELAY_CONTROLLER_H
