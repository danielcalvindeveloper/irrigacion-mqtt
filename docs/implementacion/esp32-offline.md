# ESP32: Offline y Scheduler (MVP)

## Persistencia local
- Almacén: NVS o LittleFS (clave `agenda.json`).
- Formato guardado:
```json
{
  "version": 7,
  "agendas": [ { "zona":1, "diasSemana":["LUN"], "horaInicio":"06:30", "duracionMin":10, "activa":true } ]
}
```
- Regla: solo reemplazar si `version` recibida > `version` local.
 - Validaciones: `zona` 1..4; `horaInicio` HH:MM; `duracionMin` 1..180; `diasSemana` no vacío.
 - `nodeId`: UUID almacenado en flash; mismo valor en todos los topics.

## Scheduler local
- Evalúa cada minuto: para cada zona, si día/hora coincide y no hay ejecución en curso.
- Sin solape por zona: ignorar arranque si ya está ON manual.
- Comando manual > agenda: si llega `cmd ON/OFF`, interrumpe agenda activa y aplica comando.
 - Duración: riego ON por `duracionMin` y apaga al completar o si recibe `OFF` manual.

## Buffer de eventos
- Tipo: circular en RAM con spill a flash si se requiere.
- Capacidad: 50–100 eventos (timestamp, zona, duracion, origen).
- Envío: al reconectar, publicar batch y luego en tiempo real.
 - Orden: preservar FIFO; marcar eventos enviados para evitar duplicado.

## Telemetría
- Publicar humedad, uptime, rssi periódicamente.
- Frecuencia ajustable; bajar frecuencia si offline prolongado.
 - No publicar datos de agenda; solo estado/telemetría.

## Conectividad
- MQTT sobre TLS (HiveMQ Cloud).
- Credenciales user/pass hardcodeadas en MVP; parametrizar en OTA post-MVP.

## Identidad de nodo
- `nodeId` fijo (UUID) embebido en firmware o provisionado en flash.
- Todos los topics incluyen `nodeId`.
