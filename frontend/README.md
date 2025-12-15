# Frontend - Irrigación MQTT

Aplicación web Vue.js 3 para gestión de sistema de riego inteligente.

## 🚀 Inicio Rápido

### Requisitos Previos

- Node.js 18+ instalado
- npm o yarn

### Instalación

```bash
# Instalar dependencias
npm install
```

### Desarrollo

```bash
# Iniciar servidor de desarrollo
npm run dev

# La aplicación estará disponible en http://localhost:5173
```

### Producción

```bash
# Build para producción
npm run build

# Preview del build de producción
npm run preview
```

## 📦 Scripts Disponibles

| Comando | Descripción |
|---------|-------------|
| `npm run dev` | Inicia servidor de desarrollo (solo localhost) |
| `npm run dev:mobile` | Inicia servidor expuesto en red local (acceso desde móviles) |
| `npm run build` | Genera build optimizado para producción |
| `npm run preview` | Preview del build de producción |
| `npm run lint` | Ejecuta linter (ESLint) |
| `npm run format` | Formatea código con Prettier |

## 🛠️ Stack Tecnológico

- **Vue 3** - Framework JavaScript
- **Vuetify 3** - Librería de componentes Material Design
- **Vue Router** - Enrutamiento
- **Pinia** - State management
- **Axios** - Cliente HTTP
- **Vite** - Build tool

## 📁 Estructura del Proyecto

```
frontend/
├── public/              # Archivos estáticos
├── src/
│   ├── assets/         # Estilos, iconos, imágenes
│   ├── components/     # Componentes reutilizables
│   ├── views/          # Vistas/páginas
│   ├── router/         # Configuración de rutas
│   ├── stores/         # Stores de Pinia
│   ├── services/       # Servicios API
│   ├── composables/    # Composables de Vue
│   ├── plugins/        # Plugins (Vuetify, etc.)
│   ├── App.vue         # Componente raíz
│   └── main.js         # Punto de entrada
├── .env.example        # Ejemplo de variables de entorno
├── package.json
├── vite.config.js
└── README.md
```

## ⚙️ Configuración

### Variables de Entorno

Copia el archivo `.env.example` a `.env` y ajusta las variables:

```bash
cp .env.example .env
```

Variables disponibles:

```env
VITE_API_BASE_URL=http://localhost:8080/api
VITE_WS_URL=ws://localhost:8080/ws
```

### Backend

El frontend se conecta al backend en `http://localhost:8080/api` por defecto.

Asegúrate de que el backend esté corriendo antes de iniciar el frontend:

```bash
# En el directorio raíz del proyecto
docker-compose up -d
```

## 🎨 Diseño Mobile-First

La aplicación está optimizada para dispositivos móviles:

- **Breakpoints**: 360px, 414px, 768px, 1024px
- **Bottom Navigation**: Navegación principal en la parte inferior
- **Touch Targets**: Mínimo 48x48px para elementos táctiles
- **Safe Areas**: Compatible con notch (iPhone X+)

Ver [docs/frontend-diseno-mobile-first.md](../docs/frontend-diseno-mobile-first.md) para más detalles.

## 📱 Desarrollo en Dispositivos Móviles

### Acceder desde tu teléfono en la misma red

```bash
# 1. Inicia el servidor de desarrollo con exposición en red
npm run dev:mobile

# 2. Vite mostrará las URLs disponibles:
#    Local:   http://localhost:5173
#    Network: http://192.168.x.x:5173  ← Usa esta en tu móvil

# 3. Accede desde tu teléfono usando la URL Network
```

**Nota Windows**: Si el firewall bloquea la conexión:
```powershell
netsh advfirewall firewall add rule name="Vite Dev Server" dir=in action=allow protocol=TCP localport=5173
```

### Herramientas de desarrollo

- **Chrome DevTools**: Inspección remota de dispositivos Android
- **Safari Web Inspector**: Inspección de dispositivos iOS

## 🐛 Troubleshooting

### El servidor no inicia

```bash
# Verifica que las dependencias estén instaladas
npm install

# Limpia caché de Vite
rm -rf node_modules/.vite
npm run dev
```

### Error de CORS

Verifica que el backend tenga CORS configurado para el origen del frontend:

```yaml
# En backend/src/main/resources/application.yml
cors:
  allowed-origins: http://localhost:5173
```

### Hot reload no funciona

```bash
# Intenta con --host
npm run dev -- --host
```

## 📚 Recursos

- [Vue 3 Documentation](https://vuejs.org/)
- [Vuetify 3 Documentation](https://vuetifyjs.com/)
- [Vite Documentation](https://vitejs.dev/)
- [Vue Router Documentation](https://router.vuejs.org/)
- [Pinia Documentation](https://pinia.vuejs.org/)

## 🔗 Enlaces del Proyecto

- [Requerimientos Funcionales](../docs/01-requerimientos-funcionales.md)
- [Arquitectura General](../docs/03-arquitectura-general.md)
- [Diseño Mobile-First](../docs/frontend-diseno-mobile-first.md)
- [API Backend](../docs/implementacion/contratos-mqtt-http.md)

---

**Última actualización:** 14 de diciembre de 2025
