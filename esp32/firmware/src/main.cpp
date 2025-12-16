#include <Arduino.h>
#include "config/Config.h"
#include "config/Secrets.h"

// ============================================================================
// FIRMWARE ESP32 - SISTEMA DE RIEGO MQTT
// ============================================================================

// Forward declarations de funciones
void initHardware();
void initSerial();
void printBanner();
void mainLoop();

// Estado global del sistema
SystemState currentState = INIT;
unsigned long lastStatusPublish = 0;
unsigned long lastHumidityRead = 0;

// ============================================================================
// SETUP - Inicialización del sistema
// ============================================================================
void setup() {
    // Inicializar comunicación serial
    initSerial();
    
    // Banner de inicio
    printBanner();
    
    // Inicializar hardware
    initHardware();
    
    // TODO: Inicializar módulos
    // - SPIFFSManager
    // - WiFiManager
    // - MqttManager
    // - RelayController
    // - HumiditySensor
    // - AgendaManager
    // - TimeSync
    
    Serial.println("[INFO] Sistema inicializado");
    Serial.println("[INFO] Esperando implementación de módulos...");
    
    // Cambiar estado
    currentState = WIFI_CONNECTING;
}

// ============================================================================
// LOOP - Loop principal
// ============================================================================
void loop() {
    // Máquina de estados principal
    mainLoop();
    
    // Delay para evitar watchdog timeout
    delay(LOOP_DELAY_MS);
}

// ============================================================================
// FUNCIONES DE INICIALIZACIÓN
// ============================================================================

void initSerial() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);  // Esperar a que serial esté listo
    
    while (!Serial && millis() < 5000) {
        // Esperar hasta 5 segundos para serial
        delay(100);
    }
}

void initHardware() {
    Serial.println("[INFO] Inicializando hardware...");
    
    // LED integrado
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    // Pines de relés (todos OFF al inicio)
    for (int i = 0; i < MAX_ZONES; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], RELAY_OFF);
    }
    Serial.printf("[INFO] %d relés inicializados (todos OFF)\n", MAX_ZONES);
    
    // Pines de sensores (INPUT)
    for (int i = 0; i < MAX_SENSORS; i++) {
        pinMode(SENSOR_PINS[i], INPUT);
    }
    Serial.printf("[INFO] %d sensores ADC configurados\n", MAX_SENSORS);
    
    // Parpadear LED para indicar que hardware está listo
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

void printBanner() {
    Serial.println("\n\n");
    Serial.println("====================================");
    Serial.println("  SISTEMA DE RIEGO ESP32 - MQTT");
    Serial.println("====================================");
    Serial.printf(" Firmware: v%s\n", FIRMWARE_VERSION);
    Serial.printf(" Build: %s %s\n", BUILD_DATE, BUILD_TIME);
    Serial.printf(" Node ID: %s\n", NODE_ID);
    Serial.printf(" Max Zonas: %d\n", MAX_ZONES);
    Serial.printf(" Sensores: %d\n", MAX_SENSORS);
    Serial.println("====================================");
    Serial.println();
}

// ============================================================================
// MÁQUINA DE ESTADOS
// ============================================================================

void mainLoop() {
    static unsigned long lastStateChange = 0;
    
    // Log de cambio de estado
    static SystemState lastState = INIT;
    if (currentState != lastState) {
        Serial.printf("[INFO] Estado cambiado: %s → %s\n", 
                      STATE_NAMES[lastState], 
                      STATE_NAMES[currentState]);
        lastState = currentState;
        lastStateChange = millis();
    }
    
    // Procesar según estado actual
    switch (currentState) {
        case INIT:
            // Estado inicial, ya manejado en setup()
            break;
            
        case WIFI_CONNECTING:
            // TODO: Implementar lógica de conexión WiFi
            Serial.println("[WARN] WiFiManager no implementado");
            delay(5000);
            // Simular timeout
            if (millis() - lastStateChange > WIFI_TIMEOUT) {
                Serial.println("[WARN] WiFi timeout, entrando en modo OFFLINE");
                currentState = OFFLINE;
            }
            break;
            
        case WIFI_CONNECTED:
            // TODO: Sincronizar tiempo con NTP
            Serial.println("[INFO] Sincronizando tiempo...");
            currentState = MQTT_CONNECTING;
            break;
            
        case MQTT_CONNECTING:
            // TODO: Implementar conexión MQTT
            Serial.println("[WARN] MqttManager no implementado");
            delay(5000);
            // Simular timeout
            if (millis() - lastStateChange > MQTT_RECONNECT_DELAY) {
                Serial.println("[WARN] MQTT timeout, reintentar WiFi");
                currentState = WIFI_CONNECTING;
            }
            break;
            
        case ONLINE:
            // TODO: Modo online completo
            // - Procesar mensajes MQTT
            // - Publicar estado
            // - Ejecutar agendas
            Serial.println("[INFO] Modo ONLINE (stub)");
            delay(10000);
            break;
            
        case OFFLINE:
            // TODO: Modo offline
            // - Ejecutar agendas locales
            // - Intentar reconectar cada 60s
            Serial.println("[INFO] Modo OFFLINE (stub)");
            
            // Intentar reconectar
            if (millis() - lastStateChange > OFFLINE_RETRY_INTERVAL) {
                Serial.println("[INFO] Reintentando conexión...");
                currentState = WIFI_CONNECTING;
            }
            
            delay(10000);
            break;
    }
    
    // Tareas comunes independientes del estado
    // TODO: Implementar
    // - Actualizar timers de relés
    // - Verificar agendas programadas
    // - Leer sensores periódicamente
    
    // Indicador visual de vida
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastBlink = millis();
    }
}

// ============================================================================
// FUNCIONES AUXILIARES (STUBS)
// ============================================================================

// TODO: Implementar en módulos correspondientes
void publishZoneStatus() {
    // Publicar estado de todas las zonas via MQTT
}

void readAndPublishHumidity() {
    // Leer sensores y publicar via MQTT
}

void checkAndExecuteAgendas() {
    // Verificar si hay agendas para ejecutar
}

void updateRelayTimers() {
    // Decrementar timers y apagar relés cuando lleguen a 0
}
