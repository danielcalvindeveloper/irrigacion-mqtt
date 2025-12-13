# Pruebas y Escenarios (MVP)

## Escenarios críticos
- Conectividad básica: ESP32 conecta por TLS y publica telemetría.
- Comando manual: backend publica `cmd ON`, ESP32 acciona relé y reporta `status ON`.
- Agenda offline: sin Internet, ESP32 ejecuta agenda programada; al volver, sincroniza eventos y estado.
- Versionado: backend envía `version` n+1, ESP32 reemplaza; si recibe n-1, ignora.
- Solape: intentar crear agendas solapadas en backend → debe rechazar.
- Manual > agenda: durante riego por agenda, enviar `OFF` manual → detiene; siguiente slot de agenda funciona.

## Casos de prueba sugeridos
- API agendas: crear, editar, borrar; validar que `version` incrementa y se publica sync MQTT.
- WebSocket: recibir eventos de estado en vivo tras comando manual.
- Buffer reconexión: generar 60 eventos offline, reconectar y verificar envío batch ordenado.
- Retención: verificar cleanup de humedad cruda >30 días y eventos >12 meses.
- Seguridad: JWT requerido en APIs; CORS solo dominio UI; rechazo de token inválido.
- Validación de solape: intentar agendas que se traslapan en misma zona → debe fallar con 400.
- Rango de comandos: rechazar `duracion` fuera de 1..7200 y `zona` fuera de 1..4.

## Automatización sugerida (mock MQTT)
- Emplear contenedor HiveMQ o Mosquitto en local para pruebas.
- Test de integración Spring Boot con cliente MQTT embebido:
	- Publicar `status` y verificar persistencia en DB.
	- Simular `sync` de agenda recibido por ESP32 mock y validar almacenamiento local (si se mockea firmware) o ack de versión.
- Scripts de carga moderada: 100 comandos manuales/sec durante 10s para verificar rate-limit y estabilidad.

## Datos de prueba
- Nodo: `nodeId = test-node-1`
- Zonas: 1..2
- Agenda: LUN-MIE-VIE 06:30 dur 10 min; MAR-JUE 18:00 dur 8 min.
- Comandos manuales: ON 120s, OFF inmediato.
