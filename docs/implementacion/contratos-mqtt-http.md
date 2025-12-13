# Contratos MQTT y HTTP (MVP)

## MQTT
### Comando manual
- Topic: `riego/{nodeId}/cmd/zona/{id}`
- Payload:
```json
{
  "accion": "ON",
  "duracion": 600
}
```
- Reglas:
  - `accion` ∈ {"ON","OFF"}
  - `duracion` (segundos) obligatorio si `accion=ON`, rango 1..7200; prohibido si `OFF`.
  - `id` de zona entero 1..4 (configurable, pero validar en backend al publicar).

### Estado
- Topic: `riego/{nodeId}/status/zona/{id}`
- Payload:
```json
{
  "estado": "ON",
  "origen": "agenda"
}
```
- Reglas: `estado` ∈ {"ON","OFF"}; `origen` ∈ {"agenda","manual"} (humedad solo lectura en MVP).

### Sync de agenda
- Topic: `riego/{nodeId}/agenda/sync`
- Payload:
```json
{
  "version": 7,
  "updatedAt": "2025-09-12T10:30:00Z",
  "agendas": [
    {
      "id": "a1",
      "zona": 1,
      "diasSemana": ["LUN","MIE","VIE"],
      "horaInicio": "06:30",
      "duracionMin": 10,
      "activa": true
    }
  ]
}
```
- Reglas:
  - `version` entero incremental por `nodeId`.
  - `horaInicio` HH:MM 24h; `duracionMin` 1..180.
  - `diasSemana` ∈ {LUN..DOM}; lista no vacía.
  - ESP32 reemplaza si recibe versión mayor; ignora si menor/igual.

### Telemetría
- Topic: `riego/{nodeId}/telemetria`
- Payload:
```json
{
  "humedad": 42,
  "uptime": 123456,
  "rssi": -60
}
```
- Reglas: `humedad` 0..100 (% relativo), `uptime` seg, `rssi` dBm negativo; frecuencia configurable.

## HTTP/WS Backend (MVP)

### REST (contratos y validaciones clave)
- `GET /api/nodos`: lista nodos registrados.
- `GET /api/nodos/{nodeId}/agendas`: devuelve agendas + `version`.
- `POST /api/nodos/{nodeId}/agendas`: crea/actualiza; valida:
  - `zona` 1..4
  - `horaInicio` HH:MM, `duracionMin` 1..180
  - sin solape de intervalos por zona
  - incrementa `version` global por `nodeId` y publica sync MQTT
- `DELETE /api/nodos/{nodeId}/agendas/{agendaId}`: marca inactiva o elimina y sube `version`.
- `GET /api/nodos/{nodeId}/eventos?zona=&desde=&hasta=`: histórico de riego, pagina.
- `GET /api/nodos/{nodeId}/humedad?zona=&desde=&hasta=`: lecturas humedad.
- `POST /api/nodos/{nodeId}/cmd`: valida zona y acción; publica MQTT; registra intento.

### WebSocket
- Canal: `/ws/estado`
- Mensajes push:
```json
{
  "nodeId": "...",
  "zona": 1,
  "estado": "ON",
  "origen": "manual",
  "ts": "2025-09-12T10:30:00Z"
}
```
- Reglas: autenticado por JWT; CORS restringido; usar `nodeId` para filtrar suscripciones.
