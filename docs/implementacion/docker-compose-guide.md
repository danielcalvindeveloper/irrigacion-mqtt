# Guía Docker Compose - Stack completo

## Configuración inicial

⚠️ **IMPORTANTE:** Antes de ejecutar Docker Compose, debes configurar las variables de entorno.

### 1. Copiar archivo de configuración

Desde la raíz del proyecto (`irrigacion-mqtt-repo/`):

```powershell
cp .env.example .env
```

### 2. Editar credenciales (opcional para desarrollo)

El archivo `.env` contiene las credenciales que usará Docker Compose.

**Para desarrollo local:** Puedes usar los valores por defecto

**Para producción:** DEBES cambiar:
- `POSTGRES_PASSWORD` - Contraseña segura (mínimo 16 caracteres)
- `APP_MQTT_USERNAME` y `APP_MQTT_PASSWORD` - Credenciales MQTT
- `APP_MQTT_TLS=true` - Habilitar TLS

### 3. Verificar configuración

Puedes ver las variables que Docker Compose usará:

```powershell
docker-compose config
```

## Primera ejecución (build completo)

```powershell
docker-compose build --no-cache backend
docker-compose up -d
```

Esto levanta:
- **Postgres** en `localhost:5432` (credenciales desde `.env`)
- **HiveMQ MQTT** en `localhost:1883` (sin auth en desarrollo), Web UI en `http://localhost:8000`
- **Backend Spring Boot** en `http://localhost:8080`

El backend espera a que Postgres esté healthy y MQTT esté arrancado, luego ejecuta Flyway y se conecta al broker.

💡 **Nota de seguridad:** El archivo `.env` con credenciales reales NO se sube a Git (está en `.gitignore`)

## Ejecuciones posteriores

Si ya tienes las imágenes construidas:

```powershell
docker-compose up -d
```

## Ver logs

```powershell
# Todos los servicios
docker-compose logs -f

# Solo backend
docker-compose logs -f backend

# Solo MQTT
docker-compose logs -f mqtt
```

## Verificar estado

```powershell
docker-compose ps
```

## Detener sin borrar datos

```powershell
docker-compose stop
```

## Reiniciar

```powershell
docker-compose restart
```

## Detener y eliminar contenedores (mantiene volumen DB)

```powershell
docker-compose down
```

## Detener y eliminar TODO (incluyendo datos de Postgres)

```powershell
docker-compose down -v
```

## Reconstruir backend tras cambios de código

```powershell
docker-compose down
docker-compose build --no-cache backend
docker-compose up -d
```

**Importante:** Usa `--no-cache` para asegurar que Maven recompile todo. Los datos de Postgres persisten en el volumen `postgres-data`.

## Probar endpoints

Una vez levantado:

**Health check:**
```
http://localhost:8080/actuator/health
```

**Listar agendas:**
```
http://localhost:8080/api/nodos/{UUID}/agendas
```

**Ver tópicos MQTT (con MQTT Explorer):**
- Host: `localhost`
- Port: `1883`

O con mosquitto_sub:
```powershell
mosquitto_sub -h localhost -p 1883 -t "riego/#" -v
```

## Persistencia de datos

Los datos de las agendas persisten en el volumen Docker `postgres-data`. Puedes:
- Detener y reiniciar: `docker-compose stop` / `docker-compose start`
- Bajar y subir: `docker-compose down` / `docker-compose up -d`

**Los datos se mantienen** siempre que no uses el flag `-v` en `docker-compose down`.

Para **borrar todos los datos** (reset completo):
```powershell
docker-compose down -v
docker volume rm irrigacion-mqtt-repo_postgres-data
```

## Troubleshooting

**Backend no arranca con "no main manifest attribute":**
- Verifica que `pom.xml` tenga el plugin `spring-boot-maven-plugin` con `<executions><goal>repackage</goal></executions>`
- Reconstruye con `--no-cache`

**Backend en loop "Restarting":** 
- Revisa logs: `docker logs irrigacion-backend --tail 50`
- Si falla conexión a Postgres, verifica que esté `healthy`: `docker-compose ps`
- Si falla MQTT, espera ~10 segundos más (HiveMQ tarda ~5s en arrancar)

**No se conecta a Postgres:** 
- Asegúrate de que el servicio `postgres` tenga status `healthy` con `docker-compose ps`
- Si persiste, recrea el volumen: `docker-compose down -v` y vuelve a `up -d`

**MQTT no conecta:** 
- Verifica que el puerto 1883 no esté ocupado: `netstat -ano | findstr 1883`
- Revisa logs de HiveMQ: `docker logs irrigacion-mqtt`
