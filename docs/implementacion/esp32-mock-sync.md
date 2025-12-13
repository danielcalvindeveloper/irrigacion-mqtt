# Alineación con firmware / mock ESP32

## Formato de agenda local
- JSON en flash (NVS/LittleFS):
```json
{
  "version": 7,
  "agendas": [
    {"id":"a1","zona":1,"diasSemana":["LUN"],"horaInicio":"06:30","duracionMin":10,"activa":true}
  ]
}
```
- Reglas: aceptar solo si `version` > local; validar rangos (zona 1..4, duracion 1..180, diasSemana no vacío, hora HH:MM).

## nodeId
- UUID persistido en flash; mismo valor usado en todos los topics.
- Provisionar en build o primera configuración; no cambiar sin reflashear credenciales MQTT.

## Mock / stub para pruebas
- Opción 1: Sketch Arduino que suscribe a `agenda/sync` y publica `status`/`telemetria` con mismos topics; usar en CI con emulador (arduino-cli) o hardware-in-the-loop.
- Opción 2: Servicio JVM/Node que emule ESP32:
  - Suscribe `riego/{nodeId}/cmd/zona/+` y publica `status` con eco de la acción.
  - Al recibir `agenda/sync` guarda versión en memoria y responde con `status`.
  - Publica `telemetria` periódica.
- Recomendado: Opción 2 para CI rápida; Opción 1 para validación en dispositivo real.

## Checklist de compatibilidad
- Coincidencia exacta de topics con `nodeId`.
- Codificación UTF-8, JSON minificado o pretty indiferente.
- TLS: en mock CI puede ser plaintext; en prod TLS obligatorio.
- Retener mensajes: desactivado para `cmd`; permitido para último `agenda/sync` (opcional) si broker lo soporta y no rompe flujo.
