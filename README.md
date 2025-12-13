# Irrigación MQTT (Repo base)

Repo base para sistema de riego por zonas con ESP32 + MQTT (HiveMQ) + Backend Java + Frontend Vue.

## Estructura
- `docs/` documentación funcional/técnica (MVP)
- `backend/` Spring Boot (esqueleto)
- `frontend/` Vue 3 (esqueleto)
- `esp32/` notas y placeholders de firmware

## ⚠️ Seguridad

### Desarrollo vs Producción
- Las credenciales en `.env` son **SOLO para desarrollo local**
- NUNCA uses estas configuraciones en producción sin cambiarlas
- PostgreSQL: Cambiar `POSTGRES_PASSWORD` en producción
- MQTT: Habilitar TLS y autenticación en producción

### Configuración Inicial

1. **Copiar archivo de variables de entorno:**
   ```bash
   cp .env.example .env
   ```

2. **Editar `.env` con tus credenciales:**
   - Para desarrollo local, puedes usar los valores por defecto
   - Para producción, usa contraseñas fuertes y únicas

3. **El archivo `.env` NO se sube a Git** (está en `.gitignore`)

### Configuración de Producción
- Usa variables de entorno para credenciales
- Habilita TLS en MQTT (puerto 8883)
- Usa contraseñas fuertes (mínimo 16 caracteres)
- Mantén los archivos de configuración de Cloudflare fuera del repo
- Considera usar gestores de secretos (AWS Secrets Manager, Azure Key Vault, etc.)

Para más detalles, consulta [docs/auditoria-seguridad.md](docs/auditoria-seguridad.md)

## Primeros pasos (recomendado)
1. Copiar y configurar el archivo `.env` (ver arriba)
2. Configurar HiveMQ Cloud (cluster + credenciales)
3. Probar publish/subscribe con el **Web Client** de HiveMQ
4. Implementar conexión MQTT en ESP32 y probar comandos manuales
5. Recién después: backend + UI

## Requisitos
- Docker Desktop (para desarrollo local)
- Java 17+ (para backend)
- Node 18+ (para frontend)
- VSCode + GitHub Copilot (Agent) + GPT-5.1 Codex

## Notas
- MQTT debe ir **sobre TLS** en producción.
- La UI **no** debe conectarse directo al broker. Va contra el backend (REST/WebSocket).
