#include "HttpClient.h"
#include <base64.h>

// ============================================================================
// Constructor
// ============================================================================
HttpClient::HttpClient() {
    baseUrl = "";
    backendHost = BACKEND_HOST;
    backendPort = BACKEND_PORT;
    backendUser = BACKEND_USER;
    backendPassword = BACKEND_PASSWORD;
    nodeId = NODE_ID;
}

void HttpClient::setRuntimeConfig(const String& newBackendHost, uint16_t newBackendPort,
                                  const String& newBackendUser, const String& newBackendPassword,
                                  const String& newNodeId) {
    if (newBackendHost.length() > 0) {
        backendHost = newBackendHost;
    }

    if (newBackendPort > 0) {
        backendPort = newBackendPort;
    }

    backendUser = newBackendUser;
    backendPassword = newBackendPassword;

    if (newNodeId.length() > 0) {
        nodeId = newNodeId;
    }
}

// ============================================================================
// Inicializar cliente HTTP
// ============================================================================
void HttpClient::init() {
    Logger::info("Inicializando HttpClient...");
    
    // Construir base URL
    baseUrl = String("http://") + backendHost + ":" + String(backendPort) + "/api";
    
    // Construir header de autenticación
    basicAuthHeader = buildBasicAuth(backendUser.c_str(), backendPassword.c_str());
    
    Logger::logf(LOG_LEVEL_INFO, "Backend URL: %s", baseUrl.c_str());
}

// ============================================================================
// Construir header de autenticación Basic
// ============================================================================
String HttpClient::buildBasicAuth(const char* user, const char* password) {
    String credentials = String(user) + ":" + String(password);
    String encoded = base64::encode(credentials);
    return "Basic " + encoded;
}

// ============================================================================
// Obtener agendas desde backend
// ============================================================================
String HttpClient::fetchAgendas() {
    if (baseUrl.length() == 0) {
        Logger::error("HttpClient no inicializado");
        return "";
    }
    
    HTTPClient http;
    String url = baseUrl + "/nodos/" + nodeId + "/agendas";
    
    Logger::logf(LOG_LEVEL_INFO, "Solicitando agendas desde: %s", url.c_str());
    
    http.begin(wifiClient, url);
    http.addHeader("Authorization", basicAuthHeader);
    http.setTimeout(10000); // 10 segundos timeout
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
        Logger::logf(LOG_LEVEL_INFO, "HTTP GET respuesta: %d", httpCode);
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Logger::logf(LOG_LEVEL_INFO, "Agendas recibidas: %d bytes", payload.length());
            http.end();
            return payload;
        } else if (httpCode == HTTP_CODE_UNAUTHORIZED) {
            Logger::error("Autenticacion fallida - verificar credenciales");
        } else {
            Logger::logf(LOG_LEVEL_ERROR, "HTTP error code: %d", httpCode);
        }
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "HTTP GET fallo: %s", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return "";
}

// ============================================================================
// Verificar disponibilidad del backend
// ============================================================================
bool HttpClient::isBackendAvailable() {
    if (baseUrl.length() == 0) {
        return false;
    }
    
    HTTPClient http;
    String url = baseUrl + "/nodos/" + nodeId + "/status";
    
    http.begin(wifiClient, url);
    http.addHeader("Authorization", basicAuthHeader);
    http.setTimeout(5000);
    
    int httpCode = http.GET();
    http.end();
    
    return (httpCode == HTTP_CODE_OK);
}
