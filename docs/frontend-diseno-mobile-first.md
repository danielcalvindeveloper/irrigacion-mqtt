# Frontend - Diseño Estético Mobile-First

**Fecha:** 14 de diciembre de 2025  
**Prioridad:** 📱 **MÓVIL PRIMERO** (Teléfonos como dispositivo principal)  
**Principio:** La experiencia en teléfono debe ser perfecta, desktop es secundario

---

## 🎯 Filosofía de Diseño

### Principios Mobile-First

1. **Teléfono es el dispositivo principal** 📱
   - 90% del uso será en móvil
   - Diseñar primero para pantallas 360-414px
   - Desktop se adapta desde móvil, no al revés

2. **Una mano, pulgar dominante** 👍
   - Controles importantes en zona del pulgar
   - Botones grandes (mínimo 48x48px)
   - Navegación inferior (bottom navigation)

3. **Información crítica primero** 🎯
   - Estado actual visible sin scroll
   - Acción inmediata (ON/OFF) en un tap
   - Datos secundarios colapsables

4. **Feedback táctil claro** ✨
   - Animaciones sutiles en interacciones
   - Estados activos bien diferenciados
   - Loading states evidentes

---

## 📐 Especificaciones Mobile

### Breakpoints Definidos

```css
/* Mobile First - Diseño base */
/* Teléfonos pequeños: 320px - 374px */
.container {
  padding: 12px;
  max-width: 100%;
}

/* Teléfonos estándar: 375px - 413px */
@media (min-width: 375px) {
  .container {
    padding: 16px;
  }
}

/* Teléfonos grandes: 414px - 767px */
@media (min-width: 414px) {
  .container {
    padding: 20px;
  }
}

/* Tablets: 768px - 1023px */
@media (min-width: 768px) {
  .container {
    max-width: 720px;
    margin: 0 auto;
  }
}

/* Desktop: 1024px+ */
@media (min-width: 1024px) {
  .container {
    max-width: 960px;
    margin: 0 auto;
  }
}
```

### Tamaños de Toque Recomendados

```
Botón primario:     56px altura (táctil óptimo)
Botón secundario:   48px altura (mínimo táctil)
Íconos clicables:   48x48px área (WCAG AAA)
Toggle/Switch:      48px altura
Input fields:       56px altura
Spacing entre botones: 12px mínimo
```

---

## 🎨 Sistema de Diseño Visual

### Paleta de Colores Mobile-Optimizada

```css
:root {
  /* === COLORES PRIMARIOS === */
  --water-blue: #2196F3;        /* Azul agua - Primary action */
  --water-blue-light: #64B5F6;  /* Azul claro - Hover/Focus */
  --water-blue-dark: #1976D2;   /* Azul oscuro - Pressed */
  
  --plant-green: #4CAF50;       /* Verde planta - Success/Active */
  --plant-green-light: #81C784; /* Verde claro - Hover */
  --plant-green-dark: #388E3C;  /* Verde oscuro - Pressed */
  
  /* === ESTADOS DE ZONA === */
  --zone-active: #4CAF50;       /* Verde brillante - Riego activo */
  --zone-idle: #E0E0E0;         /* Gris claro - Inactivo */
  --zone-pending: #FF9800;      /* Naranja - Próximo riego */
  --zone-error: #F44336;        /* Rojo - Error/Desconectado */
  --zone-manual: #9C27B0;       /* Morado - Control manual */
  
  /* === SUPERFICIES === */
  --surface-1: #FFFFFF;         /* Blanco puro - Cards principales */
  --surface-2: #FAFAFA;         /* Gris muy claro - Background */
  --surface-3: #F5F5F5;         /* Gris claro - Disabled */
  
  /* === TEXTO === */
  --text-primary: #212121;      /* Negro - Texto principal */
  --text-secondary: #757575;    /* Gris medio - Texto secundario */
  --text-disabled: #BDBDBD;     /* Gris claro - Disabled */
  --text-on-primary: #FFFFFF;   /* Blanco - Texto sobre azul/verde */
  
  /* === SOMBRAS (importante para mobile) === */
  --shadow-1: 0 2px 4px rgba(0,0,0,0.1);   /* Elevación suave */
  --shadow-2: 0 4px 8px rgba(0,0,0,0.15);  /* Elevación media */
  --shadow-3: 0 8px 16px rgba(0,0,0,0.2);  /* Elevación alta */
  
  /* === RADIOS === */
  --radius-sm: 8px;   /* Botones pequeños */
  --radius-md: 12px;  /* Cards, inputs */
  --radius-lg: 16px;  /* Cards grandes */
  --radius-xl: 24px;  /* Bottom sheets */
  
  /* === TIMING === */
  --transition-fast: 150ms;
  --transition-normal: 250ms;
  --transition-slow: 350ms;
}
```

### Tipografía Mobile

```css
/* Sistema de tipografía optimizado para legibilidad móvil */

/* Títulos */
h1, .text-h1 {
  font-size: 28px;      /* Título principal */
  line-height: 1.2;
  font-weight: 700;
  letter-spacing: -0.5px;
}

h2, .text-h2 {
  font-size: 24px;      /* Subtítulos importantes */
  line-height: 1.3;
  font-weight: 600;
  letter-spacing: -0.3px;
}

h3, .text-h3 {
  font-size: 20px;      /* Títulos de sección */
  line-height: 1.4;
  font-weight: 600;
}

/* Cuerpo */
.text-body-1 {
  font-size: 16px;      /* Texto principal (nunca menor) */
  line-height: 1.5;
  font-weight: 400;
}

.text-body-2 {
  font-size: 14px;      /* Texto secundario */
  line-height: 1.5;
  font-weight: 400;
}

/* Botones y labels */
.text-button {
  font-size: 16px;      /* Botones grandes */
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.text-caption {
  font-size: 12px;      /* Captions y notas */
  line-height: 1.4;
  font-weight: 400;
  color: var(--text-secondary);
}

/* Números grandes (estado) */
.text-display {
  font-size: 36px;      /* Números de estado */
  line-height: 1;
  font-weight: 700;
  letter-spacing: -1px;
}
```

---

## 📱 Diseño de Pantallas Mobile

### 1. **Dashboard - Vista Principal** 🏠

#### Layout Mobile (360px)

```
┌─────────────────────────────────┐
│ ≡  Irrigación         🔔  ⚙️   │ ← Header (60px fijo)
├─────────────────────────────────┤
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 💧 Zona 1: Jardín Frontal  │ │ ← Card expandida
│ │                             │ │   (120px altura mínima)
│ │ ● ACTIVO - 5 min restantes │ │
│ │ ━━━━━━━━━━━━━━░░░░░░        │ │ ← Progress bar
│ │                             │ │
│ │ ┌─────────────┐ ┌─────────┐ │ │
│ │ │ 🚿 DETENER  │ │  INFO  │ │ │ ← Botones 48px altura
│ │ └─────────────┘ └─────────┘ │ │
│ └─────────────────────────────┘ │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 💧 Zona 2: Jardín Lateral   │ │ ← Card colapsada
│ │ ○ Inactivo                  │ │   (80px altura)
│ │ Próximo: Hoy 19:00 (15min) │ │
│ │ ┌────────┐                  │ │
│ │ │  REGAR │ [›]             │ │ ← Botón + expand
│ │ └────────┘                  │ │
│ └─────────────────────────────┘ │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 💧 Zona 3: Huerta           │ │
│ │ ○ Inactivo                  │ │
│ │ ┌────────┐                  │ │
│ │ │  REGAR │ [›]             │ │
│ │ └────────┘                  │ │
│ └─────────────────────────────┘ │
│                                 │
│ [Scroll para más zonas...]      │
│                                 │
├─────────────────────────────────┤
│  🏠    📅    📊    👤          │ ← Bottom Nav (56px)
│ Inicio Agenda Histórico Perfil │
└─────────────────────────────────┘
```

#### Interacciones Táctiles

**Zona Card - Estados:**
```css
/* Card normal */
.zone-card {
  background: var(--surface-1);
  border-radius: var(--radius-lg);
  padding: 16px;
  box-shadow: var(--shadow-1);
  transition: all var(--transition-normal);
  /* Tap area aumentada */
  min-height: 80px;
}

/* Card activa (riego en curso) */
.zone-card.active {
  border: 3px solid var(--zone-active);
  box-shadow: var(--shadow-2), 0 0 0 3px rgba(76, 175, 80, 0.1);
  /* Animación sutil de pulsación */
  animation: pulse 2s ease-in-out infinite;
}

@keyframes pulse {
  0%, 100% { 
    box-shadow: var(--shadow-2), 0 0 0 3px rgba(76, 175, 80, 0.1);
  }
  50% { 
    box-shadow: var(--shadow-2), 0 0 0 6px rgba(76, 175, 80, 0.2);
  }
}

/* Touch feedback */
.zone-card:active {
  transform: scale(0.98);
  box-shadow: var(--shadow-1);
}
```

**Botón "REGAR" - Hero Button:**
```css
.btn-regar {
  /* Botón táctil optimizado */
  min-height: 56px;
  min-width: 140px;
  background: linear-gradient(135deg, var(--water-blue) 0%, var(--water-blue-dark) 100%);
  color: var(--text-on-primary);
  border-radius: var(--radius-md);
  border: none;
  box-shadow: var(--shadow-2);
  
  /* Tipografía */
  font-size: 16px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
  
  /* Transición */
  transition: all var(--transition-fast);
  
  /* Feedback táctil */
  -webkit-tap-highlight-color: transparent;
}

.btn-regar:active {
  transform: scale(0.95);
  box-shadow: var(--shadow-1);
}

/* Variante "DETENER" */
.btn-detener {
  background: linear-gradient(135deg, var(--zone-error) 0%, #D32F2F 100%);
}
```

**Progress Bar (Riego activo):**
```css
.progress-bar {
  height: 8px;
  background: var(--surface-3);
  border-radius: 4px;
  overflow: hidden;
  margin: 12px 0;
}

.progress-bar-fill {
  height: 100%;
  background: linear-gradient(90deg, 
    var(--zone-active) 0%, 
    var(--plant-green-light) 100%
  );
  border-radius: 4px;
  transition: width var(--transition-normal);
  /* Animación de progreso */
  position: relative;
  overflow: hidden;
}

.progress-bar-fill::after {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(
    90deg,
    transparent 0%,
    rgba(255,255,255,0.3) 50%,
    transparent 100%
  );
  animation: shimmer 2s infinite;
}

@keyframes shimmer {
  0% { transform: translateX(-100%); }
  100% { transform: translateX(100%); }
}
```

---

### 2. **Control Manual - Bottom Sheet** 🎛️

Cuando el usuario toca "REGAR", aparece un bottom sheet:

```
┌─────────────────────────────────┐
│                                 │
│ [Contenido detrás, oscurecido] │
│                                 │
│ ╭───────────────────────────────╮
│ │ ─── (Handle para arrastrar)  │ │
│ │                               │ │
│ │ 🚿 Regar Zona 1               │ │
│ │ Jardín Frontal                │ │
│ │                               │ │
│ │ ⏱️ Duración                   │ │
│ │ ┌───────────────────────────┐ │ │
│ │ │   [  -  ]  15 min  [  +  ]│ │ │ ← Botones +/- grandes
│ │ └───────────────────────────┘ │ │   (48x48px cada uno)
│ │                               │ │
│ │ ┌───────────────────────────┐ │ │
│ │ │    5   15   30   60 min   │ │ │ ← Chips de tiempo rápido
│ │ └───────────────────────────┘ │ │
│ │                               │ │
│ │ ┌───────────────────────────┐ │ │
│ │ │  🚿 INICIAR RIEGO         │ │ │ ← Botón primario grande
│ │ └───────────────────────────┘ │ │   (56px altura)
│ │                               │ │
│ │ ┌───────────────────────────┐ │ │
│ │ │  CANCELAR                 │ │ │ ← Botón secundario
│ │ └───────────────────────────┘ │ │
│ ╰───────────────────────────────╯
└─────────────────────────────────┘
```

**Estilos del Bottom Sheet:**
```css
.bottom-sheet {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  background: var(--surface-1);
  border-radius: var(--radius-xl) var(--radius-xl) 0 0;
  box-shadow: 0 -4px 20px rgba(0,0,0,0.15);
  padding: 24px 20px 32px;
  
  /* Animación de entrada */
  animation: slideUp var(--transition-normal) ease-out;
}

@keyframes slideUp {
  from {
    transform: translateY(100%);
    opacity: 0;
  }
  to {
    transform: translateY(0);
    opacity: 1;
  }
}

/* Handle arrastrable */
.bottom-sheet-handle {
  width: 40px;
  height: 4px;
  background: var(--text-disabled);
  border-radius: 2px;
  margin: 0 auto 16px;
  cursor: grab;
}

/* Control de duración */
.duration-control {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 16px;
  margin: 20px 0;
}

.duration-button {
  width: 56px;
  height: 56px;
  border-radius: 50%;
  background: var(--surface-2);
  border: 2px solid var(--water-blue);
  color: var(--water-blue);
  font-size: 24px;
  font-weight: 600;
  transition: all var(--transition-fast);
}

.duration-button:active {
  transform: scale(0.9);
  background: var(--water-blue);
  color: white;
}

.duration-value {
  font-size: 36px;
  font-weight: 700;
  color: var(--text-primary);
  min-width: 100px;
  text-align: center;
}

/* Chips de tiempo rápido */
.time-chips {
  display: flex;
  gap: 8px;
  overflow-x: auto;
  -webkit-overflow-scrolling: touch;
  padding: 8px 0;
}

.time-chip {
  flex-shrink: 0;
  min-width: 60px;
  height: 40px;
  padding: 0 16px;
  background: var(--surface-2);
  border: 2px solid transparent;
  border-radius: var(--radius-md);
  font-size: 14px;
  font-weight: 600;
  color: var(--text-secondary);
  transition: all var(--transition-fast);
}

.time-chip.selected {
  background: var(--water-blue);
  color: white;
  border-color: var(--water-blue-dark);
}
```

---

### 3. **Gestión de Agendas - Mobile** 📅

#### Vista Lista de Agendas

```
┌─────────────────────────────────┐
│ ‹  Agendas - Zona 1          +  │ ← Header con back y add
├─────────────────────────────────┤
│                                 │
│ ┌─────────────────────────────┐ │
│ │ ✓ Riego Matutino            │ │ ← Agenda activa (verde)
│ │                             │ │
│ │ 🕐 LUN, MIE, VIE · 07:00   │ │
│ │ ⏱️ 15 minutos               │ │
│ │                             │ │
│ │ [✓ Activa] [Editar] [···]  │ │ ← Toggle + acciones
│ └─────────────────────────────┘ │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ ○ Riego Vespertino          │ │ ← Agenda inactiva (gris)
│ │                             │ │
│ │ 🕐 MAR, JUE, SAB · 19:00   │ │
│ │ ⏱️ 20 minutos               │ │
│ │                             │ │
│ │ [○ Inactiva] [Editar] [···]│ │
│ └─────────────────────────────┘ │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ + Agregar nueva agenda      │ │ ← Card para agregar
│ └─────────────────────────────┘ │
│                                 │
├─────────────────────────────────┤
│  🏠    📅    📊    👤          │
└─────────────────────────────────┘
```

#### Formulario de Agenda (Slide-in)

```
┌─────────────────────────────────┐
│ ‹ Nueva Agenda            [✓]   │ ← Header con back y guardar
├─────────────────────────────────┤
│                                 │
│ Nombre                          │
│ ┌───────────────────────────┐   │
│ │ Riego matutino            │   │ ← Input 56px altura
│ └───────────────────────────┘   │
│                                 │
│ Días de la semana              │
│ ┌───────────────────────────┐   │
│ │ [L] [M] [M] [J] [V] [S] [D]│  │ ← Chips seleccionables
│ └───────────────────────────┘   │   (48x48px cada uno)
│                                 │
│ Hora de inicio                 │
│ ┌───────────────────────────┐   │
│ │       07 : 00             │   │ ← Picker grande
│ │       ─   ─               │   │
│ └───────────────────────────┘   │
│                                 │
│ Duración                       │
│ ┌───────────────────────────┐   │
│ │  [ - ]  15 min  [ + ]     │   │ ← Control +/-
│ └───────────────────────────┘   │
│                                 │
│ ┌───────────────────────────┐   │
│ │ [✓] Agenda activa         │   │ ← Switch grande
│ └───────────────────────────┘   │
│                                 │
│ ┌───────────────────────────┐   │
│ │  💾 GUARDAR AGENDA        │   │ ← Botón primario (56px)
│ └───────────────────────────┘   │
│                                 │
│ [Cancelar]                      │ ← Link secundario
│                                 │
└─────────────────────────────────┘
```

**Selector de Días (Componente especial):**
```css
.day-selector {
  display: flex;
  justify-content: space-between;
  gap: 4px;
  margin: 12px 0;
}

.day-chip {
  flex: 1;
  min-width: 44px;
  height: 56px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: var(--surface-2);
  border: 2px solid var(--surface-3);
  border-radius: var(--radius-md);
  transition: all var(--transition-fast);
  cursor: pointer;
  -webkit-tap-highlight-color: transparent;
}

.day-chip-letter {
  font-size: 18px;
  font-weight: 700;
  color: var(--text-secondary);
}

.day-chip-name {
  font-size: 10px;
  font-weight: 500;
  color: var(--text-disabled);
  margin-top: 2px;
}

/* Estado seleccionado */
.day-chip.selected {
  background: var(--water-blue);
  border-color: var(--water-blue-dark);
  box-shadow: var(--shadow-1);
}

.day-chip.selected .day-chip-letter,
.day-chip.selected .day-chip-name {
  color: white;
}

/* Feedback táctil */
.day-chip:active {
  transform: scale(0.92);
}
```

---

### 4. **Histórico - Mobile** 📊

```
┌─────────────────────────────────┐
│ ‹ Histórico              [🔍]   │
├─────────────────────────────────┤
│ Filtros activos: Todos · 7 días│ ← Chip de filtros
├─────────────────────────────────┤
│                                 │
│ Hoy, 14 Dic                     │ ← Agrupación por día
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 07:00 · Zona 1 · 15min      │ │
│ │ 📅 Agenda: Riego matutino   │ │
│ │ ✓ Completado                │ │
│ └─────────────────────────────┘ │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 14:30 · Zona 2 · 5min       │ │
│ │ 🎯 Manual                    │ │
│ │ ✓ Completado                │ │
│ └─────────────────────────────┘ │
│                                 │
│ Ayer, 13 Dic                    │
│                                 │
│ ┌─────────────────────────────┐ │
│ │ 19:00 · Zona 1 · 20min      │ │
│ │ 📅 Agenda: Riego vespertino │ │
│ │ ✓ Completado                │ │
│ └─────────────────────────────┘ │
│                                 │
│ [Cargar más...]                 │
│                                 │
├─────────────────────────────────┤
│  🏠    📅    📊    👤          │
└─────────────────────────────────┘
```

**Filtros (Bottom Sheet):**
```
Cuando el usuario toca [🔍]:

╭───────────────────────────────╮
│ Filtros                       │
│                               │
│ Zona                          │
│ ○ Todas  ○ 1  ○ 2  ○ 3  ○ 4 │
│                               │
│ Período                       │
│ [Hoy] [7 días] [30 días]     │
│ [Personalizado]               │
│                               │
│ Tipo                          │
│ ☑ Agenda   ☑ Manual           │
│                               │
│ [APLICAR FILTROS]             │
╰───────────────────────────────╯
```

---

## 🎭 Animaciones y Micro-interacciones

### Principios de Animación Mobile

```css
/* === TIMING FUNCTIONS === */
:root {
  /* Material Design easing */
  --ease-standard: cubic-bezier(0.4, 0.0, 0.2, 1);
  --ease-decelerate: cubic-bezier(0.0, 0.0, 0.2, 1);
  --ease-accelerate: cubic-bezier(0.4, 0.0, 1, 1);
  --ease-bounce: cubic-bezier(0.68, -0.55, 0.265, 1.55);
}

/* === ANIMACIONES CLAVE === */

/* 1. Tap feedback (crítico para mobile) */
.interactive-element {
  transition: transform var(--transition-fast) var(--ease-standard);
}

.interactive-element:active {
  transform: scale(0.95);
}

/* 2. Loading (riego en proceso) */
@keyframes rotate {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}

.loading-icon {
  animation: rotate 1s linear infinite;
}

/* 3. Success feedback (comando enviado) */
@keyframes success-scale {
  0% { transform: scale(1); }
  50% { transform: scale(1.1); }
  100% { transform: scale(1); }
}

.success-feedback {
  animation: success-scale 0.3s var(--ease-bounce);
}

/* 4. Fade in cards (al cargar) */
@keyframes fadeInUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.zone-card {
  animation: fadeInUp var(--transition-normal) var(--ease-decelerate);
  /* Stagger delay para múltiples cards */
  animation-fill-mode: both;
}

.zone-card:nth-child(1) { animation-delay: 0ms; }
.zone-card:nth-child(2) { animation-delay: 50ms; }
.zone-card:nth-child(3) { animation-delay: 100ms; }
.zone-card:nth-child(4) { animation-delay: 150ms; }

/* 5. Ripple effect (Material Design) */
.btn-ripple {
  position: relative;
  overflow: hidden;
}

.btn-ripple::after {
  content: '';
  position: absolute;
  top: 50%;
  left: 50%;
  width: 0;
  height: 0;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.5);
  transform: translate(-50%, -50%);
  transition: width 0.6s, height 0.6s;
}

.btn-ripple:active::after {
  width: 300px;
  height: 300px;
}
```

### Transiciones de Página

```css
/* Vue Router transitions */
.page-enter-active,
.page-leave-active {
  transition: all var(--transition-normal) var(--ease-standard);
}

/* Slide left (forward) */
.page-enter-from {
  opacity: 0;
  transform: translateX(20px);
}

.page-leave-to {
  opacity: 0;
  transform: translateX(-20px);
}

/* Bottom sheet (modal) */
.modal-enter-active,
.modal-leave-active {
  transition: all var(--transition-normal) var(--ease-standard);
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
  transform: translateY(100%);
}

/* Backdrop */
.backdrop-enter-active,
.backdrop-leave-active {
  transition: opacity var(--transition-normal);
}

.backdrop-enter-from,
.backdrop-leave-to {
  opacity: 0;
}
```

---

## 📐 Componentes UI Específicos

### Bottom Navigation (Crítico)

```css
.bottom-nav {
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  height: 56px;
  background: var(--surface-1);
  border-top: 1px solid var(--surface-3);
  display: flex;
  justify-content: space-around;
  align-items: center;
  padding: 0 8px;
  z-index: 100;
  /* Safe area for notched phones */
  padding-bottom: env(safe-area-inset-bottom);
}

.bottom-nav-item {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 4px;
  padding: 8px 12px;
  border-radius: var(--radius-sm);
  color: var(--text-secondary);
  text-decoration: none;
  transition: all var(--transition-fast);
  /* Área táctil aumentada */
  position: relative;
}

.bottom-nav-item::before {
  content: '';
  position: absolute;
  top: -8px;
  bottom: -8px;
  left: -8px;
  right: -8px;
}

.bottom-nav-icon {
  font-size: 24px;
  transition: transform var(--transition-fast);
}

.bottom-nav-label {
  font-size: 11px;
  font-weight: 500;
}

/* Estado activo */
.bottom-nav-item.active {
  color: var(--water-blue);
}

.bottom-nav-item.active .bottom-nav-icon {
  transform: scale(1.1);
}

/* Feedback táctil */
.bottom-nav-item:active {
  background: var(--surface-2);
}
```

### Toast Notifications (Mobile)

```css
.toast {
  position: fixed;
  top: 16px;
  left: 16px;
  right: 16px;
  background: var(--text-primary);
  color: white;
  padding: 16px 20px;
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-3);
  display: flex;
  align-items: center;
  gap: 12px;
  z-index: 1000;
  animation: toastSlideDown 0.3s var(--ease-decelerate);
}

@keyframes toastSlideDown {
  from {
    opacity: 0;
    transform: translateY(-100%);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.toast-success {
  background: var(--zone-active);
}

.toast-error {
  background: var(--zone-error);
}

.toast-icon {
  font-size: 24px;
  flex-shrink: 0;
}

.toast-message {
  flex: 1;
  font-size: 14px;
  font-weight: 500;
}

.toast-close {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  background: rgba(255, 255, 255, 0.1);
  border: none;
  color: white;
  font-size: 20px;
  cursor: pointer;
}
```

### Pull to Refresh

```css
.pull-refresh-indicator {
  position: absolute;
  top: -60px;
  left: 50%;
  transform: translateX(-50%);
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.3s;
}

.pulling .pull-refresh-indicator {
  top: 20px;
}

.pull-refresh-spinner {
  width: 24px;
  height: 24px;
  border: 3px solid var(--surface-3);
  border-top-color: var(--water-blue);
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}
```

---

## 🌙 Modo Oscuro (Opcional Post-MVP)

```css
/* Dark mode toggle */
@media (prefers-color-scheme: dark) {
  :root {
    --surface-1: #1E1E1E;
    --surface-2: #121212;
    --surface-3: #2C2C2C;
    --text-primary: #FFFFFF;
    --text-secondary: #B0B0B0;
    --text-disabled: #6B6B6B;
    
    /* Ajustar sombras para dark mode */
    --shadow-1: 0 2px 4px rgba(0,0,0,0.3);
    --shadow-2: 0 4px 8px rgba(0,0,0,0.4);
    --shadow-3: 0 8px 16px rgba(0,0,0,0.5);
  }
}
```

---

## 📱 Consideraciones de Hardware

### Notch / Safe Areas (iPhone X+)

```css
/* Respetar áreas seguras */
.app-container {
  padding-top: env(safe-area-inset-top);
  padding-bottom: env(safe-area-inset-bottom);
  padding-left: env(safe-area-inset-left);
  padding-right: env(safe-area-inset-right);
}

/* Header fijo */
.app-header {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  padding-top: env(safe-area-inset-top);
  background: var(--surface-1);
}

/* Bottom nav fijo */
.bottom-nav {
  padding-bottom: calc(8px + env(safe-area-inset-bottom));
}
```

### Performance Mobile

```css
/* Usar will-change para animaciones críticas */
.zone-card {
  will-change: transform;
}

/* Optimizar re-renders */
.static-content {
  content-visibility: auto;
}

/* Reducir repaints */
* {
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

/* Hardware acceleration */
.animated-element {
  transform: translateZ(0);
  backface-visibility: hidden;
}
```

---

## 🎨 Mockups Visuales Finales

### Flujo Principal (Happy Path)

```
1. ABRIR APP
   ↓
   [Splash Screen con logo de gota]
   ↓
2. DASHBOARD
   - Ver 4 zonas
   - Zona 1 ACTIVA (verde, progress bar)
   - Zonas 2,3,4 INACTIVAS (gris)
   ↓
3. TAP en Zona 2 "REGAR"
   ↓
   [Bottom sheet aparece desde abajo]
   - Selector de tiempo
   - Botón grande "INICIAR RIEGO"
   ↓
4. TAP "INICIAR RIEGO"
   ↓
   [Toast verde: "Riego iniciado en Zona 2"]
   [Card de Zona 2 cambia a verde con progress bar]
   ↓
5. TIEMPO REAL
   - Progress bar avanza
   - Contador regresivo actualiza
   ↓
6. COMPLETADO
   [Toast: "Riego completado en Zona 2"]
   [Card vuelve a gris]
```

---

## ✅ Checklist de Implementación Mobile

### Must Have (MVP)
- [ ] Bottom navigation fija y funcional
- [ ] Cards de zona con estados visuales claros
- [ ] Botones táctiles mínimo 48x48px
- [ ] Bottom sheet para control manual
- [ ] Progress bars animados para riego activo
- [ ] Toast notifications
- [ ] Loading states
- [ ] Safe area insets para notch
- [ ] Scroll suave y natural
- [ ] Feedback táctil en todas las interacciones

### Should Have (MVP+)
- [ ] Pull to refresh
- [ ] Animaciones de transición
- [ ] Selector de días táctil optimizado
- [ ] Time picker grande y fácil de usar
- [ ] Swipe gestures (opcional)

### Nice to Have (Post-MVP)
- [ ] Modo oscuro
- [ ] Haptic feedback (vibración)
- [ ] Gestos avanzados (swipe to delete)
- [ ] Animaciones complejas
- [ ] Skeleton loaders

---

**Diseñado para:** Teléfonos como dispositivo principal  
**Optimizado para:** Experiencia táctil intuitiva  
**Principio:** Una mano, pulgar dominante, acciones rápidas
