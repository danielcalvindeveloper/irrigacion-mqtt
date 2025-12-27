#include <Arduino.h>
#include "config/Config.h"
#include "config/Secrets.h"
#include "network/WiFiManager.h"
#include "network/TimeSync.h"
#include "network/MqttManager.h"
#include "network/HttpClient.h"
#include "hardware/RelayController.h"
#include "storage/SPIFFSManager.h"
#include "scheduler/AgendaManager.h"
#include "display/DisplayManager.h"
#include "utils/Logger.h"

// ============================================================================
// FIRMWARE ESP8266 - SISTEMA DE RIEGO MQTT
// ============================================================================

// Forward declarations de funciones
void initHardware();
void initSerial();
void printBanner();
void mainLoop();
void onMqttCommand(int zona, String accion, int duracion);
void onAgendaSync(String payload);
void onZoneStateChanged(int zona, bool estado);
void showStoredAgenda();
void fetchAndStoreAgendas();

// Estado global del sistema
SystemState currentState = INIT;
unsigned long lastStatusPublish = 0;
unsigned long lastHumidityRead = 0;
unsigned long lastDisplayUpdate = 0;

// Módulos del sistema
WiFiManager wifiManager;
TimeSync timeSync;
MqttManager mqttManager;
HttpClient httpClient;
RelayController relayController;
SPIFFSManager spiffsManager;
AgendaManager* agendaManager = nullptr;
DisplayManager displayManager;
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
    
    // Inicializar Display OLED
    displayManager.init(I2C_SDA, I2C_SCL, OLED_ADDRESS, OLED_WIDTH, OLED_HEIGHT);
    displayManager.clear();
    displayManager.showStatusLine("Configurando...");
    displayManager.display();
    
    // Inicializar módulos
    Logger::info("Inicializando módulos del sistema...");
    
    // SPIFFSManager (primero para poder leer configuracion)
    displayManager.showStatusLine("Preparando almacen.");
    displayManager.display();
    spiffsManager.init();
    spiffsManager.printInfo();
    
    // Mostrar agenda almacenada si existe
    showStoredAgenda();
    
    // WiFiManager
    displayManager.showStatusLine("Iniciando WiFi...");
    displayManager.display();
    wifiManager.init();
    
    // TimeSync (se sincronizará cuando WiFi esté conectado)
    timeSync.init();
    
    // HttpClient (para solicitar agendas al backend)
    httpClient.init();
    
    // MqttManager
    displayManager.showStatusLine("Iniciando MQTT...");
    displayManager.display();
    mqttManager.init();
    mqttManager.setCommandCallback(onMqttCommand);
    mqttManager.setAgendaSyncCallback(onAgendaSync);
    
    // RelayController
    displayManager.showStatusLine("Iniciando reles...");
    displayManager.display();
    relayController.init();
    relayController.setStateChangedCallback(onZoneStateChanged);
    
    // AgendaManager (requiere SPIFFSManager, TimeSync, RelayController)
    displayManager.showStatusLine("Preparando agendas");
    displayManager.display();
    agendaManager = new AgendaManager(&spiffsManager, &timeSync, &relayController);
    agendaManager->init();
    
    // TODO: Inicializar otros modulos
    // - HumiditySensor (hardware no disponible)
    
    Logger::info("Sistema inicializado - iniciando conexión WiFi");
    
    // Cambiar estado
    currentState = WIFI_CONNECTING;
}

// ============================================================================
// LOOP - Loop principal
// ============================================================================
void loop() {
    // Actualizar módulos
    wifiManager.loop();
    mqttManager.loop();
    relayController.loop();  // CRITICO: actualizar timers de zonas
    
    // TimeSync solo actualiza si WiFi está conectado
    if (wifiManager.isConnected()) {
        timeSync.loop();
    }
    
    // AgendaManager - verificar y ejecutar agendas programadas
    if (agendaManager != nullptr) {
        agendaManager->loop();
    }
    
    // Actualizar iconos de estado en display cada 2 segundos
    unsigned long now = millis();
    if (now - lastDisplayUpdate >= 2000) {
        lastDisplayUpdate = now;
        int rssi = wifiManager.isConnected() ? wifiManager.getRSSI() : -100;
        displayManager.updateStatusIcons(rssi, wifiManager.isConnected(), mqttManager.isConnected());
        
        // Actualizar indicadores de zonas
        bool zone1Active = relayController.isActive(1);
        bool zone2Active = relayController.isActive(2);
        bool zone3Active = relayController.isActive(3);
        bool zone4Active = relayController.isActive(4);
        displayManager.updateZoneIndicators(zone1Active, zone2Active, zone3Active, zone4Active);
        
        displayManager.display();
    }
    
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
    static bool statusMessageShown = false;
    
    // Log de cambio de estado
    static SystemState lastState = INIT;
    if (currentState != lastState) {
        Serial.printf("[INFO] Estado cambiado: %s → %s\n", 
                      STATE_NAMES[lastState], 
                      STATE_NAMES[currentState]);
        lastState = currentState;
        lastStateChange = millis();
        statusMessageShown = false;
    }
    
    // Procesar según estado actual
    switch (currentState) {
        case INIT:
            // Estado inicial, ya manejado en setup()
            break;
            
        case WIFI_CONNECTING:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Conectando WiFi...");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Intentar conectar WiFi
            if (wifiManager.isConnected()) {
                Logger::info("WiFi conectado exitosamente");
                wifiManager.printInfo();
                currentState = WIFI_CONNECTED;
            } else if (millis() - lastStateChange > WIFI_TIMEOUT * 2) {
                // Timeout extendido: Si después de varios intentos no se conecta
                Logger::warn("WiFi no disponible - entrando en modo OFFLINE");
                displayManager.showStatusLine("WiFi no disponible");
                displayManager.display();
                currentState = OFFLINE;
            } else if (millis() - lastStateChange > 5000) {
                // Primer intento de conexión después de 5 segundos
                static bool firstAttempt = true;
                if (firstAttempt) {
                    Logger::info("Iniciando conexión WiFi...");
                    wifiManager.connect();
                    firstAttempt = false;
                }
            }
            break;
            
        case WIFI_CONNECTED:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Sinc. fecha y hora");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Sincronizar tiempo con NTP
            if (!timeSync.isSynchronized()) {
                Logger::info("Sincronizando tiempo con NTP...");
                if (timeSync.sync()) {
                    Logger::info("Tiempo sincronizado correctamente");
                    currentState = MQTT_CONNECTING;
                } else {
                    // Si falla NTP, continuar igual (no es crítico)
                    Logger::warn("Fallo sincronización NTP, continuando sin tiempo exacto");
                    currentState = MQTT_CONNECTING;
                }
            } else {
                currentState = MQTT_CONNECTING;
            }
            break;
            
        case MQTT_CONNECTING:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Conectando MQTT...");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Intentar conectar MQTT
            if (mqttManager.isConnected()) {
                Logger::info("MQTT conectado, sistema ONLINE");
                mqttManager.printInfo();
                currentState = ONLINE;
            } else if (millis() - lastStateChange > MQTT_RECONNECT_DELAY * 2) {
                // Si MQTT falla despues de varios intentos, volver a verificar WiFi
                Logger::warn("MQTT no disponible, verificando WiFi...");
                if (wifiManager.isConnected()) {
                    // WiFi OK pero MQTT falla, reintentar
                    static unsigned long lastMqttRetry = 0;
                    if (millis() - lastMqttRetry > 30000) {
                        Logger::info("Reintentando conexion MQTT...");
                        mqttManager.connect();
                        lastMqttRetry = millis();
                    }
                } else {
                    // WiFi caido, volver a conectar
                    currentState = WIFI_CONNECTING;
                }
            } else if (millis() - lastStateChange > 5000) {
                // Primer intento despues de 5 segundos
                static bool firstMqttAttempt = true;
                if (firstMqttAttempt) {
                    Logger::info("Iniciando conexion MQTT...");
                    mqttManager.connect();
                    firstMqttAttempt = false;
                }
            }
            break;
            
        case ONLINE:
            // Actualizar mensaje de estado al entrar
            if (!statusMessageShown) {
                displayManager.showStatusLine("Actualizando agenda");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Modo online completo - verificar que todo sigue conectado
            if (!wifiManager.isConnected()) {
                Logger::warn("WiFi desconectado, volviendo a WIFI_CONNECTING");
                currentState = WIFI_CONNECTING;
            } else if (!mqttManager.isConnected()) {
                Logger::warn("MQTT desconectado, volviendo a MQTT_CONNECTING");
                currentState = MQTT_CONNECTING;
            }
            
            // Solicitar agendas del backend al entrar en modo ONLINE (una vez)
            {
                static bool agendasFetched = false;
                static unsigned long lastFetchAttempt = 0;
                
                if (!agendasFetched) {
                    Logger::info("Solicitando agendas al backend...");
                    fetchAndStoreAgendas();
                    agendasFetched = true;
                    lastFetchAttempt = millis();
                    
                    // Cambiar mensaje después de actualizar agendas
                    displayManager.showStatusLine("Sistema listo");
                    displayManager.display();
                } else {
                    // Reintentar cada 5 minutos si la primera vez falló
                    if (millis() - lastFetchAttempt > 300000) { // 5 minutos
                        Logger::info("Reintentando sincronización de agendas...");
                        fetchAndStoreAgendas();
                        lastFetchAttempt = millis();
                    }
                }
            }
            
            // Publicar estado de zonas activas cada 5 segundos
            {
                static unsigned long lastStatusPublish = 0;
                if (millis() - lastStatusPublish > 5000) {
                    for (int zona = 1; zona <= MAX_ZONES; zona++) {
                        if (relayController.isActive(zona)) {
                            int remaining = relayController.getRemainingTime(zona);
                            mqttManager.publishZoneStatus(zona, true, remaining);
                        }
                    }
                    lastStatusPublish = millis();
                }
            }
            
            break;
            
        case OFFLINE:
            if (!statusMessageShown) {
                displayManager.showStatusLine("Modo offline");
                displayManager.display();
                statusMessageShown = true;
            }
            
            // Modo offline - el WiFiManager intentará reconectar automáticamente
            // Si se reconecta, cambiar de estado
            if (wifiManager.isConnected()) {
                Logger::info("WiFi reconectado desde modo OFFLINE");
                currentState = WIFI_CONNECTED;
            }
            
            // TODO: Ejecutar agendas locales desde SPIFFS
            
            static unsigned long lastOfflineLog = 0;
            if (millis() - lastOfflineLog > 30000) {
                Logger::info("Modo OFFLINE - sistema funcionando de forma autonoma");
                lastOfflineLog = millis();
            }
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
    
    // Debug: Mostrar tiempo cada 30 segundos si está sincronizado
    static unsigned long lastTimeDebug = 0;
    if (timeSync.isSynchronized() && millis() - lastTimeDebug > 30000) {
        Logger::logf(LOG_LEVEL_INFO, "Tiempo actual: %s", timeSync.getDateTimeString().c_str());
        lastTimeDebug = millis();
    }
}

// ============================================================================
// FUNCIONES AUXILIARES (STUBS)
// ============================================================================

// TODO: Implementar en módulos correspondientes
void publishZoneStatus() {
    // Publicar estado de todas las zonas via MQTT
}


// ============================================================================
// CALLBACKS MQTT
// ============================================================================

void onMqttCommand(int zona, String accion, int duracion) {
    Logger::logf(LOG_LEVEL_INFO, ">>> Comando recibido - Zona: %d, Accion: %s, Duracion: %d seg", 
               zona, accion.c_str(), duracion);
    
    // Ejecutar comando en relayController
    if (accion == "ON") {
        relayController.turnOn(zona, duracion);
    } else if (accion == "OFF") {
        relayController.turnOff(zona);
    }
    
    // Publicar estado actualizado inmediatamente con tiempo restante
    bool estado = relayController.isActive(zona);
    int remaining = relayController.getRemainingTime(zona);
    mqttManager.publishZoneStatus(zona, estado, remaining);
    
    // Log de confirmacion
    if (estado) {
        Logger::logf(LOG_LEVEL_INFO, "Zona %d encendida, tiempo restante: %d seg", zona, remaining);
    }
}

void onAgendaSync(String payload) {
    Logger::logf(LOG_LEVEL_INFO, ">>> Sincronizacion de agenda recibida (%d bytes)", payload.length());
    
    // Guardar agenda en SPIFFS
    if (spiffsManager.isInitialized()) {
        const char* agendaFile = "/agenda.json";
        
        if (spiffsManager.writeFile(agendaFile, payload)) {
            Logger::logf(LOG_LEVEL_INFO, "Agenda guardada en SPIFFS: %s", agendaFile);
            
            // Mostrar info de almacenamiento
            Logger::logf(LOG_LEVEL_INFO, "Espacio usado: %.1f%% (%d/%d bytes)", 
                       spiffsManager.getUsagePercent(),
                       spiffsManager.getUsedBytes(),
                       spiffsManager.getTotalBytes());
            
            // Listar archivos
            spiffsManager.listFiles();
            
            // Mostrar contenido de la agenda guardada
            showStoredAgenda();
        } else {
            Logger::error("Error al guardar agenda en SPIFFS");
        }
    } else {
        Logger::warn("SPIFFS no inicializado, agenda no guardada");
    }
    
    // TODO: Parsear agenda y cargar en AgendaManager
}

void onZoneStateChanged(int zona, bool estado) {
    // Callback llamado cuando una zona cambia de estado (ej: auto-apagado por timer)
    Logger::logf(LOG_LEVEL_INFO, ">>> Cambio de estado zona %d: %s", zona, estado ? "ON" : "OFF");
    
    // Publicar estado actualizado via MQTT
    if (mqttManager.isConnected()) {
        int remaining = estado ? relayController.getRemainingTime(zona) : 0;
        mqttManager.publishZoneStatus(zona, estado, remaining);
    }
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

// ============================================================================
// Mostrar agenda almacenada en SPIFFS
// ============================================================================
void showStoredAgenda() {
    const char* agendaFile = "/agenda.json";
    
    if (!spiffsManager.isInitialized()) {
        Logger::warn("SPIFFS no inicializado, no se puede leer agenda");
        return;
    }
    
    if (!spiffsManager.exists(agendaFile)) {
        Logger::info("No hay agenda almacenada en SPIFFS");
        return;
    }
    
    String agendaContent = spiffsManager.readFile(agendaFile);
    
    if (agendaContent.length() == 0) {
        Logger::error("Archivo de agenda vacio o error al leer");
        return;
    }
    
    Serial.println("\n╔════════════════════════════════════════════════════════════════");
    Serial.println("║ AGENDA ALMACENADA EN SPIFFS");
    Serial.println("╠════════════════════════════════════════════════════════════════");
    Serial.printf("║ Archivo: %s\n", agendaFile);
    Serial.printf("║ Tamaño: %d bytes\n", agendaContent.length());
    Serial.println("╠════════════════════════════════════════════════════════════════");
    Serial.println("║ Contenido JSON:");
    Serial.println("╠────────────────────────────────────────────────────────────────");
    
    // Imprimir el JSON con indentación básica
    Serial.println(agendaContent);
    
    Serial.println("╚════════════════════════════════════════════════════════════════\n");
}

// ============================================================================
// Solicitar y almacenar agendas desde backend via HTTP
// ============================================================================
void fetchAndStoreAgendas() {
    if (!wifiManager.isConnected()) {
        Logger::warn("WiFi no conectado, no se pueden solicitar agendas");
        Logger::info("Continuando con agendas almacenadas localmente");
        return;
    }
    
    // Obtener agendas desde backend
    String agendasJson = httpClient.fetchAgendas();
    
    if (agendasJson.length() == 0) {
        Logger::warn("No se pudieron obtener agendas del backend");
        
        // Verificar si hay agendas almacenadas localmente
        if (spiffsManager.exists("/agenda.json")) {
            Logger::info("Continuando con agendas almacenadas localmente");
            showStoredAgenda();
        } else {
            Logger::warn("No hay agendas locales - sistema funcionara sin agendas programadas");
        }
        return;
    }
    
    Logger::info("Agendas obtenidas exitosamente del backend");
    
    // Procesar como si fuera una sincronización MQTT
    onAgendaSync(agendasJson);
}
