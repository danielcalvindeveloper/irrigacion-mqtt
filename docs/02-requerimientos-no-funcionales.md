# Requerimientos No Funcionales

## Hardware de riego
- Controlador: NODEMCU (ESP32) con CP2102
- Módulo de relés: 4CH TTL Relay (1 relé = 1 zona)
- Sensor de humedad de suelo (recomendación):
  - **Capacitivo v2.0** (típico “Capacitive Soil Moisture Sensor v2.0”)
  - Motivo: más durable que resistivo (menos corrosión), mejor para uso continuo

## Backend
- Lenguaje: Java
- Framework: Spring Boot
- Responsabilidades:
  - Scheduler
  - Reglas de riego
  - Persistencia de agendas y eventos
  - Integración MQTT

## Comunicación
- Protocolo: MQTT
- Broker: HiveMQ Cloud
- Seguridad: MQTT over TLS (obligatorio)

## Frontend
- Framework: Vue.js 3
- Uso: móvil y PC
- Modalidad: Web / PWA

## Calidad
- Baja latencia en comandos
- Tolerancia a cortes de red
- Escalabilidad a múltiples nodos de riego
