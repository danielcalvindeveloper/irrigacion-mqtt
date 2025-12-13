# MQTT Topics y Mensajes

> Convención sugerida: `{base}/{tipo}/zona/{id}` y un canal de sync para agenda.

## Comandos
**Topic:** `riego/cmd/zona/{id}`

Payload:
```json
{
  "accion": "ON | OFF",
  "duracion": 600
}
```

## Estado
**Topic:** `riego/status/zona/{id}`

Payload:
```json
{
  "estado": "ON | OFF",
  "origen": "agenda | manual | humedad"
}
```

## Agenda (sync)
**Topic:** `riego/agenda/sync`

Payload:
```json
{
  "version": 3,
  "agendas": []
}
```

## Telemetría
**Topic:** `riego/telemetria`

Payload:
```json
{
  "humedad": 42,
  "uptime": 123456
}
```
