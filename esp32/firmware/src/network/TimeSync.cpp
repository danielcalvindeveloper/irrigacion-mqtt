#include "TimeSync.h"

// ============================================================================
// Constructor
// ============================================================================
TimeSync::TimeSync() {
    timeClient = nullptr;
    synchronized = false;
    lastSyncAttempt = 0;
    lastSuccessfulSync = 0;
    currentEpoch = 0;
}

// ============================================================================
// Destructor
// ============================================================================
TimeSync::~TimeSync() {
    if (timeClient != nullptr) {
        delete timeClient;
    }
}

// ============================================================================
// Inicialización
// ============================================================================
void TimeSync::init() {
    Logger::info("Inicializando TimeSync...");
    
    // Crear cliente NTP con offset de zona horaria
    timeClient = new NTPClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, 60000);
    
    Logger::logf(LOG_LEVEL_INFO, "Servidor NTP: %s", NTP_SERVER);
    Logger::logf(LOG_LEVEL_INFO, "Zona horaria: GMT%+d", GMT_OFFSET_SEC / 3600);
}

// ============================================================================
// Sincronizar con NTP
// ============================================================================
bool TimeSync::sync() {
    Logger::info("Sincronizando tiempo con NTP...");
    lastSyncAttempt = millis();
    
    // Iniciar cliente NTP
    timeClient->begin();
    
    // Intentar actualizar (con timeout de 5 segundos)
    int attempts = 0;
    const int MAX_ATTEMPTS = 5;
    
    while (attempts < MAX_ATTEMPTS) {
        if (timeClient->update()) {
            // Sincronización exitosa
            currentEpoch = timeClient->getEpochTime();
            updateTimeInfo();
            
            synchronized = true;
            lastSuccessfulSync = millis();
            
            Logger::info("Tiempo sincronizado exitosamente");
            printTime();
            
            return true;
        }
        
        attempts++;
        delay(1000);
    }
    
    Logger::error("Fallo al sincronizar tiempo con NTP");
    synchronized = false;
    return false;
}

// ============================================================================
// Loop principal
// ============================================================================
void TimeSync::loop() {
    if (!synchronized) {
        // Si no está sincronizado, intentar cada minuto
        if (millis() - lastSyncAttempt > RETRY_INTERVAL) {
            sync();
        }
        return;
    }
    
    // Actualizar tiempo desde NTP periódicamente
    unsigned long now = millis();
    if (now - lastSuccessfulSync > SYNC_INTERVAL) {
        Logger::info("Resincronizando tiempo (sincronización periódica)...");
        sync();
    } else {
        // Actualizar tiempo local (sin consultar NTP)
        timeClient->update();
        currentEpoch = timeClient->getEpochTime();
        updateTimeInfo();
    }
}

// ============================================================================
// Verificar sincronización
// ============================================================================
bool TimeSync::isSynchronized() {
    return synchronized;
}

// ============================================================================
// Obtener epoch
// ============================================================================
time_t TimeSync::getEpoch() {
    if (synchronized) {
        return timeClient->getEpochTime();
    }
    return 0;
}

// ============================================================================
// Obtener estructura tm
// ============================================================================
struct tm TimeSync::getTimeInfo() {
    if (synchronized) {
        currentEpoch = timeClient->getEpochTime();
        updateTimeInfo();
    }
    return timeInfo;
}

// ============================================================================
// Actualizar estructura tm desde epoch
// ============================================================================
void TimeSync::updateTimeInfo() {
    time_t rawtime = currentEpoch;
    struct tm* ptm = gmtime(&rawtime);
    if (ptm != nullptr) {
        timeInfo = *ptm;
    }
}

// ============================================================================
// Obtener componentes individuales
// ============================================================================
int TimeSync::getYear() {
    getTimeInfo();
    return timeInfo.tm_year + 1900;
}

int TimeSync::getMonth() {
    getTimeInfo();
    return timeInfo.tm_mon + 1;
}

int TimeSync::getDay() {
    getTimeInfo();
    return timeInfo.tm_mday;
}

int TimeSync::getHour() {
    getTimeInfo();
    return timeInfo.tm_hour;
}

int TimeSync::getMinute() {
    getTimeInfo();
    return timeInfo.tm_min;
}

int TimeSync::getSecond() {
    getTimeInfo();
    return timeInfo.tm_sec;
}

int TimeSync::getWeekDay() {
    getTimeInfo();
    return timeInfo.tm_wday;
}

// ============================================================================
// Obtener strings formateados
// ============================================================================
String TimeSync::getDateString() {
    if (!synchronized) return "0000-00-00";
    
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", 
             getYear(), getMonth(), getDay());
    return String(buffer);
}

String TimeSync::getTimeString() {
    if (!synchronized) return "00:00:00";
    
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", 
             getHour(), getMinute(), getSecond());
    return String(buffer);
}

String TimeSync::getDateTimeString() {
    if (!synchronized) return "0000-00-00 00:00:00";
    
    return getDateString() + " " + getTimeString();
}

String TimeSync::getWeekDayName() {
    if (!synchronized) return "---";
    
    int wday = getWeekDay();
    // Convertir de 0=Dom a 0=Lun
    int dayIndex = (wday == 0) ? 6 : wday - 1;
    
    if (dayIndex >= 0 && dayIndex < 7) {
        return String(DIAS_SEMANA[dayIndex]);
    }
    return "---";
}

// ============================================================================
// Imprimir tiempo actual
// ============================================================================
void TimeSync::printTime() {
    if (!synchronized) {
        Logger::warn("Tiempo no sincronizado");
        return;
    }
    
    Serial.println("\n=== Tiempo Actual ===");
    Serial.printf("Fecha: %s (%s)\n", getDateString().c_str(), getWeekDayName().c_str());
    Serial.printf("Hora: %s\n", getTimeString().c_str());
    Serial.printf("Epoch: %lu\n", (unsigned long)getEpoch());
    Serial.printf("Última sincronización: hace %lu segundos\n", 
                  (millis() - lastSuccessfulSync) / 1000);
    Serial.println("=====================\n");
}

// ============================================================================
// Obtener tiempo desde última sincronización
// ============================================================================
unsigned long TimeSync::getTimeSinceLastSync() {
    if (lastSuccessfulSync == 0) return 0;
    return millis() - lastSuccessfulSync;
}

// ============================================================================
// Forzar resincronización
// ============================================================================
void TimeSync::forceSync() {
    Logger::info("Forzando resincronización de tiempo...");
    sync();
}
