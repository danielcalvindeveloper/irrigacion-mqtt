#include "WiFiManager.h"

// ============================================================================
// Constructor
// ============================================================================
WiFiManager::WiFiManager() {
    connected = false;
    lastConnectionAttempt = 0;
    connectionStartTime = 0;
    reconnectAttempts = 0;
    rssi = 0;
}

// ============================================================================
// Inicialización
// ============================================================================
void WiFiManager::init() {
    Logger::info("Inicializando WiFiManager...");
    
    // Configurar modo WiFi
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false); // No guardar en flash
    WiFi.setAutoReconnect(false); // Manejamos nosotros la reconexión
    
    // Obtener MAC address
    macAddress = WiFi.macAddress();
    Logger::logf(LOG_LEVEL_INFO, "MAC Address: %s", macAddress.c_str());
    
    // Configurar hostname
    #ifdef MDNS_HOSTNAME
    WiFi.hostname(MDNS_HOSTNAME);
    Logger::logf(LOG_LEVEL_INFO, "Hostname: %s", MDNS_HOSTNAME);
    #endif
}

// ============================================================================
// Conectar a WiFi (bloqueante con timeout)
// ============================================================================
bool WiFiManager::connect() {
    if (WiFi.status() == WL_CONNECTED) {
        Logger::info("WiFi ya estaba conectado");
        return true;
    }
    
    Logger::logf(LOG_LEVEL_INFO, "Conectando a WiFi: %s", WIFI_SSID);
    
    // Iniciar conexión
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    connectionStartTime = millis();
    
    // Esperar conexión con timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT) {
            Logger::error("Timeout conectando a WiFi");
            reconnectAttempts++;
            return false;
        }
        
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    // Conexión exitosa
    onConnected();
    reconnectAttempts = 0;
    return true;
}

// ============================================================================
// Desconectar WiFi
// ============================================================================
void WiFiManager::disconnect() {
    Logger::info("Desconectando WiFi...");
    WiFi.disconnect();
    connected = false;
    onDisconnected();
}

// ============================================================================
// Loop principal
// ============================================================================
void WiFiManager::loop() {
    // Verificar estado de conexión
    bool currentStatus = (WiFi.status() == WL_CONNECTED);
    
    // Detectar cambio de estado
    if (currentStatus != connected) {
        if (currentStatus) {
            onConnected();
        } else {
            onDisconnected();
        }
        connected = currentStatus;
    }
    
    // Si está desconectado, intentar reconectar
    if (!connected) {
        unsigned long now = millis();
        
        // Verificar si es momento de reintentar
        if (now - lastConnectionAttempt >= RECONNECT_DELAY) {
            if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                Logger::logf(LOG_LEVEL_INFO, "Reintentando conexión WiFi (intento %d/%d)...", 
                           reconnectAttempts + 1, MAX_RECONNECT_ATTEMPTS);
                lastConnectionAttempt = now;
                connect();
            } else {
                // Demasiados intentos fallidos, esperar más tiempo
                if (now - lastConnectionAttempt >= RECONNECT_DELAY * 6) { // 60 segundos
                    Logger::warn("Reiniciando contador de intentos WiFi");
                    reconnectAttempts = 0;
                }
            }
        }
    } else {
        // Actualizar RSSI periódicamente (cada 30 segundos)
        static unsigned long lastRssiUpdate = 0;
        if (millis() - lastRssiUpdate > 30000) {
            rssi = WiFi.RSSI();
            lastRssiUpdate = millis();
        }
    }
}

// ============================================================================
// Verificar conexión
// ============================================================================
bool WiFiManager::isConnected() {
    return connected && (WiFi.status() == WL_CONNECTED);
}

// ============================================================================
// Obtener IP local
// ============================================================================
String WiFiManager::getLocalIP() {
    if (connected) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

// ============================================================================
// Obtener MAC address
// ============================================================================
String WiFiManager::getMacAddress() {
    return macAddress;
}

// ============================================================================
// Obtener RSSI
// ============================================================================
int WiFiManager::getRSSI() {
    if (connected) {
        return WiFi.RSSI();
    }
    return 0;
}

// ============================================================================
// Obtener SSID
// ============================================================================
String WiFiManager::getSSID() {
    if (connected) {
        return WiFi.SSID();
    }
    return "";
}

// ============================================================================
// Forzar reconexión
// ============================================================================
void WiFiManager::forceReconnect() {
    Logger::info("Forzando reconexión WiFi...");
    disconnect();
    reconnectAttempts = 0;
    delay(1000);
    connect();
}

// ============================================================================
// Obtener uptime de conexión
// ============================================================================
unsigned long WiFiManager::getConnectionUptime() {
    if (connected && connectionStartTime > 0) {
        return millis() - connectionStartTime;
    }
    return 0;
}

// ============================================================================
// Imprimir información de red
// ============================================================================
void WiFiManager::printInfo() {
    if (!connected) {
        Logger::warn("WiFi no conectado");
        return;
    }
    
    Serial.println("\n=== Información de Red ===");
    Serial.printf("SSID: %s\n", getSSID().c_str());
    Serial.printf("IP Local: %s\n", getLocalIP().c_str());
    Serial.printf("MAC: %s\n", getMacAddress().c_str());
    Serial.printf("RSSI: %d dBm\n", getRSSI());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Uptime: %lu ms\n", getConnectionUptime());
    Serial.println("==========================\n");
}

// ============================================================================
// Callbacks internos
// ============================================================================
void WiFiManager::onConnected() {
    connected = true;
    localIP = WiFi.localIP().toString();
    rssi = WiFi.RSSI();
    connectionStartTime = millis();
    
    Logger::logf(LOG_LEVEL_INFO, "WiFi conectado a: %s", WIFI_SSID);
    Logger::logf(LOG_LEVEL_INFO, "IP: %s", localIP.c_str());
    Logger::logf(LOG_LEVEL_INFO, "RSSI: %d dBm", rssi);
}

void WiFiManager::onDisconnected() {
    connected = false;
    Logger::warn("WiFi desconectado");
    lastConnectionAttempt = millis();
}
