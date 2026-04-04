#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "../config/Config.h"
#include "../config/Secrets.h"
#include "../utils/Logger.h"

// ============================================================================
// HttpClient - Cliente HTTP para comunicación con backend REST
// ============================================================================
// Gestiona peticiones HTTP al backend para obtener agendas y configuración

class HttpClient {
private:
    WiFiClient wifiClient;
    String baseUrl;
    String basicAuthHeader;
    String backendHost;
    uint16_t backendPort;
    String backendUser;
    String backendPassword;
    String nodeId;
    
    // Construir header de autenticación Basic
    String buildBasicAuth(const char* user, const char* password);

public:
    // Constructor
    HttpClient();
    
    // Inicializar cliente HTTP
    void init();

    // Configuración dinámica del backend y nodo
    void setRuntimeConfig(const String& backendHost, uint16_t backendPort,
                          const String& backendUser, const String& backendPassword,
                          const String& nodeId);
    
    // Obtener agendas desde backend
    // Retorna JSON string o string vacío si falla
    String fetchAgendas();
    
    // Verificar si backend está disponible
    bool isBackendAvailable();
};

#endif // HTTP_CLIENT_H
