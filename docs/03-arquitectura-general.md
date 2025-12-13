# Arquitectura General

## Nodos intervinientes
1. ESP32 (sistema de riego)
2. Broker MQTT (HiveMQ Cloud)
3. Backend (Java, cloud/VPS)
4. Cliente Web (Vue.js)

## Principios
- El ESP32 **no recibe conexiones entrantes**
- Toda comunicación es saliente vía MQTT
- El backend es el cerebro
- El frontend nunca habla MQTT directamente

## Flujo
[Web] → HTTP/WebSocket → [Backend] → MQTT → [Broker]  
[ESP32] → MQTT → [Broker] → MQTT → [Backend]
