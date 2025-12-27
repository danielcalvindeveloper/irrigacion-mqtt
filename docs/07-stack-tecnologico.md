# Stack Tecnológico

## Hardware
- **ESP8266 NodeMCU** (CP2102)
  - Microcontrolador: ESP8266 (Tensilica L106 32-bit @ 80/160MHz)
  - RAM: 80KB (usuario) + 36KB (sistema)
  - Flash: 4MB
  - WiFi: 802.11 b/g/n (2.4GHz)
  - GPIO: 11 pines digitales disponibles
  - ADC: 1 canal analógico (10-bit, 0-1V)
- **Relés 4CH TTL**
  - Lógica invertida (LOW=ON, HIGH=OFF)
  - 4 zonas de riego controladas
- **Display OLED SSD1306** ⭐ NUEVO
  - Resolución: 128x64 píxeles monocromático
  - Interfaz: I2C (dirección 0x3C)
  - Muestra: Estado WiFi/MQTT, zonas activas, mensajes del sistema
- **Sensor capacitivo de humedad de suelo** (v2.0)
  - 1 sensor conectado al ADC (A0)
  - Expansión futura requiere multiplexor externo (ej: CD4051)

## Backend
- Java 17+
- Spring Boot 3.4.0
- Package: `ar.net.dac.iot.irrigacion`
- PostgreSQL 15.15
- Flyway 10.20.1 (migraciones de BD)
- HiveMQ MQTT Client 1.3.3
- Maven (gestión de dependencias)
- Autenticación HTTP Basic
- Frontend embebido (Vue.js servido desde `/static`)

## Frontend
- Vue 3 (Composition API)
- Vuetify 3 (Material Design)
- Pinia (state management)
- Axios (cliente HTTP)
- Vite (build tool)
- Embebido en backend (producción)
- Dev server independiente (desarrollo)

## Dev
- VSCode
- GitHub Copilot (Agent)
- GPT-5.1 Codex
