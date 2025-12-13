# Docker Compose para Producción - Ejemplo

⚠️ **ESTE ES UN ARCHIVO DE EJEMPLO - NO USAR DIRECTAMENTE EN PRODUCCIÓN**

Este archivo muestra cómo configurar Docker Compose para un entorno de producción utilizando variables de entorno y mejores prácticas de seguridad.

## Uso

1. Copiar este archivo:
   ```bash
   cp docker-compose.prod.example.yml docker-compose.prod.yml
   ```

2. Configurar variables de entorno en `.env`

3. Ejecutar:
   ```bash
   docker-compose -f docker-compose.prod.yml up -d
   ```

---

```yaml
version: '3.8'

services:
  postgres:
    image: postgres:15
    container_name: irrigacion-postgres
    environment:
      # 🔐 Credenciales desde variables de entorno
      POSTGRES_DB: ${POSTGRES_DB}
      POSTGRES_USER: ${POSTGRES_USER}
      POSTGRES_PASSWORD: ${POSTGRES_PASSWORD}
      TZ: ${TZ}
    ports:
      # ⚠️ En producción, considerar NO exponer el puerto o usar firewall
      - "5432:5432"
    volumes:
      - postgres-data:/var/lib/postgresql/data
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U ${POSTGRES_USER}"]
      interval: 5s
      timeout: 3s
      retries: 5
    restart: unless-stopped
    # 🔐 Seguridad adicional en producción
    security_opt:
      - no-new-privileges:true
    # Límites de recursos
    deploy:
      resources:
        limits:
          cpus: '1'
          memory: 512M
        reservations:
          cpus: '0.5'
          memory: 256M

  mqtt:
    image: hivemq/hivemq-ce:2023.5
    container_name: irrigacion-mqtt
    ports:
      # ⚠️ En producción: usar puerto 8883 (TLS) en lugar de 1883
      - "1883:1883"
      # ⚠️ NO exponer Web UI (8000) en producción o proteger con auth
      # - "8000:8000"
    restart: unless-stopped
    security_opt:
      - no-new-privileges:true
    deploy:
      resources:
        limits:
          cpus: '1'
          memory: 512M

  backend:
    # 🔐 En producción: usar imagen pre-construida en lugar de build
    image: ${DOCKERHUB_USERNAME}/irrigacion-backend:latest
    # build:
    #   context: ./backend
    #   dockerfile: Dockerfile
    container_name: irrigacion-backend
    environment:
      # 🔐 Base de datos - credenciales desde variables de entorno
      SPRING_DATASOURCE_URL: ${SPRING_DATASOURCE_URL}
      SPRING_DATASOURCE_USERNAME: ${SPRING_DATASOURCE_USERNAME}
      SPRING_DATASOURCE_PASSWORD: ${SPRING_DATASOURCE_PASSWORD}
      
      # 🔐 MQTT - Con TLS y autenticación en producción
      APP_MQTT_ENABLED: ${APP_MQTT_ENABLED}
      APP_MQTT_HOST: ${APP_MQTT_HOST}
      APP_MQTT_PORT: ${APP_MQTT_PORT}
      APP_MQTT_USERNAME: ${APP_MQTT_USERNAME}
      APP_MQTT_PASSWORD: ${APP_MQTT_PASSWORD}
      APP_MQTT_TLS: ${APP_MQTT_TLS}
      APP_MQTT_CLIENT_ID: ${APP_MQTT_CLIENT_ID}
      
      # Configuración adicional de Spring
      SPRING_PROFILES_ACTIVE: prod
      JAVA_OPTS: "-Xms256m -Xmx512m"
    ports:
      # ⚠️ En producción: usar reverse proxy (nginx/traefik) en lugar de exponer directamente
      - "8080:8080"
    depends_on:
      postgres:
        condition: service_healthy
      mqtt:
        condition: service_started
    restart: unless-stopped
    security_opt:
      - no-new-privileges:true
    deploy:
      resources:
        limits:
          cpus: '2'
          memory: 1G
        reservations:
          cpus: '0.5'
          memory: 512M
    # 🔐 Health check para el backend
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/actuator/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 60s

volumes:
  postgres-data:
    # 🔐 En producción: considerar backup automático de este volumen

# 🔐 Red personalizada (opcional pero recomendado)
networks:
  default:
    driver: bridge
```

---

## ⚠️ Checklist de Seguridad para Producción

Antes de desplegar, verificar:

### Credenciales
- [ ] `POSTGRES_PASSWORD` cambiada (mínimo 16 caracteres)
- [ ] `APP_MQTT_USERNAME` y `APP_MQTT_PASSWORD` configurados
- [ ] `APP_MQTT_TLS=true` habilitado
- [ ] Archivo `.env` con credenciales reales NO está en Git

### Red y Puertos
- [ ] PostgreSQL NO expuesto públicamente (o con firewall estricto)
- [ ] Web UI de MQTT deshabilitada o protegida
- [ ] Backend detrás de reverse proxy con HTTPS
- [ ] Certificados SSL válidos configurados

### Recursos
- [ ] Límites de CPU y memoria configurados
- [ ] Health checks habilitados
- [ ] Logs rotando correctamente
- [ ] Monitoreo configurado (Prometheus, Grafana, etc.)

### Backup y Recuperación
- [ ] Backup automático del volumen de PostgreSQL
- [ ] Plan de recuperación ante desastres documentado
- [ ] Backups probados (restaurar periódicamente)

### Actualizaciones
- [ ] Proceso de actualización documentado
- [ ] Estrategia de rollback definida
- [ ] Imágenes de Docker actualizadas regularmente

---

## Documentación Adicional

- [Guía de Seguridad](../auditoria-seguridad.md)
- [Configuración de Variables de Entorno](../../.env.example)
- [Docker Compose para Desarrollo](../../docker-compose.yml)
