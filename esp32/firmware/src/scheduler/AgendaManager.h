#ifndef AGENDA_MANAGER_H
#define AGENDA_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

class SPIFFSManager;
class TimeSync;
class RelayController;

// ============================================================================
// AgendaManager - Gestión y ejecución de agendas programadas
// ============================================================================
// Lee agendas desde SPIFFS y las ejecuta automáticamente verificando
// la hora actual contra las programaciones configuradas.

class AgendaManager {
private:
    SPIFFSManager* spiffsManager;
    TimeSync* timeSyncManager;
    RelayController* relayController;
    
    bool enabled;
    unsigned long lastCheckTime;
    int lastMinuteChecked;
    
    static const unsigned long CHECK_INTERVAL = 10000;
    
    void checkAndExecuteAgendas();
    bool shouldExecuteAgenda(JsonObject agenda, int currentDayOfWeek, int currentHour, int currentMinute);
    String getDayOfWeekString(int dayOfWeek);

public:
    AgendaManager(SPIFFSManager* spiffs, TimeSync* timeSync, RelayController* relay);
    ~AgendaManager();
    
    void init();
    void loop();
    
    void enable();
    void disable();
    bool isEnabled();
};

#endif // AGENDA_MANAGER_H
