# Irrigación MQTT (Repo base)

Repo base para sistema de riego por zonas con ESP32 + MQTT (HiveMQ) + Backend Java + Frontend Vue.

## Estructura
- `docs/` documentación funcional/técnica (MVP)
- `backend/` Spring Boot (esqueleto)
- `frontend/` Vue 3 (esqueleto)
- `esp32/` notas y placeholders de firmware

## Primeros pasos (recomendado)
1. Configurar HiveMQ Cloud (cluster + credenciales)
2. Probar publish/subscribe con el **Web Client** de HiveMQ
3. Implementar conexión MQTT en ESP32 y probar comandos manuales
4. Recién después: backend + UI

## Requisitos
- Java 17+ (para backend)
- Node 18+ (para frontend)
- VSCode + GitHub Copilot (Agent) + GPT-5.1 Codex

## Notas
- MQTT debe ir **sobre TLS**.
- La UI **no** debe conectarse directo al broker. Va contra el backend (REST/WebSocket).
