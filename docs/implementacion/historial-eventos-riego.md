# Sistema de Historial de Eventos de Riego

## Fecha de implementación
28 de diciembre de 2025

## Resumen
Implementación completa del sistema de logging de eventos de riego, permitiendo rastrear con precisión cada ejecución de riego (tanto por agenda como manual) con timestamps, duraciones reales y origen.

## Arquitectura

### Flujo de datos
```
ESP32/ESP8266 (Hardware)
    ↓ (Relay ON)
    ↓ Publica evento "inicio" → Topic: riego/{nodeId}/evento
    ↓
    ↓ (Relay OFF - timer o manual)
    ↓ Publica evento "fin" → Topic: riego/{nodeId}/evento
    ↓
Backend MQTT Subscriber
    ↓ Recibe eventos
    ↓ Guarda en BD (tabla riego_evento)
    ↓
REST API
    ↓ GET /api/nodos/{nodeId}/historial
    ↓ Retorna historial ordenado
```

## Componentes implementados

### 1. Backend - Modelo de datos

**RiegoEvento.java** (Entity JPA)
- Mapea tabla `riego_evento` existente
- Campos:
  - `id`: UUID autogenerado
  - `nodeId`: UUID del nodo
  - `zona`: Short (1-4)
  - `timestamp`: Instant del evento
  - `duracionSeg`: Duración real en segundos
  - `origen`: String ("agenda" | "manual")
  - `versionAgenda`: Integer nullable (versión de agenda que ejecutó)
  - `raw`: String (JSON original del evento)
  - `createdAt`: Instant de inserción en BD

### 2. Backend - Repository

**RiegoEventoRepository.java**
- Métodos:
  - `findByNodeIdOrderByTimestampDesc()`: Paginado por nodo
  - `findByNodeIdAndTimestampBetweenOrderByTimestampDesc()`: Por rango de fechas
  - `findByNodeIdAndZonaOrderByTimestampDesc()`: Filtro por zona

### 3. Backend - Service

**RiegoEventoService.java**
- `procesarEvento(UUID nodeId, String payload)`:
  - Parsea payload JSON del MQTT
  - Solo guarda eventos de tipo "fin" (con duración real)
  - Extrae timestamp, origen, duración, versión agenda
  - Persiste en BD con raw JSON completo

- `getHistorial(UUID nodeId, int limit)`: Query paginado
- `getHistorialPorZona(UUID nodeId, Short zona)`: Filtro por zona

**MqttEventoSubscriber.java**
- Se auto-registra en topic `riego/+/evento` al iniciar
- Extrae nodeId del topic
- Delega procesamiento a `RiegoEventoService`

### 4. Backend - Controller

**RiegoEventoController.java**
- `GET /api/nodos/{nodeId}/historial?limit=50`:
  - Retorna últimos N eventos de un nodo
  - Default limit=50

- `GET /api/nodos/{nodeId}/historial/zona/{zona}`:
  - Filtra por zona específica

**DTO**: `RiegoEventoResponse.java` (sin campo `raw` para API limpia)

### 5. ESP32/ESP8266 - MQTT Publisher

**MqttManager.h/cpp**
- Nuevo método: `publishRiegoEvento(int zona, String evento, String origen, int duracion, int versionAgenda)`
- Topic: `riego/{nodeId}/evento`
- Payload JSON:
  ```json
  {
    "zona": 1,
    "evento": "inicio" | "fin",
    "timestamp": epochSeconds,
    "origen": "agenda" | "manual",
    "duracionProgramada": 600,  // solo en inicio
    "duracionReal": 598,        // solo en fin
    "versionAgenda": 7          // null si manual
  }
  ```

### 6. ESP32/ESP8266 - Relay Controller

**RelayController.h/cpp**
- Nuevos campos de estado:
  - `zoneDuracionProgramada[]`: duración inicial en segundos
  - `zoneOrigen[]`: "manual" o "agenda"
  - `zoneVersionAgenda[]`: versión de agenda ejecutada

- Nuevo callback: `RiegoEventCallback(zona, evento, origen, duracion, versionAgenda)`

- **turnOn()** modificado:
  - Acepta parámetros opcionales: `origen` (default "manual"), `versionAgenda` (default 0)
  - Guarda información de contexto
  - Publica evento "inicio" via callback

- **turnOff()** modificado:
  - Calcula duración real: `duracionReal = duracionProgramada - timerRestante`
  - Publica evento "fin" via callback
  - Limpia campos de contexto

- **loop()** modificado:
  - Al expirar timer automático, publica evento "fin" antes de apagar relay

### 7. ESP32/ESP8266 - Main

**main.cpp**
- Nuevo callback: `onRiegoEvent(zona, evento, origen, duracion, versionAgenda)`
  - Publica via `mqttManager.publishRiegoEvento()`
  - Log informativo

- Conecta callback al RelayController en setup:
  ```cpp
  relayController.setRiegoEventCallback(onRiegoEvent);
  ```

### 8. ESP32/ESP8266 - Agenda Manager

**AgendaManager.cpp**
- Al ejecutar agenda automática:
  - Extrae campo `version` del JSON de agendas
  - Llama `turnOn(zona, duracion, "agenda", versionAgenda)`

## Protocolo MQTT actualizado

Ver: [contratos-mqtt-http.md](contratos-mqtt-http.md)

### Nuevo topic de eventos

**Topic**: `riego/{nodeId}/evento`

**Evento inicio**:
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

**Evento fin**:
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

## Base de datos

Tabla `riego_evento` (ya existía, ahora se usa):
```sql
CREATE TABLE riego_evento (
    id UUID PRIMARY KEY,
    node_id UUID NOT NULL,
    zona SMALLINT NOT NULL,
    timestamp TIMESTAMP NOT NULL,
    duracion_seg INTEGER NOT NULL,
    origen VARCHAR(10) NOT NULL CHECK (origen IN ('agenda', 'manual')),
    version_agenda INTEGER,
    raw JSONB,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_riego_evento_node_ts 
    ON riego_evento(node_id, zona, timestamp DESC);
```

## Casos de uso

### Riego manual via MQTT
1. Backend publica comando: `riego/{nodeId}/cmd/zona/1` → `{"accion":"ON", "duracion":600}`
2. ESP32 recibe comando, ejecuta: `relayController.turnOn(1, 600, "manual", 0)`
3. ESP32 publica evento inicio
4. Backend recibe evento inicio (log, no persiste)
5. Tras 600s (o comando OFF), ESP32 publica evento fin
6. Backend persiste evento fin en `riego_evento`

### Riego automático por agenda
1. AgendaManager detecta hora programada (ej: 06:30)
2. Extrae versión de agenda del JSON (ej: version=7)
3. Ejecuta: `relayController.turnOn(1, 600, "agenda", 7)`
4. ESP32 publica evento inicio con `versionAgenda: 7`
5. Backend recibe evento inicio (log, no persiste)
6. Tras 600s, ESP32 publica evento fin con `duracionReal: 600`
7. Backend persiste evento con `origen="agenda"` y `version_agenda=7`

### Consulta de historial
```bash
# Últimos 50 eventos del nodo
GET /api/nodos/550e8400-e29b-41d4-a716-446655440000/historial

# Últimos 100 eventos
GET /api/nodos/550e8400-e29b-41d4-a716-446655440000/historial?limit=100

# Solo eventos de zona 1
GET /api/nodos/550e8400-e29b-41d4-a716-446655440000/historial/zona/1
```

## Testing

### Backend
1. Compilar: `mvn clean compile` ✅ (compilado exitosamente)
2. Levantar: `docker-compose up -d --build backend`
3. Verificar suscripción MQTT en logs: "Suscrito a topic MQTT de eventos: riego/+/evento"

### ESP32
1. Compilar firmware: `cd esp32/firmware; pio run`
2. Cargar: `pio run --target upload`
3. Monitor serial: `pio device monitor`
4. Verificar logs de eventos publicados

### End-to-end
1. Ejecutar riego manual via Postman/curl:
   ```bash
   curl -X POST http://localhost:8080/api/nodos/{nodeId}/cmd/zona/1 \
     -H "Content-Type: application/json" \
     -d '{"accion":"ON", "duracion":60}'
   ```
2. Esperar 60s o enviar OFF
3. Verificar evento en BD:
   ```sql
   SELECT * FROM riego_evento ORDER BY timestamp DESC LIMIT 5;
   ```
4. Consultar historial:
   ```bash
   curl http://localhost:8080/api/nodos/{nodeId}/historial
   ```

## Decisiones de diseño

### ¿Por qué solo persistir eventos "fin"?
- Evento "inicio" es informativo (duración es estimada)
- Evento "fin" tiene duración real ejecutada
- Reduce almacenamiento y simplifica queries
- Si se necesita inicio, se puede inferir: `timestamp - duracionSeg`

### ¿Por qué usar callback en lugar de instancia global?
- Desacoplamiento: RelayController no conoce MqttManager
- Testeable: se pueden inyectar mocks
- Flexible: permite múltiples listeners

### ¿Por qué timestamp en ESP32 y no en backend?
- ESP32 tiene NTP sincronizado
- Refleja momento exacto del evento en hardware
- Backend puede corregir si detecta deriva excesiva

## Próximos pasos

1. ✅ Implementación completa backend + ESP32
2. ⏳ Compilar y desplegar backend con Docker
3. ⏳ Cargar firmware en ESP8266
4. ⏳ Testing end-to-end con riegos manuales
5. ⏳ Testing con agendas programadas
6. ⏳ Frontend: Pantalla de historial con gráficos
7. ⏳ Analytics: Consumo total por zona/mes
8. ⏳ Alertas: Desvíos entre duraciones programadas vs reales

## Archivos modificados/creados

### Backend
- ✅ `model/RiegoEvento.java` (nuevo)
- ✅ `repository/RiegoEventoRepository.java` (nuevo)
- ✅ `dto/RiegoEventoResponse.java` (nuevo)
- ✅ `service/RiegoEventoService.java` (nuevo)
- ✅ `service/MqttEventoSubscriber.java` (nuevo)
- ✅ `controller/RiegoEventoController.java` (nuevo)

### ESP32/ESP8266
- ✅ `network/MqttManager.h` (modificado)
- ✅ `network/MqttManager.cpp` (modificado - nuevo método publishRiegoEvento)
- ✅ `hardware/RelayController.h` (modificado - nuevos campos y callback)
- ✅ `hardware/RelayController.cpp` (modificado - eventos en turnOn/turnOff/loop)
- ✅ `scheduler/AgendaManager.cpp` (modificado - pasa origen y versión)
- ✅ `main.cpp` (modificado - conecta callback onRiegoEvent)

### Documentación
- ✅ `docs/implementacion/contratos-mqtt-http.md` (actualizado con topic evento y endpoints)
- ✅ `docs/implementacion/historial-eventos-riego.md` (este documento)

## Estado del proyecto
**Implementación completa** ✅
- Backend compilado exitosamente
- ESP32 firmware listo (sin cargar en hardware)
- Documentación actualizada
- Endpoints REST definidos
- Protocolo MQTT extendido

**Pendiente de testing** ⏳
- Despliegue de backend con Docker
- Carga de firmware en ESP8266
- Pruebas end-to-end de publicación/persistencia
