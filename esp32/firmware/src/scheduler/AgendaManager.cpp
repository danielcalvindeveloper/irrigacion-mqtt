#include "AgendaManager.h"
#include "../storage/SPIFFSManager.h"
#include "../network/TimeSync.h"
#include "../hardware/RelayController.h"
#include "../utils/Logger.h"
#include <time.h>

// ============================================================================
// Constructor y Destructor
// ============================================================================
AgendaManager::AgendaManager(SPIFFSManager* spiffs, TimeSync* timeSync, RelayController* relay) {
    spiffsManager = spiffs;
    timeSyncManager = timeSync;
    relayController = relay;
    enabled = true;
    lastCheckTime = 0;
    lastMinuteChecked = -1;
}

AgendaManager::~AgendaManager() {
}

// ============================================================================
// Inicialización
// ============================================================================
void AgendaManager::init() {
    Logger::info("Inicializando AgendaManager...");
    enabled = true;
    lastCheckTime = 0;
    lastMinuteChecked = -1;
    Logger::info("AgendaManager inicializado correctamente");
}

// ============================================================================
// Loop principal
// ============================================================================
void AgendaManager::loop() {
    if (!enabled) return;
    
    unsigned long now = millis();
    
    // Verificar agendas cada CHECK_INTERVAL
    if (now - lastCheckTime >= CHECK_INTERVAL) {
        lastCheckTime = now;
        checkAndExecuteAgendas();
    }
}

// ============================================================================
// Verificar y ejecutar agendas
// ============================================================================
void AgendaManager::checkAndExecuteAgendas() {
    // Verificar que la hora esté sincronizada
    if (!timeSyncManager->isSynchronized()) {
        return;
    }
    
    // Obtener tiempo actual
    time_t now = timeSyncManager->getEpoch();
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    int currentDayOfWeek = timeinfo.tm_wday; // 0=DOM, 1=LUN, ..., 6=SAB
    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;
    
    // Evitar ejecutar múltiples veces en el mismo minuto
    int currentMinuteId = currentHour * 60 + currentMinute;
    if (currentMinuteId == lastMinuteChecked) {
        return;
    }
    lastMinuteChecked = currentMinuteId;
    
    Logger::logf(LOG_LEVEL_DEBUG, "Verificando agendas: %s %02d:%02d", 
                 getDayOfWeekString(currentDayOfWeek).c_str(), currentHour, currentMinute);
    
    // Leer archivo de agendas
    String jsonContent = spiffsManager->readFile("/agenda.json");
    if (jsonContent.length() == 0) {
        Logger::debug("No hay agendas en SPIFFS");
        return;
    }
    
    // Parsear JSON (usar DynamicJsonDocument dimensionado según el tamaño del payload)
    DynamicJsonDocument doc(jsonContent.length() + 1024);
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Logger::logf(LOG_LEVEL_ERROR, "Error parseando agendas: %s", error.c_str());
        return;
    }
    
    // Verificar que exista el array de agendas
    if (!doc.containsKey("agendas")) {
        Logger::warn("JSON no contiene campo 'agendas'");
        return;
    }
    
    JsonArray agendas = doc["agendas"].as<JsonArray>();
    
    // Obtener versión de la agenda si existe
    int versionAgenda = doc["version"] | 0;
    
    // Iterar sobre cada agenda
    for (JsonObject agenda : agendas) {
        if (shouldExecuteAgenda(agenda, currentDayOfWeek, currentHour, currentMinute)) {
            int zona = agenda["zona"] | 0;
            int duracionMin = agenda["duracionMin"] | 0;
            String id = agenda["id"] | "unknown";
            
            Logger::logf(LOG_LEVEL_INFO, "Ejecutando agenda [%s]: Zona %d por %d minutos (version %d)", 
                         id.c_str(), zona, duracionMin, versionAgenda);
            
            // Activar zona con origen "agenda" y versión
            int duracionSeg = duracionMin * 60;
            relayController->turnOn(zona, duracionSeg, "agenda", versionAgenda);
        }
    }
}

// ============================================================================
// Verificar si una agenda debe ejecutarse
// ============================================================================
bool AgendaManager::shouldExecuteAgenda(JsonObject agenda, int currentDayOfWeek, int currentHour, int currentMinute) {
    // Verificar que la agenda esté activa
    bool activa = agenda["activa"] | false;
    if (!activa) {
        return false;
    }
    
    // Verificar día de la semana
    JsonArray diasSemana = agenda["diasSemana"];
    if (!diasSemana) {
        return false;
    }
    
    String currentDay = getDayOfWeekString(currentDayOfWeek);
    bool diaCoincide = false;
    
    for (JsonVariant dia : diasSemana) {
        String diaStr = dia.as<String>();
        if (diaStr.equalsIgnoreCase(currentDay)) {
            diaCoincide = true;
            break;
        }
    }
    
    if (!diaCoincide) {
        return false;
    }
    
    // Verificar hora
    String horaInicio = agenda["horaInicio"] | "";
    if (horaInicio.length() == 0) {
        return false;
    }
    
    // Parsear hora (formato "HH:MM")
    int separatorPos = horaInicio.indexOf(':');
    if (separatorPos < 0) {
        return false;
    }
    
    int agendaHour = horaInicio.substring(0, separatorPos).toInt();
    int agendaMinute = horaInicio.substring(separatorPos + 1).toInt();
    
    // Comparar hora y minuto
    if (agendaHour == currentHour && agendaMinute == currentMinute) {
        return true;
    }
    
    return false;
}

// ============================================================================
// Convertir día de la semana a string
// ============================================================================
String AgendaManager::getDayOfWeekString(int dayOfWeek) {
    switch (dayOfWeek) {
        case 0: return "DOM";
        case 1: return "LUN";
        case 2: return "MAR";
        case 3: return "MIE";
        case 4: return "JUE";
        case 5: return "VIE";
        case 6: return "SAB";
        default: return "???";
    }
}

// ============================================================================
// Control de ejecución
// ============================================================================
void AgendaManager::enable() {
    enabled = true;
    lastMinuteChecked = -1;
    Logger::info("AgendaManager habilitado");
}

void AgendaManager::disable() {
    enabled = false;
    Logger::info("AgendaManager deshabilitado");
}

bool AgendaManager::isEnabled() {
    return enabled;
}
