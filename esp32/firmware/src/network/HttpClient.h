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
    
    // Construir header de autenticación Basic
    String buildBasicAuth(const char* user, const char* password);

public:
    // Constructor
    HttpClient();
    
    // Inicializar cliente HTTP
    void init();
    
    // Obtener agendas desde backend
    // Retorna JSON string o string vacío si falla
    String fetchAgendas();
    
    // Verificar si backend está disponible
    bool isBackendAvailable();
};

#endif // HTTP_CLIENT_H
