#include "MqttManager.h"

// Instancia estática para callback
MqttManager* MqttManager::instance = nullptr;

// ============================================================================
// Constructor
// ============================================================================
MqttManager::MqttManager() {
    mqttClient = nullptr;
    connected = false;
    lastConnectionAttempt = 0;
    lastSuccessfulConnection = 0;
    reconnectAttempts = 0;
    commandCallback = nullptr;
    agendaSyncCallback = nullptr;
    instance = this;
}

// ============================================================================
// Destructor
// ============================================================================
MqttManager::~MqttManager() {
    if (mqttClient != nullptr) {
        delete mqttClient;
    }
}

// ============================================================================
// Inicialización
// ============================================================================
void MqttManager::init() {
    Logger::info("Inicializando MqttManager...");
    
    // Crear cliente MQTT
    mqttClient = new PubSubClient(espClient);
    mqttClient->setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient->setCallback(messageCallback);
    mqttClient->setKeepAlive(MQTT_KEEP_ALIVE);
    
    // Aumentar buffer MQTT para recibir mensajes de agenda (por defecto es 256 bytes)
    if (!mqttClient->setBufferSize(1024)) {
        Logger::error("No se pudo establecer buffer MQTT de 1024 bytes");
    } else {
        Logger::info("Buffer MQTT configurado a 1024 bytes");
    }
    
    // Construir patterns de topics
    cmdTopicPattern = String("riego/") + NODE_ID + "/cmd/zona/+";
    agendaSyncTopic = String("riego/") + NODE_ID + "/agenda/sync";
    
    Logger::logf(LOG_LEVEL_INFO, "Broker: %s:%d", MQTT_BROKER, MQTT_PORT);
    Logger::logf(LOG_LEVEL_INFO, "Client ID: %s", getClientId().c_str());
    Logger::logf(LOG_LEVEL_INFO, "Node ID: %s", NODE_ID);
}

// ============================================================================
// Conectar al broker MQTT
// ============================================================================
bool MqttManager::connect() {
    if (mqttClient->connected()) {
        Logger::info("MQTT ya estaba conectado");
        return true;
    }
    
    Logger::logf(LOG_LEVEL_INFO, "Conectando a broker MQTT: %s:%d", MQTT_BROKER, MQTT_PORT);
    lastConnectionAttempt = millis();
    
    String clientId = getClientId();
    
    // Intentar conexión (con o sin autenticación)
    bool result;
    if (strlen(MQTT_USER) > 0) {
        result = mqttClient->connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        result = mqttClient->connect(clientId.c_str());
    }
    
    if (result) {
        onConnected();
        reconnectAttempts = 0;
        
        // Suscribirse a topics
        subscribeToCommands();
        subscribeToAgendaSync();
        
        return true;
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Fallo conexión MQTT. Estado: %d", mqttClient->state());
        reconnectAttempts++;
        return false;
    }
}

// ============================================================================
// Desconectar del broker
// ============================================================================
void MqttManager::disconnect() {
    Logger::info("Desconectando MQTT...");
    if (mqttClient != nullptr) {
        mqttClient->disconnect();
    }
    connected = false;
    onDisconnected();
}

// ============================================================================
// Loop principal
// ============================================================================
void MqttManager::loop() {
    // Procesar mensajes MQTT
    if (mqttClient->connected()) {
        mqttClient->loop();
        connected = true;
    } else {
        connected = false;
        
        // Intentar reconectar
        unsigned long now = millis();
        if (now - lastConnectionAttempt >= RECONNECT_DELAY) {
            if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                Logger::logf(LOG_LEVEL_INFO, "Reintentando conexión MQTT (intento %d/%d)...", 
                           reconnectAttempts + 1, MAX_RECONNECT_ATTEMPTS);
                connect();
            } else {
                // Demasiados intentos fallidos, esperar más tiempo
                if (now - lastConnectionAttempt >= RECONNECT_DELAY * 4) { // 60 segundos
                    Logger::warn("Reiniciando contador de intentos MQTT");
                    reconnectAttempts = 0;
                }
            }
        }
    }
}

// ============================================================================
// Verificar conexión
// ============================================================================
bool MqttManager::isConnected() {
    return connected && mqttClient->connected();
}

// ============================================================================
// Publicar estado de zona (legacy)
// ============================================================================
bool MqttManager::publishZoneStatus(int zona, bool estado, String origen) {
    // Delegar a la versión con tiempo restante
    return publishZoneStatus(zona, estado, 0);
}

// ============================================================================
// Publicar estado de zona con tiempo restante
// ============================================================================
bool MqttManager::publishZoneStatus(int zona, bool estado, int tiempoRestante) {
    if (!isConnected()) return false;
    
    // Construir topic: riego/{NODE_ID}/status/zona/{zona}
    char topic[128];
    snprintf(topic, sizeof(topic), "riego/%s/status/zona/%d", NODE_ID, zona);
    
    // Construir payload JSON con formato esperado por backend
    // Backend espera: {"activa": true/false, "tiempoRestante": seconds}
    StaticJsonDocument<JSON_BUFFER_SMALL> doc;
    doc["activa"] = estado;
    doc["tiempoRestante"] = tiempoRestante;
    
    char payload[JSON_BUFFER_SMALL];
    serializeJson(doc, payload);
    
    bool result = mqttClient->publish(topic, payload, false);
    
    if (result) {
        Logger::logf(LOG_LEVEL_DEBUG, "Publicado estado zona %d: %s", zona, payload);
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Fallo al publicar estado zona %d", zona);
    }
    
    return result;
}

// ============================================================================
// Publicar evento de riego
// ============================================================================
bool MqttManager::publishRiegoEvento(int zona, String evento, String origen, int duracion, int versionAgenda) {
    if (!isConnected()) return false;
    
    // Construir topic: riego/{NODE_ID}/evento
    char topic[128];
    snprintf(topic, sizeof(topic), "riego/%s/evento", NODE_ID);
    
    // Construir payload JSON
    StaticJsonDocument<JSON_BUFFER_SMALL> doc;
    doc["zona"] = zona;
    doc["evento"] = evento; // "inicio" o "fin"
    doc["timestamp"] = millis() / 1000; // timestamp en segundos (será reemplazado por NTP)
    doc["origen"] = origen; // "agenda" o "manual"
    
    if (evento == "fin") {
        doc["duracionReal"] = duracion;
    } else {
        doc["duracionProgramada"] = duracion;
    }
    
    if (versionAgenda > 0) {
        doc["versionAgenda"] = versionAgenda;
    } else {
        doc["versionAgenda"] = nullptr;
    }
    
    char payload[JSON_BUFFER_SMALL];
    serializeJson(doc, payload);
    
    bool result = mqttClient->publish(topic, payload, false);
    
    if (result) {
        Logger::logf(LOG_LEVEL_INFO, "Publicado evento riego zona %d: %s (%s)", zona, evento.c_str(), origen.c_str());
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Fallo al publicar evento zona %d", zona);
    }
    
    return result;
}

// ============================================================================
// Publicar telemetría
// ============================================================================
bool MqttManager::publishTelemetry(int zona, int humedad) {
    if (!isConnected()) return false;
    
    // Construir topic: riego/{NODE_ID}/humedad/zona/{zona}
    char topic[128];
    snprintf(topic, sizeof(topic), "riego/%s/humedad/zona/%d", NODE_ID, zona);
    
    // Construir payload JSON
    StaticJsonDocument<JSON_BUFFER_SMALL> doc;
    doc["humedad"] = humedad;
    doc["uptime"] = millis() / 1000;
    
    char payload[JSON_BUFFER_SMALL];
    serializeJson(doc, payload);
    
    bool result = mqttClient->publish(topic, payload, false);
    
    if (result) {
        Logger::logf(LOG_LEVEL_DEBUG, "Publicada telemetría zona %d: %d%%", zona, humedad);
    } else {
        Logger::logf(LOG_LEVEL_ERROR, "Fallo al publicar telemetría zona %d", zona);
    }
    
    return result;
}

// ============================================================================
// Publicar estado del sistema
// ============================================================================
bool MqttManager::publishSystemStatus(String status) {
    if (!isConnected()) return false;
    
    char topic[128];
    snprintf(topic, sizeof(topic), "riego/%s/status/system", NODE_ID);
    
    StaticJsonDocument<JSON_BUFFER_MEDIUM> doc;
    doc["status"] = status;
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    
    char payload[JSON_BUFFER_MEDIUM];
    serializeJson(doc, payload);
    
    return mqttClient->publish(topic, payload, false);
}

// ============================================================================
// Suscribir a comandos
// ============================================================================
bool MqttManager::subscribeToCommands() {
    if (!isConnected()) return false;
    
    Logger::logf(LOG_LEVEL_INFO, "Suscribiendo a: %s", cmdTopicPattern.c_str());
    
    // Suscribirse a comandos de todas las zonas
    // Pattern: riego/{NODE_ID}/cmd/zona/+
    bool result = mqttClient->subscribe(cmdTopicPattern.c_str(), MQTT_QOS);
    
    if (result) {
        Logger::info("Suscripción a comandos exitosa");
    } else {
        Logger::error("Fallo al suscribirse a comandos");
    }
    
    return result;
}

// ============================================================================
// Suscribir a sincronización de agendas
// ============================================================================
bool MqttManager::subscribeToAgendaSync() {
    if (!isConnected()) return false;
    
    Logger::logf(LOG_LEVEL_INFO, "Suscribiendo a: %s", agendaSyncTopic.c_str());
    
    bool result = mqttClient->subscribe(agendaSyncTopic.c_str(), MQTT_QOS);
    
    if (result) {
        Logger::info("Suscripción a agenda sync exitosa");
    } else {
        Logger::error("Fallo al suscribirse a agenda sync");
    }
    
    return result;
}

// ============================================================================
// Registrar callbacks
// ============================================================================
void MqttManager::setCommandCallback(MqttCommandCallback callback) {
    commandCallback = callback;
}

void MqttManager::setAgendaSyncCallback(MqttAgendaSyncCallback callback) {
    agendaSyncCallback = callback;
}

// ============================================================================
// Callback estático MQTT
// ============================================================================
void MqttManager::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance != nullptr) {
        instance->handleMessage(topic, payload, length);
    }
}

// ============================================================================
// Procesar mensaje recibido
// ============================================================================
void MqttManager::handleMessage(char* topic, byte* payload, unsigned int length) {
    // Convertir payload a String
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Logger::logf(LOG_LEVEL_INFO, "Mensaje MQTT recibido en [%s]: %s", topic, message);
    
    String topicStr = String(topic);
    
    // Debug: mostrar análisis del topic
    Logger::logf(LOG_LEVEL_DEBUG, "Analizando topic: %s", topicStr.c_str());
    
    // Verificar si es comando de zona
    if (topicStr.indexOf("/cmd/zona/") >= 0) {
        Logger::debug("Detectado como comando de zona");
        // Extraer número de zona del topic
        int lastSlash = topicStr.lastIndexOf('/');
        int zona = topicStr.substring(lastSlash + 1).toInt();
        
        // Parsear JSON
        StaticJsonDocument<JSON_BUFFER_SMALL> doc;
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
            Logger::logf(LOG_LEVEL_ERROR, "Error parseando JSON: %s", error.c_str());
            return;
        }
        
        String accion = doc["accion"] | "OFF";
        int duracion = doc["duracion"] | 0;
        
        Logger::logf(LOG_LEVEL_INFO, "Comando zona %d: %s, duración: %d seg", 
                   zona, accion.c_str(), duracion);
        
        // Llamar callback si está registrado
        if (commandCallback != nullptr) {
            commandCallback(zona, accion, duracion);
        }
    }
    // Verificar si es sincronización de agenda
    else if (topicStr.indexOf("/agenda/sync") >= 0) {
        Logger::info("Detectado como sincronizacion de agenda");
        
        // Llamar callback si está registrado
        if (agendaSyncCallback != nullptr) {
            Logger::info("Ejecutando callback de agenda sync");
            agendaSyncCallback(String(message));
        } else {
            Logger::warn("Callback de agenda sync no registrado");
        }
    }
    else {
        Logger::logf(LOG_LEVEL_WARN, "Topic desconocido: %s", topicStr.c_str());
    }
}

// ============================================================================
// Forzar reconexión
// ============================================================================
void MqttManager::forceReconnect() {
    Logger::info("Forzando reconexión MQTT...");
    disconnect();
    reconnectAttempts = 0;
    delay(1000);
    connect();
}

// ============================================================================
// Obtener tiempo desde última conexión
// ============================================================================
unsigned long MqttManager::getTimeSinceLastConnection() {
    if (lastSuccessfulConnection == 0) return 0;
    return millis() - lastSuccessfulConnection;
}

// ============================================================================
// Obtener Client ID único
// ============================================================================
String MqttManager::getClientId() {
    if (strlen(MQTT_CLIENT_ID) > 0) {
        return String(MQTT_CLIENT_ID);
    }
    // Usar NODE_ID si no hay CLIENT_ID configurado
    return String("ESP8266_") + String(NODE_ID).substring(0, 8);
}

// ============================================================================
// Imprimir información MQTT
// ============================================================================
void MqttManager::printInfo() {
    Serial.println("\n=== Información MQTT ===");
    Serial.printf("Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    Serial.printf("Client ID: %s\n", getClientId().c_str());
    Serial.printf("Conectado: %s\n", isConnected() ? "SI" : "NO");
    if (isConnected()) {
        Serial.printf("Tiempo conectado: %lu s\n", getTimeSinceLastConnection() / 1000);
    }
    Serial.println("========================\n");
}

// ============================================================================
// Callbacks internos
// ============================================================================
void MqttManager::onConnected() {
    connected = true;
    lastSuccessfulConnection = millis();
    Logger::info("MQTT conectado exitosamente");
}

void MqttManager::onDisconnected() {
    connected = false;
    Logger::warn("MQTT desconectado");
}
