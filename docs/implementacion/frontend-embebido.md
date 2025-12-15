# Frontend Embebido - Guía de Implementación

**Fecha**: 15 de diciembre de 2025  
**Estado**: ✅ Implementado

---

## 📋 Resumen

El frontend Vue.js ahora se **embebe dentro del JAR del backend** para producción, manteniendo el flujo de desarrollo separado con hot reload.

### Beneficios
- ✅ Un solo artefacto JAR/contenedor
- ✅ Sin problemas de CORS en producción
- ✅ Ideal para móvil/PWA/WebView
- ✅ Despliegue simplificado (Raspberry Pi, móvil)
- ✅ Menor latencia (sin saltos HTTP entre servicios)

---

## 🔧 Arquitectura

### Desarrollo (Frontend Separado)
```
Terminal 1: docker-compose up -d        → Backend en :8080
Terminal 2: npm run dev:mobile          → Frontend en :5173 (hot reload)
```

- Frontend accede a backend vía CORS
- Cambios en Vue se ven instantáneamente
- Ideal para iterar rápidamente

### Producción (Frontend Embebido)
```
docker-compose up -d --build backend    → Todo en :8080
```

- Navegar a `http://localhost:8080` → Sirve el frontend
- API en `http://localhost:8080/api/*`
- Vue Router funciona (SPA routing)

---

## 📁 Archivos Modificados

### 1. `backend/Dockerfile` (Multi-stage)

```dockerfile
# Stage 1: Build frontend
FROM node:18-alpine AS frontend-build
WORKDIR /frontend
COPY ../frontend/package*.json ./
RUN npm ci
COPY ../frontend/ ./
RUN npm run build

# Stage 2: Build backend
FROM eclipse-temurin:17-jdk-alpine AS backend-build
WORKDIR /app
COPY pom.xml .
COPY src ./src
# Copiar dist a /static
COPY --from=frontend-build /frontend/dist ./src/main/resources/static
RUN apk add --no-cache maven && mvn clean package -DskipTests

# Stage 3: Runtime
FROM eclipse-temurin:17-jre-alpine
WORKDIR /app
COPY --from=backend-build /app/target/irrigacion-*.jar app.jar
ENV JAVA_OPTS="-Duser.timezone=UTC"
EXPOSE 8080
ENTRYPOINT ["sh", "-c", "java $JAVA_OPTS -jar app.jar"]
```

**Cambios clave:**
- Stage 1 construye el frontend con `npm run build`
- Stage 2 copia `/frontend/dist` a `src/main/resources/static` antes de Maven
- Maven empaqueta el frontend dentro del JAR

### 2. `docker-compose.yml`

```yaml
backend:
  build:
    context: .                    # ← Cambio: raíz del proyecto
    dockerfile: backend/Dockerfile
  # ... resto sin cambios
```

**Cambio:** `context` apunta a raíz para acceder a `/frontend` y `/backend`

### 3. `backend/src/main/java/.../config/WebConfig.java`

```java
@Override
public void addResourceHandlers(ResourceHandlerRegistry registry) {
    // Servir assets del frontend
    registry.addResourceHandler("/assets/**")
            .addResourceLocations("classpath:/static/assets/");

    // SPA routing: todo lo demás → index.html
    registry.addResourceHandler("/**")
            .addResourceLocations("classpath:/static/")
            .resourceChain(true)
            .addResolver(new PathResourceResolver() {
                @Override
                protected Resource getResource(String resourcePath, Resource location) {
                    Resource requestedResource = location.createRelative(resourcePath);
                    
                    if (requestedResource.exists() && requestedResource.isReadable()) {
                        return requestedResource;
                    }
                    
                    // Si no es /api/*, devolver index.html para Vue Router
                    if (!resourcePath.startsWith("api/")) {
                        return new ClassPathResource("/static/index.html");
                    }
                    
                    return null;
                }
            });
}
```

**Función:** Permite que Vue Router maneje rutas como `/agendas`, `/zonas` sin errores 404

### 4. `backend/src/main/resources/application.yml`

```yaml
spring:
  web:
    resources:
      static-locations: classpath:/static/
      add-mappings: true
  mvc:
    static-path-pattern: /**
```

**Función:** Habilita servir archivos estáticos desde `/static`

### 5. `frontend/.env.production`

```env
VITE_API_BASE_URL=/api
VITE_WS_URL=ws://localhost:8080/ws
VITE_DEFAULT_NODE_ID=550e8400-e29b-41d4-a716-446655440000
```

**Función:** En build de producción, usa rutas relativas (mismo origen)

### 6. `frontend/vite.config.js`

```javascript
build: {
  outDir: 'dist',
  assetsDir: 'assets',
  rollupOptions: {
    output: {
      manualChunks: {
        'vendor': ['vue', 'vue-router', 'pinia'],
        'vuetify': ['vuetify']
      }
    }
  }
}
```

**Optimizaciones:** Code splitting para mejor performance

---

## 🚀 Uso

### Desarrollo (No cambió)
```bash
# Terminal 1: Backend
docker-compose up -d

# Terminal 2: Frontend con hot reload
cd frontend
npm run dev:mobile

# Acceder: http://localhost:5173
```

### Producción (Nueva forma)
```bash
# Build y levantar todo
docker-compose up -d --build backend

# Acceder: http://localhost:8080
```

### Solo rebuild del backend (sin borrar BD)
```bash
docker-compose up -d --build backend
```

---

## 🧪 Verificación

### 1. Comprobar que frontend está embebido
```bash
# Construir
docker-compose build backend

# Verificar que el JAR contiene /static
docker run --rm irrigacion-mqtt-repo-backend jar tf app.jar | grep "static/index.html"
```

Debería mostrar: `BOOT-INF/classes/static/index.html`

### 2. Probar en navegador
```bash
docker-compose up -d

# Abrir http://localhost:8080
# Debería mostrar la aplicación Vue (Dashboard)

# Verificar que API funciona
curl http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas
```

### 3. Verificar SPA routing
- Navegar a `http://localhost:8080/agendas`
- Refrescar página (F5)
- **Esperado:** La página carga correctamente (no error 404)
- **Confirmación:** Vue Router funciona con backend

---

## 📝 Notas Técnicas

### ¿Por qué multi-stage?
- **Reducir tamaño**: Stage final solo tiene JRE + JAR (no Node.js ni Maven)
- **Seguridad**: Menos superficie de ataque
- **Performance**: Imagen más liviana (~200MB vs >500MB)

### ¿Por qué mantener CORS?
- Necesario para desarrollo con frontend en puerto 5173
- No afecta producción (mismo origen = sin CORS)

### ¿Qué pasa con WebSocket?
- En producción: `ws://localhost:8080/ws` (mismo origen)
- En desarrollo: `ws://localhost:8080/ws` (CORS permite cross-origin WebSocket)

### ¿Cómo actualizar solo frontend?
```bash
# Opción A: Rebuild completo
docker-compose up -d --build backend

# Opción B: Build local y copiar al contenedor (más rápido para desarrollo)
cd frontend
npm run build
docker cp dist/. irrigacion-backend:/app/BOOT-INF/classes/static/
docker restart irrigacion-backend
```

---

## 🔗 Documentación Relacionada

- [PROYECTO_CONTEXTO.md](../PROYECTO_CONTEXTO.md) - Arquitectura completa
- [docker-compose-guide.md](./docker-compose-guide.md) - Comandos Docker
- [frontend/README.md](../frontend/README.md) - Desarrollo del frontend

---

**Última actualización**: 2025-12-15
