# Tests de integración (backend + MQTT)

## Objetivo
Validar flujos clave usando broker MQTT en contenedor (Testcontainers con HiveMQ/Mosquitto).

## Infra de prueba
- Dependencia Testcontainers (JUnit 5).
- Broker MQTT expuesto (1883/8883 opcional si TLS en pruebas).
- Perfiles `test` con DB efímera (H2/Postgres Testcontainer).

## Casos
1) Sync de agenda
- Setup: insertar agenda en DB y publicar sync.
- Verificar: mensaje MQTT `riego/{nodeId}/agenda/sync` con version++ y payload esperado.

2) Comando manual
- Llamar `POST /api/nodos/{nodeId}/cmd` con ON 120s.
- Verificar publish en topic `cmd` y registro de intento.

3) Ingreso de status/evento
- Publicar en `riego/{nodeId}/status/zona/1` payload ON/manual.
- Verificar persistencia en `riego_evento` y broadcast por WebSocket (mock Stomp client o canal interno).

4) Telemetría humedad
- Publicar en `riego/{nodeId}/telemetria` humedad/rssi/uptime.
- Verificar inserción en `humedad` y validaciones de rango.

5) Versionado
- Crear agenda versión n, enviar sync n+1, confirmar incremento en DB y que n-1 sea ignorado.

6) Solape
- Intentar crear dos agendas solapadas misma zona → API responde 400 y no publica sync.

## Herramientas sugeridas
- Testcontainers: HiveMQ/Mosquitto, Postgres.
- Awaitility para esperar publicaciones.
- WireMock opcional si se mockea servicios externos.

## Ejemplo de setup (pseudo)
```java
class MqttIntegrationTest {
  static HiveMQContainer mqtt = new HiveMQContainer();
  static PostgreSQLContainer<?> db = new PostgreSQLContainer<>("postgres:16");

  @BeforeAll
  static void init() {
    mqtt.start();
    db.start();
    // Override spring props: mqtt.host/port, datasource URL
  }
}
```
