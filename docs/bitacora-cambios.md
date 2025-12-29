# Bitácora de Cambios - Sistema de Riego MQTT (NodeMCU/ESP8266)

Este documento registra todos los cambios relevantes realizados en el firmware, hardware y documentación del proyecto. Debe mantenerse actualizado en cada sesión de trabajo.

## 2025-12-28

### Feature: Sistema Completo de Historial de Eventos de Riego

**Objetivo:**
- Implementar tracking preciso de cada ejecución de riego (agenda y manual)
- Registrar inicio/fin con timestamps, duraciones reales y origen
- Proveer API REST para consultar historial
- Base para analytics y detección de anomalías

**Problema Anterior:**
- Tabla `riego_evento` existía en BD pero no se usaba
- No había forma de saber qué riegos realmente se ejecutaron
- El historial se debería inferir desde las agendas (datos no reales)
- Sin trazabilidad de duraciones reales vs programadas

**Implementación:**

**Backend - 6 Clases Nuevas:**

1. **RiegoEvento.java** (Model/Entity)
   - Mapea tabla `riego_evento` con JPA
   - Campos: id, nodeId, zona, timestamp, duracionSeg, origen, versionAgenda, raw, createdAt
   - Origen: "agenda" | "manual"
   - VersionAgenda: nullable (null si manual)

2. **RiegoEventoRepository.java** (Repository)
   - `findByNodeIdOrderByTimestampDesc()`: Paginado por nodo
   - `findByNodeIdAndTimestampBetweenOrderByTimestampDesc()`: Por rango fechas
   - `findByNodeIdAndZonaOrderByTimestampDesc()`: Filtro por zona

3. **RiegoEventoResponse.java** (DTO)
   - Respuesta API sin campo `raw` (JSON limpio)
   - Campos: id, zona, timestamp, duracionSeg, origen, versionAgenda

4. **RiegoEventoService.java** (Service)
   - `procesarEvento(nodeId, payload)`: Parsea JSON MQTT
   - Solo persiste eventos tipo "fin" (con duración real)
   - Eventos "inicio" se loggean pero no persisten
   - Guarda payload raw en campo JSONB

5. **MqttEventoSubscriber.java** (Service)
   - Se auto-registra al arrancar en topic `riego/+/evento`
   - Extrae nodeId del topic pattern
   - Usa Mqtt5BlockingClient (igual que resto del sistema)
   - Thread separado para no bloquear startup

6. **RiegoEventoController.java** (Controller)
   - `GET /api/nodos/{nodeId}/historial?limit=50`: Últimos N eventos
   - `GET /api/nodos/{nodeId}/historial/zona/{zona}`: Filtro por zona
   - Default limit=50, configurable

**ESP32/ESP8266 - 6 Archivos Modificados:**

1. **MqttManager.h/cpp**
   - Nuevo método: `publishRiegoEvento(zona, evento, origen, duracion, versionAgenda)`
   - Topic: `riego/{nodeId}/evento`
   - Payload JSON con evento "inicio" o "fin"

2. **RelayController.h/cpp**
   - Nuevos campos: `zoneDuracionProgramada[]`, `zoneOrigen[]`, `zoneVersionAgenda[]`
   - Nuevo callback: `RiegoEventCallback`
   - `turnOn()` modificado: acepta parámetros origen y versionAgenda
   - `turnOn()`: Publica evento "inicio" al activar relay
   - `turnOff()`: Calcula duración real, publica evento "fin"
   - `loop()`: Publica evento "fin" cuando timer expira automáticamente

3. **AgendaManager.cpp**
   - Extrae campo `version` del JSON de agendas
   - Pasa origen="agenda" y versión al llamar `turnOn()`

4. **main.cpp**
   - Nuevo callback: `onRiegoEvent(zona, evento, origen, duracion, versionAgenda)`
   - Conecta callback en setup: `relayController.setRiegoEventCallback(onRiegoEvent)`
   - Publica eventos via `mqttManager.publishRiegoEvento()`

**Protocolo MQTT Extendido:**

Nuevo topic: `riego/{nodeId}/evento`

Evento inicio:
```json
{
  "zona": 1,
  "evento": "inicio",
  "timestamp": 1735415280,
  "origen": "agenda",
  "duracionProgramada": 600,
  "versionAgenda": 7
}
```

Evento fin:
```json
{
  "zona": 1,
  "evento": "fin",
  "timestamp": 1735415880,
  "origen": "manual",
  "duracionReal": 598,
  "versionAgenda": null
}
```

**Flujo Completo:**

1. **Riego manual:**
   - Backend → MQTT cmd → ESP8266
   - ESP8266: `turnOn(zona, 600, "manual", 0)`
   - ESP8266 → MQTT evento "inicio"
   - Backend: recibe inicio (log, no persiste)
   - Tras 600s: ESP8266 → MQTT evento "fin"
   - Backend: persiste en `riego_evento`

2. **Riego automático (agenda):**
   - AgendaManager detecta hora programada
   - ESP8266: `turnOn(zona, 600, "agenda", 7)`
   - ESP8266 → MQTT evento "inicio" con versionAgenda
   - Tras 600s: ESP8266 → MQTT evento "fin" con duracionReal
   - Backend: persiste con origen="agenda" y version_agenda=7

**Archivos Modificados:**
- `backend/src/main/java/ar/net/dac/iot/irrigacion/model/RiegoEvento.java` (nuevo)
- `backend/src/main/java/ar/net/dac/iot/irrigacion/repository/RiegoEventoRepository.java` (nuevo)
- `backend/src/main/java/ar/net/dac/iot/irrigacion/dto/RiegoEventoResponse.java` (nuevo)
- `backend/src/main/java/ar/net/dac/iot/irrigacion/service/RiegoEventoService.java` (nuevo)
- `backend/src/main/java/ar/net/dac/iot/irrigacion/service/MqttEventoSubscriber.java` (nuevo)
- `backend/src/main/java/ar/net/dac/iot/irrigacion/controller/RiegoEventoController.java` (nuevo)
- `esp32/firmware/src/network/MqttManager.h` (modificado)
- `esp32/firmware/src/network/MqttManager.cpp` (modificado)
- `esp32/firmware/src/hardware/RelayController.h` (modificado)
- `esp32/firmware/src/hardware/RelayController.cpp` (modificado)
- `esp32/firmware/src/scheduler/AgendaManager.cpp` (modificado)
- `esp32/firmware/src/main.cpp` (modificado)
- `docs/implementacion/contratos-mqtt-http.md` (actualizado)
- `docs/implementacion/historial-eventos-riego.md` (nuevo - documentación completa)

**Testing:**
```bash
# Backend
mvn clean compile  # ✅ Compilado exitosamente

# ESP8266
cd esp32/firmware
pio run            # ✅ Compilado exitosamente (379 KB Flash, 46% RAM)
pio run --target upload  # ✅ Cargado en hardware

# Desplegar
docker-compose up -d --build backend

# Verificar suscripción MQTT en logs
docker logs irrigacion-backend | grep "Suscrito a topic MQTT de eventos"

# Probar endpoint
curl -u admin:dev123 "http://localhost:8080/api/nodos/{nodeId}/historial"
```

**Próximos Pasos:**
- Testing end-to-end con riegos manuales
- Testing con agendas programadas
- Frontend: Pantalla de historial con gráficos
- Analytics: Consumo total por zona/mes
- Alertas: Desvíos entre duraciones programadas vs reales

---

### Bugfix: Cálculo Incorrecto de Próximo Riego (Mejora Algoritmo)

**Problema:**
- El cálculo de "próximo riego" no elegía correctamente la agenda más cercana en el tiempo
- Ejemplo: Con agenda A (sábado 17:13 ya pasada → próximo sábado en 7 días) y agenda B (domingo 17:15 → mañana), elegía incorrectamente A en lugar de B
- La lógica comparaba solo días adelante y hora, pero no el tiempo absoluto total

**Causa:**
- El método `calcularProximoRiego()` comparaba `diasAdelante` y hora por separado
- No calculaba el tiempo absoluto en minutos hasta cada ejecución
- Al comparar agendas en días distintos, no determinaba cuál estaba más cerca temporalmente

**Solución:**
- Refactorizado el algoritmo para calcular minutos absolutos hasta cada agenda:
  - Para hoy: `Duration.between(ahora, horaAgenda).toMinutes()`
  - Para días futuros: `(días × 24 × 60) - minutosQueYaPasaronHoy + minutosDesdeMedianocheHastaAgenda`
- Ahora compara `minutosMinimos` y selecciona la agenda con menor tiempo de espera
- Garantiza que siempre se elige la ejecución MÁS CERCANA en tiempo absoluto

**Archivos Modificados:**
- `backend/src/main/java/ar/net/dac/iot/irrigacion/service/ZoneStatusService.java` (método `calcularProximoRiego`)
- `backend/src/test/java/ar/net/dac/iot/irrigacion/service/ZoneStatusServiceTest.java` (nuevo archivo con tests unitarios)

**Tests Creados:**
- `testCalcularProximoRiego_sinAgendas`: Verifica retorno null cuando no hay agendas
- `testCalcularProximoRiego_agendaHoyFutura`: Agenda para hoy que no ha pasado
- `testCalcularProximoRiego_agendaHoyPasada`: Agenda de hoy ya pasada → debe buscar próxima semana
- `testCalcularProximoRiego_multipleAgendasEligeMasCercana`: 3 agendas en diferentes días → elige la de mañana
- `testCalcularProximoRiego_mismoDiaDiferentesHoras`: 2 agendas el mismo día → elige la más temprana
- `testCalcularProximoRiego_agendaTodosLosDias`: Agenda diaria → debe ser hoy o mañana según hora actual
- `testCalcularProximoRiego_agendaSoloUnDiaRemoto`: Agenda en el día 7 (límite del algoritmo)

**Prueba:**
```bash
# Ejecutar tests
cd backend
mvn test -Dtest=ZoneStatusServiceTest

# Rebuild backend
docker-compose up -d --build backend

# Probar API con curl
curl -u admin:dv123 "http://localhost:8080/api/nodos/{nodeId}/status" | jq '.[] | {zona, nombre, proximoRiego}'
```

**Resultado Esperado:**
- Todas las zonas con múltiples agendas deben mostrar la ejecución MÁS CERCANA
- Frontend debe reflejar correctamente los tiempos de próximo riego

---

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

---

### Hotfix: Parsing de agendas en firmware (NoMemory) — 2025-12-28

**Resumen:**
- Se detectó error `NoMemory` en el ESP8266 al parsear el JSON de agendas cuando el payload HTTP/ MQTT era ~1KB o mayor.
- Resultado: agendas no se ejecutaban y el dispositivo registraba `Error parseando agendas: NoMemory`.

**Causa raíz:**
- `AgendaManager` usaba `StaticJsonDocument<1024>` que no era suficiente para el JSON real almacenado (~1.07 KB más overhead de ArduinoJson).

**Acción tomada:**
- En el firmware se sustituyó el `StaticJsonDocument<1024>` por `DynamicJsonDocument jsonContent.length() + 1024` en `esp32/firmware/src/scheduler/AgendaManager.cpp`.
- Se limpió además el mensaje retenido en el topic MQTT `riego/{nodeId}/agenda/sync` (broker HiveMQ en el entorno local) para evitar reentregas antiguas.

**Archivos afectados:**
- `esp32/firmware/src/scheduler/AgendaManager.cpp` (parche: uso de DynamicJsonDocument)
- `esp32/firmware/src/main.cpp` (comportamiento existente: `fetchAndStoreAgendas()` envuelve array HTTP en objeto `{"agendas": ...}` )

**Impacto:**
- Dispositivo volvió a cargar y ejecutar agendas correctamente; verificado en logs y display.
- No se modificó el backend ni los endpoints HTTP.

**Recomendaciones:**
- Para cargas mayores o crecimiento de agendas, implementar parsing por-objeto o paginación en backend.
- Mantener prácticas de broker: evitar retained con payloads grandes o limpiarlos cuando se actualizan formatos.

**Rollback:**
- Revertir cambio en firmware restaurando `StaticJsonDocument` si es necesario, pero esto reintroducirá el problema.

**Hecho por:** [registro de cambios automatizado por herramienta de integración]

## Notas
- Mantener la compatibilidad con todo el código y hardware existente
- Registrar aquí cada modificación relevante de código, hardware, configuración o documentación
- Consultar este documento antes de realizar cambios mayores
- Display OLED es componente crítico para monitoreo visual del sistema

