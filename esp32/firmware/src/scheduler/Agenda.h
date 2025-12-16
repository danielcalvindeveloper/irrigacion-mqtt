#ifndef AGENDA_H
#define AGENDA_H

#include <Arduino.h>

// ============================================================================
// Estructura de datos para Agendas
// ============================================================================

struct HoraMinuto {
    int hour;    // 0-23
    int minute;  // 0-59
    
    HoraMinuto() : hour(0), minute(0) {}
    HoraMinuto(int h, int m) : hour(h), minute(m) {}
    
    // Convertir a string "HH:MM"
    String toString() {
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
        return String(buffer);
    }
    
    // Parsear desde string "HH:MM"
    static HoraMinuto fromString(String str) {
        int sep = str.indexOf(':');
        if (sep < 0) return HoraMinuto(0, 0);
        
        int h = str.substring(0, sep).toInt();
        int m = str.substring(sep + 1).toInt();
        return HoraMinuto(h, m);
    }
};

struct Agenda {
    String id;              // UUID de la agenda
    int zona;               // Número de zona (1-8)
    String nombre;          // Nombre descriptivo (ej: "Césped mañana")
    HoraMinuto horaInicio;  // Hora de inicio (HH:MM)
    int duracionMin;        // Duración en minutos
    bool diasSemana[7];     // L M X J V S D (true = activo)
    bool activa;            // true = habilitada, false = deshabilitada
    
    // Constructor por defecto
    Agenda() : zona(1), duracionMin(0), activa(false) {
        for (int i = 0; i < 7; i++) {
            diasSemana[i] = false;
        }
    }
    
    // Verificar si se debe ejecutar en un día específico
    // wday: 0=Domingo, 1=Lunes, ..., 6=Sábado (formato tm_wday)
    bool shouldRunOnDay(int wday) {
        // Convertir wday (0=Dom) a índice diasSemana (0=Lun)
        int dayIndex = (wday == 0) ? 6 : wday - 1;
        return diasSemana[dayIndex];
    }
    
    // Verificar si se debe ejecutar en hora específica
    bool shouldRunAtTime(int hour, int minute) {
        return (horaInicio.hour == hour && horaInicio.minute == minute);
    }
    
    // Convertir días activos a string para debug
    String getDaysString() {
        String result = "";
        const char* dias[] = {"L", "M", "X", "J", "V", "S", "D"};
        for (int i = 0; i < 7; i++) {
            if (diasSemana[i]) {
                if (result.length() > 0) result += ",";
                result += dias[i];
            }
        }
        return result;
    }
};

#endif // AGENDA_H
