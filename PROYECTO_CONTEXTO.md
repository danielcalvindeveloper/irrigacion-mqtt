# 📋 Contexto del Proyecto - Sistema de Riego MQTT

> **Propósito**: Este documento centraliza el estado completo del proyecto (arquitectura, implementación, decisiones) para mantener contexto consistente durante el desarrollo.
> 
> **Última actualización**: 2025-12-15

---

## 📌 Stack Tecnológico

### Hardware
- **ESP32 NodeMCU** (CP2102) - Nodo de riego con WiFi
- **Relés 4CH TTL** - Control de 4 zonas de riego
- **Sensor capacitivo de humedad** (v2.0) - Monitoreo de suelo

### Backend
- **Java 17+** con Spring Boot 3.4.0
- **PostgreSQL 15.15** - Base de datos relacional
- **HiveMQ MQTT Client 1.3.3** - Cliente MQTT para pub/sub
- **Flyway 10.20.1** - Migraciones de BD
- **Maven** - Gestión de dependencias
- **Broker MQTT**: HiveMQ en `localhost:1883`

### Frontend
- **Vue 3** (Composition API) - Framework reactivo
- **Vuetify 3** - Framework de componentes Material Design
- **Pinia** - State management
- **Axios** - Cliente HTTP
- **Vite** - Build tool y dev server
- **PWA** (futuro) - Progressive Web App

### DevOps
- **Docker Compose** - Orquestación de servicios (PostgreSQL, backend)
- **VSCode** - IDE principal
- **Python 3** - Scripts de mock ESP32

---

## 🏗️ Arquitectura del Sistema

### Flujo de Datos Principal

```
[Frontend Vue] ←HTTP→ [Backend Spring Boot] ←MQTT→ [ESP32 Nodo]
                              ↕
                       [PostgreSQL DB]
```

### Comunicación MQTT

#### Topics implementados:
1. **Comandos**: `riego/{nodeId}/cmd/zona/{zona}`
   - Backend → ESP32
   - Payload: `{"accion": "ON|OFF", "duracion": segundos}`

2. **Estado**: `riego/{nodeId}/status/zona/{zona}`
   - ESP32 → Backend
   - Payload: `{"activa": boolean, "tiempoRestante": segundos}`

3. **Sincronización de agenda**: `riego/{nodeId}/agenda/sync`
   - Backend → ESP32
   - Payload: Ver contrato en `docs/implementacion/contratos-mqtt-http.md`

### Node ID por defecto
- **UUID**: `550e8400-e29b-41d4-a716-446655440000`
- Configurado en `.env` del frontend como `VITE_DEFAULT_NODE_ID`

---

## 💾 Modelo de Datos

### Base de Datos (PostgreSQL)

#### Tabla: `agenda`
```sql
id              UUID PRIMARY KEY
node_id         UUID NOT NULL
nombre          VARCHAR(100) NULL         -- Agregado en V2
zona            SMALLINT (1..4)
dias_semana     TEXT (CSV: LUN,MAR,MIE,etc)
hora_inicio     TIME
duracion_min    SMALLINT (1..180)
activa          BOOLEAN DEFAULT TRUE
version         INTEGER
updated_at      TIMESTAMPTZ
```

**Migraciones aplicadas**:
- `V1__init.sql`: Schema inicial (agenda, agenda_version, riego_evento, humedad)
- `V2__add_nombre_to_agenda.sql`: Campo `nombre` opcional

#### Tabla: `agenda_version`
```sql
node_id         UUID PRIMARY KEY
version         INTEGER
updated_at      TIMESTAMPTZ
```

#### Tabla: `riego_evento` (histórico)
```sql
id              UUID PRIMARY KEY
node_id         UUID
zona            SMALLINT
timestamp       TIMESTAMPTZ
duracion_seg    INTEGER
origen          TEXT ('agenda'|'manual')
version_agenda  INTEGER
raw             JSONB
created_at      TIMESTAMPTZ
```

#### Tabla: `humedad` (sensor)
```sql
id              UUID PRIMARY KEY
node_id         UUID
zona            SMALLINT
timestamp       TIMESTAMPTZ
valor           SMALLINT (0..100)
raw             JSONB
created_at      TIMESTAMPTZ
```

---

## 🔌 APIs Implementadas

### REST Endpoints (Backend)

#### Agendas
- `GET /api/nodos/{nodeId}/agendas`
  - Lista todas las agendas de un nodo
  - Response: `AgendaResponse[]`

- `POST /api/nodos/{nodeId}/agendas`
  - **Upsert** (crea o actualiza según `id`)
  - Request: `AgendaRequest`
  - Response: `AgendaResponse`
  - Side-effect: Incrementa `version` y publica MQTT sync

- `DELETE /api/nodos/{nodeId}/agendas/{agendaId}`
  - Elimina agenda
  - Response: 204 No Content
  - Side-effect: Incrementa `version` y publica MQTT sync

#### Estado de Zonas
- `GET /api/nodos/{nodeId}/status`
  - Estado actual de las 4 zonas
  - Response: `ZoneStatusResponse[]`
  - Fuente: Caché en memoria actualizado por MQTT

#### Comandos Manuales
- `POST /api/nodos/{nodeId}/cmd`
  - Envía comando manual de riego
  - Request: `CommandRequest`
  - Response: 202 Accepted (comando enviado) | 503 (MQTT no disponible)
  - Side-effect: Publica comando en MQTT

### DTOs (Java)

#### `AgendaRequest`
```java
UUID id             // Requerido
UUID nodeId         // Requerido, debe coincidir con path
String nombre       // Opcional (max 100)
short zona          // 1..4
List<String> diasSemana  // ["LUN", "MAR", ...] no vacío
String horaInicio   // HH:MM formato 24h
short duracionMin   // 1..180
boolean activa      // default true
```

#### `AgendaResponse`
```java
// Extiende AgendaRequest con:
int version         // Versión de sincronización
OffsetDateTime updatedAt
```

#### `ZoneStatusResponse`
```java
int zona                    // 1..4
String nombre               // "Zona 1" (actualmente hardcoded)
boolean activa              // ¿Está regando?
Integer tiempoRestanteSeg   // null si inactiva
String proximoRiego         // "Hoy 18:30 (10min)" o null
```

#### `CommandRequest`
```java
UUID nodeId         // Requerido, debe coincidir con path
short zona          // 1..4
String accion       // "ON" | "OFF"
Integer duracion    // Segundos 1..7200, requerido si ON
```

### Frontend Services

#### `api.js` (Axios)
```javascript
getAgendas(nodeId)
crearAgenda(nodeId, agendaData)
eliminarAgenda(nodeId, agendaId)
getZonesStatus(nodeId)
iniciarRiegoManual(nodeId, zona, duracion)
detenerRiego(nodeId, zona)
```

---

## 🎯 Features Implementadas

### ✅ Gestión de Agendas
- **Vista**: `AgendaView.vue`
- **Store**: `stores/agendas.js`
- Listar agendas existentes con nombre auto-generado si vacío
- Crear/editar agendas con formulario modal
- Campo `nombre` opcional (genera "Zona X - HH:MM" si vacío)
- Toggle activa/inactiva
- Eliminar agendas
- Días de semana mostrados como chips (L, M, X, J, V, S, D)
- Validación de campos (horaInicio HH:MM, duracionMin 1..180)

### ✅ Dashboard de Zonas
- **Vista**: `DashboardView.vue`
- **Store**: `stores/zones.js`
- Estado en tiempo real de 4 zonas (polling 5s)
- Indicador visual: activa/inactiva
- Tiempo restante en minutos (convertido desde segundos)
- Barra de progreso de riego
- Próximo riego programado
- Botones de control manual: iniciar (con duración) y detener

### ✅ Backend MQTT Integration
- **Servicio**: `MqttStatusSubscriber.java`
  - Suscripción automática a `riego/+/status/zona/+`
  - Actualiza caché en memoria cuando ESP32 publica estado
  - Usa HiveMQ async client: `mqttClient.toAsync().subscribeWith().callback().send()`

- **Servicio**: `ZoneStatusService.java`
  - Caché en memoria: `Map<String, Map<Integer, ZoneStatus>>`
  - Calcula próximo riego desde agendas activas (busca 7 días adelante)
  - Devuelve estado de 4 zonas con tiempo restante y próxima agenda

- **Servicio**: `MqttGateway.java`
  - Publica comandos en `riego/{nodeId}/cmd/zona/{zona}`
  - Publica agenda sync en `riego/{nodeId}/agenda/sync`

### ✅ Mock ESP32
- **Script**: `esp32/mock_esp32.py`
- Simula nodo ESP32 conectado a MQTT
- Recibe comandos y agendas
- **Limitación actual**: NO simula finalización automática del riego
  - Publica estado inicial al recibir comando ON
  - No decrementa tiempo restante
  - No publica finalización al llegar a 0

---

## 🚧 Limitaciones Conocidas

### Configuración de Zonas
- **Número fijo**: Siempre 4 zonas (hardcoded)
- **Nombres genéricos**: Backend devuelve "Zona 1", "Zona 2", etc.
- **No persistente**: No hay tabla de configuración de zonas
- **Fallback inconsistente**: Mock del store tiene nombres descriptivos que no se usan

### Mock ESP32
- No simula el transcurso del tiempo
- No notifica finalización automática
- Estado se mantiene indefinidamente hasta comando OFF manual

### CORS en Mobile
- Funciona desde PC pero falla desde mobile
- Configuración actual usa patrón de origen, puede requerir origen específico

### Actualizaciones en Tiempo Real
- Frontend usa polling cada 5 segundos
- WebSocket no implementado (planeado para futuro)

---

## 🔧 Configuración del Entorno

### Variables de Entorno

#### Backend (`backend/src/main/resources/application.yml`)
```yaml
spring.datasource.url: jdbc:postgresql://postgres:5432/irrigacion
mqtt.host: mosquitto
mqtt.port: 1883
mqtt.enabled: true
```

#### Frontend (`.env`)
```
VITE_API_BASE_URL=http://localhost:8080/api
VITE_DEFAULT_NODE_ID=550e8400-e29b-41d4-a716-446655440000
```

### Docker Compose
- **PostgreSQL**: Puerto 5432
- **Backend**: Puerto 8080, depende de postgres
- **Frontend**: Dev server Vite en puerto 5173
- **MQTT Broker**: HiveMQ en puerto 1883

---

## 📝 Decisiones de Diseño

### ¿Por qué POST para create y update?
Backend implementa **upsert** en un solo endpoint POST. Si el `id` existe, actualiza; si no, crea. Simplifica lógica de frontend (no necesita distinguir create vs update).

### ¿Por qué estado en memoria?
ZoneStatusService mantiene estado en memoria (no en BD) porque:
- Es información volátil (tiempo restante)
- Alta frecuencia de actualización
- No requiere persistencia histórica
- Simplifica arquitectura MVP

### ¿Por qué nombre opcional en agenda?
Permite flexibilidad: usuario puede personalizar o dejar que se auto-genere un nombre descriptivo "Zona X - HH:MM".

### ¿Por qué días de semana como String CSV?
Simplifica queries y validación. Alternativa (tabla relacional) sería over-engineering para MVP.

### ¿Por qué HiveMQ Client?
- API moderna y asíncrona
- Mejor soporte de backpressure
- Documentación clara
- Compatible con Spring Boot

---

## 🗺️ Roadmap Futuro

### Features Planeadas (No Implementadas)
- [ ] WebSocket para actualizaciones en tiempo real
- [ ] Vista de histórico de riego (`GET /api/nodos/{nodeId}/eventos`)
- [ ] Vista de lecturas de humedad (`GET /api/nodos/{nodeId}/humedad`)
- [ ] Configuración de zonas (nombres personalizados, habilitar/deshabilitar)
- [ ] Gestión de múltiples nodos
- [ ] Mock ESP32 con simulación de tiempo real
- [ ] PWA (instalable en móvil)
- [ ] Notificaciones push
- [ ] Gráficos de consumo de agua
- [ ] Autenticación y multi-usuario

### Mejoras Técnicas
- [ ] Tests de integración E2E
- [ ] CI/CD pipeline
- [ ] Métricas y observabilidad
- [ ] Rate limiting en API
- [ ] Validación de solapamiento de agendas por zona
- [ ] Retry logic en MQTT
- [ ] Manejo de desconexiones ESP32

---

## 📚 Documentación Relacionada

### Carpeta `docs/`
- **Requerimientos**: `01-requerimientos-funcionales.md`, `02-requerimientos-no-funcionales.md`
- **Arquitectura**: `03-arquitectura-general.md`
- **Diseño**: `04-modelo-de-agenda.md`, `05-mqtt-topics-y-mensajes.md`
- **Roadmap**: `09-roadmap-mvp.md`

### Carpeta `docs/implementacion/`
- **Contratos**: `contratos-mqtt-http.md` ⭐ **ACTUALIZADO**
- **Diseño Backend**: `backend-diseno.md`
- **Testing**: `tests-integracion-mqtt.md`, `testing-end-to-end-mock.md`
- **Guías**: `docker-compose-guide.md`, `manual-postman-agendas-cmd.md`

---

## 🔄 Cómo Mantener Este Documento

### Cuándo actualizar:
1. **Cambios en arquitectura** (nuevos servicios, tablas, endpoints)
2. **Nuevas features completadas** (mover de roadmap a implementadas)
3. **Decisiones técnicas importantes** (cambio de librería, patrón, etc.)
4. **Limitaciones descubiertas** (bugs conocidos, restricciones)
5. **Cambios en configuración** (variables de entorno, puertos)

### Secciones clave a revisar frecuentemente:
- **Última actualización** (fecha al inicio)
- **APIs Implementadas** (sincronizar con código real)
- **Features Implementadas** (marcar ✅ cuando se completan)
- **Limitaciones Conocidas** (agregar bugs/restricciones descubiertos)
- **Roadmap Futuro** (mover items completados a "Implementadas")

---

## 💡 Cómo Usar Este Documento

### Para GitHub Copilot / Agentes AI:
1. **Inicio de sesión**: Leer este documento COMPLETO para tener contexto
2. **Durante desarrollo**: Consultar secciones específicas (DTOs, APIs, arquitectura)
3. **Antes de commits**: Verificar si hay cambios que documentar
4. **Al completar features**: Actualizar secciones correspondientes

### Para Desarrolladores:
- **Onboarding**: Leer secciones Stack, Arquitectura, y Features
- **Implementación**: Consultar DTOs, APIs, y Contratos MQTT
- **Debugging**: Revisar Limitaciones Conocidas y Configuración
- **Planning**: Consultar Roadmap y Decisiones de Diseño

### Comandos útiles:
```bash
# Ver contexto completo
cat PROYECTO_CONTEXTO.md

# Buscar información específica
grep -i "mqtt" PROYECTO_CONTEXTO.md

# Validar sincronización con código
# (comparar DTOs documentados vs Java/TypeScript)
```

---

**Documento vivo** - Debe evolucionar con el proyecto. Prefiere precisión sobre completitud.
