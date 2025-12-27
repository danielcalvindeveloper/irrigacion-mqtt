#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
#include "../config/Config.h"
#include "../utils/Logger.h"

// ============================================================================
// TimeSync - Sincronización de tiempo con servidor NTP
// ============================================================================
// Sincroniza el reloj del ESP8266 con un servidor NTP y proporciona funciones
// para obtener fecha y hora actual considerando zona horaria.

class TimeSync {
private:
    WiFiUDP ntpUDP;
    NTPClient* timeClient;
    
    bool synchronized;
    unsigned long lastSyncAttempt;
    unsigned long lastSuccessfulSync;
    
    // Control de sincronización periódica
    static const unsigned long SYNC_INTERVAL = 3600000; // 1 hora
    static const unsigned long RETRY_INTERVAL = 60000;  // 1 minuto si falla
    
    // Información de tiempo
    time_t currentEpoch;
    struct tm timeInfo;
    
    // Actualizar estructura tm desde epoch
    void updateTimeInfo();

public:
    // Constructor
    TimeSync();
    
    // Destructor
    ~TimeSync();
    
    // Inicializar cliente NTP
    void init();
    
    // Sincronizar con servidor NTP (bloqueante)
    bool sync();
    
    // Loop principal - sincronización periódica
    void loop();
    
    // Verificar si está sincronizado
    bool isSynchronized();
    
    // Obtener epoch (Unix timestamp en segundos)
    time_t getEpoch();
    
    // Obtener estructura tm con fecha/hora actual
    struct tm getTimeInfo();
    
    // Obtener componentes de fecha/hora
    int getYear();        // Año completo (ej: 2025)
    int getMonth();       // Mes (1-12)
    int getDay();         // Día del mes (1-31)
    int getHour();        // Hora (0-23)
    int getMinute();      // Minuto (0-59)
    int getSecond();      // Segundo (0-59)
    int getWeekDay();     // Día de la semana (0=Dom, 1=Lun, ..., 6=Sab)
    
    // Obtener strings formateados
    String getDateString();      // "2025-12-27"
    String getTimeString();      // "14:30:45"
    String getDateTimeString();  // "2025-12-27 14:30:45"
    String getWeekDayName();     // "LUN", "MAR", etc.
    
    // Imprimir información de tiempo (debug)
    void printTime();
    
    // Obtener tiempo desde última sincronización (ms)
    unsigned long getTimeSinceLastSync();
    
    // Forzar resincronización
    void forceSync();
};

#endif // TIME_SYNC_H
