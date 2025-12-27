#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include "../utils/Logger.h"

// ============================================================================
// SPIFFSManager - Gestion de almacenamiento en flash (LittleFS)
// ============================================================================
// Gestiona la persistencia de datos en memoria flash usando LittleFS.
// Usado para almacenar agendas de riego, configuracion y logs.

class SPIFFSManager {
private:
    bool initialized;
    size_t totalBytes;
    size_t usedBytes;
    
    // Actualizar informacion de espacio
    void updateStorageInfo();

public:
    // Constructor
    SPIFFSManager();
    
    // Inicializar sistema de archivos
    bool init();
    
    // Verificar si esta inicializado
    bool isInitialized();
    
    // Formatear sistema de archivos (CUIDADO: borra todo)
    bool format();
    
    // Leer archivo completo como String
    String readFile(const char* path);
    
    // Escribir String a archivo (sobrescribe)
    bool writeFile(const char* path, const String& content);
    
    // Agregar contenido al final de archivo
    bool appendFile(const char* path, const String& content);
    
    // Verificar si archivo existe
    bool exists(const char* path);
    
    // Eliminar archivo
    bool deleteFile(const char* path);
    
    // Listar todos los archivos
    void listFiles();
    
    // Obtener informacion de espacio
    size_t getTotalBytes();
    size_t getUsedBytes();
    size_t getFreeBytes();
    float getUsagePercent();
    
    // Imprimir informacion del sistema de archivos
    void printInfo();
};

#endif // SPIFFS_MANAGER_H
