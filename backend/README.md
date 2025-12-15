# Backend - Sistema de Riego MQTT

## Stack Implementado
- **Java 17+** con Spring Boot 3.4.0
- **Package**: `ar.net.dac.iot.irrigacion`
- **PostgreSQL 15** con Flyway para migraciones
- **HiveMQ MQTT Client 1.3.3**
- **Maven** para gestión de dependencias

## Estructura de Packages
```
ar.net.dac.iot.irrigacion/
├── config/          # Configuración (Security, MQTT, CORS)
├── controller/      # REST Controllers
├── dto/            # Data Transfer Objects
├── entity/         # JPA Entities
├── repository/     # Spring Data JPA Repositories
├── service/        # Business Logic
└── mqtt/           # MQTT Integration
```

## Funcionalidades
- ✅ API REST para gestión de agendas de riego
- ✅ Configuración de zonas (hasta 8 zonas)
- ✅ Comandos manuales de riego
- ✅ Integración MQTT (pub/sub)
- ✅ Estado en tiempo real de zonas
- ✅ Autenticación HTTP Basic
- ✅ Frontend Vue.js embebido

## Seguridad
- **Autenticación**: HTTP Basic (usuario/password desde `.env`)
- **Configuración**: Ver `SecurityConfig.java`
- **Variables**: `APP_SECURITY_USERNAME`, `APP_SECURITY_PASSWORD`

## Ejecución
Ver [docker-compose-guide.md](../docs/implementacion/docker-compose-guide.md) para instrucciones completas.
