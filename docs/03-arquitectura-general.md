# Arquitectura General

## Nodos intervinientes
1. **ESP32** (sistema de riego) - Hardware de control
2. **Broker MQTT** (HiveMQ CE local) - Puerto 1883
3. **Backend** (Spring Boot + PostgreSQL) - Puerto 8080
4. **Frontend** (Vue.js embebido en backend) - Servido desde `/static`

## Implementación Actual

### Desarrollo
```
[Frontend Vite:5173] ←HTTP/CORS→ [Backend:8080] ←MQTT→ [HiveMQ:1883]
                                       ↕                      ↕
                                [PostgreSQL:5432]         [ESP32]
```

### Producción (Frontend Embebido)
```
[Browser] → HTTP → [Backend:8080] ←MQTT→ [HiveMQ:1883]
                        ↕                      ↕
                   [PostgreSQL:5432]      [ESP32]
```

## Principios
- El ESP32 **no recibe conexiones entrantes**
- Toda comunicación es saliente vía MQTT
- El backend es el cerebro (lógica de negocio, agenda, estado)
- El frontend nunca habla MQTT directamente
- **Autenticación**: HTTP Basic en todos los endpoints
- **Credenciales**: Gestionadas con variables de entorno (`.env`)

## Flujo de Datos

### Comando Manual de Riego
```
[Frontend] → POST /api/nodos/{id}/cmd → [Backend]
                                           ↓
                            Publica: riego/{nodeId}/cmd/zona/{zona}
                                           ↓
                                       [MQTT Broker]
                                           ↓
                                    [ESP32 Suscrito]
```

### Estado de Zona (Feedback)
```
[ESP32] → Publica: riego/{nodeId}/status/zona/{zona}
                                ↓
                          [MQTT Broker]
                                ↓
                    [Backend Suscrito] → Actualiza caché
                                ↓
[Frontend] ← GET /api/nodos/{id}/status ← [Backend]
```

### Sincronización de Agenda
```
[Frontend] → POST /api/nodos/{id}/agendas → [Backend]
                                               ↓
                              1. Guarda en PostgreSQL
                              2. Incrementa version
                              3. Publica: riego/{nodeId}/agenda/sync
                                               ↓
                                          [MQTT Broker]
                                               ↓
                                        [ESP32 Actualiza]
```
