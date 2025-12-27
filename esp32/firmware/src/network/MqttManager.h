#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "../config/Config.h"
#include "../config/Secrets.h"
#include "../utils/Logger.h"

// ============================================================================
// MqttManager - Gestión de comunicación MQTT
// ============================================================================
// Maneja la conexión al broker MQTT, suscripción a topics, publicación de
// mensajes y procesamiento de comandos recibidos.

// Forward declaration para callback
typedef void (*MqttCommandCallback)(int zona, String accion, int duracion);
typedef void (*MqttAgendaSyncCallback)(String payload);

class MqttManager {
private:
    WiFiClient espClient;
    PubSubClient* mqttClient;
    
    // Estado de conexión
    bool connected;
    unsigned long lastConnectionAttempt;
    unsigned long lastSuccessfulConnection;
    int reconnectAttempts;
    
    // Control de reconexión
    static const int MAX_RECONNECT_ATTEMPTS = 5;
    static const unsigned long RECONNECT_DELAY = 15000; // 15 segundos
    
    // Topics suscritos
    String cmdTopicPattern;
    String agendaSyncTopic;
    
    // Callbacks para eventos
    MqttCommandCallback commandCallback;
    MqttAgendaSyncCallback agendaSyncCallback;
    
    // Procesar mensaje MQTT recibido
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    
    // Instancia estática para callback
    static MqttManager* instance;
    
    // Procesar mensaje internamente
    void handleMessage(char* topic, byte* payload, unsigned int length);
    
    // Construir client ID único
    String getClientId();
    
    // Callback interno
    void onConnected();
    void onDisconnected();

public:
    // Constructor
    MqttManager();
    
    // Destructor
    ~MqttManager();
    
    // Inicializar cliente MQTT
    void init();
    
    // Conectar al broker MQTT (bloqueante con timeout)
    bool connect();
    
    // Desconectar del broker
    void disconnect();
    
    // Loop principal - mantener conexión y procesar mensajes
    void loop();
    
    // Verificar si está conectado
    bool isConnected();
    
    // Publicar estado de zona
    bool publishZoneStatus(int zona, bool estado, String origen);
    bool publishZoneStatus(int zona, bool estado, int tiempoRestante);
    
    // Publicar telemetría (humedad, uptime, etc)
    bool publishTelemetry(int zona, int humedad);
    
    // Publicar estado general del sistema
    bool publishSystemStatus(String status);
    
    // Suscribir a comandos
    bool subscribeToCommands();
    
    // Suscribir a sincronización de agendas
    bool subscribeToAgendaSync();
    
    // Registrar callback para comandos recibidos
    void setCommandCallback(MqttCommandCallback callback);
    
    // Registrar callback para sincronización de agendas
    void setAgendaSyncCallback(MqttAgendaSyncCallback callback);
    
    // Forzar reconexión
    void forceReconnect();
    
    // Obtener tiempo desde última conexión (ms)
    unsigned long getTimeSinceLastConnection();
    
    // Imprimir información de conexión MQTT
    void printInfo();
};

#endif // MQTT_MANAGER_H
