#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURACIÓN GENERAL DEL SISTEMA DE RIEGO ESP32
// ============================================================================


// ============= Hardware Config para NodeMCU ESP8266 =============
#define MAX_ZONES 4
#define MAX_SENSORS 1  // Solo 1 pin ADC disponible (A0)
#define LED_PIN 2      // LED integrado en NodeMCU (GPIO2)

// Pines de relés (OUTPUT - Lógica invertida: LOW=ON, HIGH=OFF)
// D1=GPIO5, D2=GPIO4, D5=GPIO14, D6=GPIO12
static const int RELAY_PINS[MAX_ZONES] = {
    5,   // D1 - Zona 1
    4,   // D2 - Zona 2
    14,  // D5 - Zona 3
    12   // D6 - Zona 4
};

// Pines de sensores de humedad (INPUT - ADC 10-bit)
// Solo A0 disponible en ESP8266
static const int SENSOR_PINS[MAX_SENSORS] = {
    A0   // Sensor único conectado a A0
    // Para más sensores, se requiere multiplexor externo
};

// Pines I2C para pantalla OLED
#define I2C_SDA 13  // D7 (GPIO13)
#define I2C_SCL 0   // D3 (GPIO0)
#define OLED_ADDRESS 0x3C  // Dirección 7-bit (0x78 >> 1)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Calibración de sensores de humedad
// ⚠️ IMPORTANTE: Calibrar cada sensor individualmente
// - Medir valor en aire seco → SENSOR_DRY_VALUE
// - Medir valor en agua → SENSOR_WET_VALUE
#define SENSOR_WET_VALUE 700    // Valor ADC en agua (≈ 100% humedad)
#define SENSOR_DRY_VALUE 3000   // Valor ADC en aire (≈ 0% humedad)

// ============= Network Config =============
#define WIFI_TIMEOUT 30000           // 30 segundos para conectar WiFi
#define MQTT_RECONNECT_DELAY 15000   // 15 segundos entre intentos MQTT
#define OFFLINE_RETRY_INTERVAL 60000 // 60 segundos en modo offline

// NTP - Sincronización de tiempo
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC -10800        // GMT-3 (Argentina) = -3 * 3600
#define DAYLIGHT_OFFSET_SEC 0        // Sin horario de verano

// ============= MQTT Topics Templates =============
// Usar snprintf para reemplazar %s con NODE_ID
#define TOPIC_CMD_PATTERN "riego/%s/cmd/zona/+"
#define TOPIC_AGENDA_SYNC_PATTERN "riego/%s/agenda/sync"
#define TOPIC_STATUS_PATTERN "riego/%s/status/zona/%d"
#define TOPIC_HUMIDITY_PATTERN "riego/%s/humedad/zona/%d"

// Nota: MQTT NO requiere autenticación en desarrollo (broker HiveMQ local)
// El backend Spring Boot usa HTTP Basic Auth (admin:dev123) pero eso es
// para endpoints HTTP REST (/api/**), no afecta a la comunicación MQTT

// ============= Timing Config =============
#define STATUS_PUBLISH_INTERVAL 5000    // Publicar estado cada 5 segundos
#define HUMIDITY_READ_INTERVAL 60000    // Leer sensores cada 60 segundos
#define AGENDA_CHECK_INTERVAL 1000      // Verificar agendas cada 1 segundo
#define RELAY_UPDATE_INTERVAL 1000      // Actualizar timers cada 1 segundo

// Límites de duración de riego
#define MIN_RIEGO_DURATION 1       // Mínimo 1 segundo
#define MAX_RIEGO_DURATION 7200    // Máximo 2 horas (7200 segundos)

// ============= Storage Config (SPIFFS/LittleFS) =============
#define AGENDA_FILE "/agendas.json"
#define CONFIG_FILE "/config.json"
#define MAX_AGENDAS 30  // Máximo de agendas totales (todas las zonas)

// Tamaño de buffers JSON
#define JSON_BUFFER_SMALL 256   // Comandos, estado
#define JSON_BUFFER_MEDIUM 512  // Lecturas de sensores
#define JSON_BUFFER_LARGE 2048  // Sincronización de agendas

// ============= Debug Config =============
#define DEBUG_SERIAL true
#define SERIAL_BAUD_RATE 115200

// Niveles de log
#define LOG_LEVEL_NONE  0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_INFO  3
#define LOG_LEVEL_DEBUG 4

// Nivel actual (cambiar para más/menos verbose)
#define LOG_LEVEL LOG_LEVEL_INFO

// ============= System States =============
enum SystemState {
    INIT,              // Inicializando hardware
    WIFI_CONNECTING,   // Conectando a WiFi
    WIFI_CONNECTED,    // WiFi OK, pendiente MQTT
    MQTT_CONNECTING,   // Conectando a MQTT
    ONLINE,            // Totalmente conectado y operativo
    OFFLINE            // Sin conexión, modo autónomo
};

// Nombres de estados para logs
static const char* STATE_NAMES[] = {
    "INIT",
    "WIFI_CONNECTING",
    "WIFI_CONNECTED",
    "MQTT_CONNECTING",
    "ONLINE",
    "OFFLINE"
};

// ============= Días de la Semana =============
// Mapeo para agendas
static const char* DIAS_SEMANA[] = {
    "LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"
};

// ============= Configuración de Relés =============
// Los módulos de relés suelen tener lógica invertida
#define RELAY_ON LOW    // LOW activa el relé
#define RELAY_OFF HIGH  // HIGH desactiva el relé

// ============= Watchdog Config =============
// Tiempo máximo sin yield() antes de reset automático
#define LOOP_DELAY_MS 100  // Delay mínimo en loop()

// ============= Version del Firmware =============
#define FIRMWARE_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

#endif // CONFIG_H
