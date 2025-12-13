# ESP32 - Nodo de riego

## Mock ESP32 (Simulador en Python)

Para testing sin hardware real, usa el simulador Python que emula un ESP32.

### Requisitos

- Python 3.7+
- pip

### Instalación

```powershell
cd esp32
pip install -r requirements.txt
```

### Uso básico

1. Asegúrate de que el stack Docker esté corriendo:
   ```powershell
   docker-compose up -d
   ```

2. Ejecuta el mock con un nodeId:
   ```powershell
   python mock_esp32.py --node-id 550e8400-e29b-41d4-a716-446655440000
   ```

3. En otra terminal, crea una agenda para ese nodeId usando Postman o curl (ver [manual-postman-agendas-cmd.md](../docs/implementacion/manual-postman-agendas-cmd.md))

4. El mock mostrará la agenda recibida en consola

### Opciones

```
--node-id       UUID del nodo (requerido)
--mqtt-host     Host del broker (default: localhost)
--mqtt-port     Puerto del broker (default: 1883)
```

### Ejemplo completo

```powershell
# Terminal 1: Levantar stack
docker-compose up -d

# Terminal 2: Ejecutar mock
python esp32/mock_esp32.py --node-id 550e8400-e29b-41d4-a716-446655440000

# Terminal 3: Crear agenda
curl -X POST http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas \
  -H "Content-Type: application/json" \
  -d '{
    "nombre": "Agenda de prueba",
    "activa": true,
    "programaciones": [
      {
        "zona": 1,
        "hora": "07:00",
        "duracionMinutos": 15,
        "diasSemana": ["LUNES", "MIERCOLES", "VIERNES"]
      }
    ]
  }'

# Ver en Terminal 2 cómo el mock recibe la agenda
```

### Qué hace el mock

- ✅ Se conecta al broker MQTT (HiveMQ)
- ✅ Se suscribe a `riego/{nodeId}/agenda/sync`
- ✅ Se suscribe a `riego/{nodeId}/cmd/zona/+`
- ✅ Muestra en consola las agendas recibidas
- ✅ Simula la ejecución de comandos manuales
- ✅ Valida versiones de agenda (ignora duplicados)

---

## Firmware ESP32 real (futuro)

Objetivo:
- Conectar a MQTT (WiFi + TLS)
- Controlar relés/electroválvulas
- Modo offline con persistencia en SPIFFS/LittleFS
- Sincronizar agendas automáticamente

**Estado:** Pendiente de implementación
