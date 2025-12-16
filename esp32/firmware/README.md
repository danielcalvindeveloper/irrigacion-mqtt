# Firmware ESP32 - Sistema de Riego MQTT

Firmware para ESP32 NodeMCU que controla hasta 8 zonas de riego mediante MQTT.

## 🚀 Quick Start

### 1. Configurar Credenciales

Copiar el template de secrets:
```bash
cp src/config/Secrets.h.example src/config/Secrets.h
```

Editar `src/config/Secrets.h` con tus credenciales:
```cpp
#define WIFI_SSID "tu_wifi"
#define WIFI_PASSWORD "tu_password"
#define MQTT_BROKER "192.168.1.100"  // IP del backend
#define NODE_ID "tu-node-uuid"
```

### 2. Compilar y Subir

Con PlatformIO:
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

Con Arduino IDE:
1. Abrir `src/main.cpp`
2. Tools → Board → "ESP32 Dev Module"
3. Tools → Upload Speed → "921600"
4. Sketch → Upload

## 📂 Estructura del Proyecto

```
firmware/
├── platformio.ini          # Configuración PlatformIO
├── src/
│   ├── main.cpp           # Loop principal
│   ├── config/
│   │   ├── Config.h       # Constantes globales
│   │   ├── Secrets.h      # Credenciales (gitignored)
│   │   └── Secrets.h.example  # Template
│   ├── network/
│   │   ├── WiFiManager.cpp/h   # Gestión WiFi
│   │   └── MqttManager.cpp/h   # Cliente MQTT
│   ├── hardware/
│   │   ├── RelayController.cpp/h     # Control de relés
│   │   └── HumiditySensor.cpp/h      # Lectura de sensores
│   ├── scheduler/
│   │   ├── Agenda.h              # Modelo de datos
│   │   └── AgendaManager.cpp/h   # Ejecución de agendas
│   ├── storage/
│   │   └── SPIFFSManager.cpp/h   # Persistencia JSON
│   └── utils/
│       ├── Logger.cpp/h          # Debug serial
│       └── TimeSync.cpp/h        # Sincronización NTP
└── test/                         # Tests unitarios (futuro)
```

## 🔧 Configuración de Hardware

Ver documentación detallada en [../../docs/implementacion/esp32-desarrollo.md](../../docs/implementacion/esp32-desarrollo.md)

### Pines de Relés (Activo BAJO)
- GPIO4  → Zona 1
- GPIO5  → Zona 2
- GPIO13 → Zona 3
- GPIO14 → Zona 4
- GPIO15 → Zona 5
- GPIO16 → Zona 6
- GPIO17 → Zona 7
- GPIO18 → Zona 8

### Pines de Sensores ADC
- GPIO32 → Sensor Zona 1
- GPIO33 → Sensor Zona 2
- GPIO34 → Sensor Zona 3
- GPIO35 → Sensor Zona 4
- GPIO36 → Sensor Zona 5
- GPIO39 → Sensor Zona 6

## 📡 Topics MQTT

### Suscripción (ESP32 escucha)
- `riego/{nodeId}/cmd/zona/+` - Comandos manuales
- `riego/{nodeId}/agenda/sync` - Sincronización de agendas

### Publicación (ESP32 publica)
- `riego/{nodeId}/status/zona/{N}` - Estado de cada zona
- `riego/{nodeId}/humedad/zona/{N}` - Lecturas de sensores

## 🧪 Testing

### Test básico de compilación
```bash
pio run
```

### Test con mock backend
1. Levantar stack Docker:
   ```bash
   cd ../..
   docker-compose up -d
   ```

2. Ejecutar mock Python:
   ```bash
   cd ..
   python mock_esp32.py --node-id {tu-node-id}
   ```

3. Subir firmware real al ESP32

### Monitor serial
```bash
pio device monitor --baud 115200 --filter esp32_exception_decoder
```

## 🐛 Debugging

### Logs seriales
El firmware incluye logs detallados:
```
[INFO] Sistema de riego ESP32 iniciando...
[INFO] WiFi conectado: 192.168.1.50
[INFO] MQTT conectado
[INFO] Zona 1 ON (600s)
[WARN] Reconectando MQTT...
[ERROR] JSON inválido en comando
```

### Niveles de log
Configurar en `platformio.ini`:
```ini
build_flags = 
    -DCORE_DEBUG_LEVEL=3  ; 0=None, 1=Error, 2=Warn, 3=Info, 4=Debug, 5=Verbose
```

### Herramientas
- **MQTT Explorer**: Visualizar mensajes MQTT
- **Serial Plotter**: Graficar lecturas de sensores
- **PlatformIO Debugger**: Breakpoints y step-through

## 📚 Documentación Relacionada

- [Guía de Desarrollo ESP32](../../docs/implementacion/esp32-desarrollo.md)
- [Diagramas de Conexión](../../docs/implementacion/esp32-diagramas-conexion.json)
- [Diagramas de Flujo](../../docs/implementacion/esp32-diagramas-flujo.md)
- [Contratos MQTT/HTTP](../../docs/implementacion/contratos-mqtt-http.md)

## 🔒 Seguridad

### Credenciales
- **NUNCA** subir `Secrets.h` al repositorio
- Usar credenciales diferentes para desarrollo y producción
- Habilitar MQTT con TLS en producción

### MQTT en Producción
```cpp
#define MQTT_TLS true
#define MQTT_PORT 8883
#define MQTT_USER "esp32_user"
#define MQTT_PASSWORD "strong_password"
```

### Autenticación del Backend

**El backend Spring Boot usa HTTP Basic Auth** (`admin:dev123` en desarrollo).

**¿El ESP32 necesita autenticarse?**
- **MQTT**: ❌ NO - El broker HiveMQ local no requiere auth
- **HTTP REST** (opcional): ✅ SÍ - Si implementas peticiones HTTP

Ver `Secrets.h.example` para configuración de credenciales HTTP.

## 🚀 Próximos Pasos

1. ✅ Configurar credenciales
2. ✅ Compilar y subir firmware
3. ✅ Verificar conexión WiFi/MQTT en serial
4. ⏳ Implementar módulos faltantes
5. ⏳ Probar con electroválvulas reales
6. ⏳ Implementar OTA para actualizaciones remotas

## 📝 Estado de Implementación

- [x] Estructura del proyecto
- [x] Configuración PlatformIO
- [ ] WiFiManager (en desarrollo)
- [ ] MqttManager (en desarrollo)
- [ ] RelayController (en desarrollo)
- [ ] HumiditySensor (en desarrollo)
- [ ] AgendaManager (en desarrollo)
- [ ] SPIFFSManager (en desarrollo)
- [ ] TimeSync (en desarrollo)
- [ ] Tests unitarios (pendiente)
- [ ] OTA updates (pendiente)

## 🤝 Contribuir

Ver [CONTRIBUTING.md](../../CONTRIBUTING.md) para guías de contribución.

## 📄 Licencia

Ver [LICENSE](../../LICENSE) para detalles.
