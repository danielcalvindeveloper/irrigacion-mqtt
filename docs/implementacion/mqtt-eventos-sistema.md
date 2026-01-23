# Eventos del Sistema MQTT - ESP8266

## Overview

El ESP8266 publica eventos automáticos vía MQTT para tracking completo de todas las operaciones del sistema de riego. Esto permite al backend monitorear el estado del dispositivo, detectar errores y mantener historial completo.

## Topics MQTT

### 1. Eventos de Riego (ya existente)
**Topic:** `riego/{nodeId}/evento`

Publicado automáticamente al inicio y fin de cada riego.

#### Evento: Inicio de Riego
```json
{
  "zona": 2,
  "evento": "inicio",
  "timestamp": 1735415280,
  "origen": "agenda",
  "duracionProgramada": 600,
  "versionAgenda": 7
}
```

**Campos:**
- `zona` (int): Número de zona (1-8)
- `evento` (string): Siempre "inicio"
- `timestamp` (int): Epoch time en segundos
- `origen` (string): "agenda" o "manual"
- `duracionProgramada` (int): Duración programada en segundos
- `versionAgenda` (int): Versión de la agenda (0 si manual)

#### Evento: Fin de Riego
```json
{
  "zona": 2,
  "evento": "fin",
  "timestamp": 1735415880,
  "origen": "manual",
  "duracionReal": 598,
  "versionAgenda": null
}
```

**Campos:**
- `zona` (int): Número de zona
- `evento` (string): Siempre "fin"
- `timestamp` (int): Epoch time en segundos
- `origen` (string): "agenda" o "manual"
- `duracionReal` (int): Duración real en segundos
- `versionAgenda` (int|null): Versión de la agenda o null si manual

---

### 2. Eventos del Sistema (NUEVO)
**Topic:** `riego/{nodeId}/sistema/evento`

Publicado automáticamente en operaciones críticas del sistema.

## Tipos de Eventos

### 2.1. Sincronización de Agendas Exitosa (MQTT)
```json
{
  "tipo": "agenda_sync_ok",
  "timestamp": 1735415280,
  "detalles": "Agenda sincronizada correctamente (2575 bytes)",
  "memoriaLibre": 42368
}
```

**Cuándo:** Al recibir mensaje MQTT en topic `riego/{nodeId}/agenda/sync` y guardarlo exitosamente en SPIFFS.

### 2.2. Carga Inicial Exitosa (HTTP)
```json
{
  "tipo": "agenda_initial_load_ok",
  "timestamp": 1735415280,
  "detalles": "Agendas cargadas desde backend HTTP (2575 bytes)",
  "memoriaLibre": 42368
}
```

**Cuándo:** Al conectarse al backend HTTP al inicio y descargar agendas exitosamente.

### 2.3. Agendas Parseadas Correctamente
```json
{
  "tipo": "agenda_sync_ok",
  "timestamp": 1735415280,
  "detalles": "Agendas cargadas: 9 total, 9 activas",
  "agendasCargadas": 9,
  "memoriaLibre": 38256
}
```

**Cuándo:** Después de parsear exitosamente el JSON de agendas en `AgendaManager::checkAndExecuteAgendas()`.

**Nota:** Solo se publica la primera vez que se parsea exitosamente (no en cada loop).

### 2.4. Error de Parseo de Agendas
```json
{
  "tipo": "agenda_parse_error",
  "timestamp": 1735415280,
  "detalles": "Error parseando agendas: NoMemory (JSON: 2575 bytes, Buffer: 4096 bytes)",
  "agendasCargadas": 0,
  "memoriaLibre": 18240
}
```

**Cuándo:** 
- JSON mal formado
- Buffer insuficiente (NoMemory)
- Error de deserialización de ArduinoJson

### 2.5. Error de Formato de Agendas
```json
{
  "tipo": "agenda_format_error",
  "timestamp": 1735415280,
  "detalles": "JSON no contiene campo 'agendas'",
  "agendasCargadas": 0,
  "memoriaLibre": 40128
}
```

**Cuándo:** JSON válido pero sin el campo `agendas` esperado.

### 2.6. Error de Almacenamiento (SPIFFS)
```json
{
  "tipo": "agenda_storage_error",
  "timestamp": 1735415280,
  "detalles": "Error al guardar agenda en SPIFFS (2575 bytes, 1024 bytes libres)",
  "agendasCargadas": 0,
  "memoriaLibre": 41216
}
```

**Cuándo:**
- SPIFFS no inicializado
- Sin espacio disponible en SPIFFS
- Error de escritura de archivo

### 2.7. Error de Carga Inicial Crítico
```json
{
  "tipo": "agenda_load_error",
  "timestamp": 1735415280,
  "detalles": "Sin agendas: backend no disponible y sin cache local",
  "agendasCargadas": 0,
  "memoriaLibre": 43520
}
```

**Cuándo:** 
- Backend HTTP no responde
- No hay agendas almacenadas localmente
- Sistema arranca sin agendas programadas

### 2.8. Advertencia de Carga
```json
{
  "tipo": "agenda_fetch_warning",
  "timestamp": 1735415280,
  "detalles": "Backend no disponible - usando agendas locales",
  "memoriaLibre": 42880
}
```

**Cuándo:**
- WiFi no conectado al intentar fetch
- Backend HTTP no responde pero hay cache local

## Campos Comunes

Todos los eventos del sistema incluyen:

| Campo | Tipo | Descripción |
|-------|------|-------------|
| `tipo` | string | Tipo de evento (ver tabla abajo) |
| `timestamp` | int | Epoch time en segundos (millis/1000) |
| `detalles` | string | Descripción legible del evento |
| `memoriaLibre` | int | Bytes de RAM libre (ESP.getFreeHeap()) |
| `agendasCargadas` | int | Número de agendas (opcional, -1 si N/A) |

## Tabla de Tipos de Eventos

| Tipo | Severidad | Descripción |
|------|-----------|-------------|
| `agenda_sync_ok` | INFO | Agenda sincronizada vía MQTT exitosamente |
| `agenda_initial_load_ok` | INFO | Agendas cargadas desde HTTP al inicio |
| `agenda_parse_error` | ERROR | Error al parsear JSON de agendas |
| `agenda_format_error` | ERROR | JSON sin formato esperado |
| `agenda_storage_error` | ERROR | Error al guardar en SPIFFS |
| `agenda_load_error` | CRITICAL | Sistema sin agendas (cache y backend fallan) |
| `agenda_fetch_warning` | WARNING | Backend no disponible, usando cache |

## Flujos de Eventos

### Flujo Normal: Inicio Exitoso
```
1. [INFO] agenda_initial_load_ok  → Descarga HTTP exitosa
2. [INFO] agenda_sync_ok          → Guardado en SPIFFS exitoso
3. [INFO] agenda_sync_ok          → Parseo exitoso (N agendas cargadas)
```

### Flujo Error: Sin Backend, Con Cache
```
1. [WARNING] agenda_fetch_warning → Backend no responde
2. [INFO] agenda_sync_ok          → Parseo exitoso de cache local
```

### Flujo Error: Sin Backend, Sin Cache
```
1. [CRITICAL] agenda_load_error   → Sin agendas disponibles
```

### Flujo Error: Problema de Memoria
```
1. [INFO] agenda_sync_ok          → Guardado en SPIFFS OK
2. [ERROR] agenda_parse_error     → NoMemory al parsear
```

## Recomendaciones para el Backend

### Suscribirse a Eventos del Sistema
```java
@Service
public class MqttSistemaSubscriber {
    
    @PostConstruct
    public void init() {
        // Suscribirse a eventos de todos los nodos
        mqttClient.subscribe("riego/+/sistema/evento", this::procesarEvento);
    }
    
    private void procesarEvento(String topic, String payload) {
        // Extraer nodeId del topic
        String nodeId = extractNodeId(topic);
        
        // Parsear JSON
        JsonNode evento = objectMapper.readTree(payload);
        String tipo = evento.get("tipo").asText();
        
        // Logging según severidad
        switch (tipo) {
            case "agenda_parse_error":
            case "agenda_storage_error":
            case "agenda_load_error":
                logger.error("Error en nodo {}: {}", nodeId, evento.get("detalles"));
                // Enviar alerta
                break;
                
            case "agenda_fetch_warning":
                logger.warn("Advertencia nodo {}: {}", nodeId, evento.get("detalles"));
                break;
                
            default:
                logger.info("Evento nodo {}: {}", nodeId, evento.get("detalles"));
        }
        
        // Persistir en BD para analytics
        eventoRepository.save(new EventoSistema(nodeId, tipo, evento));
    }
}
```

### Monitoreo de Salud del Dispositivo

Detectar problemas:
- **Memoria crítica**: `memoriaLibre < 20000` → Alerta
- **Errores repetidos**: 3+ `agenda_parse_error` → Revisar configuración
- **Sin agendas**: `agenda_load_error` → Verificar conectividad

## Integración con Historial Existente

Los eventos de riego (`inicio`/`fin`) ya se procesan en:
- `RiegoEventoService.procesarEvento()` (backend)
- `MqttEventoSubscriber` (backend)

Los nuevos eventos del sistema deberían tener su propio subscriber:
- `MqttSistemaSubscriber` (nuevo)
- `SistemaEventoService` (nuevo)

## Testing

### Probar Eventos Manualmente

```bash
# Ver eventos de sistema
mosquitto_sub -h localhost -p 1883 -t "riego/+/sistema/evento" -v

# Ver eventos de riego
mosquitto_sub -h localhost -p 1883 -t "riego/+/evento" -v

# Simular sincronización de agenda
mosquitto_pub -h localhost -p 1883 \
  -t "riego/550e8400-e29b-41d4-a716-446655440000/agenda/sync" \
  -m '{"agendas": [...]}'
```

### Verificar en Logs del ESP8266

```
[INFO] Publicado evento sistema: agenda_sync_ok - Agenda sincronizada correctamente (2575 bytes)
[INFO] Publicado evento sistema: agenda_sync_ok - Agendas cargadas: 9 total, 9 activas
```

## Changelog

**2026-01-23:**
- ✅ Implementado sistema completo de eventos del sistema
- ✅ Agregado tracking de sincronización de agendas (OK/Error)
- ✅ Agregado tracking de carga inicial (HTTP/Local)
- ✅ Agregado diagnóstico de memoria en eventos
- ✅ Documentación completa de eventos y flujos
