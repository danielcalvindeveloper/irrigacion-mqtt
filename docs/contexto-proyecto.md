
# Documento de Contexto - Sistema de Riego MQTT

Este documento debe leerse al inicio de cada sesión de trabajo. Resume el estado actual del proyecto, decisiones clave, arquitectura y lineamientos para mantener la compatibilidad y coherencia.

## Estado Actual (27/12/2025)

### Firmware (NodeMCU ESP8266)
- Migrado desde ESP32, respetando toda la lógica y diagramas existentes.
- 4 zonas de riego (relés en D1, D2, D5, D6).
- 1 sensor de humedad en A0 (expansión futura requiere multiplexor externo).
- MQTT_BROKER configurado con la IP local del host Docker.
- Inicialización de hardware y logs por Serial.
- Módulos pendientes: WiFiManager, MqttManager, AgendaManager, SPIFFSManager, TimeSync, integración LCD.

### Backend (Java Spring Boot)
- Java 17+, Spring Boot 3.4, PostgreSQL, Flyway, HiveMQ MQTT Client.
- Lógica de negocio centralizada: gestión de agendas, eventos de riego, telemetría, versionado, retención de datos.
- Expone API REST autenticada (HTTP Basic) y WebSocket push.
- Publica y consume mensajes MQTT según contratos documentados.
- Mantiene compatibilidad con los diagramas y contratos definidos en docs/implementacion/ y docs/05-mqtt-topics-y-mensajes.md.

### MQTT
- Broker local (HiveMQ CE, puerto 1883).
- Topics y payloads definidos en docs/05-mqtt-topics-y-mensajes.md.
- Flujos: comando manual, feedback de estado, sincronización de agenda, telemetría.
- Toda comunicación es saliente desde el ESP8266.

### Frontend (Vue 3 + Vuetify)
- Arquitectura mobile-first, diseño y requerimientos en docs/frontend-diseno-mobile-first.md y frontend-resumen-estado-actual.md.
- No implementado aún, pero definido: CRUD de agendas, visualización de estado, integración con backend vía HTTP.
- Servido embebido en backend en producción.

### Documentación y Diagramas
- Todos los diagramas, contratos y especificaciones en docs/ y docs/implementacion/ deben respetarse y mantenerse actualizados.
- Cualquier cambio mayor debe reflejarse en los diagramas y documentación correspondiente.

## Lineamientos
- Mantener compatibilidad con todo el código, hardware, diagramas y documentación existente.
- Toda modificación relevante debe registrarse en la bitácora (docs/bitacora-cambios.md).
- Revisar este contexto y la bitácora antes de cada sesión.
- Documentar nuevas decisiones y cambios aquí y en los diagramas/contratos si corresponde.

## Manejo de la Bitácora
- La bitácora de cambios se encuentra en docs/bitacora-cambios.md.
- Debe actualizarse en cada sesión de trabajo.
- Es la referencia principal para el historial del proyecto.
