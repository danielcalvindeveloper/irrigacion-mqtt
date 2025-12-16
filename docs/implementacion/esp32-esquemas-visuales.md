# 🔌 Esquemas de Conexión ESP32 - Sistema de Riego

> **Documento Visual**: Diagramas de conexión listos para usar  
> **Última actualización**: 2025-12-16

---

## 📋 Índice
1. [Vista General del Sistema](#vista-general-del-sistema)
2. [Conexiones ESP32 → Módulo de Relés](#conexiones-esp32--módulo-de-relés)
3. [Conexiones ESP32 → Sensores de Humedad](#conexiones-esp32--sensores-de-humedad)
4. [Conexiones Relés → Electroválvulas](#conexiones-relés--electroválvulas)
5. [Alimentación Eléctrica](#alimentación-eléctrica)
6. [Layout en Protoboard](#layout-en-protoboard)
7. [Checklist de Verificación](#checklist-de-verificación)

---

## Vista General del Sistema

```
                        ╔════════════════════╗
                        ║  Fuente 5V 3A DC   ║
                        ╚═════╤══════╤═══════╝
                              │      │
                    ┌─────────┘      └─────────┐
                    │                           │
         ╔══════════▼═══════════╗    ╔═════════▼══════════╗
         ║      ESP32 NodeMCU   ║    ║  Módulo Relés 8CH  ║
         ║     (38 pines)       ║    ║   (Lógica BAJA)    ║
         ╚══╤═══════════════╤═══╝    ╚═╤═╤═╤═╤═╤═╤═╤═╤═══╝
            │               │           │ │ │ │ │ │ │ │
            │    8x GPIO    ├───────────┴─┴─┴─┴─┴─┴─┴─┘
            │    Control    │         Señales IN1-IN8
            │               │
            │    6x ADC     ├───┐
            │    Sensores   │   │
            │               │   │
         ╔══▼═══════════════▼═══╝
         ║  6x Sensores         
         ║  Humedad Capacitivos 
         ╚══════════════════════╝
         
                           ┌──────────────────────────────┐
                           │  8x Electroválvulas 24V AC   │
                           │  (Conectadas a relés COM/NO) │
                           └──────────────────────────────┘
```

---

## Conexiones ESP32 → Módulo de Relés

### Tabla de Conexiones

| ESP32 Pin | GPIO | Módulo Relé | Zona | Función | Color Cable |
|-----------|------|-------------|------|---------|-------------|
| D4        | 4    | IN1         | 1    | Control Relé 1 | 🟠 Naranja |
| D5        | 5    | IN2         | 2    | Control Relé 2 | 🟡 Amarillo |
| D13       | 13   | IN3         | 3    | Control Relé 3 | 🟢 Verde |
| D14       | 14   | IN4         | 4    | Control Relé 4 | 🔵 Azul |
| D15       | 15   | IN5         | 5    | Control Relé 5 | 🟣 Violeta |
| D16       | 16   | IN6         | 6    | Control Relé 6 | ⚫ Gris |
| D17       | 17   | IN7         | 7    | Control Relé 7 | ⚪ Blanco |
| D18       | 18   | IN8         | 8    | Control Relé 8 | 🟤 Marrón |
| GND       | -    | GND         | -    | Tierra común   | ⚫ Negro |

### Diagrama de Conexión Detallado

```
ESP32 NodeMCU                    Módulo Relés 8CH
┌─────────────┐                 ┌──────────────────┐
│             │                 │  VCC ← 5V (fuente externa)
│             │                 │  GND ← GND común  │
│             │                 │                   │
│  GPIO4  ────┼─────────────────┼→ IN1 (Zona 1)    │
│  GPIO5  ────┼─────────────────┼→ IN2 (Zona 2)    │
│  GPIO13 ────┼─────────────────┼→ IN3 (Zona 3)    │
│  GPIO14 ────┼─────────────────┼→ IN4 (Zona 4)    │
│  GPIO15 ────┼─────────────────┼→ IN5 (Zona 5)    │
│  GPIO16 ────┼─────────────────┼→ IN6 (Zona 6)    │
│  GPIO17 ────┼─────────────────┼→ IN7 (Zona 7)    │
│  GPIO18 ────┼─────────────────┼→ IN8 (Zona 8)    │
│             │                 │                   │
│  GND    ────┼─────────────────┼→ GND              │
│             │                 │                   │
└─────────────┘                 └──────────────────┘
```

### ⚠️ Importante: Lógica Invertida

Los módulos de relés típicamente usan **lógica ACTIVO BAJO**:

```cpp
// En el código:
digitalWrite(pin, LOW);   // ← Relé ON (electroválvula abierta)
digitalWrite(pin, HIGH);  // ← Relé OFF (electroválvula cerrada)
```

Indicador visual: El LED del módulo de relés se **enciende** cuando el relé está activo.

---

## Conexiones ESP32 → Sensores de Humedad

### Tabla de Conexiones

| Sensor | ESP32 Pin | GPIO | ADC | VCC | GND | Notas |
|--------|-----------|------|-----|-----|-----|-------|
| Zona 1 | D32       | 32   | ADC1_CH4 | 3.3V | GND | ✅ Puede usar pull-up/down |
| Zona 2 | D33       | 33   | ADC1_CH5 | 3.3V | GND | ✅ Puede usar pull-up/down |
| Zona 3 | D34       | 34   | ADC1_CH6 | 3.3V | GND | ⚠️ Solo INPUT (no pull-up) |
| Zona 4 | D35       | 35   | ADC1_CH7 | 3.3V | GND | ⚠️ Solo INPUT (no pull-up) |
| Zona 5 | D36       | 36   | ADC1_CH0 | 3.3V | GND | ⚠️ Solo INPUT (no pull-up) |
| Zona 6 | D39       | 39   | ADC1_CH3 | 3.3V | GND | ⚠️ Solo INPUT (no pull-up) |

### Diagrama de Conexión por Sensor

```
Sensor Capacitivo v2.0          ESP32
┌────────────────┐             ┌─────────────┐
│                │             │             │
│  VCC  ─────────┼─────────────┼→ 3.3V       │
│  GND  ─────────┼─────────────┼→ GND        │
│  AOUT ─────────┼─────────────┼→ GPIO32-39  │
│                │             │   (según    │
└────────────────┘             │    zona)    │
                               └─────────────┘
```

### Calibración de Sensores

Cada sensor debe calibrarse individualmente:

1. **En aire seco** → Anotar valor ADC (típico: 2500-3500)
2. **En agua** → Anotar valor ADC (típico: 300-800)
3. **Actualizar** `Config.h`:

```cpp
// Ejemplo con sensor específico:
#define SENSOR_DRY_VALUE 3000  // Tu lectura en aire
#define SENSOR_WET_VALUE 700   // Tu lectura en agua
```

### Conexión Múltiple de Sensores

```
           3.3V Rail ──┬──┬──┬──┬──┬── (Power común)
                       │  │  │  │  │
                      ┌▼┐┌▼┐┌▼┐┌▼┐┌▼┐
                      │S││S││S││S││S│  ← Sensores 1-5
                      │1││2││3││4││5│
                      └┬┘└┬┘└┬┘└┬┘└┬┘
                       │  │  │  │  │
ESP32: GPIO32 ────────┘  │  │  │  │
       GPIO33 ───────────┘  │  │  │
       GPIO34 ──────────────┘  │  │
       GPIO35 ─────────────────┘  │
       GPIO36 ────────────────────┘
                       │  │  │  │  │
           GND Rail ──┴──┴──┴──┴──┴── (Tierra común)
```

---

## Conexiones Relés → Electroválvulas

### Esquema por Zona (x8)

```
Fuente 24V AC                 Relé Zona N              Electroválvula
┌──────────┐                 ┌─────────────┐          ┌──────────┐
│          │                 │             │          │          │
│  Fase ~  ├─────────────────┤ COM         │          │  Term. L │
│          │                 │             │          │          │
│          │                 │      NO ────┼──────────┤  Term. N │
│          │                 │             │          │          │
│  Neutro ~├─────┐           │     NC (X)  │          └──────────┘
│          │     │           └─────────────┘
└──────────┘     │
                 └──────────────────────────────────────┘
                           Retorno de neutro
```

### Tabla de Terminales del Relé

| Terminal | Conexión | Estado Relé OFF | Estado Relé ON |
|----------|----------|-----------------|----------------|
| COM      | Neutro 24V AC | - | Conduce a NO |
| NO (Normalmente Abierto) | Terminal N válvula | Abierto | Cerrado (pasa corriente) |
| NC (Normalmente Cerrado) | No usar | Cerrado | Abierto |

### ⚠️ Seguridad Eléctrica

```
╔════════════════════════════════════════════════════════╗
║  ⚡ ADVERTENCIA - ALTA TENSIÓN (24V AC / 220V AC)     ║
╠════════════════════════════════════════════════════════╣
║  ❌ NO tocar con sistema energizado                    ║
║  ✅ Usar cable apropiado para 220V AC                  ║
║  ✅ Instalar disyuntor diferencial                     ║
║  ✅ Aislar todas las conexiones                        ║
║  ✅ Mantener separado de circuitos de baja tensión     ║
║  ⚠️  Si no tienes experiencia, consultar electricista  ║
╚════════════════════════════════════════════════════════╝
```

### Diagrama Completo de 2 Zonas (ejemplo)

```
          24V AC Source
             │    │
             F    N
             │    │
    ┌────────┴────┴────────┐
    │                      │
    │  ┌──────────┐    ┌──────────┐
    └──┤ COM  (R1)│    │ COM  (R2)│
       │          │    │          │
       │ NO       ├────┤ Válvula 1├── N
       │ NC   (X) │    └──────────┘
       └──────────┘    
                       ┌──────────┐
       ┌──────────┐    │          │
       │ COM  (R2)├────┤ Válvula 2├── N
       │ NO       │    └──────────┘
       │ NC   (X) │
       └──────────┘
              │
              └──────────────────────┘
                  Retorno neutro
```

---

## Alimentación Eléctrica

### Diagrama de Distribución de Potencia

```
                    ┌──────────────────────┐
                    │  Fuente 5V DC 3A     │
                    │  (Switching)         │
                    └────┬────────┬────────┘
                         │ +5V    │ GND
                         │        │
          ┌──────────────┼────────┼──────────────┐
          │              │        │              │
          │              │        │              │
     ┌────▼────┐    ┌────▼────┐  │         ┌────▼────┐
     │  ESP32  │    │ Módulo  │  │         │ Sensores│
     │ NodeMCU │    │ Relés   │  │         │ (3.3V)  │
     │         │    │         │  │         │         │
     │  VIN    │    │  VCC    │  │         │  VCC ───┼─┐
     │  (5V)   │    │  (5V)   │  │         │         │ │
     │         │    │         │  │         │         │ │
     │  3.3V───┼────┼─────────┼──┼─────────┤  (Rail) │ │
     │  (OUT)  │    │         │  │         └─────────┘ │
     │         │    │         │  │                     │
     │  GND ───┼────┼─────────┼──┼─────────────────────┤
     │         │    │  GND    │  │         GND (común) │
     └─────────┘    └─────────┘  │                     │
                                  │                     │
                         GND ─────┴─────────────────────┘
                          (Tierra común - MUY IMPORTANTE)
```

### Tabla de Consumos

| Componente | Voltaje | Corriente | Potencia |
|------------|---------|-----------|----------|
| ESP32 (WiFi activo) | 5V | ~250mA | 1.25W |
| Módulo relés (8 canales) | 5V | ~15mA x 8 | 0.6W |
| Sensores de humedad | 3.3V | ~5mA x 6 | 0.1W |
| **Total estimado** | - | **~370mA** | **~2W** |
| **Fuente recomendada** | 5V | **3A mínimo** | 15W |

### ⚠️ Advertencias de Alimentación

```
┌─────────────────────────────────────────────────┐
│  ❌ NO alimentar módulo de relés desde pin 5V   │
│     del ESP32 (corriente insuficiente)          │
│                                                  │
│  ✅ Usar fuente externa 5V 3A para relés        │
│  ✅ Conectar GND común entre todos componentes  │
│  ✅ ESP32 puede alimentarse por USB (testing)   │
│     o desde fuente 5V por pin VIN (producción) │
└─────────────────────────────────────────────────┘
```

---

## Layout en Protoboard

### Vista Superior del Protoboard (Testing)

```
   Protoboard 830 puntos
   ═══════════════════════════════════════════════════════
   
   Rail +5V ──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──┬──
              │  │  │  │  │  │  │  │  │  │  │  │  │  │
   ┌──────────┴──▼──────────────────────────────────────┐
   │                                                     │
   │  ┌─────────────┐     ┌──────────────┐             │
   │  │  ESP32      │     │ Módulo       │             │
   │  │  NodeMCU    │     │ Relés 8CH    │             │
   │  │  (38 pins)  │     │ ┌───┬───┬──┐ │             │
   │  │             │     │ │ 1 │ 2 │..│ │             │
   │  │ GPIO4─────GPIO18  │ │LED│LED│  │ │             │
   │  │   │         │     │ └───┴───┴──┘ │             │
   │  └───┼─────────┼─────┘ IN1-IN8      │             │
   │      │         │       │  │  │      │             │
   │      └─────────┼───────┘  │  │      │             │
   │                │          │  │      └─────────────┘
   │                │          │  │
   │  ┌─────┐ ┌─────┐ ┌─────┐ │  │      (Sensores en
   │  │ S1  │ │ S2  │ │ S3  │ │  │       área separada)
   │  │ Hum │ │ Hum │ │ Hum │ │  │
   │  └──┬──┘ └──┬──┘ └──┬──┘ │  │
   │     │       │       │    │  │
   │  GPIO32  GPIO33  GPIO34  │  │
   │                           │  │
   └───────────────────────────┼──┼──────────────────────┘
              │  │  │  │  │  │  │  │  │  │  │  │  │  │
   Rail GND ──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──
   ═══════════════════════════════════════════════════════
   
   Leyenda:
   • Rail superior: +5V desde fuente
   • Rail inferior: GND común
   • Área izquierda: ESP32 + sensores
   • Área derecha: Módulo de relés
```

### Conexiones de Cables en Protoboard

| Origen | Destino | Color | Longitud |
|--------|---------|-------|----------|
| Fuente 5V (+) | Rail +5V | Rojo | - |
| Fuente 5V (-) | Rail GND | Negro | - |
| ESP32 VIN | Rail +5V | Rojo | 5cm |
| ESP32 GND | Rail GND | Negro | 5cm |
| ESP32 3.3V | Sensores VCC | Rojo | 10cm |
| Relés VCC | Rail +5V | Rojo | 10cm |
| Relés GND | Rail GND | Negro | 10cm |
| GPIO4-18 | Relés IN1-8 | Colores | 15cm |

---

## Checklist de Verificación

### Paso 1: Verificación Visual (SIN ENERGIZAR)

```
[ ] Módulo de relés conectado a fuente externa (no a ESP32)
[ ] GND común entre TODOS los componentes
[ ] Cables de señal GPIO sin cortocircuitos
[ ] Sensores conectados a 3.3V (no 5V)
[ ] GPIO34-39 sin pull-ups externos (solo INPUT)
[ ] No hay cables pelados expuestos
```

### Paso 2: Verificación con Multímetro (SIN CARGA)

```
[ ] Fuente 5V: Medir entre VCC y GND → ~5V DC
[ ] ESP32 3.3V pin: Entre 3.3V y GND → ~3.3V
[ ] Relés VCC: Entre VCC relé y GND → ~5V
[ ] Continuidad GND: Todos los GND están conectados
[ ] No hay cortocircuitos entre pines adyacentes
```

### Paso 3: Test de Software

```
[ ] Subir sketch de test (Blink LED)
[ ] LED GPIO2 parpadea → ESP32 OK
[ ] Serial monitor muestra logs → Comunicación OK
[ ] Subir sketch test relés (secuencial ON/OFF)
[ ] Cada LED de relé enciende en secuencia
[ ] Medir continuidad COM-NO cuando relé ON
```

### Paso 4: Test de Sensores

```
[ ] Leer ADC de cada sensor (aire) → >2000
[ ] Sumergir sensor en agua → <1000
[ ] Valores estables (sin fluctuación >50)
[ ] Todos los sensores responden correctamente
```

### Paso 5: Test con Electroválvulas

```
⚠️  ATENCIÓN: Alta tensión desde este punto

[ ] Verificar instalación eléctrica con electricista
[ ] Conectar UNA válvula de prueba
[ ] Activar relé correspondiente
[ ] Verificar apertura de válvula (flujo de agua)
[ ] Desactivar relé → Válvula cierra
[ ] Repetir para todas las zonas
[ ] Verificar que NO haya fugas
```

---

## 🔧 Troubleshooting Visual

### Problema: Relé No Activa

```
Diagnóstico:

1. ¿LED del relé enciende?
   NO → Verificar señal GPIO (medir voltaje en IN)
   SÍ → Continuar paso 2

2. ¿Hay continuidad COM-NO?
   NO → Relé defectuoso (reemplazar)
   SÍ → Continuar paso 3

3. ¿Voltaje en COM del relé?
   NO → Verificar fuente 24V AC
   SÍ → Problema en válvula o cableado
```

### Problema: Sensor Lectura Errática

```
Diagnóstico:

            Lectura estable?
                  │
         ┌────────┴────────┐
        NO                 SÍ
         │                  │
    ¿Oscila mucho?     Calibrado?
         │                  │
    ┌────┴────┐        ┌───┴───┐
   SÍ         NO       SÍ      NO
    │          │        │       │
 Alejar de   GND     Sensor    Calibrar
 cables AC   común?   OK!      valores
             flojo
```

---

## 📸 Referencias Visuales Adicionales

### Pinout ESP32 NodeMCU

```
                        ┌─────┐
                        │ USB │
                        └──┬──┘
    EN ────○  1      30  ○──── VP (GPIO36) ADC
    VP ────○  2      29  ○──── VN (GPIO39) ADC
    VN ────○  3      28  ○──── GPIO34 ADC (INPUT only)
   GPIO35──○  4      27  ○──── GPIO35 ADC (INPUT only)
   GPIO32──○  5      26  ○──── GPIO32 ADC
   GPIO33──○  6      25  ○──── GPIO33 ADC
   GPIO25──○  7      24  ○──── GPIO27
   GPIO26──○  8      23  ○──── GPIO14 ← RELÉ 4
   GPIO27──○  9      22  ○──── GPIO12
   GPIO14──○ 10      21  ○──── GPIO13 ← RELÉ 3
   GPIO12──○ 11      20  ○──── GND
   GPIO13──○ 12      19  ○──── VIN (5V)
    GND ───○ 13      18  ○──── GPIO23
    VIN ───○ 14      17  ○──── GPIO22
   GPIO23──○ 15      16  ○──── TX0
   GPIO22──○ 16      15  ○──── RX0
    TX0 ───○ 17      14  ○──── GPIO21
    RX0 ───○ 18      13  ○──── GPIO19
   GPIO21──○ 19      12  ○──── GPIO18 ← RELÉ 8
   GPIO19──○ 20      11  ○──── GPIO5  ← RELÉ 2
   GPIO18──○ 21      10  ○──── GPIO17 ← RELÉ 7
   GPIO5 ──○ 22       9  ○──── GPIO16 ← RELÉ 6
   GPIO17──○ 23       8  ○──── GPIO4  ← RELÉ 1
   GPIO16──○ 24       7  ○──── GPIO0
   GPIO4 ──○ 25       6  ○──── GPIO2  (LED)
   GPIO0 ──○ 26       5  ○──── GPIO15 ← RELÉ 5
   GPIO2 ──○ 27       4  ○──── GND
   GPIO15──○ 28       3  ○──── 3.3V
    GND ───○ 29       2  ○──── GND
    3.3V──○ 30       1  ○──── EN
                        │
                     ─────
```

---

## 📦 Lista de Compras con Links

### Componentes Electrónicos

| #  | Componente | Cantidad | Precio Aprox | Dónde Comprar |
|----|------------|----------|--------------|---------------|
| 1  | ESP32 NodeMCU DevKit v1 | 1 | $8-12 USD | AliExpress / Amazon |
| 2  | Módulo relé 8 canales 5V | 1 | $10-15 USD | AliExpress / Amazon |
| 3  | Sensor humedad capacitivo v2 | 6-8 | $2-3 c/u | Pack x10 en AliExpress |
| 4  | Fuente 5V 3A switching | 1 | $5-8 USD | Tienda electrónica |
| 5  | Cables Dupont M-F 20cm | 40 | $3-5 USD | Kit en AliExpress |
| 6  | Cables Dupont M-M 20cm | 40 | $3-5 USD | Kit en AliExpress |
| 7  | Protoboard 830 puntos | 1 | $3-5 USD | Tienda electrónica |
| 8  | Borneras tornillo 2P | 10 | $0.50 c/u | Tienda electrónica |
| 9  | Borneras tornillo 3P | 10 | $0.70 c/u | Tienda electrónica |

### Instalación Permanente

| #  | Componente | Cantidad | Precio Aprox | Dónde Comprar |
|----|------------|----------|--------------|---------------|
| 10 | Caja estanca IP65 | 1 | $15-20 USD | Ferretería / Online |
| 11 | Cable eléctrico 2x0.75mm | 50m | $15-20 USD | Ferretería |
| 12 | Prensaestopas PG7 | 8-10 | $1 c/u | Tienda electrónica |
| 13 | Electroválvulas 24V AC 1/2" | 8 | $8-12 c/u | Proveedor riego |
| 14 | Transformador 220V→24V AC | 1 | $15-20 USD | Tienda electrónica |

**Costo Total Estimado**: ~$150-200 USD (componentes electrónicos + instalación básica)

---

## 📝 Notas Finales

### Para Producción

1. **PCB Custom**: Considera diseñar una PCB en lugar de protoboard
2. **Conectores**: Usar conectores JST o similares para facilitar mantenimiento
3. **Fusibles**: Agregar fusibles en alimentación 5V y 24V AC
4. **Ventilación**: La caja necesita ventilación para evitar condensación
5. **Sellado**: Usar silicona/goma para sellar entradas de cable

### Herramientas Necesarias

- Multímetro digital
- Soldador + estaño (para PCB)
- Destornilladores (plano y Phillips)
- Pela cables / alicate de corte
- Taladro (para caja estanca)
- Pistola de silicona

---

**Última actualización**: 2025-12-16  
**Próximo documento**: [esp32-desarrollo.md](./esp32-desarrollo.md) para implementación del firmware
