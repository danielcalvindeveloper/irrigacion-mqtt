# Diseño Backend (MVP)

## Stack
- Spring Boot (Java 17+)
- HiveMQ MQTT Client
- Base de datos relacional (PostgreSQL recomendado)

## Modelo de datos (tablas mínimas)
- `agenda`:
  - `id` (PK), `node_id`, `zona`, `dias_semana` (array/JSON), `hora_inicio` (HH:MM), `duracion_min`, `activa`, `version`, `updated_at`.
- `riego_evento`:
  - `id` (PK), `node_id`, `zona`, `timestamp`, `duracion_seg`, `origen` (agenda/manual), `version_agenda` (opcional), `raw` (JSON payload original).
- `humedad`:
  - `id` (PK), `node_id`, `zona`, `timestamp`, `valor`, `raw` (JSON).
- Índices por (`node_id`,`zona`,`timestamp`).

## Migraciones iniciales (DDL sugerido)
- `agenda`
  - PK: (`id` uuid)
  - UNIQUE: (`node_id`,`id`) y (`node_id`,`zona`,`dias_semana`,`hora_inicio`) para evitar duplicados exactos
  - CHECK: `duracion_min` between 1 and 180; `hora_inicio` formato HH:MM; `zona` between 1 and 4
- `agenda_version`
  - `node_id` PK, `version` int not null, `updated_at` timestamptz
  - Facilita incrementos atómicos por nodo
- `riego_evento`
  - PK: uuid
  - IDX: (`node_id`,`zona`,`timestamp` DESC)
  - CHECK: `duracion_seg` between 1 and 7200; `origen` in ('agenda','manual')
- `humedad`
  - PK: uuid
  - IDX: (`node_id`,`zona`,`timestamp` DESC)
  - CHECK: `valor` between 0 and 100

## Retención y mantenimiento
- Eventos: borrar >12 meses (job diario); Humedad cruda: borrar >30 días o agregar por día.
- Crear job SQL o tarea schedulada en backend con ventana deslizante por `node_id`.

## Servicios
- AgendaService: CRUD + validación de solapes por zona; gestión de versionado incremental por `nodeId`.
- MqttBridgeService: publica comandos, consume status/telemetría, encola eventos.
- TelemetriaService: persiste humedad y eventos; aplica retención.
- WebSocketPush: publica estado en vivo a clientes.

## Versionado de agenda
- `version` entero por `nodeId` almacenado en `agenda_version`.
- En cada cambio de agenda: `version++`, guardar `agenda` y publicar `sync` MQTT.
- Concurrencia: usar `SELECT ... FOR UPDATE` o versión en DB para evitar doble incremento.

## Flujos clave
- Alta/edición de agenda:
  - Valida solape.
  - Incrementa `version` global por `nodeId`.
  - Publica mensaje de sync por MQTT.
- Comando manual:
  - Registra intento.
  - Publica MQTT `cmd`.
- Ingreso de status/evento:
  - Guarda en `riego_evento` con timestamp de origen.
  - Empuja por WebSocket a suscriptores.
- Telemetría humedad:
  - Guarda en `humedad`.
  - Aplica política de retención (crudo 30 días o agregado diario).

## Observabilidad
- Actuator: health, metrics, info.
- Métricas recomendadas: latencia publish/ack MQTT, reconexiones, eventos procesados/s, errores de validación de agenda.

## Seguridad

### Implementación Actual (MVP)
- **HTTP Basic Authentication**: Usuario/password desde variables de entorno
  - Variables: `APP_SECURITY_USERNAME`, `APP_SECURITY_PASSWORD`
  - Configuración en `SecurityConfig.java` (package: `ar.net.dac.iot.irrigacion.config`)
  - In-memory authentication (sin base de datos)
- **CORS**: Habilitado para desarrollo (frontend:5173)
- **MQTT**: Sin autenticación en desarrollo (TLS planeado para producción)
- **Credenciales**: Gestionadas en archivo `.env` (excluido de Git)

### Futuro
- Migrar a JWT para multi-usuario
- MQTT con TLS y credenciales por dispositivo
- Rate limiting en `POST /cmd`
- WebSocket authentication
