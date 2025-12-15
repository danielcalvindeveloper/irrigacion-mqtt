# 📋 Contexto del Proyecto - Sistema de Riego MQTT

> **Propósito**: Este documento centraliza el estado completo del proyecto (arquitectura, implementación, decisiones) para mantener contexto consistente durante el desarrollo.
> 
<<<<<<< HEAD
> **Última actualización**: 2025-12-15 (Refactoring de packages completado - Sistema operativo con autenticación)
=======
> **Última actualización**: 2025-12-15
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

---

## 📌 Stack Tecnológico

### Hardware
- **ESP32 NodeMCU** (CP2102) - Nodo de riego con WiFi
<<<<<<< HEAD
- **Relés 4CH TTL** - Control de hasta 8 zonas de riego
=======
- **Relés 4CH TTL** - Control de 4 zonas de riego
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
- **Sensor capacitivo de humedad** (v2.0) - Monitoreo de suelo

### Backend
- **Java 17+** con Spring Boot 3.4.0
<<<<<<< HEAD
- **Package base**: `ar.net.dac.iot.irrigacion` (refactorizado desde estructura inicial)
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
- **PostgreSQL 15.15** - Base de datos relacional
- **HiveMQ MQTT Client 1.3.3** - Cliente MQTT para pub/sub
- **Flyway 10.20.1** - Migraciones de BD
- **Maven** - Gestión de dependencias
- **Broker MQTT**: HiveMQ en `localhost:1883`
<<<<<<< HEAD
- **Frontend embebido**: Sirve la SPA Vue.js desde `/static`
- **Autenticación**: HTTP Basic con credenciales desde variables de entorno
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

### Frontend
- **Vue 3** (Composition API) - Framework reactivo
- **Vuetify 3** - Framework de componentes Material Design
- **Pinia** - State management
- **Axios** - Cliente HTTP
- **Vite** - Build tool y dev server
- **PWA** (futuro) - Progressive Web App

### DevOps
<<<<<<< HEAD
- **Docker Compose** - Orquestación de servicios
- **Multi-stage Dockerfile** - Build frontend + backend en una imagen
=======
- **Docker Compose** - Orquestación de servicios (PostgreSQL, backend)
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
- **VSCode** - IDE principal
- **Python 3** - Scripts de mock ESP32

---

## 🏗️ Arquitectura del Sistema

### Flujo de Datos Principal

<<<<<<< HEAD
#### Desarrollo (frontend separado)
```
[Frontend Vite:5173] ←HTTP/CORS→ [Backend:8080] ←MQTT→ [ESP32]
                                       ↕
                                [PostgreSQL DB]
```

#### Producción (frontend embebido)
```
[Frontend embebido en Backend:8080] ←MQTT→ [ESP32]
                    ↕
            [PostgreSQL DB]
```

**Ventajas del enfoque embebido:**
- ✅ Un solo contenedor/artefacto JAR
- ✅ Sin problemas de CORS
- ✅ Ideal para móvil/PWA/WebView
- ✅ Despliegue simplificado (ej: Raspberry Pi)
- ✅ Menor latencia (sin saltos HTTP entre servicios)

**Desarrollo:** Frontend usa `npm run dev:mobile` con hot reload en puerto 5173

=======
```
[Frontend Vue] ←HTTP→ [Backend Spring Boot] ←MQTT→ [ESP32 Nodo]
                              ↕
                       [PostgreSQL DB]
```

>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
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

<<<<<<< HEAD
### Seguridad y Autenticación

#### HTTP Basic Authentication
El backend utiliza autenticación HTTP Basic para proteger todos los endpoints:

**Configuración:**
- Variables de entorno en `.env`:
  - `APP_SECURITY_USERNAME`: Usuario (default: `admin`)
  - `APP_SECURITY_PASSWORD`: Contraseña
- **Desarrollo local**: Usuario `admin` / Password `dev123`
- **Producción**: DEBE cambiar las credenciales en `.env`

**Implementación:**
- Clase: `SecurityConfig.java` (`ar.net.dac.iot.irrigacion.config`)
- Protege: Todos los endpoints `/api/**`, `/actuator/**`, y contenido estático
- Método: HTTP Basic con in-memory authentication
- CORS: Habilitado para desarrollo (frontend en puerto 5173)

**Archivo `.env`:**
- ✅ Excluido de Git (`.gitignore`)
- ✅ Template disponible en `.env.example`
- ⚠️ NO subir credenciales reales al repositorio

=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
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
<<<<<<< HEAD
- `V3__zona_config.sql`: Tabla zona_config + constraints hasta 8 zonas
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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

<<<<<<< HEAD
#### Tabla: `zona_config` ⭐ NUEVO
```sql
node_id         UUID NOT NULL
zona            SMALLINT NOT NULL (1..8)
nombre          VARCHAR(100)
habilitada      BOOLEAN DEFAULT TRUE
icono           VARCHAR(50)
orden           SMALLINT DEFAULT 0
created_at      TIMESTAMPTZ
updated_at      TIMESTAMPTZ
PRIMARY KEY     (node_id, zona)
```

=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
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
<<<<<<< HEAD
  - Estado actual de las zonas configuradas
  - Response: `ZoneStatusResponse[]`
  - Fuente: Caché en memoria actualizado por MQTT
  - Nombres desde `zona_config`

#### Configuración de Zonas ⭐ NUEVO
- `GET /api/nodos/{nodeId}/zonas?soloHabilitadas=false`
  - Lista configuración de zonas
  - Response: `ZoneConfigResponse[]`

- `GET /api/nodos/{nodeId}/zonas/{zona}`
  - Obtiene configuración de zona específica
  - Response: `ZoneConfigResponse`

- `POST /api/nodos/{nodeId}/zonas`
  - Crear o actualizar configuración (upsert)
  - Request: `ZoneConfigRequest`
  - Response: `ZoneConfigResponse`

- `PATCH /api/nodos/{nodeId}/zonas/{zona}/nombre`
  - Actualizar solo nombre
  - Request: `{"nombre": "string"}`
  - Response: `ZoneConfigResponse`

- `PATCH /api/nodos/{nodeId}/zonas/{zona}/toggle`
  - Habilitar/deshabilitar zona
  - Response: `ZoneConfigResponse`

- `DELETE /api/nodos/{nodeId}/zonas/{zona}`
  - Deshabilitar zona (soft delete)
  - Response: 204 No Content

- `PUT /api/nodos/{nodeId}/zonas/orden`
  - Reordenar zonas
  - Request: `{"zonas": [1, 3, 2, 4]}`
  - Response: `ZoneConfigResponse[]`
=======
  - Estado actual de las 4 zonas
  - Response: `ZoneStatusResponse[]`
  - Fuente: Caché en memoria actualizado por MQTT
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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
<<<<<<< HEAD
short zona          // 1..8
=======
short zona          // 1..4
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
String accion       // "ON" | "OFF"
Integer duracion    // Segundos 1..7200, requerido si ON
```

<<<<<<< HEAD
#### `ZoneConfigRequest` ⭐ NUEVO
```java
UUID nodeId         // Requerido
short zona          // 1..8
String nombre       // Requerido, max 100
boolean habilitada  // default true
String icono        // "garden", "lawn", "vegetables", "flowers", "water_drop", "sprinkler"
short orden         // 0..100, para ordenar en UI
```

#### `ZoneConfigResponse` ⭐ NUEVO
```java
// Extiende ZoneConfigRequest con:
OffsetDateTime createdAt
OffsetDateTime updatedAt
```

=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
### Frontend Services

#### `api.js` (Axios)
```javascript
<<<<<<< HEAD
// Agendas
getAgendas(nodeId)
crearAgenda(nodeId, agendaData)
eliminarAgenda(nodeId, agendaId)

// Estado y comandos
getZonesStatus(nodeId)
iniciarRiegoManual(nodeId, zona, duracion)
detenerRiego(nodeId, zona)

// Configuración de zonas ⭐ NUEVO
getZoneConfigs(nodeId, soloHabilitadas)
getZoneConfig(nodeId, zona)
upsertZoneConfig(nodeId, configData)
updateZoneNombre(nodeId, zona, n, `stores/zoneConfig.js`
- Listar agendas existentes con nombre auto-generado si vacío
- Crear/editar agendas con formulario modal
- Campo `nombre` opcional (genera "Zona X - HH:MM" si vacío)
- Toggle activa/inactiva
- Eliminar agendas
- Días de semana mostrados como chips (L, M, X, J, V, S, D)
- **Selector dinámico de zonas** desde configuración
=======
getAgendas(nodeId)
crearAgenda(nodeId, agendaData)
eliminarAgenda(nodeId, agendaId)
getZonesStatus(nodeId)
iniciarRiegoManual(nodeId, zona, duracion)
detenerRiego(nodeId, zona)
```

---
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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
<<<<<<< HEAD
Configuración de Zonas ⭐ NUEVO
- **Vista**: `ZoneConfigView.vue`
- **Store**: `stores/zoneConfig.js`
- Listar zonas configuradas (hasta 8)
- Editar nombre inline (blur o Enter para guardar)
- Habilitar/deshabilitar zonas con switch
- Agregar nuevas zonas con dialog
- Selección de iconos (water_drop, sprinkler, garden, lawn, vegetables, flowers)
- Orden configurable (futuro)
- Datos por defecto: 4 zonas "Zona 1-4" para nodo existente

### ✅ 
=======

>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
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

<<<<<<< HEAD
### ~~Configuración de Zonas~~ ✅ RESUELTO
- ~~**Número fijo**: Siempre 4 zonas (hardcoded)~~ → Ahora soporta hasta 8 zonas configurables
- ~~**Nombres genéricos**: Backend devuelve "Zona 1", "Zona 2", etc.~~ → Nombres personalizables
- ~~**No persistente**: No hay tabla de configuración de zonas~~ → Tabla zona_config implementada
- ~~**Fallback inconsistente**: Mock del store tiene nombres descriptivos que no se usan~~ → Usa configuración real
=======
### Configuración de Zonas
- **Número fijo**: Siempre 4 zonas (hardcoded)
- **Nombres genéricos**: Backend devuelve "Zona 1", "Zona 2", etc.
- **No persistente**: No hay tabla de configuración de zonas
- **Fallback inconsistente**: Mock del store tiene nombres descriptivos que no se usan
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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

<<<<<<< HEAD
#### Archivo `.env` (raíz del proyecto)
El proyecto utiliza un archivo `.env` para gestionar todas las credenciales de forma segura:

```bash
# PostgreSQL
POSTGRES_PASSWORD=postgres  # Cambiar en producción

# MQTT
APP_MQTT_ENABLED=true
APP_MQTT_HOST=mqtt
APP_MQTT_PORT=1883
APP_MQTT_TLS=false  # Habilitar en producción

# Autenticación HTTP Basic
APP_SECURITY_USERNAME=admin
APP_SECURITY_PASSWORD=dev123  # Cambiar en producción
```

⚠️ **Importante**: El archivo `.env` está excluido de Git. Usar `.env.example` como template.

#### Backend (`backend/src/main/resources/application.yml`)
```yaml
spring.datasource.url: jdbc:postgresql://postgres:5432/irrigacion
mqtt.host: mqtt
mqtt.port: 1883
mqtt.enabled: true
app.security.username: ${APP_SECURITY_USERNAME:admin}
app.security.password: ${APP_SECURITY_PASSWORD:cambiar_en_produccion}
```

#### Frontend (`.env` - desarrollo)
=======
#### Backend (`backend/src/main/resources/application.yml`)
```yaml
spring.datasource.url: jdbc:postgresql://postgres:5432/irrigacion
mqtt.host: mosquitto
mqtt.port: 1883
mqtt.enabled: true
```

#### Frontend (`.env`)
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
```
VITE_API_BASE_URL=http://localhost:8080/api
VITE_DEFAULT_NODE_ID=550e8400-e29b-41d4-a716-446655440000
```

<<<<<<< HEAD
#### Frontend (`.env.production` - embebido)
```
VITE_API_BASE_URL=/api
VITE_WS_URL=ws://localhost:8080/ws
VITE_DEFAULT_NODE_ID=550e8400-e29b-41d4-a716-446655440000
```

### Docker Compose
- **PostgreSQL**: Puerto 5432 (credenciales desde `.env`)
- **Backend**: Puerto 8080
  - Sirve frontend embebido en `/` y API en `/api`
  - Autenticación HTTP Basic habilitada
  - Variables de entorno inyectadas desde `.env`
- **MQTT Broker**: HiveMQ en puerto 1883
  - Sin autenticación en desarrollo
  - TLS + credenciales requeridas en producción
- **Frontend Dev** (solo desarrollo): `npm run dev:mobile` en puerto 5173

### Dockerfile Multi-Stage
```
Stage 1: Node.js Alpine - Build frontend → /frontend/dist
Stage 2: JDK 17 Alpine - Build backend + copiar frontend a /static
Stage 3: JRE 17 Alpine - Runtime con JAR unificado
```
=======
### Docker Compose
- **PostgreSQL**: Puerto 5432
- **Backend**: Puerto 8080, depende de postgres
- **Frontend**: Dev server Vite en puerto 5173
- **MQTT Broker**: HiveMQ en puerto 1883
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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
<<<<<<< HEAD
- Documentación clara
- Compatible con Spring Boot
- Soporte para TLS y autenticación

### ¿Por qué HTTP Basic en lugar de JWT?
**Decisión actual (MVP)**: HTTP Basic Authentication
- ✅ Simplicidad: No requiere gestión de tokens, refresh, o expiración
- ✅ Integrado en Spring Security sin dependencias adicionales
- ✅ Suficiente para MVP con un solo usuario
- ✅ Frontend embebido simplifica autenticación (sin CORS)
- ⚠️ **Futuro**: Migrar a JWT cuando se implemente multi-usuario

**Configuración actual:**
- Usuario/password desde variables de entorno
- In-memory authentication (no requiere BD)
- Protege todos los endpoints automáticamente

### ¿Por qué refactoring de packages?
**Package final**: `ar.net.dac.iot.irrigacion`
- Estructura profesional siguiendo convenciones Java
- Refleja dominio real (.net = organización, .dac = subdominio)
- Agrupa funcionalidad IoT de irrigación claramente
- Facilita escalabilidad futura (otros proyectos IoT bajo `ar.net.dac.iot`)
=======
- Mejor soporte de backpressure
- Documentación clara
- Compatible con Spring Boot
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

---

## 🗺️ Roadmap Futuro

<<<<<<< HEAD
### ✅ Completado Recientemente
- ✅ Refactoring de packages a `ar.net.dac.iot.irrigacion` (2025-12-15)
- ✅ Autenticación HTTP Basic con variables de entorno (2025-12-15)
- ✅ Configuración de zonas (nombres personalizados, habilitar/deshabilitar)
- ✅ Frontend embebido en backend (arquitectura unificada)

=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
### Features Planeadas (No Implementadas)
- [ ] WebSocket para actualizaciones en tiempo real
- [ ] Vista de histórico de riego (`GET /api/nodos/{nodeId}/eventos`)
- [ ] Vista de lecturas de humedad (`GET /api/nodos/{nodeId}/humedad`)
<<<<<<< HEAD
- [ ] Drag & drop para reordenar zonas en ZoneConfigView
=======
- [ ] Configuración de zonas (nombres personalizados, habilitar/deshabilitar)
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
- [ ] Gestión de múltiples nodos
- [ ] Mock ESP32 con simulación de tiempo real
- [ ] PWA (instalable en móvil)
- [ ] Notificaciones push
- [ ] Gráficos de consumo de agua
<<<<<<< HEAD
- [ ] Autenticación JWT y multi-usuario (migración desde HTTP Basic)

### Mejoras Técnicas
- ✅ Gestión segura de credenciales con `.env`
- ✅ Separación desarrollo/producción documentada
- ✅ `.gitignore` mejorado con patrones de seguridad
=======
- [ ] Autenticación y multi-usuario

### Mejoras Técnicas
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
- [ ] Tests de integración E2E
- [ ] CI/CD pipeline
- [ ] Métricas y observabilidad
- [ ] Rate limiting en API
- [ ] Validación de solapamiento de agendas por zona
- [ ] Retry logic en MQTT
- [ ] Manejo de desconexiones ESP32
<<<<<<< HEAD
- [ ] MQTT con TLS en producción
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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

<<<<<<< HEAD
## � Historial de Cambios Importantes

### 2025-12-15: Refactoring de Packages y Autenticación
**Cambios realizados:**
1. **Refactoring de packages completado**
   - Estructura final: `ar.net.dac.iot.irrigacion`
   - Migración desde estructura inicial
   - Actualizado `pom.xml` con `groupId` correcto
   - Imagen Docker regenerada exitosamente

2. **Autenticación HTTP Basic implementada**
   - Configuración en `SecurityConfig.java`
   - Credenciales desde variables de entorno (`.env`)
   - Usuario: `APP_SECURITY_USERNAME` (default: `admin`)
   - Password: `APP_SECURITY_PASSWORD`
   - Desarrollo: `admin:dev123`
   - Producción: DEBE cambiar credenciales

3. **Sistema completamente operativo**
   - ✅ Backend: http://localhost:8080 (UP)
   - ✅ PostgreSQL: Conectado y funcionando
   - ✅ MQTT: Broker activo en puerto 1883
   - ✅ API REST: Respondiendo con autenticación
   - ✅ 3 agendas activas (zonas 1, 2 y 5)

**Archivos modificados:**
- Todos los archivos `.java` con nuevo package
- `pom.xml`: groupId actualizado
- `Dockerfile`: Regenerado con nueva estructura
- `PROYECTO_CONTEXTO.md`: Actualizado con nueva información
- `backend-diseno.md`: Documentación de seguridad actualizada
- `backend/README.md`: Estructura de packages documentada
- `07-stack-tecnologico.md`: Stack actualizado

**Comandos ejecutados:**
```bash
docker-compose build --no-cache backend
docker-compose up -d
```

**Verificación:**
```powershell
# Health check
$cred = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes("admin:dev123"))
Invoke-RestMethod -Uri http://localhost:8080/actuator/health -Headers @{Authorization="Basic $cred"}
# Resultado: status = UP

# Test de API
Invoke-RestMethod -Uri http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas -Headers @{Authorization="Basic $cred"}
# Resultado: 3 agendas listadas correctamente
```

---

=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
## 🔄 Cómo Mantener Este Documento

### Cuándo actualizar:
1. **Cambios en arquitectura** (nuevos servicios, tablas, endpoints)
2. **Nuevas features completadas** (mover de roadmap a implementadas)
3. **Decisiones técnicas importantes** (cambio de librería, patrón, etc.)
4. **Limitaciones descubiertas** (bugs conocidos, restricciones)
5. **Cambios en configuración** (variables de entorno, puertos)
<<<<<<< HEAD
6. **Refactorings importantes** (packages, estructura de código)
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1

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
