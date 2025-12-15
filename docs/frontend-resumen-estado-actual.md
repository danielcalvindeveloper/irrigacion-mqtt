# Frontend - Resumen y Estado Actual

**Fecha:** 13 de diciembre de 2025  
**Framework:** Vue.js 3  
**Estado:** 🔴 No implementado (solo esqueleto)

---

## 📊 Estado General

### 🔴 Estado Actual: PLACEHOLDER

El frontend actualmente solo contiene un archivo README.md con instrucciones básicas. **No hay código implementado**.

**Contenido actual:**
- 📁 `frontend/` → Solo contiene `README.md`
- ❌ No hay proyecto Vue inicializado
- ❌ No hay componentes
- ❌ No hay rutas
- ❌ No hay servicios HTTP
- ❌ No hay configuración de build

---

## 🎯 Alcance Funcional Definido

### Requerimientos Funcionales (según docs)

#### RF-1: Gestión de Agenda por Zona
**Prioridad:** 🔴 Alta

- ✅ **Documentado en:** [01-requerimientos-funcionales.md](01-requerimientos-funcionales.md)
- 📋 **Pantallas necesarias:**
  - CRUD de agendas (Crear, Editar, Eliminar)
  - Vista de agenda activa por zona
  - Selector de zona (1-4)

**Funcionalidades requeridas:**
- Crear nueva agenda con:
  - Días de la semana (checkboxes LUN-DOM)
  - Hora de inicio (time picker)
  - Duración en minutos (input numérico)
  - Estado activo/inactivo (toggle)
- Editar agenda existente
- Eliminar agenda
- Listar todas las agendas por zona

---

#### RF-1.1: Visualización de Agenda Activa
**Prioridad:** 🔴 Alta

- ✅ **Documentado en:** [01-requerimientos-funcionales.md](01-requerimientos-funcionales.md)
- 📋 **Pantalla:** Dashboard principal

**Información a mostrar por zona:**
- Estado actual (ON/OFF) con indicador visual
- Próxima activación (fecha y hora)
- Duración configurada
- Nombre de la zona
- Indicador de conectividad del ESP32

**Diseño sugerido:**
- Vista tipo card/panel por zona
- Indicadores de color (verde=ON, gris=OFF)
- Cuenta regresiva para próxima activación
- Iconos visuales (gota de agua, reloj, etc.)

---

#### RF-1.2: Consulta Histórica
**Prioridad:** 🟡 Media

- ✅ **Documentado en:** [01-requerimientos-funcionales.md](01-requerimientos-funcionales.md)
- 📋 **Pantalla:** Histórico/Reportes

**Funcionalidades:**
- Filtro por período (fecha desde/hasta)
- Filtro por zona
- Tabla con:
  - Fecha y hora
  - Zona
  - Duración real
  - Motivo (agenda/manual/humedad)
- Exportar a CSV (post-MVP)

---

#### RF-2: Control Manual por Zona
**Prioridad:** 🔴 Alta (implícito en docs)

- 📋 **Ubicación:** Dashboard o sección dedicada

**Funcionalidades:**
- Botones ON/OFF por zona
- Input para duración (en minutos)
- Confirmación antes de activar
- Feedback visual del comando
- Estado de ejecución en tiempo real

---

### Requerimientos No Funcionales

#### RNF-1: Responsividad
**Documentado en:** [02-requerimientos-no-funcionales.md](02-requerimientos-no-funcionales.md)

- 📱 **Móvil:** Diseño mobile-first
- 💻 **Desktop:** Adaptable a pantallas grandes
- 📐 **Breakpoints:** Considerar teléfonos, tablets y desktop

---

#### RNF-2: PWA (Opcional en MVP)
**Documentado en:** [07-stack-tecnologico.md](07-stack-tecnologico.md), [10-brechas-dudas-para-aclarar.md](10-brechas-dudas-para-aclarar.md)

- 🔹 Instalación opcional
- ❌ NO offline UI en MVP (solo online)
- ✅ Considerar para post-MVP

---

#### RNF-3: Actualización en Tiempo Real
**Documentado en:** [10-brechas-dudas-para-aclarar.md](10-brechas-dudas-para-aclarar.md)

- 🟢 **Preferido:** WebSocket para estado en vivo
- 🟡 **Fallback:** Polling
- 📊 Actualizar estado de zonas sin refresh manual

---

#### RNF-4: Seguridad
**Documentado en:** [10-brechas-dudas-para-aclarar.md](10-brechas-dudas-para-aclarar.md)

- 🔐 **Autenticación:** JWT
- 🔑 **Login:** Usuario/contraseña básico (MVP)
- ❌ NO OAuth en MVP
- 🌐 **CORS:** Restringido al dominio de la UI

---

## 🗺️ Estructura de Pantallas Propuesta

### 1. **Dashboard / Vista Principal** 🏠
**Prioridad:** 🔴 Alta

**Layout sugerido:**
```
┌─────────────────────────────────────┐
│  🏠 Irrigación - Dashboard          │
├─────────────────────────────────────┤
│  [Zona 1]  [Zona 2]  [Zona 3]  [Zona 4] ← Tabs/Pills
├─────────────────────────────────────┤
│                                      │
│  ┌──────────────────────────────┐  │
│  │  💧 Zona 1 - Jardín Frontal  │  │
│  │  Estado: ● ON (5 min rest.)  │  │
│  │  Próximo: Hoy 19:00 (15min)  │  │
│  │  [Detener] [Regar Manual]    │  │
│  └──────────────────────────────┘  │
│                                      │
│  📅 Agenda Activa                   │
│  ├─ LUN, MIE, VIE: 07:00 (15min)   │
│  └─ MAR, JUE: 19:00 (10min)        │
│                                      │
│  📊 Humedad: 65% (Óptimo)           │
│                                      │
└─────────────────────────────────────┘
```

**Componentes:**
- `ZoneCard.vue` - Card de zona con estado
- `ManualControl.vue` - Botones de control manual
- `AgendaSummary.vue` - Resumen de agenda activa
- `HumiditySensor.vue` - Indicador de humedad

---

### 2. **Gestión de Agendas** 📅
**Prioridad:** 🔴 Alta

**Layout sugerido:**
```
┌─────────────────────────────────────┐
│  📅 Agendas - Zona 1                │
├─────────────────────────────────────┤
│  [+ Nueva Agenda]                   │
│                                      │
│  ┌──────────────────────────────┐  │
│  │ ✅ Riego Matutino             │  │
│  │ LUN, MIE, VIE - 07:00 (15min)│  │
│  │ [Editar] [Eliminar] [Toggle] │  │
│  └──────────────────────────────┘  │
│                                      │
│  ┌──────────────────────────────┐  │
│  │ ⭕ Riego Vespertino           │  │
│  │ MAR, JUE - 19:00 (10min)     │  │
│  │ [Editar] [Eliminar] [Toggle] │  │
│  └──────────────────────────────┘  │
│                                      │
└─────────────────────────────────────┘
```

**Componentes:**
- `AgendaList.vue` - Lista de agendas
- `AgendaItem.vue` - Item individual de agenda
- `AgendaForm.vue` - Formulario crear/editar
- `DaySelector.vue` - Selector de días de la semana
- `TimePickerCustom.vue` - Selector de hora

---

### 3. **Histórico** 📊
**Prioridad:** 🟡 Media

**Layout sugerido:**
```
┌─────────────────────────────────────┐
│  📊 Histórico de Riego              │
├─────────────────────────────────────┤
│  Filtros:                           │
│  Zona: [Todas ▾]                    │
│  Desde: [2025-12-01] Hasta: [Hoy]  │
│  [Aplicar] [Limpiar]                │
├─────────────────────────────────────┤
│  Fecha/Hora  Zona  Duración  Motivo │
│  ─────────────────────────────────  │
│  12/12 07:00  1    15min    Agenda  │
│  12/12 19:30  2    5min     Manual  │
│  11/12 07:00  1    15min    Agenda  │
│  ...                                 │
│                                      │
│  [Exportar CSV]  [Ver más]          │
└─────────────────────────────────────┘
```

**Componentes:**
- `HistoryFilters.vue` - Filtros de búsqueda
- `HistoryTable.vue` - Tabla de eventos
- `ExportButton.vue` - Botón de exportación

---

### 4. **Login** 🔐
**Prioridad:** 🟡 Media (MVP simple)

**Layout sugerido:**
```
┌─────────────────────────────────────┐
│                                      │
│          💧 Irrigación MQTT          │
│                                      │
│  ┌──────────────────────────────┐  │
│  │ Usuario: [____________]      │  │
│  │ Password: [____________]     │  │
│  │                               │  │
│  │        [Iniciar Sesión]      │  │
│  └──────────────────────────────┘  │
│                                      │
└─────────────────────────────────────┘
```

**Componentes:**
- `LoginForm.vue` - Formulario de login
- Guard de rutas con autenticación

---

### 5. **Configuración** ⚙️
**Prioridad:** 🔵 Baja (Post-MVP)

**Funcionalidades futuras:**
- Configuración de nodos ESP32
- Umbrales de humedad
- Notificaciones
- Perfil de usuario

---

## 🎨 Aspectos Estéticos y UX

### Paleta de Colores Sugerida

```css
/* Tema Principal */
--primary: #2196F3;      /* Azul agua */
--secondary: #4CAF50;    /* Verde planta */
--accent: #FF9800;       /* Naranja alerta */
--danger: #F44336;       /* Rojo error */

/* Estados */
--on-state: #4CAF50;     /* Verde - Riego activo */
--off-state: #9E9E9E;    /* Gris - Apagado */
--pending: #FF9800;      /* Naranja - Pendiente */

/* Neutrales */
--background: #FAFAFA;   /* Fondo claro */
--surface: #FFFFFF;      /* Tarjetas */
--text-primary: #212121; /* Texto principal */
--text-secondary: #757575; /* Texto secundario */
```

### Iconografía Recomendada

**Librería sugerida:** Material Design Icons (MDI) o Font Awesome

- 💧 **Riego activo:** `water-pump`, `water`
- ⏱️ **Agenda:** `clock-outline`, `calendar`
- 📊 **Histórico:** `chart-line`, `history`
- 🌱 **Humedad:** `water-percent`, `sprout`
- ⚙️ **Configuración:** `cog`, `settings`
- 🔐 **Login:** `lock`, `account`

### Componentes UI Recomendados

**Librería de componentes sugerida:**

1. **Vuetify 3** (Recomendado para MVP)
   - ✅ Material Design out-of-the-box
   - ✅ Componentes pre-construidos
   - ✅ Responsive grid system
   - ✅ Bien documentado

2. **PrimeVue** (Alternativa)
   - ✅ Muchos componentes
   - ✅ Temas personalizables
   - ✅ DataTable robusto

3. **Element Plus** (Alternativa)
   - ✅ Diseño limpio
   - ✅ Componentes completos

### Consideraciones de Diseño

#### 1. **Mobile First**
- Diseñar primero para móvil
- Botones grandes (min 48x48px táctiles)
- Navegación simple (bottom nav o hamburger)

#### 2. **Feedback Visual**
- Loading spinners al hacer requests
- Toast notifications para acciones
- Confirmaciones antes de acciones destructivas

#### 3. **Estados Claros**
- Indicadores visuales de estado (color + icono)
- Deshabilitación de botones cuando no aplique
- Estados de carga (skeleton loaders)

#### 4. **Accesibilidad**
- Contraste adecuado (WCAG AA mínimo)
- Labels en todos los inputs
- Navegación por teclado

---

## 🔧 Stack Técnico Propuesto

### Core

```json
{
  "dependencies": {
    "vue": "^3.4.0",
    "vue-router": "^4.2.5",
    "pinia": "^2.1.7",           // State management
    "axios": "^1.6.0",           // HTTP client
    "socket.io-client": "^4.7.0" // WebSocket (opcional)
  }
}
```

### UI Framework (elegir uno)

```json
{
  "dependencies": {
    "vuetify": "^3.5.0"  // Opción recomendada
    // O
    // "primevue": "^3.48.0",
    // "primeicons": "^6.0.1"
  }
}
```

### Utilidades

```json
{
  "dependencies": {
    "date-fns": "^3.0.0",        // Manejo de fechas
    "vueuse": "^10.7.0",         // Composables útiles
    "chart.js": "^4.4.0",        // Gráficos (histórico)
    "vue-chartjs": "^5.3.0"
  }
}
```

### Dev Dependencies

```json
{
  "devDependencies": {
    "@vitejs/plugin-vue": "^5.0.0",
    "vite": "^5.0.0",
    "typescript": "^5.3.0",      // Opcional pero recomendado
    "eslint": "^8.56.0",
    "prettier": "^3.1.0"
  }
}
```

---

## 📁 Estructura de Proyecto Propuesta

```
frontend/
├── public/
│   ├── favicon.ico
│   └── manifest.json          # Para PWA
├── src/
│   ├── assets/
│   │   ├── styles/
│   │   │   ├── variables.css  # Variables CSS
│   │   │   └── global.css     # Estilos globales
│   │   └── icons/
│   ├── components/
│   │   ├── common/
│   │   │   ├── LoadingSpinner.vue
│   │   │   ├── ToastNotification.vue
│   │   │   └── ConfirmDialog.vue
│   │   ├── dashboard/
│   │   │   ├── ZoneCard.vue
│   │   │   ├── ManualControl.vue
│   │   │   ├── AgendaSummary.vue
│   │   │   └── HumiditySensor.vue
│   │   ├── agenda/
│   │   │   ├── AgendaList.vue
│   │   │   ├── AgendaItem.vue
│   │   │   ├── AgendaForm.vue
│   │   │   └── DaySelector.vue
│   │   └── history/
│   │       ├── HistoryFilters.vue
│   │       └── HistoryTable.vue
│   ├── views/
│   │   ├── DashboardView.vue
│   │   ├── AgendaView.vue
│   │   ├── HistoryView.vue
│   │   └── LoginView.vue
│   ├── router/
│   │   └── index.ts           # Rutas + guards
│   ├── stores/
│   │   ├── auth.ts            # Store de autenticación
│   │   ├── zones.ts           # Store de zonas
│   │   └── agendas.ts         # Store de agendas
│   ├── services/
│   │   ├── api.ts             # Cliente HTTP base
│   │   ├── authService.ts     # Servicios de auth
│   │   ├── agendaService.ts   # Servicios de agenda
│   │   └── websocket.ts       # Conexión WebSocket
│   ├── composables/
│   │   ├── useAuth.ts         # Composable de auth
│   │   └── useWebSocket.ts    # Composable de WS
│   ├── types/
│   │   ├── agenda.ts          # Tipos de agenda
│   │   ├── zone.ts            # Tipos de zona
│   │   └── api.ts             # Tipos de API
│   ├── utils/
│   │   ├── dateUtils.ts       # Utilidades de fecha
│   │   └── validators.ts      # Validaciones
│   ├── App.vue
│   └── main.ts
├── .env.example
├── .gitignore
├── index.html
├── package.json
├── tsconfig.json              # Si usas TypeScript
├── vite.config.ts
└── README.md
```

---

## 🚀 Plan de Implementación Sugerido

### Fase 1: Setup Inicial (1-2 días)
**Prioridad:** 🔴 Crítica

- [ ] Inicializar proyecto Vite + Vue 3
- [ ] Instalar dependencias core
- [ ] Configurar Vue Router
- [ ] Configurar Pinia
- [ ] Instalar y configurar Vuetify (o librería elegida)
- [ ] Setup de variables de entorno
- [ ] Configurar Axios con baseURL

**Comando inicial:**
```bash
cd frontend
npm create vite@latest . -- --template vue
npm install
npm install vue-router pinia axios vuetify
```

---

### Fase 2: Autenticación (1-2 días)
**Prioridad:** 🟡 Media

- [ ] Crear LoginView
- [ ] Implementar authService con JWT
- [ ] Crear store de autenticación
- [ ] Implementar route guards
- [ ] Manejo de tokens (localStorage/sessionStorage)
- [ ] Auto-logout en token expirado

---

### Fase 3: Dashboard Principal (2-3 días)
**Prioridad:** 🔴 Crítica

- [ ] Crear DashboardView
- [ ] Implementar ZoneCard component
- [ ] Conectar con API de backend (/api/nodos/{nodeId}/agendas)
- [ ] Mostrar estado actual de zonas
- [ ] Implementar ManualControl component
- [ ] Integrar comando manual (/api/nodos/{nodeId}/cmd)

---

### Fase 4: Gestión de Agendas (3-4 días)
**Prioridad:** 🔴 Crítica

- [ ] Crear AgendaView
- [ ] Implementar CRUD de agendas
- [ ] Crear formulario de agenda (AgendaForm)
- [ ] Implementar selector de días (DaySelector)
- [ ] Validaciones de formulario
- [ ] Feedback de operaciones (toasts)

---

### Fase 5: Histórico (2-3 días)
**Prioridad:** 🟡 Media

- [ ] Crear HistoryView
- [ ] Implementar filtros de búsqueda
- [ ] Tabla de eventos históricos
- [ ] Paginación
- [ ] Exportar a CSV (opcional)

---

### Fase 6: Tiempo Real (2-3 días)
**Prioridad:** 🟡 Media

- [ ] Implementar WebSocket client
- [ ] Actualización de estados en tiempo real
- [ ] Fallback a polling si WS falla
- [ ] Indicador de conexión en UI

---

### Fase 7: PWA (Opcional - Post MVP)
**Prioridad:** 🔵 Baja

- [ ] Configurar service worker
- [ ] Manifest.json
- [ ] Iconos de diferentes tamaños
- [ ] Estrategia de caché

---

## 🎯 Decisiones Pendientes

### Críticas (Resolver antes de empezar)

1. **¿Usar TypeScript?**
   - ✅ Recomendado: Sí (mejor DX, menos bugs)
   - ⚠️ Alternativa: JavaScript (más rápido de prototipar)

2. **¿Qué librería de componentes UI?**
   - 🟢 Vuetify (más completo, Material Design)
   - 🟡 PrimeVue (más flexible)
   - 🟡 Tailwind CSS + Headless UI (más control, más trabajo)

3. **¿Multi-nodo desde el inicio?**
   - ✅ Según docs: Sí (incluir selector de nodo)
   - 📋 UI debe considerar múltiples ESP32

4. **¿Actualización en tiempo real obligatoria en MVP?**
   - 🟢 WebSocket (ideal)
   - 🟡 Polling cada X segundos (aceptable)

### Secundarias (Pueden posponerse)

5. **¿Modo oscuro?**
   - Post-MVP (nice to have)

6. **¿Internacionalización (i18n)?**
   - Post-MVP (solo español por ahora)

7. **¿Tests unitarios?**
   - Recomendado pero puede posponerse para MVP

---

## 📝 Conclusiones y Recomendaciones

### Estado Actual: 🔴 CRÍTICO - No hay código

**Urgencia:** El frontend es una de las 4 piezas clave del sistema y actualmente no existe.

### Recomendaciones Inmediatas:

1. **Inicializar proyecto Vue 3 con Vite** 
   - Tiempo estimado: 30 minutos
   - Comando: `npm create vite@latest`

2. **Elegir librería de componentes UI**
   - Recomendación: Vuetify 3 (más rápido para MVP)
   - Instalación: 15 minutos

3. **Implementar Dashboard mínimo**
   - Mostrar estado de zonas (mock data primero)
   - Control manual básico
   - Tiempo estimado: 1-2 días

4. **Conectar con backend existente**
   - El backend ya tiene endpoints funcionando
   - Implementar llamadas a API
   - Tiempo estimado: 1 día

### Prioridades por Fase:

#### MVP Mínimo (Semana 1-2):
- ✅ Setup + Routing
- ✅ Dashboard con estado de zonas
- ✅ Control manual

#### MVP Completo (Semana 3-4):
- ✅ Gestión de agendas (CRUD)
- ✅ Histórico básico
- ✅ WebSocket para tiempo real

#### Post-MVP (Mes 2):
- PWA
- Gráficos avanzados
- Configuración de nodos
- Modo oscuro

---

## 🔗 Recursos y Referencias

### Documentación del Proyecto
- [Requerimientos Funcionales](01-requerimientos-funcionales.md)
- [Requerimientos No Funcionales](02-requerimientos-no-funcionales.md)
- [Arquitectura General](03-arquitectura-general.md)
- [Roadmap MVP](09-roadmap-mvp.md)
- [Brechas y Dudas](10-brechas-dudas-para-aclarar.md)

### Recursos Externos
- [Vue 3 Documentation](https://vuejs.org/)
- [Vuetify 3 Documentation](https://vuetifyjs.com/)
- [Vite Documentation](https://vitejs.dev/)
- [Pinia Documentation](https://pinia.vuejs.org/)

---

**Preparado por:** GitHub Copilot Agent  
**Fecha:** 13 de diciembre de 2025  
**Siguiente paso:** Discusión con el usuario sobre decisiones técnicas y prioridades
