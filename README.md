# Irrigación MQTT (Repo base)

Repo base para sistema de riego por zonas con ESP32 + MQTT (HiveMQ) + Backend Java + Frontend Vue.

## Estructura
- `docs/` documentación funcional/técnica (MVP)
- `backend/` Spring Boot 3.4.0 con frontend embebido
- `frontend/` Vue 3 SPA (embebido en producción, dev server en desarrollo)
- `esp32/` mock Python para simulación de nodo

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

## 🚀 Inicio Rápido

### Desarrollo (recomendado)
```bash
# 1. Copiar y configurar variables de entorno
cp .env.example .env

# 2. Levantar servicios (PostgreSQL + MQTT + Backend)
docker-compose up -d

# 3. Iniciar frontend con hot reload
cd frontend
npm install
npm run dev:mobile

# Acceder: http://localhost:5173
```

### Producción (frontend embebido)
```bash
# 1. Configurar .env
cp .env.example .env

# 2. Build y levantar todo (incluye frontend embebido)
docker-compose up -d --build backend

# Acceder: http://localhost:8080
```

## Autenticación

El backend utiliza **HTTP Basic Authentication**:
- Usuario y contraseña configurados en `.env`
- Variables: `APP_SECURITY_USERNAME` y `APP_SECURITY_PASSWORD`
- Desarrollo: `admin:dev123` (por defecto)
- ⚠️ **Producción**: DEBE cambiar las credenciales

Todos los endpoints requieren autenticación: `/api/**`, `/actuator/**`, y archivos estáticos.

## Requisitos
- **Docker Desktop** - Para servicios (PostgreSQL, MQTT, backend)
- **Node.js 18+** - Solo para desarrollo de frontend con hot reload
- **Java 17+** - Solo si compilas backend fuera de Docker
- **Python 3** - Solo para mock ESP32

## Arquitectura

### Desarrollo
```
Frontend (Vite:5173) ←CORS→ Backend (8080) ←MQTT→ ESP32
                              ↕
                        PostgreSQL (5432)
```

### Producción
```
Frontend embebido en Backend (8080) ←MQTT→ ESP32
              ↕
        PostgreSQL (5432)
```

## Documentación
- [PROYECTO_CONTEXTO.md](PROYECTO_CONTEXTO.md) - Estado completo del proyecto
- [docs/implementacion/frontend-embebido.md](docs/implementacion/frontend-embebido.md) - Arquitectura embebida
- [docs/implementacion/](docs/implementacion/) - Guías de implementación

## Notas de Seguridad
- MQTT debe ir **sobre TLS** en producción
- Frontend embebido elimina necesidad de CORS en producción
- Ver [docs/auditoria-seguridad.md](docs/auditoria-seguridad.md) para más detalles
