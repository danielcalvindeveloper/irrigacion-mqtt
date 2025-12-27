#include "RelayController.h"

// ============================================================================
// Constructor
// ============================================================================
RelayController::RelayController() {
    lastUpdate = 0;    stateChangedCallback = nullptr;    
    // Inicializar arrays
    for (int i = 0; i < MAX_ZONES; i++) {
        zoneState[i] = false;
        zoneTimer[i] = 0;
    }
}

// ============================================================================
// Inicializacion
// ============================================================================
void RelayController::init() {
    // Los pines ya fueron inicializados en main.cpp initHardware()
    // Solo aseguramos que todo este apagado
    for (int i = 0; i < MAX_ZONES; i++) {
        digitalWrite(RELAY_PINS[i], RELAY_OFF);
        zoneState[i] = false;
        zoneTimer[i] = 0;
    }
}

// ============================================================================
// Encender zona
// ============================================================================
void RelayController::turnOn(int zona, int duracionSeg) {
    if (!isValidZone(zona)) return;
    
    int idx = zona - 1;
    
    // Validar duracion
    if (duracionSeg < MIN_RIEGO_DURATION || duracionSeg > MAX_RIEGO_DURATION) {
        Serial.printf("[WARN] Duracion invalida para zona %d: %d seg\n", zona, duracionSeg);
        return;
    }
    
    // Activar rele (logica invertida: LOW = ON)
    digitalWrite(RELAY_PINS[idx], RELAY_ON);
    zoneState[idx] = true;
    zoneTimer[idx] = duracionSeg;
    
    Serial.printf("[INFO] Zona %d activada por %d segundos\n", zona, duracionSeg);
}

// ============================================================================
// Apagar zona
// ============================================================================
void RelayController::turnOff(int zona) {
    if (!isValidZone(zona)) return;
    
    int idx = zona - 1;
    
    // Desactivar rele
    digitalWrite(RELAY_PINS[idx], RELAY_OFF);
    zoneState[idx] = false;
    zoneTimer[idx] = 0;
    
    Serial.printf("[INFO] Zona %d desactivada\n", zona);
}

// ============================================================================
// Loop - Actualizar timers
// ============================================================================
void RelayController::loop() {
    static unsigned long loopCount = 0;
    loopCount++;
    
    unsigned long now = millis();
    
    // Inicializar en el primer loop
    if (lastUpdate == 0) {
        Serial.println("[DEBUG] RelayController::loop() - Primera ejecucion, inicializando lastUpdate");
        lastUpdate = now;
        return;
    }
    
    // Actualizar cada segundo
    if (now - lastUpdate < 1000) return;
    
    // Log cada 10 segundos para verificar que se llama
    if (loopCount % 10 == 0) {
        Serial.printf("[DEBUG] RelayController::loop() ejecutandose (count: %lu)\n", loopCount);
    }
    
    unsigned long elapsed = (now - lastUpdate) / 1000;
    lastUpdate = now;
    
    // Debug: mostrar si hay timers activos
    bool hasActiveTimers = false;
    for (int i = 0; i < MAX_ZONES; i++) {
        if (zoneTimer[i] > 0) {
            hasActiveTimers = true;
            Serial.printf("[DEBUG] Zona %d timer: %d seg (elapsed: %lu)\n", i + 1, zoneTimer[i], elapsed);
        }
    }
    
    // Decrementar timers activos
    for (int i = 0; i < MAX_ZONES; i++) {
        if (zoneTimer[i] > 0) {
            if (zoneTimer[i] <= elapsed) {
                // Timer expirado - apagar zona
                zoneTimer[i] = 0;
                digitalWrite(RELAY_PINS[i], RELAY_OFF);
                zoneState[i] = false;
                Serial.printf("[INFO] Zona %d apagada automaticamente (timer expirado)\n", i + 1);
                
                // Notificar cambio de estado
                if (stateChangedCallback != nullptr) {
                    stateChangedCallback(i + 1, false);
                }
            } else {
                // Decrementar timer
                zoneTimer[i] -= elapsed;
                Serial.printf("[DEBUG] Zona %d nuevo timer: %d seg\n", i + 1, zoneTimer[i]);
            }
        }
    }
}

// ============================================================================
// Consultas de estado
// ============================================================================
bool RelayController::isActive(int zona) {
    if (!isValidZone(zona)) return false;
    return zoneState[zona - 1];
}

int RelayController::getRemainingTime(int zona) {
    if (!isValidZone(zona)) return 0;
    return zoneTimer[zona - 1];
}

// ============================================================================
// Parada de emergencia
// ============================================================================
void RelayController::emergencyStop() {
    Serial.println("[WARN] Parada de emergencia - apagando todas las zonas");
    
    for (int i = 0; i < MAX_ZONES; i++) {
        digitalWrite(RELAY_PINS[i], RELAY_OFF);
        zoneState[i] = false;
        zoneTimer[i] = 0;
    }
}

// ============================================================================
// Validacion
// ============================================================================
bool RelayController::isValidZone(int zona) {
    if (zona < 1 || zona > MAX_ZONES) {
        Serial.printf("[ERROR] Zona invalida: %d (rango: 1-%d)\n", zona, MAX_ZONES);
        return false;
    }
    return true;
}

// ============================================================================
// Registrar callback para cambios de estado
// ============================================================================
void RelayController::setStateChangedCallback(ZoneStateChangedCallback callback) {
    stateChangedCallback = callback;
}
