# Firmware ESP8266 - Sistema de Riego MQTT

Firmware para ESP8266 NodeMCU que controla hasta 8 zonas de riego mediante MQTT, con display OLED para monitoreo visual en tiempo real.

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

Con PlatformIO (cuatro perfiles):
```bash
# PERFIL RELES (activo en bajo) - USB
pio run -e nodemcuv2
pio run -e nodemcuv2 -t upload --upload-port COM3
pio device monitor -p COM3 -b 115200

# PERFIL RELES (activo en bajo) - OTA
pio run -e nodemcuv2_ota -t upload --upload-port 192.168.10.140

# PERFIL MOSFET (activo en alto) - USB
pio run -e nodemcuv2_mosfet
pio run -e nodemcuv2_mosfet -t upload --upload-port COM3
pio device monitor -p COM3 -b 115200

# PERFIL MOSFET (activo en alto) - OTA
pio run -e nodemcuv2_mosfet_ota -t upload --upload-port 192.168.10.140
```

### 3. Actualizar Firmware por WiFi (OTA)

Una vez que el firmware con OTA fue cargado por USB al menos una vez, puedes actualizar por IP sin cable USB:

```bash
# PERFIL RELES
pio run -e nodemcuv2_ota -t upload --upload-port 192.168.10.140

# PERFIL MOSFET
pio run -e nodemcuv2_mosfet_ota -t upload --upload-port 192.168.10.140
```

Notas OTA:
- Puerto OTA por defecto: `8266`.
- El hostname OTA se publica como `riego-{NODE_ID}`.
- La IP por defecto configurada hoy es `192.168.10.140`.
- Si cambia la IP del ESP, vuelve a ejecutar el comando con la nueva IP.
- Si falla OTA, hacer una carga USB y reintentar.

### 4. Reset y Reconfiguración WiFi por Botón Físico

No existe fallback automático a AP por caída de router. El portal de configuración solo se abre por acción física.

- Botón conectado a `GPIO2` (configurado con `INPUT_PULLUP`, activo en `LOW`).
- Mantener presionado `>= 5s`: abre portal de configuración WiFi (sin borrar config existente).
- Mantener presionado `>= 10s`: factory reset (borra `config.json`) y abre portal.

Al abrir portal:
- El ESP levanta AP: `RIEGO-CONFIG-{chipid}`
- Password AP: `riego1234`
- URL: `http://192.168.4.1`
- Muestra lista de redes WiFi detectadas (escaneo local)
- Verifica conexión real antes de guardar
- Informa resultado (éxito/error) en la página y en OLED
- Permite configurar también: MQTT (`host/puerto/user/pass`), Backend HTTP (`host/puerto/user/pass`) y `Node ID`

Desde esa página se guarda toda la configuración runtime en LittleFS (`/config.json`) y el equipo se reinicia.

Notas:
- `nodemcuv2` usa salida activa en bajo (placa de relés tradicional).
- `nodemcuv2_ota` usa el mismo firmware de relés, pero sube por WiFi.
- `nodemcuv2_mosfet` usa salida activa en alto (`-DOUTPUT_ACTIVE_HIGH`).
- `nodemcuv2_mosfet_ota` usa el mismo firmware MOSFET, pero sube por WiFi.
- Si cambia el puerto USB, actualizar `COM3` por el puerto detectado.

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
- GPIO5 (D1)  → Zona 1
- GPIO4 (D2)  → Zona 2
- GPIO14 (D5) → Zona 3
- GPIO12 (D6) → Zona 4

### Pin de Sensor ADC
- A0 (GPIO17) → Sensor de humedad único
  - Nota: Expansión a más sensores requiere multiplexor externo (ej: CD4051)

### Pines de Display OLED I2C ⭐ NUEVO
- GPIO13 (D7) → SDA (datos I2C)
- GPIO0 (D3)  → SCL (clock I2C)
- Dirección I2C: 0x3C
- Resolución: 128x64 píxeles monocromático
- Layout de pantalla:
  - **Esquinas superiores**: Iconos WiFi (señal) y MQTT (conexión)
  - **Centro**: 4 indicadores de zona (relleno=ON, contorno=OFF)
  - **Línea inferior**: Mensajes de estado del sistema

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
6. ✅ OTA por WiFi habilitado (ArduinoOTA)

## 📝 Estado de Implementación

- [x] Estructura del proyecto
- [x] Configuración PlatformIO
- [x] WiFiManager ✅ Completado
- [x] TimeSync ✅ Completado  
- [x] MqttManager ✅ Completado (buffer 1024 bytes)
- [x] RelayController ✅ Completado
- [x] SPIFFSManager ✅ Completado (LittleFS)
- [x] AgendaManager ✅ Completado
- [x] DisplayManager ✅ Completado (OLED SSD1306) ⭐ NUEVO
- [ ] HumiditySensor ⏳ Bloqueado (hardware no disponible)
- [ ] Tests unitarios (pendiente)
- [x] OTA updates ✅ habilitado

**Recursos actuales:**
- Flash: 36.1% usado (376911/1044464 bytes)
- RAM: 45.5% usado (37268/81920 bytes)

## 🤝 Contribuir

Ver [CONTRIBUTING.md](../../CONTRIBUTING.md) para guías de contribución.

## 📄 Licencia

Ver [LICENSE](../../LICENSE) para detalles.
