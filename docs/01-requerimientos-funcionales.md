# Requerimientos Funcionales

## RF-1 Gestión de agenda por zona
- El sistema debe permitir **crear, modificar y eliminar agendas de riego por zona**.
- Las operaciones deben poder realizarse **desde móvil o PC**.

## RF-1.1 Visualización de agenda activa
- Desde móvil o PC se debe visualizar **claramente la agenda activa** por cada zona.
- Debe indicarse:
  - Zona
  - Próxima activación
  - Duración configurada
  - Estado actual (ON / OFF)

## RF-1.2 Consulta histórica
- Desde móvil o PC se debe poder **consultar la actividad de riego de un período determinado**.
- El período debe ser configurable (fecha desde / hasta).
- La información mínima:
  - Zona
  - Fecha y hora
  - Duración
  - Motivo (agenda / manual / humedad)

## RF-2 Definición de agenda por zona
Cada agenda por zona debe contemplar:
- Días de la semana
- Horarios de inicio
- Tiempo de activación (en minutos)

## RF-3 Operación sin conexión a Internet
- Ante pérdida de conectividad:
  - El sistema de riego debe **mantener activa la última agenda conocida** por zona.
  - La ejecución debe ser **autónoma en el ESP32**.
- Al recuperar conectividad:
  - Debe sincronizar estado y actividad con el backend.
## RF-4 Eventos y Tracking del Sistema (2026-01-23)

### RF-4.1 Eventos de Riego
**Historia de Usuario:** Como administrador del sistema, quiero recibir eventos automáticos de cada riego para auditar todas las activaciones y su origen.

- El ESP8266 debe publicar eventos MQTT al **inicio de cada riego**
- El ESP8266 debe publicar eventos MQTT al **fin de cada riego**
- Cada evento debe identificar:
  - Zona afectada
  - Timestamp (fecha/hora)
  - **Origen**: "agenda" (programado) o "manual" (comando directo)
  - Duración programada (inicio) o duración real (fin)
  - Versión de agenda (si es programado)

**Topic MQTT:** `riego/{nodeId}/evento`

**Criterios de Aceptación:**
- ✅ Evento "inicio" se publica inmediatamente al activar zona
- ✅ Evento "fin" se publica al desactivar zona (manual o automático)
- ✅ Backend puede distinguir riegos manuales vs programados
- ✅ Permite auditoría completa del historial de riego

### RF-4.2 Eventos de Sincronización de Agendas
**Historia de Usuario:** Como operador del sistema, quiero ser notificado cuando las agendas se sincronizan correctamente o fallan, para detectar problemas de configuración.

- El ESP8266 debe publicar evento cuando **recibe sincronización MQTT exitosa**
- El ESP8266 debe publicar evento cuando **carga agendas desde HTTP exitosamente**
- El ESP8266 debe publicar evento cuando **parsea agendas correctamente**
- Cada evento debe incluir:
  - Tipo de evento
  - Detalles descriptivos
  - Número de agendas cargadas
  - Memoria RAM libre (diagnóstico)

**Topic MQTT:** `riego/{nodeId}/sistema/evento`

**Tipos de Eventos (Exitosos):**
- `agenda_sync_ok` - Sincronización MQTT recibida y guardada
- `agenda_initial_load_ok` - Carga HTTP inicial exitosa
- `agenda_sync_ok` (parseo) - N agendas parseadas correctamente

**Criterios de Aceptación:**
- ✅ Evento publicado en cada operación de sincronización
- ✅ Backend puede confirmar que dispositivo recibió cambios
- ✅ Dashboard muestra última sincronización exitosa por nodo

### RF-4.3 Eventos de Errores de Agenda
**Historia de Usuario:** Como administrador del sistema, quiero ser alertado inmediatamente cuando hay errores en la sincronización o parseo de agendas, para tomar acción correctiva.

- El ESP8266 debe publicar evento cuando **falla el parseo de JSON** (NoMemory, formato inválido)
- El ESP8266 debe publicar evento cuando **JSON no tiene formato esperado** (falta campo 'agendas')
- El ESP8266 debe publicar evento cuando **falla guardar en SPIFFS** (sin espacio, error I/O)
- Cada evento debe incluir:
  - Tipo de error
  - Mensaje descriptivo con causa
  - Tamaño de JSON (si aplica)
  - Buffer asignado (si aplica)
  - Memoria RAM libre

**Tipos de Eventos (Errores):**
- `agenda_parse_error` - Error parseando JSON (NoMemory, etc)
- `agenda_format_error` - JSON válido pero sin estructura esperada
- `agenda_storage_error` - Error guardando en SPIFFS

**Criterios de Aceptación:**
- ✅ Error publicado inmediatamente al detectarse
- ✅ Mensaje incluye información suficiente para debugging
- ✅ Backend puede generar alertas automáticas
- ✅ Permite identificar problemas de configuración o memoria

### RF-4.4 Eventos de Carga Inicial Fallida
**Historia de Usuario:** Como operador del sistema, quiero ser notificado si el dispositivo arranca sin poder cargar agendas, para saber que está operando en modo degradado.

- El ESP8266 debe publicar evento **crítico** cuando:
  - Backend HTTP no responde Y no hay cache local
  - Sistema inicia sin agendas programadas
- El ESP8266 debe publicar evento de **advertencia** cuando:
  - Backend HTTP no responde PERO hay cache local válido
  - WiFi no disponible al intentar fetch

**Tipos de Eventos (Críticos/Advertencias):**
- `agenda_load_error` (CRITICAL) - Sin agendas disponibles
- `agenda_fetch_warning` (WARNING) - Backend down, usando cache

**Criterios de Aceptación:**
- ✅ Evento publicado durante secuencia de boot
- ✅ Backend distingue entre error crítico y advertencia
- ✅ Dashboard muestra estado de salud del dispositivo
- ✅ Alertas críticas enviadas a administradores

### RF-4.5 Diagnóstico de Memoria
**Historia de Usuario:** Como desarrollador, quiero monitorear el uso de memoria RAM del ESP8266 en tiempo real para prevenir crashes por falta de memoria.

- Todos los eventos del sistema deben incluir campo `memoriaLibre`
- Backend debe almacenar histórico de memoria libre
- Dashboard debe mostrar gráfico de memoria disponible vs tiempo

**Criterios de Aceptación:**
- ✅ Memoria libre incluida en todos los eventos
- ✅ Backend detecta memoria crítica (<20KB)
- ✅ Alertas automáticas cuando memoria < 20KB
- ✅ Gráfico de tendencia de memoria disponible