# Guía rápida: probar agendas y /cmd con Postman

## 1) Prerrequisitos
- Docker Desktop funcionando
- Java 17 y Maven instalados
- Postman (o equivalente)
- Puerto 8080 libre

## 2) Levantar infraestructura mínima
1. **PostgreSQL 15**
   ```powershell
   docker run -d --name irrigacion-db ^
     -e POSTGRES_DB=irrigacion ^
     -e POSTGRES_PASSWORD=postgres ^
     -p 5432:5432 postgres:15
   ```
2. **(Opcional) Broker MQTT HiveMQ CE** para probar /cmd end-to-end.
   ```powershell
   docker run -d --name irrigacion-mqtt -p 1883:1883 hivemq/hivemq-ce:2023.5
   ```

## 3) Configurar variables de entorno para el backend
En una consola nueva (PowerShell en Windows):
```powershell
# Base de datos
$env:SPRING_DATASOURCE_URL = "jdbc:postgresql://localhost:5432/irrigacion"
$env:SPRING_DATASOURCE_USERNAME = "postgres"
$env:SPRING_DATASOURCE_PASSWORD = "postgres"

# MQTT (si levantaste el broker)
$env:APP_MQTT_ENABLED = "true"        # ponlo en false si no hay broker
$env:APP_MQTT_HOST = "localhost"
$env:APP_MQTT_PORT = "1883"
$env:APP_MQTT_TLS = "false"
```

## 4) Ejecutar la API
En el folder `backend/`:
```powershell
mvn spring-boot:run
```
El server queda en `http://localhost:8080`. Flyway crea las tablas en el arranque.

## 5) Colección Postman (paso a paso)
Usa un `environment` con variable `base_url = http://localhost:8080` y un `nodeId` UUID consistente entre requests (ejemplo: `6f2f0f5c-4c57-4c3d-a5fb-8d6a6d8e8b01`).

### 5.1 Crear/actualizar agenda
- Método: POST
- URL: {{base_url}}/api/nodos/{{nodeId}}/agendas
- Body (JSON):
```json
{
  "id": "11111111-2222-3333-4444-555555555555",
  "nodeId": "{{nodeId}}",
  "zona": 1,
  "diasSemana": ["LUN", "MIE", "VIE"],
  "horaInicio": "06:30",
  "duracionMin": 20,
  "activa": true
}
```
Respuestas esperadas: 200 con la agenda y versión incrementada.

### 5.2 Listar agendas
- Método: GET
- URL: {{base_url}}/api/nodos/{{nodeId}}/agendas
- Debe devolver la lista con la agenda creada.

### 5.3 Eliminar agenda
- Método: DELETE
- URL: {{base_url}}/api/nodos/{{nodeId}}/agendas/11111111-2222-3333-4444-555555555555
- Respuesta esperada: 204 sin cuerpo.

### 5.4 Enviar comando manual (/cmd)
Requiere que el broker MQTT esté activo y `APP_MQTT_ENABLED=true`.
- Método: POST
- URL: {{base_url}}/api/nodos/{{nodeId}}/cmd
- Body (JSON) ON:
```json
{
  "nodeId": "{{nodeId}}",
  "zona": 1,
  "accion": "ON",
  "duracion": 60
}
```
- Body (JSON) OFF:
```json
{
  "nodeId": "{{nodeId}}",
  "zona": 1,
  "accion": "OFF"
}
```
Respuestas: 202 si se publica al broker; 503 si MQTT está deshabilitado/no disponible. El topic publicado es `riego/{nodeId}/cmd/zona/{zona}` con payload JSON.

## 6) Limpieza
```powershell
docker stop irrigacion-db irrigacion-mqtt
docker rm irrigacion-db irrigacion-mqtt
```

## Notas
- El `nodeId` del path debe coincidir con el del payload; de lo contrario devuelve 400.
- Validaciones: `zona` 1-4, `duracion` (ON) 1-7200 segundos, `horaInicio` HH:mm, `diasSemana` en [LUN..DOM].
- Si solo quieres probar CRUD sin MQTT, deja `APP_MQTT_ENABLED=false`; /cmd devolverá 503 (esperado).
