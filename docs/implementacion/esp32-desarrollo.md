# 🔧 ESP8266 - Guía de Desarrollo del Firmware

> **Documento**: Guía técnica completa para desarrollar el firmware del nodo ESP8266 de riego
> **Última actualización**: 2025-12-27
> **Estado**: En desarrollo activo - Hardware actualizado

---

## 📋 Índice

1. [Introducción](#introducción)
2. [Hardware Requerido](#hardware-requerido)
3. [Conexiones y Pines](#conexiones-y-pines)
4. [Arquitectura del Firmware](#arquitectura-del-firmware)
5. [Configuración del Entorno](#configuración-del-entorno)
6. [Estructura del Código](#estructura-del-código)
7. [Implementación por Módulos](#implementación-por-módulos)
8. [Testing y Debugging](#testing-y-debugging)
9. [Despliegue y OTA](#despliegue-y-ota)

---

## 🎯 Introducción

### Objetivo
Desarrollar el firmware para ESP8266 que controle hasta 4 zonas de riego mediante:
- Conexión WiFi robusta con reconexión automática
- Comunicación MQTT con el backend
- Control de relés para electroválvulas
- Lectura de sensor de humedad capacitivo (1 sensor)
- Display OLED SSD1306 para monitoreo visual
- Modo offline con persistencia local
- Sincronización automática de agendas

### Especificaciones Técnicas
- **Microcontrolador**: ESP8266 NodeMCU (CP2102)
- **Clock**: Single-core 80MHz
- **RAM**: 80KB SRAM
- **Flash**: 4MB (LittleFS para almacenamiento)
- **WiFi**: 802.11 b/g/n (2.4GHz)
- **GPIO**: 17 pines disponibles (limitados)
- **ADC**: 10-bit, 1 canal único (A0)
- **Display**: OLED SSD1306 128x64px I2C
- **Voltaje**: 3.3V lógico, alimentación 5V por USB

### Características del Sistema
- ✅ Control de 4 zonas de riego independientes
- ✅ Display OLED con indicadores visuales (WiFi/MQTT/Zonas)
- ✅ Ejecución de agendas programadas localmente
- ✅ Sincronización con backend vía MQTT
- ✅ Modo offline (continúa ejecutando última agenda conocida)
- ✅ Comandos manuales inmediatos
- ✅ Monitoreo de sensor de humedad (1 sensor en A0)
- ✅ Actualización OTA (Over-The-Air)
- ✅ Persistencia de configuración en LittleFS
- ✅ Reconexión automática WiFi/MQTT

---

## 🛠️ Hardware Requerido

### Componentes Principales

#### 1. ESP8266 NodeMCU (CP2102)
- **Modelo**: DevKit v1.0 o compatible
- **Precio estimado**: $3-6 USD
- **Características clave**:
  - 30 pines (17 GPIO utilizables)
  - USB-to-UART CP2102 integrado
  - LED integrado en GPIO2
  - Botón FLASH y RESET
  - Limitación: Solo 1 ADC (A0)

#### 2. Módulo de Relés 4 Canales
- **Tipo**: Relé de estado sólido o mecánico
- **Voltaje de control**: 3.3V (compatible con ESP8266)
- **Voltaje de conmutación**: 220V AC / 10A o 24V DC
- **Optoacoplador**: Sí (aislamiento eléctrico)
- **Lógica**: Activo BAJO (LOW = ON, HIGH = OFF)
- **Precio estimado**: $5-8 USD

**⚠️ Importante**: Verificar que el módulo sea compatible con 3.3V

#### 3. Sensor de Humedad Capacitivo v2.0
- **Cantidad**: 1 (solo un ADC disponible en A0)
- **Tipo**: Capacitivo (evita corrosión vs resistivo)
- **Voltaje**: 3.3V - 5V
- **Salida**: Analógica (ADC) 0-3.3V
- **Rango de valores**: 0-1023 (10-bit ADC ESP8266)
  - **Aire seco**: ~700-1023
  - **Húmedo**: ~300-500
  - **En agua**: ~100-250
- **Calibración**: Requerida
- **Precio estimado**: $2-3 USD

#### 4. Display OLED SSD1306 I2C
- **Resolución**: 128x64 píxeles
- **Interfaz**: I2C (2 pines: SDA, SCL)
- **Voltaje**: 3.3V - 5V
- **Dirección I2C**: 0x3C (estándar)
- **Pines ESP8266**:
  - **SDA**: D7 (GPIO13)
  - **SCL**: D3 (GPIO0)
- **Librerías**: Adafruit SSD1306 + Adafruit GFX
- **Precio estimado**: $3-5 USD
- **Características**:
  - Iconos WiFi/MQTT en cabecera
  - Indicadores de 4 zonas con estado
  - Línea de estado inferior
  - Actualización cada 2 segundos

#### 5. Fuente de Alimentación
- **Voltaje**: 5V DC
- **Corriente mínima**: 1A (ESP8266 + relés + display)
- **Recomendado**: 2A para margen de seguridad
- **Tipo**: Adaptador de pared con barrel jack o USB
- **⚠️ Advertencia**: NO alimentar relés desde pin 5V del ESP8266

#### 6. Electroválvulas
- **Tipo**: Normalmente cerradas (NC)
- **Voltaje**: 24V AC o 12V DC (según instalación)
- **Corriente**: 200-500mA típico
- **Cantidad**: 4 (una por zona)
- **Conexión**: Via relés (NO - Normalmente Abierto)

#### 7. Componentes Adicionales
- **Cables Dupont**: Macho-hembra y macho-macho
- **Protoboard**: Para testing (opcional)
- **PCB custom**: Para instalación permanente (recomendado)
- **Caja estanca**: IP65 para instalación exterior
- **Conectores**: Borneras de tornillo para cables de electroválvulas

---

## 🔌 Conexiones y Pines

### Mapeo de Pines ESP32

```
┌─────────────────────────────────────┐
│         ESP32 NodeMCU DevKit        │
├─────────────────────────────────────┤
│ Pin   │ GPIO  │ Función             │
├───────┼───────┼─────────────────────┤
│ D1    │ TX0   │ Serial Debug (USB)  │
│ D3    │ RX0   │ Serial Debug (USB)  │
│ D2    │ IO2   │ LED Integrado       │
│ D4    │ IO4   │ Relé Zona 1         │
│ D5    │ IO5   │ Relé Zona 2         │
│ D13   │ IO13  │ Relé Zona 3         │
│ D14   │ IO14  │ Relé Zona 4         │
│ D15   │ IO15  │ Relé Zona 5         │
│ D16   │ IO16  │ Relé Zona 6         │
│ D17   │ IO17  │ Relé Zona 7         │
│ D18   │ IO18  │ Relé Zona 8         │
│ D32   │ IO32  │ Sensor Humedad Z1   │
│ D33   │ IO33  │ Sensor Humedad Z2   │
│ D34   │ IO34  │ Sensor Humedad Z3   │
│ D35   │ IO35  │ Sensor Humedad Z4   │
│ D36   │ IO36  │ Sensor Humedad Z5   │
│ D39   │ IO39  │ Sensor Humedad Z6   │
│ 3V3   │ -     │ Alimentación 3.3V   │
│ GND   │ -     │ Tierra              │
│ VIN   │ -     │ Entrada 5V (USB/Ext)│
└───────┴───────┴─────────────────────┘
```

### Criterios de Selección de Pines

#### ✅ Pines SEGUROS para uso general:
- **GPIO 5 (D1)**: Relé Zona 1
- **GPIO 4 (D2)**: Relé Zona 2
- **GPIO 14 (D5)**: Relé Zona 3
- **GPIO 12 (D6)**: Relé Zona 4
- **GPIO 13 (D7)**: SDA Display OLED
- **GPIO 0 (D3)**: SCL Display OLED (pull-up requerido)
- **A0 (ADC0)**: Sensor de humedad

#### ⚠️ Pines CON RESTRICCIONES:
- **GPIO 0 (D3)**: Modo boot (pull-up externo requerido para I2C)
- **GPIO 2 (D4)**: Boot mode (evitar o configurar correctamente)
- **GPIO 15 (D8)**: Boot mode (debe estar LOW durante boot)
- **GPIO 16 (D0)**: No soporta interrupciones, solo OUTPUT/INPUT

#### ❌ Pines NO USAR:
- **GPIO 6-11**: Conectados a flash SPI (reserved)
- **GPIO 1, 3**: TX/RX Serial (debug UART)

### Esquema de Conexión Módulo de Relés

```
ESP8266 GPIO → Módulo Relé 4CH
─────────────────────────────
GPIO5  (D1) → IN1 (Zona 1)
GPIO4  (D2) → IN2 (Zona 2)
GPIO14 (D5) → IN3 (Zona 3)
GPIO12 (D6) → IN4 (Zona 4)
GND         → GND
VIN/5V      → VCC (⚠️ desde fuente externa, NO del ESP8266)
```

**⚠️ Lógica del Relé**: Activo BAJO
- `digitalWrite(pin, LOW)` → Relé ON (electroválvula abierta)
- `digitalWrite(pin, HIGH)` → Relé OFF (electroválvula cerrada)

### Esquema de Conexión Sensores de Humedad

```
Sensor → ESP8266
──────────────
VCC  → 3.3V
GND  → GND
AOUT → A0 (ADC0 - único canal ADC disponible)
```

**⚠️ Nota**: ESP8266 tiene solo un ADC, por lo que sólo puede leer un sensor. Si necesitas leer más sensores, considera usar un multiplexor analógico CD74HC4067.

### Esquema de Conexión Display OLED

```
Display OLED SSD1306 → ESP8266
────────────────────────────────
VCC  → 3.3V
GND  → GND
SDA  → D7 (GPIO13)
SCL  → D3 (GPIO0)
```

**Características del Display:**
- **Resolución**: 128x64 píxeles monocromático
- **Dirección I2C**: 0x3C (estándar)
- **Layout de pantalla**:
  ```
  ┌────────────────────────────┐
  │ WiFi:✓ MQTT:✓        12:34│ ← Iconos de estado
  ├────────────────────────────┤
  │ Z1:■ 15m  Z2:□ ---        │ ← Zonas 1-2 (■=ON □=OFF)
  │ Z3:□ ---  Z4:□ ---        │ ← Zonas 3-4
  ├────────────────────────────┤
  │ Modo: ONLINE              │ ← Línea de estado
  └────────────────────────────┘
  ```
- **Actualización**: Cada 2 segundos
- **Librerías**: Adafruit_SSD1306 + Adafruit_GFX

### Diagrama de Bloques del Sistema

```
                    ┌──────────────┐
                    │  Fuente 5V   │
                    │   2A DC      │
                    └──────┬───────┘
                           │
           ┌───────────────┼──────────────────┐
           │               │                  │
      ┌────▼────┐    ┌─────▼──────┐    ┌─────▼──────┐
      │ ESP8266 │    │ Módulo 4CH │    │   Sensor   │
      │ NodeMCU │◄──►│   Relés    │    │  Humedad   │
      └────┬────┘    └─────┬──────┘    └─────┬──────┘
           │               │                  │
      ┌────▼────┐     ┌────▼──────┐      [ADC A0]
      │ Display │     │ 4x Válvulas│
      │  OLED   │     │  Solenoid │
      │ 128x64  │     └───────────┘
      └─────────┘
           │
      WiFi │
           │
      ┌────▼─────┐
      │  Router  │
      │  WiFi    │
      └────┬─────┘
           │
      ┌────▼─────┐
      │  MQTT    │
      │  Broker  │
      │ (HiveMQ) │
      └────┬─────┘
           │
      ┌────▼─────┐
      │ Backend  │
      │ Spring   │
      └──────────┘
```

---

## 🏗️ Arquitectura del Firmware

### Diagrama de Flujo Principal

Ver [esp32-diagramas-flujo.md](./esp32-diagramas-flujo.md) para diagramas detallados en formato Mermaid.

### Módulos del Firmware

```
firmware/
├── src/
│   ├── main.cpp              # Setup y loop principal
│   ├── config/
│   │   ├── Config.h          # Constantes y configuración
│   │   └── Secrets.h         # WiFi/MQTT credentials (gitignored)
│   ├── network/
│   │   ├── WiFiManager.cpp   # Gestión WiFi con reconexión
│   │   ├── MqttManager.cpp   # Cliente MQTT + pub/sub
│   │   └── TimeSync.cpp      # Sincronización NTP
│   ├── hardware/
│   │   ├── RelayController.cpp   # Control de relés (4 zonas)
│   │   └── HumiditySensor.cpp    # Lectura ADC sensor (A0)
│   ├── display/
│   │   └── DisplayManager.cpp    # Gestión OLED SSD1306
│   ├── scheduler/
│   │   ├── Agenda.cpp         # Modelo de agenda
│   │   ├── AgendaManager.cpp  # Gestión de agendas
│   │   └── TaskScheduler.cpp  # Ejecución temporal
│   ├── storage/
│   │   └── SPIFFSManager.cpp  # Persistencia JSON (LittleFS)
│   └── utils/
│       └── Logger.cpp         # Debug serial
└── platformio.ini             # Configuración PlatformIO
```

### Estados del Sistema

```cpp
enum SystemState {
    INIT,           // Inicializando hardware
    WIFI_CONNECTING,  // Conectando a WiFi
    WIFI_CONNECTED,   // WiFi OK, MQTT pendiente
    MQTT_CONNECTING,  // Conectando a MQTT
    ONLINE,          // Totalmente conectado
    OFFLINE          // Sin WiFi/MQTT, modo local
};
```

### Máquina de Estados

```
    ┌──────┐
    │ INIT │
    └───┬──┘
        │
    ┌───▼────────────┐
    │WIFI_CONNECTING │◄──┐ Reconectar
    └───┬────────────┘   │ cada 30s
        │ Success        │
    ┌───▼────────────┐   │
    │WIFI_CONNECTED  │   │
    └───┬────────────┘   │
        │                │
    ┌───▼────────────┐   │
    │MQTT_CONNECTING │◄──┤ Reconectar
    └───┬────────────┘   │ cada 15s
        │ Success        │
    ┌───▼────────┐       │
    │   ONLINE   │───────┘ On disconnect
    └───┬────────┘
        │ WiFi/MQTT fail
    ┌───▼────────┐
    │  OFFLINE   │──┐
    └────────────┘  │ Retry cada 60s
         ▲          │
         └──────────┘
```

---

## ⚙️ Configuración del Entorno

### Opción 1: PlatformIO (Recomendado)

#### Instalación
1. Instalar Visual Studio Code
2. Instalar extensión PlatformIO IDE
3. Crear nuevo proyecto:
   ```
   Platform: Espressif 32
   Board: ESP32 Dev Module
   Framework: Arduino
   ```

#### platformio.ini
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino

; Configuración de upload
monitor_speed = 115200
upload_speed = 921600

; Librerías
lib_deps = 
    knolleary/PubSubClient@^2.8
    bblanchon/ArduinoJson@^6.21.3
    arduino-libraries/NTPClient@^3.2.1
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.3

; Sistema de archivos
board_build.filesystem = littlefs

; Optimización de memoria
build_flags = 
    -DMQTT_MAX_PACKET_SIZE=1024
    -DPIO_FRAMEWORK_ARDUINO_LWIP2_LOW_MEMORY
```

#### Compilar y Subir
```bash
# Compilar
pio run

# Subir a ESP32
pio run --target upload

# Monitor serial
pio device monitor

# Todo en uno
pio run --target upload && pio device monitor
```

### Opción 2: Arduino IDE

#### Instalación
1. Descargar Arduino IDE 2.x
2. Agregar soporte ESP32:
   - File → Preferences → Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Tools → Board → Boards Manager → buscar "ESP32" → Install

#### Configuración del Board
```
Board: "NodeMCU 1.0 (ESP-12E Module)"
Upload Speed: "921600"
CPU Frequency: "80MHz"
Flash Size: "4MB (FS:2MB OTA:~1019KB)"
Port: [Seleccionar COM port del CP2102]
```

#### Librerías Requeridas
Instalar desde Library Manager:
- PubSubClient by Nick O'Leary
- ArduinoJson by Benoit Blanchon
- NTPClient by Arduino Libraries
- Adafruit SSD1306 by Adafruit
- Adafruit GFX Library by Adafruit

---

## 📂 Estructura del Código

### Archivo Principal: main.cpp

```cpp
#include <Arduino.h>
#include "config/Config.h"
#include "config/Secrets.h"
#include "network/WiFiManager.h"
#include "network/MqttManager.h"
#include "hardware/RelayController.h"
#include "hardware/HumiditySensor.h"
#include "scheduler/AgendaManager.h"
#include "storage/SPIFFSManager.h"
#include "utils/Logger.h"
#include "utils/TimeSync.h"

// Estado global del sistema
SystemState systemState = INIT;

// Instancias de módulos
WiFiManager wifiMgr;
MqttManager mqttMgr;
RelayController relays;
HumiditySensor humidity[MAX_ZONES];
AgendaManager agendaMgr;
SPIFFSManager storage;
TimeSync timeSync;

void setup() {
    Serial.begin(115200);
    Logger::info("Sistema de riego ESP32 iniciando...");
    
    // Inicializar hardware
    relays.init();
    for (int i = 0; i < MAX_ZONES; i++) {
        humidity[i].init(SENSOR_PINS[i], i + 1);
    }
    
    // Cargar configuración desde SPIFFS
    storage.init();
    if (storage.loadConfig()) {
        Logger::info("Configuración cargada desde SPIFFS");
    }
    
    // Conectar WiFi
    wifiMgr.connect(WIFI_SSID, WIFI_PASSWORD);
    systemState = WIFI_CONNECTING;
}

void loop() {
    // Máquina de estados principal
    switch (systemState) {
        case WIFI_CONNECTING:
            if (wifiMgr.isConnected()) {
                Logger::info("WiFi conectado");
                timeSync.sync();  // Sincronizar reloj
                systemState = MQTT_CONNECTING;
            } else if (millis() > WIFI_TIMEOUT) {
                Logger::warn("WiFi timeout, modo offline");
                systemState = OFFLINE;
            }
            break;
            
        case MQTT_CONNECTING:
            if (!mqttMgr.isConnected()) {
                mqttMgr.connect(MQTT_BROKER, MQTT_PORT, NODE_ID);
            } else {
                Logger::info("MQTT conectado");
                mqttMgr.subscribe();  // Suscribir a topics
                systemState = ONLINE;
            }
            break;
            
        case ONLINE:
            mqttMgr.loop();  // Procesar mensajes MQTT
            if (!wifiMgr.isConnected() || !mqttMgr.isConnected()) {
                Logger::warn("Conexión perdida, reconectando...");
                systemState = WIFI_CONNECTING;
            }
            break;
            
        case OFFLINE:
            // Intentar reconectar cada 60s
            if (millis() % 60000 < 100) {
                wifiMgr.connect(WIFI_SSID, WIFI_PASSWORD);
                systemState = WIFI_CONNECTING;
            }
            break;
    }
    
    // Tareas comunes (online y offline)
    agendaMgr.loop();  // Ejecutar agendas programadas
    
    // Publicar estado de zonas (si online)
    if (systemState == ONLINE) {
        publishZoneStatus();  // Cada 5 segundos
    }
    
    // Leer sensores de humedad (cada 1 minuto)
    readAndPublishHumidity();
    
    delay(100);  // Evitar watchdog timeout
}
```

### Config.h - Configuración General

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ============= Hardware Config =============
#define MAX_ZONES 4
#define LED_PIN 16  // GPIO16 (D0)

// Pines de relés (Activo BAJO)
const int RELAY_PINS[MAX_ZONES] = {5, 4, 14, 12};  // D1, D2, D5, D6

// Pin de sensor de humedad (ADC único)
#define SENSOR_PIN A0  // A0 (ADC0)

// Display OLED I2C
#define OLED_SDA 13  // D7 (GPIO13)
#define OLED_SCL 0   // D3 (GPIO0)
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Calibración de sensor (ajustar por sensor)
#define SENSOR_WET_VALUE 250    // En agua
#define SENSOR_DRY_VALUE 700    // En aire

// ============= Network Config =============
#define WIFI_TIMEOUT 30000      // 30 segundos
#define MQTT_RECONNECT_DELAY 15000  // 15 segundos
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC -10800   // GMT-3 (Argentina)

// ============= MQTT Topics =============
#define TOPIC_CMD_PREFIX "riego/%s/cmd/zona/"
#define TOPIC_STATUS_PREFIX "riego/%s/status/zona/"
#define TOPIC_AGENDA_SYNC "riego/%s/agenda/sync"
#define TOPIC_HUMIDITY "riego/%s/humedad/zona/"

// ============= Timing Config =============
#define STATUS_PUBLISH_INTERVAL 5000   // 5 segundos
#define HUMIDITY_READ_INTERVAL 60000   // 1 minuto
#define DISPLAY_UPDATE_INTERVAL 2000   // 2 segundos
#define MAX_RIEGO_DURATION 7200        // 2 horas max
#define AGENDA_CHECK_INTERVAL 10000    // 10 segundos

// ============= Storage Config =============
#define AGENDA_FILE "/agendas.json"
#define CONFIG_FILE "/config.json"
#define MAX_AGENDAS_PER_ZONE 5

// ============= MQTT Config =============
#define MQTT_BUFFER_SIZE 1024  // Buffer para mensajes MQTT

// ============= Debug Config =============
#define DEBUG_SERIAL true
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

// ============= Memory Usage =============
// Flash: 376911 bytes (36.1%)
// RAM: 37268 bytes (45.5%)

#endif // CONFIG_H
```

### Secrets.h - Credenciales (⚠️ NO SUBIR A GIT)

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Credentials
#define WIFI_SSID "TU_WIFI_SSID"
#define WIFI_PASSWORD "tu_password_wifi"

// MQTT Credentials (si el broker requiere auth)
#define MQTT_BROKER "192.168.1.100"  // IP del backend/broker
#define MQTT_PORT 1883
#define MQTT_USER ""  // Opcional (vacío = sin auth)
#define MQTT_PASSWORD ""  // Opcional

// Backend HTTP API (si necesitas hacer peticiones HTTP)
// El backend usa HTTP Basic Authentication
// Desarrollo: admin:dev123
#define BACKEND_HTTP_USER "admin"
#define BACKEND_HTTP_PASSWORD "dev123"  // Cambiar en producción

// Node ID único (generar UUID)
#define NODE_ID "550e8400-e29b-41d4-a716-446655440000"

// OTA Password (opcional, para actualizaciones remotas)
#define OTA_PASSWORD "esp32_ota_pass"

#endif // SECRETS_H
```

**⚠️ Importante**: Agregar `Secrets.h` al `.gitignore`:
```gitignore
# ESP32 secrets
esp32/firmware/src/config/Secrets.h
```

---

## 🔧 Implementación por Módulos

### 1. RelayController.cpp

**Responsabilidad**: Control de 4 relés con lógica activo-bajo

```cpp
#include "RelayController.h"
#include "../config/Config.h"
#include "../utils/Logger.h"

RelayController::RelayController() {
    for (int i = 0; i < MAX_ZONES; i++) {
        zoneState[i] = false;
        zoneTimer[i] = 0;
    }
}

void RelayController::init() {
    Logger::info("Inicializando relés...");
    for (int i = 0; i < MAX_ZONES; i++) {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], HIGH);  // OFF inicial (activo bajo)
    }
    Logger::info("Relés inicializados (todos OFF)");
}

void RelayController::turnOn(int zona, int duracionSeg) {
    if (zona < 1 || zona > MAX_ZONES) {
        Logger::error("Zona inválida: " + String(zona));
        return;
    }
    
    int idx = zona - 1;
    digitalWrite(RELAY_PINS[idx], LOW);  // ON (activo bajo)
    zoneState[idx] = true;
    zoneTimer[idx] = duracionSeg;
    
    Logger::info("Zona " + String(zona) + " ON (" + String(duracionSeg) + "s)");
}

void RelayController::turnOff(int zona) {
    if (zona < 1 || zona > MAX_ZONES) return;
    
    int idx = zona - 1;
    digitalWrite(RELAY_PINS[idx], HIGH);  // OFF
    zoneState[idx] = false;
    zoneTimer[idx] = 0;
    
    Logger::info("Zona " + String(zona) + " OFF");
}

void RelayController::loop() {
    unsigned long now = millis();
    static unsigned long lastCheck = 0;
    
    // Verificar cada segundo
    if (now - lastCheck < 1000) return;
    lastCheck = now;
    
    // Decrementar timers activos
    for (int i = 0; i < MAX_ZONES; i++) {
        if (zoneState[i] && zoneTimer[i] > 0) {
            zoneTimer[i]--;
            
            // Apagar si llegó a 0
            if (zoneTimer[i] == 0) {
                turnOff(i + 1);
            }
        }
    }
}

bool RelayController::isActive(int zona) {
    if (zona < 1 || zona > MAX_ZONES) return false;
    return zoneState[zona - 1];
}

int RelayController::getRemainingTime(int zona) {
    if (zona < 1 || zona > MAX_ZONES) return 0;
    return zoneTimer[zona - 1];
}
```

### 2. MqttManager.cpp

**Responsabilidad**: Cliente MQTT con reconexión automática

```cpp
#include "MqttManager.h"
#include "../config/Config.h"
#include "../config/Secrets.h"
#include "../utils/Logger.h"
#include <ArduinoJson.h>

PubSubClient mqttClient;

MqttManager::MqttManager() : lastReconnect(0) {}

void MqttManager::init(WiFiClient& wifiClient) {
    mqttClient.setClient(wifiClient);
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->onMessage(topic, payload, length);
    });
}

bool MqttManager::connect() {
    if (mqttClient.connected()) return true;
    
    // Throttle de reconexión
    if (millis() - lastReconnect < MQTT_RECONNECT_DELAY) return false;
    lastReconnect = millis();
    
    Logger::info("Conectando a MQTT...");
    String clientId = "ESP32_" + String(NODE_ID);
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
        Logger::info("MQTT conectado");
        subscribe();
        return true;
    } else {
        Logger::error("MQTT falló, rc=" + String(mqttClient.state()));
        return false;
    }
}

void MqttManager::subscribe() {
    // Comandos por zona
    char topic[100];
    snprintf(topic, sizeof(topic), "riego/%s/cmd/zona/+", NODE_ID);
    mqttClient.subscribe(topic);
    Logger::info("Suscrito a: " + String(topic));
    
    // Sincronización de agenda
    snprintf(topic, sizeof(topic), TOPIC_AGENDA_SYNC, NODE_ID);
    mqttClient.subscribe(topic);
    Logger::info("Suscrito a: " + String(topic));
}

void MqttManager::loop() {
    if (!mqttClient.connected()) {
        connect();
    }
    mqttClient.loop();
}

void MqttManager::onMessage(char* topic, byte* payload, unsigned int length) {
    String strTopic(topic);
    String strPayload;
    for (unsigned int i = 0; i < length; i++) {
        strPayload += (char)payload[i];
    }
    
    Logger::info("MQTT ← " + strTopic + ": " + strPayload);
    
    // Parsear comando de zona
    if (strTopic.indexOf("/cmd/zona/") > 0) {
        handleCommand(strTopic, strPayload);
    }
    // Parsear sincronización de agenda
    else if (strTopic.indexOf("/agenda/sync") > 0) {
        handleAgendaSync(strPayload);
    }
}

void MqttManager::handleCommand(String topic, String payload) {
    // Extraer número de zona del topic: riego/{nodeId}/cmd/zona/{N}
    int lastSlash = topic.lastIndexOf('/');
    int zona = topic.substring(lastSlash + 1).toInt();
    
    // Parsear JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Logger::error("JSON inválido en comando");
        return;
    }
    
    String accion = doc["accion"] | "";
    int duracion = doc["duracion"] | 0;
    
    // Ejecutar comando
    if (accion == "ON") {
        relayController.turnOn(zona, duracion);
    } else if (accion == "OFF") {
        relayController.turnOff(zona);
    }
}

void MqttManager::publishZoneStatus(int zona) {
    char topic[100];
    snprintf(topic, sizeof(topic), "riego/%s/status/zona/%d", NODE_ID, zona);
    
    StaticJsonDocument<256> doc;
    doc["activa"] = relayController.isActive(zona);
    doc["tiempoRestante"] = relayController.getRemainingTime(zona);
    
    String output;
    serializeJson(doc, output);
    
    mqttClient.publish(topic, output.c_str());
}
```

### 3. AgendaManager.cpp

**Responsabilidad**: Gestionar y ejecutar agendas programadas

```cpp
#include "AgendaManager.h"
#include "../config/Config.h"
#include "../utils/Logger.h"
#include <time.h>

AgendaManager::AgendaManager() : agendaCount(0), lastCheck(0) {}

void AgendaManager::addAgenda(Agenda agenda) {
    if (agendaCount >= MAX_AGENDAS) {
        Logger::warn("Límite de agendas alcanzado");
        return;
    }
    agendas[agendaCount++] = agenda;
    Logger::info("Agenda agregada: Zona " + String(agenda.zona) + 
                 " @ " + agenda.horaInicio);
}

void AgendaManager::clearAll() {
    agendaCount = 0;
    Logger::info("Agendas limpiadas");
}

void AgendaManager::loop() {
    unsigned long now = millis();
    
    // Verificar cada segundo
    if (now - lastCheck < AGENDA_CHECK_INTERVAL) return;
    lastCheck = now;
    
    // Obtener hora actual
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    int currentHour = timeinfo->tm_hour;
    int currentMin = timeinfo->tm_min;
    int currentSec = timeinfo->tm_sec;
    int currentWday = timeinfo->tm_wday;  // 0=Domingo, 1=Lunes, ...
    
    // Solo ejecutar en el segundo 0 de cada minuto
    if (currentSec != 0) return;
    
    // Verificar cada agenda
    for (int i = 0; i < agendaCount; i++) {
        Agenda& ag = agendas[i];
        
        if (!ag.activa) continue;
        
        // Verificar día de la semana
        if (!isDayActive(ag, currentWday)) continue;
        
        // Verificar hora
        if (ag.horaInicio.hour != currentHour || 
            ag.horaInicio.minute != currentMin) continue;
        
        // ¡Ejecutar agenda!
        Logger::info("Ejecutando agenda: Zona " + String(ag.zona));
        relayController.turnOn(ag.zona, ag.duracionMin * 60);
    }
}

bool AgendaManager::isDayActive(Agenda& agenda, int wday) {
    // Convertir wday (0=Dom) a índice de diasSemana (0=Lun)
    int dayIndex = (wday == 0) ? 6 : wday - 1;  // Dom=6, Lun=0, ..., Sab=5
    return agenda.diasSemana[dayIndex];
}
```

### 4. SPIFFSManager.cpp

**Responsabilidad**: Persistencia de agendas en formato JSON

```cpp
#include "SPIFFSManager.h"
#include "../config/Config.h"
#include "../utils/Logger.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

SPIFFSManager::SPIFFSManager() {}

bool SPIFFSManager::init() {
    if (!SPIFFS.begin(true)) {  // true = format on fail
        Logger::error("SPIFFS mount falló");
        return false;
    }
    Logger::info("SPIFFS montado");
    return true;
}

bool SPIFFSManager::saveAgendas(Agenda* agendas, int count) {
    File file = SPIFFS.open(AGENDA_FILE, "w");
    if (!file) {
        Logger::error("No se pudo abrir archivo para escritura");
        return false;
    }
    
    StaticJsonDocument<2048> doc;
    JsonArray array = doc.createNestedArray("agendas");
    
    for (int i = 0; i < count; i++) {
        JsonObject obj = array.createNestedObject();
        obj["zona"] = agendas[i].zona;
        obj["hora"] = String(agendas[i].horaInicio.hour) + ":" + 
                      String(agendas[i].horaInicio.minute);
        obj["duracionMin"] = agendas[i].duracionMin;
        obj["activa"] = agendas[i].activa;
        
        JsonArray dias = obj.createNestedArray("dias");
        for (int d = 0; d < 7; d++) {
            if (agendas[i].diasSemana[d]) dias.add(d);
        }
    }
    
    serializeJson(doc, file);
    file.close();
    
    Logger::info("Agendas guardadas en SPIFFS");
    return true;
}

int SPIFFSManager::loadAgendas(Agenda* agendas, int maxCount) {
    File file = SPIFFS.open(AGENDA_FILE, "r");
    if (!file) {
        Logger::warn("Archivo de agendas no encontrado");
        return 0;
    }
    
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Logger::error("Error parseando JSON de agendas");
        return 0;
    }
    
    JsonArray array = doc["agendas"];
    int count = 0;
    
    for (JsonObject obj : array) {
        if (count >= maxCount) break;
        
        agendas[count].zona = obj["zona"];
        
        // Parsear hora (formato "HH:MM")
        String horaStr = obj["hora"];
        int sep = horaStr.indexOf(':');
        agendas[count].horaInicio.hour = horaStr.substring(0, sep).toInt();
        agendas[count].horaInicio.minute = horaStr.substring(sep + 1).toInt();
        
        agendas[count].duracionMin = obj["duracionMin"];
        agendas[count].activa = obj["activa"];
        
        // Parsear días activos
        for (int d = 0; d < 7; d++) agendas[count].diasSemana[d] = false;
        JsonArray dias = obj["dias"];
        for (int dia : dias) {
            agendas[count].diasSemana[dia] = true;
        }
        
        count++;
    }
    
    Logger::info("Cargadas " + String(count) + " agendas desde SPIFFS");
    return count;
}
```

---

## 🧪 Testing y Debugging

### Serial Monitor
- **Baudrate**: 115200
- **Logs**: Prefijos `[INFO]`, `[WARN]`, `[ERROR]`
- **Ejemplo**:
  ```
  [INFO] WiFi conectado: 192.168.1.50
  [INFO] MQTT conectado
  [INFO] Zona 1 ON (600s)
  [WARN] Reconectando MQTT...
  ```

### Testing con Mock Backend
1. Levantar Docker Compose
2. Ejecutar mock Python
3. Subir firmware a ESP32
4. Verificar en serial:
   - Conexión WiFi/MQTT
   - Recepción de comandos
   - Publicación de estado

### Debugging con LEDs
- LED integrado (GPIO2): Parpadea durante operación
- LEDs externos: Conectar a zonas para visualizar activación

### Herramientas
- **MQTT Explorer**: GUI para visualizar topics
- **PlatformIO Debugger**: Breakpoints y step-through
- **Logic Analyzer**: Verificar señales GPIO

---

## 🚀 Despliegue y OTA

### OTA (Over-The-Air Updates)
- Permite actualizar firmware sin cable USB
- Requiere WiFi conectado
- Implementar con `ArduinoOTA` library

### Configuración OTA (futuro)
```cpp
#include <ArduinoOTA.h>

void setupOTA() {
    ArduinoOTA.setHostname("ESP32-Riego");
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    ArduinoOTA.onStart([]() {
        Logger::info("Iniciando actualización OTA...");
    });
    
    ArduinoOTA.begin();
}

void loop() {
    ArduinoOTA.handle();
    // ... resto del código
}
```

---

## 📋 Checklist de Desarrollo

### Fase 1: Setup Básico
- [ ] Configurar PlatformIO/Arduino IDE
- [ ] Crear proyecto con estructura de carpetas
- [ ] Implementar `Config.h` y `Secrets.h`
- [ ] Test de compilación

### Fase 2: Hardware
- [ ] Implementar `RelayController`
- [ ] Test de activación de relés (sin electroválvulas)
- [ ] Implementar `HumiditySensor`
- [ ] Calibrar sensores de humedad

### Fase 3: Network
- [ ] Implementar `WiFiManager` con reconexión
- [ ] Implementar `MqttManager` básico
- [ ] Test de conexión y ping
- [ ] Implementar suscripción a comandos

### Fase 4: Logic
- [ ] Implementar `Agenda` y `AgendaManager`
- [ ] Test de parsing de agendas MQTT
- [ ] Implementar `SPIFFSManager` para persistencia
- [ ] Test de carga/guardado de agendas

### Fase 5: Integration
- [ ] Integrar todos los módulos en `main.cpp`
- [ ] Test end-to-end con mock backend
- [ ] Test de reconexión WiFi/MQTT
- [ ] Test de modo offline

### Fase 6: Production
- [ ] Montaje en PCB/protoboard
- [ ] Instalación en caja estanca
- [ ] Pruebas con electroválvulas reales
- [ ] Documentar troubleshooting

---

## 🔗 Referencias

- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [PubSubClient Library](https://github.com/knolleary/pubsubclient)
- [ArduinoJson Documentation](https://arduinojson.org/)
- [SPIFFS File System](https://github.com/pellepl/spiffs)
- [PlatformIO ESP32](https://docs.platformio.org/en/latest/platforms/espressif32.html)

---

**Próximos pasos**: Ver [esp32-diagramas-conexion.json](./esp32-diagramas-conexion.json) y [esp32-diagramas-flujo.md](./esp32-diagramas-flujo.md)
