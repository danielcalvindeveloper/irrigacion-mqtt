#include "SPIFFSManager.h"

// ============================================================================
// Constructor
// ============================================================================
SPIFFSManager::SPIFFSManager() {
    initialized = false;
    totalBytes = 0;
    usedBytes = 0;
}

// ============================================================================
// Inicializar LittleFS
// ============================================================================
bool SPIFFSManager::init() {
    Logger::info("Inicializando sistema de archivos LittleFS...");
    
    if (!LittleFS.begin()) {
        Logger::error("Fallo al montar LittleFS");
        
        // Intentar formatear y reinicializar
        Logger::warn("Intentando formatear LittleFS...");
        if (format()) {
            if (LittleFS.begin()) {
                Logger::info("LittleFS formateado y montado exitosamente");
                initialized = true;
                updateStorageInfo();
                return true;
            }
        }
        
        Logger::error("No se pudo inicializar LittleFS");
        initialized = false;
        return false;
    }
    
    initialized = true;
    updateStorageInfo();
    Logger::info("LittleFS montado correctamente");
    
    return true;
}

// ============================================================================
// Verificar si esta inicializado
// ============================================================================
bool SPIFFSManager::isInitialized() {
    return initialized;
}

// ============================================================================
// Formatear sistema de archivos
// ============================================================================
bool SPIFFSManager::format() {
    Logger::warn("Formateando LittleFS... (esto puede tardar)");
    
    bool result = LittleFS.format();
    
    if (result) {
        Logger::info("LittleFS formateado exitosamente");
        usedBytes = 0;
    } else {
        Logger::error("Fallo al formatear LittleFS");
    }
    
    return result;
}

// ============================================================================
// Leer archivo completo
// ============================================================================
String SPIFFSManager::readFile(const char* path) {
    if (!initialized) {
        Logger::error("LittleFS no inicializado");
        return "";
    }
    
    if (!LittleFS.exists(path)) {
        Logger::logf(LOG_LEVEL_WARN, "Archivo no existe: %s", path);
        return "";
    }
    
    File file = LittleFS.open(path, "r");
    if (!file) {
        Logger::logf(LOG_LEVEL_ERROR, "Error al abrir archivo: %s", path);
        return "";
    }
    
    String content = file.readString();
    file.close();
    
    Logger::logf(LOG_LEVEL_DEBUG, "Leido archivo: %s (%d bytes)", path, content.length());
    
    return content;
}

// ============================================================================
// Escribir archivo (sobrescribe)
// ============================================================================
bool SPIFFSManager::writeFile(const char* path, const String& content) {
    if (!initialized) {
        Logger::error("LittleFS no inicializado");
        return false;
    }
    
    File file = LittleFS.open(path, "w");
    if (!file) {
        Logger::logf(LOG_LEVEL_ERROR, "Error al crear archivo: %s", path);
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    updateStorageInfo();
    
    if (bytesWritten == content.length()) {
        Logger::logf(LOG_LEVEL_DEBUG, "Archivo escrito: %s (%d bytes)", path, bytesWritten);
        return true;
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Escritura incompleta: %s (%d/%d bytes)", 
                   path, bytesWritten, content.length());
        return false;
    }
}

// ============================================================================
// Agregar contenido al archivo
// ============================================================================
bool SPIFFSManager::appendFile(const char* path, const String& content) {
    if (!initialized) {
        Logger::error("LittleFS no inicializado");
        return false;
    }
    
    File file = LittleFS.open(path, "a");
    if (!file) {
        Logger::logf(LOG_LEVEL_ERROR, "Error al abrir archivo para append: %s", path);
        return false;
    }
    
    size_t bytesWritten = file.print(content);
    file.close();
    
    updateStorageInfo();
    
    if (bytesWritten == content.length()) {
        Logger::logf(LOG_LEVEL_DEBUG, "Contenido agregado: %s (%d bytes)", path, bytesWritten);
        return true;
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Append incompleto: %s (%d/%d bytes)", 
                   path, bytesWritten, content.length());
        return false;
    }
}

// ============================================================================
// Verificar existencia de archivo
// ============================================================================
bool SPIFFSManager::exists(const char* path) {
    if (!initialized) return false;
    return LittleFS.exists(path);
}

// ============================================================================
// Eliminar archivo
// ============================================================================
bool SPIFFSManager::deleteFile(const char* path) {
    if (!initialized) {
        Logger::error("LittleFS no inicializado");
        return false;
    }
    
    if (!LittleFS.exists(path)) {
        Logger::logf(LOG_LEVEL_WARN, "Archivo no existe: %s", path);
        return false;
    }
    
    bool result = LittleFS.remove(path);
    
    if (result) {
        Logger::logf(LOG_LEVEL_INFO, "Archivo eliminado: %s", path);
        updateStorageInfo();
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Error al eliminar archivo: %s", path);
    }
    
    return result;
}

// ============================================================================
// Listar archivos
// ============================================================================
void SPIFFSManager::listFiles() {
    if (!initialized) {
        Logger::error("LittleFS no inicializado");
        return;
    }
    
    Serial.println("\n=== Archivos en LittleFS ===");
    
    Dir dir = LittleFS.openDir("/");
    int count = 0;
    
    while (dir.next()) {
        count++;
        Serial.printf("  %d. %s (%d bytes)\n", count, dir.fileName().c_str(), dir.fileSize());
    }
    
    if (count == 0) {
        Serial.println("  (sin archivos)");
    }
    
    Serial.println("============================\n");
}

// ============================================================================
// Obtener informacion de espacio
// ============================================================================
void SPIFFSManager::updateStorageInfo() {
    if (!initialized) return;
    
    FSInfo info;
    LittleFS.info(info);
    
    totalBytes = info.totalBytes;
    usedBytes = info.usedBytes;
}

size_t SPIFFSManager::getTotalBytes() {
    return totalBytes;
}

size_t SPIFFSManager::getUsedBytes() {
    return usedBytes;
}

size_t SPIFFSManager::getFreeBytes() {
    return totalBytes - usedBytes;
}

float SPIFFSManager::getUsagePercent() {
    if (totalBytes == 0) return 0.0;
    return (float)usedBytes / (float)totalBytes * 100.0;
}

// ============================================================================
// Imprimir informacion
// ============================================================================
void SPIFFSManager::printInfo() {
    if (!initialized) {
        Serial.println("LittleFS: No inicializado");
        return;
    }
    
    updateStorageInfo();
    
    Serial.println("\n=== LittleFS Info ===");
    Serial.printf("Total: %d bytes (%.2f KB)\n", totalBytes, totalBytes / 1024.0);
    Serial.printf("Usado: %d bytes (%.2f KB)\n", usedBytes, usedBytes / 1024.0);
    Serial.printf("Libre: %d bytes (%.2f KB)\n", getFreeBytes(), getFreeBytes() / 1024.0);
    Serial.printf("Uso: %.1f%%\n", getUsagePercent());
    Serial.println("=====================\n");
}
