# 🎯 Desarrollo ESP8266 - Índice de Documentación

> **Última actualización**: 2025-12-27  
> **Estado**: Documentación actualizada - Hardware ESP8266 con Display OLED

---

## 📚 Documentación Disponible

### 1. 📖 Guía Principal de Desarrollo
**Archivo**: [`esp32-desarrollo.md`](./esp32-desarrollo.md)

**Contenido**:
- Introducción y objetivos del firmware
- Hardware requerido (ESP8266, relés 4CH, sensor, display OLED)
- Mapeo completo de pines GPIO
- Arquitectura del firmware por módulos
- Configuración del entorno (PlatformIO y Arduino IDE)
- Estructura detallada del código
- Implementación de cada módulo con código
- Testing y debugging
- Despliegue y OTA

**Cuándo leer**: Antes de comenzar cualquier desarrollo

---

### 2. 🔌 Diagramas de Conexión Hardware
**Archivo**: [`esp32-diagramas-conexion.json`](./esp32-diagramas-conexion.json)

**Contenido**:
- Especificaciones técnicas de cada componente
- Mapeo de pines ESP8266 → Relés → Electroválvulas
- Conexiones de sensor de humedad (A0)
- Conexiones del display OLED I2C (D7/D3)
- Esquema de alimentación eléctrica
- Diagrama ASCII del sistema completo
- Verificación paso a paso de conexiones
- Troubleshooting de hardware
- Lista de materiales con precios
- Notas de seguridad eléctrica

**Formato**: JSON estructurado (importable desde herramientas como Fritzing)

**Cuándo usar**: Durante armado del circuito físico

---

### 3. 📊 Diagramas de Flujo del Firmware
**Archivo**: [`esp32-diagramas-flujo.md`](./esp32-diagramas-flujo.md)

**Contenido**:
- Flujo principal del sistema (setup → loop)
- Máquina de estados de conexión WiFi/MQTT
- Procesamiento de comandos MQTT
- Sincronización de agendas
- Ejecución automática de agendas
- Gestión de relés con timers
- Lectura y publicación de sensor
- Actualización del display OLED
- Modo offline y reconexión
- Diagramas de secuencia (comando manual, sync de agenda)

**Formato**: Mermaid (visualizable en GitHub, VSCode, web)

**Cuándo usar**: Durante implementación del firmware para entender el flujo lógico

---

### 4. 💻 Código Base del Firmware
**Directorio**: [`firmware/`](../../esp32/firmware/)

**Contenido**:
```
firmware/
├── platformio.ini          # Configuración PlatformIO
├── README.md               # Guía rápida del firmware
├── .gitignore             # Exclusiones de Git
└── src/
    ├── main.cpp           # Loop principal con máquina de estados
    ├── config/
    │   ├── Config.h       # Constantes globales (pines, timings)
    │   └── Secrets.h.example  # Template de credenciales
    ├── network/
    │   ├── WiFiManager.h   # (TODO) Gestión WiFi
    │   └── MqttManager.h   # (TODO) Cliente MQTT
    ├── hardware/
    │   ├── RelayController.h     # Control de relés
    │   └── HumiditySensor.h      # Lectura de sensores
    ├── scheduler/
    │   ├── Agenda.h              # Modelo de datos
    │   └── AgendaManager.h       # Ejecución de agendas
    ├── storage/
    │   └── SPIFFSManager.h       # (TODO) Persistencia JSON
    └── utils/
        ├── Logger.h              # Sistema de logs
        └── TimeSync.h            # (TODO) Sincronización NTP
```

**Estado actual**:
- ✅ Estructura completa del proyecto
- ✅ Headers principales implementados
- ✅ main.cpp con máquina de estados
- ✅ WiFiManager implementado
- ✅ TimeSync implementado
- ✅ MqttManager implementado
- ✅ RelayController implementado (4 zonas)
- ⌚ HumiditySensor (bloqueado - sensor no conectado)
- ✅ SPIFFSManager implementado (LittleFS)
- ✅ AgendaManager implementado
- ✅ DisplayManager implementado (OLED SSD1306)

**Cuándo usar**: Para comenzar a codificar

---

## 🚀 Flujo de Trabajo Recomendado

### Fase 1: Preparación (1-2 horas)
1. ✅ Leer [`esp32-desarrollo.md`](./esp32-desarrollo.md) completo
2. ✅ Revisar [`esp32-diagramas-flujo.md`](./esp32-diagramas-flujo.md) para entender lógica
3. ✅ Comprar componentes según lista en [`esp32-diagramas-conexion.json`](./esp32-diagramas-conexion.json)

### Fase 2: Setup Hardware (2-4 horas)
1. ✅ Armar circuito en protoboard siguiendo [`esp32-diagramas-conexion.json`](./esp32-diagramas-conexion.json)
2. ✅ Verificar conexiones con multímetro
3. ✅ Probar ESP8266 con sketch básico (Blink)
4. ✅ Probar activación individual de cada relé (4 zonas)
5. ✅ Probar display OLED con sketch de ejemplo
6. ⏳ Calibrar sensor de humedad (cuando se conecte)

### Fase 3: Setup Software (1 hora)
1. ✅ Instalar PlatformIO o Arduino IDE
2. ✅ Clonar repositorio
3. ✅ Copiar `Secrets.h.example` → `Secrets.h`
4. ✅ Configurar credenciales WiFi y MQTT
5. ✅ Compilar `firmware/` y subir a ESP32
6. ✅ Verificar logs en monitor serial

### Fase 4: Desarrollo Módulos (2-3 semanas)
Orden sugerido de implementación:

1. **Logger** (30 min) ✅ Ya incluido
   - Sistema de logs funcional

2. **RelayController** (2-3 horas) ✅ Completado
   - Implementar `.cpp` según `.h`
   - Test: Encender/apagar zonas manualmente
   - Test: Verificar timers automáticos

3. **DisplayManager** (3-4 horas) ✅ Completado
   - Inicialización OLED SSD1306
   - Renderizado de iconos WiFi/MQTT
   - Indicadores de 4 zonas
   - Actualización cada 2 segundos

4. **HumiditySensor** (2-3 horas) ⏳ Pendiente (hardware)
   - Implementar lectura ADC en A0
   - Calibración y mapeo a porcentaje
   - Test: Leer sensor en loop

5. **WiFiManager** (3-4 horas) ✅ Completado
   - Conexión WiFi con reconexión
   - Test: Verificar reconexión al desconectar router

6. **TimeSync** (1-2 horas) ✅ Completado
   - Sincronización NTP
   - Test: Verificar hora correcta en serial

7. **MqttManager** (4-5 horas) ✅ Completado
   - Cliente MQTT básico
   - Suscripción a topics de comandos
   - Publicación de estado
   - Test: Enviar comando desde backend

8. **Agenda y AgendaManager** (3-4 horas) ✅ Completado
   - Modelo de datos
   - Ejecución temporal cada 10 segundos
   - Test: Crear agenda manual y verificar ejecución

9. **SPIFFSManager** (3-4 horas) ✅ Completado (LittleFS)
   - Persistencia JSON de agendas
   - Cargar al inicio
   - Test: Guardar, resetear ESP8266, verificar carga

10. **Integración MQTT + Agendas** (2-3 horas) ✅ Completado
    - Recibir sincronización de agendas
    - Guardar en LittleFS
    - Test: Crear agenda desde backend y verificar ejecución

### Fase 5: Testing End-to-End (3-5 días)
1. ✅ Levantar stack Docker del backend
2. ✅ Conectar ESP8266 con firmware completo
3. ✅ Crear agendas desde frontend
4. ✅ Verificar ejecución automática cada 10 segundos
5. ✅ Probar comandos manuales
6. ✅ Verificar display OLED mostrando estado
7. ✅ Simular desconexión WiFi (modo offline)
8. ✅ Verificar reconexión y sincronización

### Fase 6: Instalación Final (1-2 días)
1. ✅ Diseñar PCB custom (opcional) o soldar protoboard
2. ✅ Montar en caja estanca IP65
3. ✅ Instalar en ubicación definitiva
4. ✅ Conectar electroválvulas reales
5. ✅ Pruebas con sistema de riego real
6. ✅ Documentar configuración específica de instalación

---

## 🛠️ Herramientas Recomendadas

### Software
- **VSCode** con extensión PlatformIO IDE
- **MQTT Explorer** - Visualizar mensajes MQTT
- **Serial Monitor** - Logs del ESP32
- **Fritzing** - Diseño de PCB (opcional)
- **KiCad** - PCB profesional (opcional)

### Hardware
- **Multímetro** - Verificar voltajes y continuidad
- **Logic Analyzer** - Debug de señales GPIO (opcional)
- **Osciloscopio** - Debug avanzado (opcional)
- **Pinza amperimétrica** - Medir consumo (opcional)

---

## 📖 Referencias Externas

### ESP8266
- [Documentación oficial ESP8266](https://arduino-esp8266.readthedocs.io/)
- [Random Nerd Tutorials - ESP8266](https://randomnerdtutorials.com/projects-esp8266/)
- [ESP8266 Pinout Reference](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/)

### Arduino/PlatformIO
- [PlatformIO Docs](https://docs.platformio.org/)
- [Arduino Reference](https://www.arduino.cc/reference/en/)

### Librerías
- [PubSubClient (MQTT)](https://github.com/knolleary/pubsubclient)
- [ArduinoJson](https://arduinojson.org/)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)

### Hardware
- [Sensores capacitivos de humedad](https://how2electronics.com/interfacing-capacitive-soil-moisture-sensor-arduino/)
- [Módulos de relés](https://www.circuitbasics.com/setting-up-a-5v-relay-on-the-arduino/)

---

## 🤔 FAQ - Preguntas Frecuentes

### ¿Por qué PlatformIO en lugar de Arduino IDE?
- ✅ Mejor gestión de dependencias
- ✅ Compilación más rápida
- ✅ Integración con VSCode
- ✅ Soporte para múltiples placas
- ⚠️ Curva de aprendizaje inicial

**Respuesta**: Usa PlatformIO para proyectos serios, Arduino IDE para prototipos rápidos.

### ¿Puedo usar menos de 8 zonas?
Sí, el firmware soporta desde 1 hasta 8 zonas. Simplemente no conectes relés/sensores de zonas no usadas.

### ¿Funciona con relés de 5V?
Sí, pero necesitas un level shifter porque ESP32 usa 3.3V. Alternativamente, busca módulos de relés compatibles con 3.3V.

### ¿Qué pasa si se va la luz?
El ESP32 se resetea. Al reiniciar:
1. Carga última agenda desde SPIFFS
2. Sincroniza hora con NTP
3. Continúa operación normal

### ¿Puedo actualizar firmware remotamente?
Sí, implementando OTA (Over-The-Air). Ver sección OTA en [`esp32-desarrollo.md`](./esp32-desarrollo.md).

### ¿Soporta TLS/SSL para MQTT?
Sí, configurar `MQTT_TLS = true` en `Secrets.h`. Requiere certificados.

---

## 🐛 Troubleshooting Rápido

| Problema | Solución |
|----------|----------|
| ESP32 no compila | Verificar instalación de librerías en `platformio.ini` |
| No conecta WiFi | Verificar SSID/password, usar 2.4GHz no 5GHz |
| Relé no activa | Verificar lógica invertida (LOW=ON), GND común |
| Sensor lectura errática | Calibrar valores, alejar de cables de alta tensión |
| MQTT no conecta | Verificar IP del broker, puerto, firewall |
| ESP32 se resetea | Fuente insuficiente (usar mínimo 2A) |

---

## ✅ Checklist de Implementación

### Hardware
- [ ] ESP8266 NodeMCU adquirido y funcionando
- [x] Módulo de relés 4CH conectado
- [ ] Sensor de humedad calibrado (pendiente conexión)
- [x] Display OLED SSD1306 I2C funcionando
- [x] Fuente 5V 2A instalada
- [ ] 4 electroválvulas conectadas (o según zonas usadas)
- [ ] Montaje en caja estanca (para instalación final)

### Software
- [x] PlatformIO/Arduino IDE instalado
- [x] Firmware compila sin errores
- [x] Secrets.h configurado con credenciales
- [x] WiFiManager conecta exitosamente
- [x] MQTT publica/suscribe correctamente
- [x] Display OLED muestra información
- [x] Agendas se ejecutan automáticamente (cada 10s)
- [x] Modo offline funciona (reconexión automática)
- [x] LittleFS guarda/carga agendas

### Testing
- [ ] Comando manual ON/OFF funciona
- [ ] Sincronización de agendas desde backend
- [ ] Publicación de estado cada 5s
- [ ] Lectura de sensores cada 60s
- [ ] Simulación de desconexión WiFi
- [ ] Test con electroválvulas reales

### Documentación
- [ ] Configuración específica documentada
- [ ] Calibración de sensores registrada
- [ ] Diagrama de instalación final
- [ ] Manual de troubleshooting local

---

## 🎓 Siguiente Paso

**Para comenzar ahora mismo**:

```bash
cd esp32/firmware
cp src/config/Secrets.h.example src/config/Secrets.h
# Editar Secrets.h con tus credenciales
pio run --target upload && pio device monitor
```

**Lee primero**: [`esp32-desarrollo.md`](./esp32-desarrollo.md)

---

**Documento actualizado**: 2025-12-16  
**Mantenedor**: Sistema de Riego MQTT Project  
**Licencia**: Ver LICENSE en raíz del proyecto
