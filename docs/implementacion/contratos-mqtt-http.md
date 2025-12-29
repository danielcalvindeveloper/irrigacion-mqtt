# Contratos MQTT y HTTP (Implementación Actual)

## MQTT

### Comando manual
- **Topic**: `riego/{nodeId}/cmd/zona/{id}`
- **Payload**:
```json
{
  "accion": "ON",
  "duracion": 600
}
```
- **Reglas**:
  - `accion` ∈ {"ON","OFF"} (requerido)
  - `duracion` (segundos) obligatorio si `accion=ON`, rango 1..7200; omitir si `OFF`
  - `id` de zona entero 1..4

### Estado de zona
- **Topic**: `riego/{nodeId}/status/zona/{id}`
- **Payload** (publicado por ESP32):
```json
{
  "activa": true,
  "tiempoRestante": 600
}
```
- **Reglas**:
  - `activa`: boolean, indica si la zona está regando
  - `tiempoRestante`: segundos restantes de riego (0 si inactiva)

### Evento de riego
- **Topic**: `riego/{nodeId}/evento`
- **Payload** (publicado por ESP32):
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
```json
{
  "zona": 1,
  "evento": "fin",
  "timestamp": 1735415880,
  "origen": "agenda",
  "duracionReal": 598,
  "versionAgenda": 7
}
```
- **Reglas**:
  - `zona`: int 1..4, zona que ejecutó el riego
  - `evento`: string ∈ {"inicio", "fin"}
  - `timestamp`: epoch time en segundos (Unix timestamp)
  - `origen`: string ∈ {"agenda", "manual"} indica si fue programado o comando directo
  - `duracionProgramada`: segundos (solo en evento "inicio")
  - `duracionReal`: segundos ejecutados (solo en evento "fin")
  - `versionAgenda`: int nullable, versión de agenda que ejecutó (null si origen=manual)

### Sync de agenda
- **Topic**: `riego/{nodeId}/agenda/sync`
- **Payload** (publicado por backend):
```json
{
  "version": 7,
  "nombre": "Agenda Zona 1 - 06:30",
  "activa": true,
  "programaciones": [
    {
      "zona": 1,
      "hora": "06:30",
      "duracionMinutos": 10,
      "diasSemana": ["LUN", "MIE", "VIE"]
    }
  ]
}
```
- **Reglas**:
  - `version`: entero incremental por `nodeId`
  - `nombre`: string opcional, identificador de la agenda
  - `activa`: boolean, si la agenda está habilitada
  - `hora`: HH:MM 24h
  - `duracionMinutos`: 1..180
  - `diasSemana`: array de ["LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"]
  - ESP32 reemplaza si recibe versión mayor; ignora si menor/igual

## HTTP REST Backend

### Agendas

#### `GET /api/nodos/{nodeId}/agendas`
Lista todas las agendas de un nodo.

**Response** (200 OK):
```json
[
  {
    "id": "uuid",
    "nodeId": "uuid",
    "nombre": "Zona 1 - 06:30",
    "zona": 1,
    "diasSemana": ["LUN", "MIE", "VIE"],
    "horaInicio": "06:30",
    "duracionMin": 10,
    "activa": true,
    "version": 7,
    "updatedAt": "2025-12-15T10:30:00Z"
  }
]
```

#### `POST /api/nodos/{nodeId}/agendas`
Crea o actualiza una agenda (upsert). Si el `id` existe, actualiza; si no, crea nueva.

**Request**:
```json
{
  "id": "uuid",
  "nodeId": "uuid",
  "nombre": "Zona 1 - 06:30",
  "zona": 1,
  "diasSemana": ["LUN", "MIE", "VIE"],
  "horaInicio": "06:30",
  "duracionMin": 10,
  "activa": true
}
```

**Validaciones**:
- `id`: UUID requerido
- `nodeId`: UUID requerido, debe coincidir con path
- `nombre`: string opcional (máx 100 caracteres)
- `zona`: short 1..4
- `diasSemana`: array no vacío, valores ∈ {LUN, MAR, MIE, JUE, VIE, SAB, DOM}
- `horaInicio`: string formato HH:MM (requerido)
- `duracionMin`: short 1..180
- `activa`: boolean, default true

**Response** (200 OK): Mismo formato que GET

**Nota**: Al guardar, incrementa `version` y publica sync MQTT automáticamente.

#### `DELETE /api/nodos/{nodeId}/agendas/{agendaId}`
Elimina una agenda.

**Response**: 204 No Content

**Nota**: Incrementa `version` y publica sync MQTT automáticamente.

### Estado de zonas

#### `GET /api/nodos/{nodeId}/status`
Obtiene el estado actual de todas las zonas de un nodo.

**Response** (200 OK):
```json
[
  {
    "zona": 1,
    "nombre": "Zona 1",
    "activa": true,
    "tiempoRestanteSeg": 540,
    "proximoRiego": "Hoy 18:30 (10min)"
  },
  {
    "zona": 2,
    "nombre": "Zona 2",
    "activa": false,
    "tiempoRestanteSeg": null,
    "proximoRiego": "Mañana 07:00 (15min)"
  }
]
```

**Campos**:
- `zona`: int, número de zona (1..4)
- `nombre`: string, nombre descriptivo de la zona
- `activa`: boolean, si está regando actualmente
- `tiempoRestanteSeg`: int nullable, segundos restantes de riego (null si inactiva)
- `proximoRiego`: string, descripción legible del próximo riego programado

### Historial de riego

#### `GET /api/nodos/{nodeId}/historial`
Obtiene el historial de eventos de riego de un nodo.

**Query params**:
- `limit`: int opcional (default 50, max 200), número de eventos a retornar

**Response** (200 OK):
```json
[
  {
    "id": "uuid",
    "zona": 1,
    "timestamp": "2025-12-28T10:30:00Z",
    "duracionSeg": 598,
    "origen": "agenda",
    "versionAgenda": 7
  },
  {
    "id": "uuid",
    "zona": 2,
    "timestamp": "2025-12-28T09:15:00Z",
    "duracionSeg": 120,
    "origen": "manual",
    "versionAgenda": null
  }
]
```

**Campos**:
- `id`: UUID del evento
- `zona`: int 1..4
- `timestamp`: ISO 8601 timestamp del fin del riego
- `duracionSeg`: int, duración real en segundos
- `origen`: string ∈ {"agenda", "manual"}
- `versionAgenda`: int nullable, versión de agenda (null si manual)

#### `GET /api/nodos/{nodeId}/historial/zona/{zona}`
Filtra el historial por zona específica.

**Response** (200 OK): Mismo formato que endpoint anterior
- `proximoRiego`: string nullable, próximo riego programado con formato "Hoy/Mañana HH:MM (Xmin)"

**Nota**: El estado se actualiza en memoria cuando el backend recibe mensajes MQTT del topic `riego/{nodeId}/status/zona/{id}`.

### Comandos

#### `POST /api/nodos/{nodeId}/cmd`
Envía un comando manual de riego a una zona.

**Request**:
```json
{
  "nodeId": "uuid",
  "zona": 1,
  "accion": "ON",
  "duracion": 600
}
```

**Validaciones**:
- `nodeId`: UUID requerido, debe coincidir con path
- `zona`: short 1..4
- `accion`: string ∈ {"ON", "OFF"} (requerido)
- `duracion`: int 1..7200 (segundos), requerido si `accion=ON`, omitir si `OFF`

**Response**: 
- 202 Accepted: comando publicado en MQTT
- 503 Service Unavailable: MQTT no disponible

**Nota**: El comando se publica en el topic `riego/{nodeId}/cmd/zona/{zona}`.

## Endpoints futuros (no implementados)

Los siguientes endpoints están planificados pero no implementados:
- `GET /api/nodos`: lista nodos registrados
- `GET /api/nodos/{nodeId}/eventos`: histórico de riego
- `GET /api/nodos/{nodeId}/humedad`: lecturas de humedad
- WebSocket `/ws/estado`: actualizaciones en tiempo real
