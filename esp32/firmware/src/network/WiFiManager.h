#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include "../config/Config.h"
#include "../config/Secrets.h"
#include "../utils/Logger.h"

// ============================================================================
// WiFiManager - Gestión de conexión WiFi con reconexión automática
// ============================================================================
// Maneja la conexión a WiFi, reconexión automática y proporciona información
// del estado de la red.

class WiFiManager {
private:
    // Estado de conexión
    bool connected;
    unsigned long lastConnectionAttempt;
    unsigned long connectionStartTime;
    int reconnectAttempts;
    
    // Información de red
    String localIP;
    String macAddress;
    int rssi;
    
    // Control de reconexión
    static const int MAX_RECONNECT_ATTEMPTS = 5;
    static const unsigned long RECONNECT_DELAY = 10000; // 10 segundos entre intentos
    
    // Callback para eventos de conexión
    void onConnected();
    void onDisconnected();

public:
    // Constructor
    WiFiManager();
    
    // Inicializar WiFi
    void init();
    
    // Conectar a WiFi (bloqueante con timeout)
    bool connect();
    
    // Desconectar WiFi
    void disconnect();
    
    // Loop principal - verificar estado y reconectar si es necesario
    void loop();
    
    // Verificar si está conectado
    bool isConnected();
    
    // Obtener IP local
    String getLocalIP();
    
    // Obtener MAC address
    String getMacAddress();
    
    // Obtener RSSI (señal WiFi)
    int getRSSI();
    
    // Obtener nombre de red (SSID)
    String getSSID();
    
    // Forzar reconexión
    void forceReconnect();
    
    // Obtener tiempo de uptime de conexión (milisegundos)
    unsigned long getConnectionUptime();
    
    // Imprimir información de red
    void printInfo();
};

#endif // WIFI_MANAGER_H
