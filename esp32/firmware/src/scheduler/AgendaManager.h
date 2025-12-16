#ifndef AGENDA_MANAGER_H
#define AGENDA_MANAGER_H

#include <Arduino.h>
#include "Agenda.h"
#include "../config/Config.h"

// ============================================================================
// AgendaManager - Gestión y ejecución de agendas programadas
// ============================================================================
// Almacena agendas en memoria y las ejecuta automáticamente verificando
// la hora actual contra las programaciones configuradas.

class AgendaManager {
private:
    // Array de agendas en memoria
    Agenda agendas[MAX_AGENDAS];
    int agendaCount;
    
    // Control de verificación
    unsigned long lastCheck;
    int lastMinuteChecked;  // Para evitar ejecutar múltiples veces por minuto
    
    // Verificar si debe ejecutarse una agenda
    bool shouldExecute(Agenda& agenda, int wday, int hour, int minute);

public:
    // Constructor
    AgendaManager();
    
    // Agregar agenda a la lista
    bool addAgenda(Agenda agenda);
    
    // Eliminar agenda por ID
    bool removeAgenda(String id);
    
    // Actualizar agenda existente
    bool updateAgenda(Agenda agenda);
    
    // Limpiar todas las agendas
    void clearAll();
    
    // Obtener cantidad de agendas
    int getCount();
    
    // Obtener agenda por índice
    Agenda* getAgenda(int index);
    
    // Buscar agenda por ID
    Agenda* findById(String id);
    
    // Loop principal - verificar y ejecutar agendas
    void loop();
    
    // Forzar verificación inmediata (útil después de cargar de SPIFFS)
    void forceCheck();
    
    // Listar todas las agendas (debug)
    void printAll();
};

#endif // AGENDA_MANAGER_H
