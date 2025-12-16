#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "../config/Config.h"

// ============================================================================
// Logger - Sistema de logging con niveles
// ============================================================================
// Proporciona funciones de logging con diferentes niveles de verbosidad
// controlados por LOG_LEVEL en Config.h

class Logger {
public:
    // Log de error (nivel 1)
    static void error(String message) {
        if (LOG_LEVEL >= LOG_LEVEL_ERROR) {
            Serial.print("[ERROR] ");
            Serial.println(message);
        }
    }
    
    // Log de warning (nivel 2)
    static void warn(String message) {
        if (LOG_LEVEL >= LOG_LEVEL_WARN) {
            Serial.print("[WARN] ");
            Serial.println(message);
        }
    }
    
    // Log de info (nivel 3)
    static void info(String message) {
        if (LOG_LEVEL >= LOG_LEVEL_INFO) {
            Serial.print("[INFO] ");
            Serial.println(message);
        }
    }
    
    // Log de debug (nivel 4)
    static void debug(String message) {
        if (LOG_LEVEL >= LOG_LEVEL_DEBUG) {
            Serial.print("[DEBUG] ");
            Serial.println(message);
        }
    }
    
    // Log con formato printf
    static void logf(int level, const char* format, ...) {
        if (LOG_LEVEL < level) return;
        
        const char* prefix;
        switch (level) {
            case LOG_LEVEL_ERROR: prefix = "[ERROR] "; break;
            case LOG_LEVEL_WARN:  prefix = "[WARN] ";  break;
            case LOG_LEVEL_INFO:  prefix = "[INFO] ";  break;
            case LOG_LEVEL_DEBUG: prefix = "[DEBUG] "; break;
            default: prefix = ""; break;
        }
        
        Serial.print(prefix);
        
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        Serial.println(buffer);
    }
};

#endif // LOGGER_H
