# ESP32 - Nodo de riego

Este directorio contiene todo lo necesario para el desarrollo del nodo ESP32 de riego.

---

## 📂 Estructura

```
esp32/
├── README.md              # Este archivo
├── mock_esp32.py          # Simulador Python para testing
├── requirements.txt       # Dependencias Python
├── run-mock.bat          # Script para ejecutar mock
└── firmware/             # ⭐ FIRMWARE REAL ESP32
    ├── platformio.ini    # Configuración PlatformIO
    ├── README.md         # Guía del firmware
    └── src/              # Código fuente C++
```

---

## 🚀 Quick Start

### Opción 1: Firmware Real (Hardware ESP32)

**Documentación completa**: [`../docs/implementacion/esp32-indice.md`](../docs/implementacion/esp32-indice.md)

```bash
cd firmware
cp src/config/Secrets.h.example src/config/Secrets.h
# Editar Secrets.h con tus credenciales
pio run --target upload && pio device monitor
```

**Leer primero**: [`../docs/implementacion/esp32-desarrollo.md`](../docs/implementacion/esp32-desarrollo.md)

### Opción 2: Mock Python (Testing sin hardware)

Para testing sin hardware real, usa el simulador Python que emula un ESP32.

#### Requisitos
- Python 3.7+
- pip

#### Instalación
```powershell
cd esp32
pip install -r requirements.txt
```

#### Uso básico

1. Asegúrate de que el stack Docker esté corriendo:
   ```powershell
   docker-compose up -d
   ```

2. Ejecuta el mock con un nodeId:
   ```powershell
   python mock_esp32.py --node-id 550e8400-e29b-41d4-a716-446655440000
   ```

3. El mock mostrará la agenda recibida en consola

#### Opciones del mock
```
--node-id       UUID del nodo (requerido)
--mqtt-host     Host del broker (default: localhost)
--mqtt-port     Puerto del broker (default: 1883)
```

#### Qué hace el mock
- ✅ Se conecta al broker MQTT (HiveMQ)
- ✅ Se suscribe a `riego/{nodeId}/agenda/sync`
- ✅ Se suscribe a `riego/{nodeId}/cmd/zona/+`
- ✅ Muestra en consola las agendas recibidas
- ✅ Simula la ejecución de comandos manuales
- ✅ Valida versiones de agenda (ignora duplicados)

---

## 📚 Documentación

### 🎯 Índice Principal
**[`../docs/implementacion/esp32-indice.md`](../docs/implementacion/esp32-indice.md)**

Punto de entrada a toda la documentación de desarrollo ESP32.

### 📖 Guías Detalladas

1. **Guía de Desarrollo Completa**  
   [`../docs/implementacion/esp32-desarrollo.md`](../docs/implementacion/esp32-desarrollo.md)
   - Hardware requerido
   - Mapeo de pines
   - Arquitectura del firmware
   - Configuración del entorno
   - Implementación módulo por módulo

2. **Diagramas de Conexión Hardware**  
   [`../docs/implementacion/esp32-diagramas-conexion.json`](../docs/implementacion/esp32-diagramas-conexion.json)
   - Especificaciones de componentes
   - Esquemas de conexión
   - Lista de materiales
   - Troubleshooting hardware

3. **Diagramas de Flujo del Firmware**  
   [`../docs/implementacion/esp32-diagramas-flujo.md`](../docs/implementacion/esp32-diagramas-flujo.md)
   - Flujo principal del sistema
   - Máquina de estados
   - Diagramas de secuencia
   - Formato Mermaid (visualizable en GitHub/VSCode)

4. **Firmware - Código Base**  
   [`firmware/README.md`](firmware/README.md)
   - Quick start del firmware
   - Estructura del código
   - Configuración
   - Testing

---

## 🔧 Hardware Requerido

- **ESP32 NodeMCU** (CP2102)
- **Módulo de relés 8 canales** (3.3V o 5V)
- **Sensores de humedad capacitivos** v2.0 (hasta 6)
- **Fuente de alimentación 5V 3A**
- **Electroválvulas** 24V AC o 12V DC

**Lista completa con precios**: Ver [`esp32-diagramas-conexion.json`](../docs/implementacion/esp32-diagramas-conexion.json)

---

## 🎓 Flujo de Trabajo Sugerido

1. **Leer documentación**: Comenzar por [`esp32-indice.md`](../docs/implementacion/esp32-indice.md)
2. **Comprar componentes**: Según lista en diagramas de conexión
3. **Armar circuito**: Seguir esquemas de [`esp32-diagramas-conexion.json`](../docs/implementacion/esp32-diagramas-conexion.json)
4. **Configurar firmware**: Copiar `Secrets.h.example` y completar credenciales
5. **Compilar y subir**: Usar PlatformIO o Arduino IDE
6. **Testing**: Verificar con mock del backend
7. **Instalación final**: Montar en caja estanca y conectar electroválvulas

---

## 📝 Estado de Implementación

### Mock Python ✅
- [x] Simulador MQTT funcional
- [x] Recibe comandos y agendas
- [x] Valida versiones
- [ ] Simula timers de riego (limitación conocida)

### Firmware Real ⏳
- [x] Estructura del proyecto
- [x] Configuración PlatformIO
- [x] Headers principales
- [x] main.cpp con máquina de estados
- [ ] WiFiManager (en desarrollo)
- [ ] MqttManager (en desarrollo)
- [ ] RelayController (en desarrollo)
- [ ] HumiditySensor (en desarrollo)
- [ ] AgendaManager (en desarrollo)
- [ ] SPIFFSManager (en desarrollo)
- [ ] TimeSync (en desarrollo)

---

## 🤝 Contribuir

Ver guías de desarrollo en [`../docs/implementacion/`](../docs/implementacion/) antes de contribuir código.

---

## 📄 Licencia

Ver [LICENSE](../LICENSE) para detalles.
