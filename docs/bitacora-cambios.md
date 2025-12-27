# Bitácora de Cambios - Sistema de Riego MQTT (NodeMCU/ESP8266)

Este documento registra todos los cambios relevantes realizados en el firmware, hardware y documentación del proyecto. Debe mantenerse actualizado en cada sesión de trabajo.

## 2025-12-27 (Sesión Tarde)

### Bugfix: Frontend No Muestra "Próximo Riego" Correctamente

**Problema:**
- La vista de Inicio del frontend no muestra correctamente el próximo riego programado
- Zonas con agendas configuradas mostraban "Próximo: Sin agenda"
- Ejemplo: Zona 4 con agenda para Sábado a las 17:13 mostraba "Sin agenda"

**Causa:**
- La lógica en `ZoneStatusService.calcularProximoRiego()` solo buscaba en los próximos 7 días (0-6)
- Si la hora de hoy ya pasó (17:13 < 19:28), no incluía el próximo día de la semana (día 7)
- Para agendas de un solo día (ej: solo Sábado), no encontraba el próximo Sábado

**Solución:**
- Modificado el bucle para buscar hasta 7 días adelante (0-7 inclusive)
- Esto permite encontrar la próxima ocurrencia semanal del mismo día
- Mejorada la lógica de selección para elegir la agenda más próxima
- Corregida la condición de hora pasada: `!isAfter(ahora)` en lugar de `isBefore(ahora)`

**Archivos Modificados:**
- `backend/src/main/java/ar/net/dac/iot/irrigacion/service/ZoneStatusService.java`

**Prueba:**
- Reiniciar backend
- Verificar que todas las zonas con agendas muestren "Próximo: [fecha] [hora]"
- Para Zona 4 (Sábado 17:13): debería mostrar "Próximo sábado 17:13"

---

### Bugfix: Agendas No Ejecutándose

**Problema:**
- Las agendas dejaron de ejecutarse después de la sincronización HTTP
- Log mostraba error: `"JSON no contiene campo 'agendas'"`
- El ESP8266 estaba guardando el JSON del endpoint HTTP directamente, que retorna un array `[...]`
- El AgendaManager espera formato MQTT con objeto envolvente: `{"agendas": [...]}`

**Causa:**
- Inconsistencia de formato entre endpoint HTTP y mensaje MQTT
- `GET /api/nodos/{nodeId}/agendas` retorna `List<AgendaResponse>` (array directo)
- Mensaje MQTT `riego/{nodeId}/agenda/sync` envía `{"version": X, "agendas": [...]}`
- La función `fetchAndStoreAgendas()` en main.cpp guardaba el array HTTP sin envolver

**Solución:**
- Modificado `fetchAndStoreAgendas()` en [main.cpp](esp32/firmware/src/main.cpp)
- Ahora envuelve el array HTTP en objeto: `{"agendas": [...]}`
- Mantiene compatibilidad con AgendaManager sin cambios en el backend
- Las agendas ahora se cargan correctamente desde HTTP y MQTT

**Archivos Modificados:**
- `esp32/firmware/src/main.cpp` (línea ~586)

**Prueba:**
- Verificar que las agendas se ejecuten correctamente
- Compilar y flashear firmware actualizado
- Monitorear logs para confirmar carga correcta de agendas

---

## 2025-12-27 (Sesión Mañana)

### Firmware ESP8266 - Implementación Completa de Módulos Base

**Migración y Setup Inicial:**
- Migración de firmware de ESP32 a NodeMCU ESP8266 (CP2102)
- Reducción de zonas de riego de 8 a 4 por limitaciones de hardware
- Sensores reducidos a 1 (A0 analógico único del ESP8266)
- Confirmada compatibilidad con el código y hardware existente
- Preparado para futura expansión de sensores (requiere multiplexor ADC externo)

**Módulos Implementados y Probados:**

1. ✅ **WiFiManager** (`src/network/WiFiManager.h/cpp`)
   - Conexión WiFi con timeout y reintentos configurables
   - Auto-reconexión en caso de pérdida de señal
   - Monitoreo de RSSI y calidad de conexión
   - Soporte para resolución de hostname vía DNS

2. ✅ **TimeSync** (`src/network/TimeSync.h/cpp`)
   - Sincronización NTP con servidor pool.ntp.org
   - Timezone configurado: GMT-3 (Argentina)
   - Re-sincronización automática cada hora
   - Formato de fecha/hora legible para logs

3. ✅ **MqttManager** (`src/network/MqttManager.h/cpp`)
   - Cliente MQTT con reconexión automática (max 5 intentos, delay 15s)
   - Suscripción a comandos: `riego/{nodeId}/cmd/zona/+`
   - Suscripción a agenda sync: `riego/{nodeId}/agenda/sync`
   - **Buffer aumentado a 1024 bytes** para recibir mensajes JSON grandes (agendas de 4 zonas)
   - Publicación de estado con formato correcto: `{"activa": bool, "tiempoRestante": seconds}`
   - Callbacks para comandos y sincronización de agenda
   - Broker configurado: 192.168.10.108:1883 (IP directa por firewall)

4. ✅ **RelayController** (`src/hardware/RelayController.h/cpp`)
   - Control de 4 relés con lógica invertida (LOW=ON, HIGH=OFF)
   - Pines: GPIO5 (D1), GPIO4 (D2), GPIO14 (D5), GPIO12 (D6)
   - Timers automáticos con auto-shutoff funcionando correctamente
   - Callback para notificar cambios de estado (auto-apagado)
   - Integración con MQTT para publicar estado en tiempo real
   - Debug logs para monitoreo de timers

5. ⏳ **HumiditySensor** - BLOQUEADO (hardware no disponible)
   - Lectura de sensor analógico A0
   - Conversión ADC a porcentaje de humedad
   - Calibración seco/húmedo
   - Publicación periódica vía MQTT
   - **Nota**: Implementación pospuesta hasta obtener sensores físicos

6. ✅ **SPIFFSManager** (`src/storage/SPIFFSManager.h/cpp`)
   - Sistema de archivos LittleFS montado exitosamente
   - Lectura/escritura de archivos JSON funcionando
   - Persistencia de agendas en `/agenda.json`
   - Gestión de espacio disponible
   - Formato JSON array compatible con backend

7. ✅ **AgendaManager** (`src/scheduler/AgendaManager.h/cpp`)
   - Parser de agendas desde JSON almacenado en LittleFS
   - Verificación automática cada 10 segundos de hora/día para ejecución
   - Validación de días de semana ("LUN", "MAR", "MIE", etc.)
   - Trigger automático de riego según agenda programada
   - Prevención de ejecución duplicada con control de último minuto ejecutado
   - Integración con RelayController para activación de zonas
   - Modo offline completamente funcional con agendas locales

8. ✅ **DisplayManager** (`src/display/DisplayManager.h/cpp`) ⭐ NUEVO
   - Pantalla OLED SSD1306 I2C 128x64px (dirección 0x3C)
   - Pines: SDA=GPIO13 (D7), SCL=GPIO0 (D3)
   - **Iconos de estado en esquinas superiores:**
     - WiFi (derecha): Barras de señal según RSSI (-50/-60/-70/-80 dBm)
     - MQTT (izquierda): "M" con checkmark (conectado) o "X" (desconectado)
   - **Línea de estado inferior:** Mensajes de progreso del sistema
     - "Configurando...", "Conectando WiFi...", "Conectando MQTT...", "Sistema listo", etc.
   - **Indicadores de zonas (centro):** 4 cajas 20x20px
     - Relleno cuando zona está ON (regando)
     - Solo contorno cuando zona está OFF
     - Número de zona debajo de cada caja
   - Actualización cada 2 segundos en loop principal
   - Librería: Adafruit SSD1306 2.5.16 + Adafruit GFX 1.12.4

**Integración y Testing:**
- Main loop actualizado con llamadas a todos los módulos (.loop())
- Máquina de estados funcional: INIT → WIFI_CONNECTING → WIFI_CONNECTED → MQTT_CONNECTING → ONLINE
- Publicación MQTT de estados automática (cada 5s para zonas activas + eventos de cambio)
- Frontend Vue3 recibiendo actualizaciones en tiempo real
- Timers decrementando correctamente y apagado automático verificado
- Display mostrando información completa: conectividad, estado de sistema, zonas activas
- Sincronización de agendas desde backend vía MQTT funcionando correctamente

**Issues Resueltos:**
- ❌→✅ Comentarios en español causando errores de compilación (sintaxis `//` requerida)
- ❌→✅ Multiple definition de arrays en Config.h (agregado `static` a todas las definiciones)
- ❌→✅ MQTT connection -2 error (cambiado hostname a IP por firewall)
- ❌→✅ Frontend mostrando "1" en lugar de "15" (ancho de input aumentado a 120px)
- ❌→✅ RelayController timers no decrementaban (faltaba `relayController.loop()` en main loop)
- ❌→✅ Frontend no mostraba zona activa (formato MQTT incorrecto: estado→activa, agregado tiempoRestante)
- ❌→✅ Frontend no actualizaba al apagado automático (faltaba publicación MQTT en callback)
- ❌→✅ MQTT no recibía agendas del backend (buffer 256 bytes insuficiente → aumentado a 1024 bytes)
- ❌→✅ Display variable/method name conflict (renombrado pointer interno a oledDisplay)
- ❌→✅ Corrupción en main.cpp durante edición (archivo reconstruido limpiamente)

**Configuración Final del Hardware:**
- **Placa:** NodeMCU ESP8266 (CP2102), 80MHz, 4MB Flash
- **Relés:** 4 canales en D1, D2, D5, D6 (lógica invertida)
- **Display:** OLED I2C en D7 (SDA), D3 (SCL), 3.3V
- **Sensor:** 1 analógico en A0 (pendiente implementación)
- **Node ID:** 550e8400-e29b-41d4-a716-446655440000
- **MQTT Broker:** 192.168.10.108:1883
- **Backend:** 192.168.10.108:8080 (Basic Auth admin:dev123)
- **Timezone:** GMT-3 (Argentina)
- **NTP:** pool.ntp.org

**Recursos del Firmware:**
- **Flash:** 36.1% usado (376911/1044464 bytes)
- **RAM:** 45.5% usado (37268/81920 bytes)
- **Librerías:** PubSubClient 2.8, ArduinoJson 6.21.5, NTPClient 3.2.1, Adafruit SSD1306 2.5.16, Adafruit GFX 1.12.4

**Estado Actual:**
✅ Sistema completamente operacional en modo online y offline
✅ Display OLED mostrando información completa en tiempo real
✅ Agendas ejecutándose automáticamente según programación
✅ Sincronización MQTT bidireccional funcionando
✅ Frontend recibiendo actualizaciones en tiempo real
✅ Listo para testing con sistema de riego real

**Próximos Pasos:**
- [ ] Implementar HumiditySensor cuando hardware esté disponible
- [ ] Testing exhaustivo con electroválvulas reales
- [ ] Optimizar consumo de memoria si es necesario
- [ ] Implementar OTA (Over-The-Air updates)
- [ ] Documentar procedimientos de instalación final

## Notas
- Mantener la compatibilidad con todo el código y hardware existente
- Registrar aquí cada modificación relevante de código, hardware, configuración o documentación
- Consultar este documento antes de realizar cambios mayores
- Display OLED es componente crítico para monitoreo visual del sistema

