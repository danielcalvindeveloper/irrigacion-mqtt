# Testing End-to-End con Mock ESP32

Esta guía te permite probar el flujo completo del sistema usando el simulador Python.

## Preparación

### 1. Levantar el stack Docker

```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo
docker-compose up -d
```

Verifica que todo esté corriendo:
```powershell
docker-compose ps
```

Deberías ver:
- `irrigacion-postgres` (healthy)
- `irrigacion-mqtt` (up)
- `irrigacion-backend` (up)

### 2. Instalar dependencias Python

```powershell
cd esp32
pip install -r requirements.txt
```

## Escenario 1: Crear agenda y verificar sincronización

### Terminal 1: Mock ESP32

```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo\esp32
python mock_esp32.py --node-id 550e8400-e29b-41d4-a716-446655440000
```

O simplemente ejecuta el script helper:
```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo\esp32
.\run-mock.bat
```

Salida esperada:
```
============================================================
  MOCK ESP32 - Simulador de nodo de riego
============================================================

Conectando a localhost:1883...
✓ Conectado al broker MQTT en localhost:1883
✓ Node ID: 550e8400-e29b-41d4-a716-446655440000
✓ Suscrito a: riego/550e8400-e29b-41d4-a716-446655440000/agenda/sync
✓ Suscrito a: riego/550e8400-e29b-41d4-a716-446655440000/cmd/zona/+

--- Esperando mensajes MQTT ---
```

### Terminal 2: Crear agenda vía API

**Opción A - Con curl:**

```powershell
curl -X POST http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas -H "Content-Type: application/json" -d "{\"nombre\": \"Riego matutino\", \"activa\": true, \"programaciones\": [{\"zona\": 1, \"hora\": \"07:00\", \"duracionMinutos\": 15, \"diasSemana\": [\"LUNES\", \"MIERCOLES\", \"VIERNES\"]}, {\"zona\": 2, \"hora\": \"07:20\", \"duracionMinutos\": 10, \"diasSemana\": [\"LUNES\", \"MIERCOLES\", \"VIERNES\"]}]}"
```

**Opción B - Con Postman:**

```
POST http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas

Body (JSON):
{
  "nombre": "Riego matutino",
  "activa": true,
  "programaciones": [
    {
      "zona": 1,
      "hora": "07:00",
      "duracionMinutos": 15,
      "diasSemana": ["LUNES", "MIERCOLES", "VIERNES"]
    },
    {
      "zona": 2,
      "hora": "07:20",
      "duracionMinutos": 10,
      "diasSemana": ["LUNES", "MIERCOLES", "VIERNES"]
    }
  ]
}
```

### Resultado esperado en Terminal 1

El mock ESP32 mostrará:

```
============================================================
[14:23:45] 📅 AGENDA RECIBIDA
============================================================
Versión: 1
Nombre: Riego matutino
Activa: SÍ
Programaciones: 2
✓ Agenda actualizada (versión 1)

Programaciones:
  1. Zona 1 - 07:00 - 15min - Días: ['LUNES', 'MIERCOLES', 'VIERNES']
  2. Zona 2 - 07:20 - 10min - Días: ['LUNES', 'MIERCOLES', 'VIERNES']
============================================================
```

---

## Escenario 2: Actualizar agenda (versión incremental)

### En Terminal 2, actualiza la agenda:

```powershell
curl -X PUT http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/agendas/1 -H "Content-Type: application/json" -d "{\"nombre\": \"Riego matutino + vespertino\", \"activa\": true, \"programaciones\": [{\"zona\": 1, \"hora\": \"07:00\", \"duracionMinutos\": 15, \"diasSemana\": [\"LUNES\", \"MIERCOLES\", \"VIERNES\"]}, {\"zona\": 1, \"hora\": \"19:00\", \"duracionMinutos\": 10, \"diasSemana\": [\"MARTES\", \"JUEVES\"]}]}"
```

### Resultado en Terminal 1

```
============================================================
[14:25:12] 📅 AGENDA RECIBIDA
============================================================
Versión: 2
Nombre: Riego matutino + vespertino
Activa: SÍ
Programaciones: 2
✓ Agenda actualizada (versión 2)

Programaciones:
  1. Zona 1 - 07:00 - 15min - Días: ['LUNES', 'MIERCOLES', 'VIERNES']
  2. Zona 1 - 19:00 - 10min - Días: ['MARTES', 'JUEVES']
============================================================
```

Nota que la **versión se incrementó a 2**.

---

## Escenario 3: Comando manual de riego

### En Terminal 2, envía un comando manual:

```powershell
curl -X POST http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/cmd -H "Content-Type: application/json" -d "{\"zona\": 1, \"accion\": \"REGAR\", \"duracionMinutos\": 5}"
```

### Resultado en Terminal 1

```
============================================================
[14:27:34] 💧 COMANDO MANUAL RECIBIDO
============================================================
Zona: 1
Acción: REGAR
Duración: 5 minutos

🚿 Simulando riego en zona 1...
   └─ Electroválvula de zona 1 ABIERTA
   └─ Esperando 5 minutos...
   └─ (En un ESP32 real, aquí activarías el relé/GPIO)
============================================================
```

---

## Escenario 4: Comando de detención

```powershell
curl -X POST http://localhost:8080/api/nodos/550e8400-e29b-41d4-a716-446655440000/cmd -H "Content-Type: application/json" -d "{\"zona\": 1, \"accion\": \"DETENER\"}"
```

### Resultado en Terminal 1

```
============================================================
[14:28:15] 💧 COMANDO MANUAL RECIBIDO
============================================================
Zona: 1
Acción: DETENER

⏹️  Deteniendo riego en zona 1
   └─ Electroválvula de zona 1 CERRADA
============================================================
```

---

## Escenario 5: Múltiples nodos simultáneos

Puedes correr múltiples mocks en terminales diferentes para simular varios ESP32:

**Terminal A:**
```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo\esp32
python mock_esp32.py --node-id 550e8400-e29b-41d4-a716-446655440000
```

**Terminal B:**
```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo\esp32
python mock_esp32.py --node-id 660e8400-e29b-41d4-a716-446655440001
```

**Terminal C:**
```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo\esp32
python mock_esp32.py --node-id 770e8400-e29b-41d4-a716-446655440002
```

Cada uno recibirá solo las agendas y comandos de su propio nodeId.

---

## Verificación con MQTT Explorer (opcional)

Si quieres ver los mensajes MQTT en tiempo real:

1. Descarga [MQTT Explorer](http://mqtt-explorer.com/)
2. Conecta a:
   - Host: `localhost`
   - Port: `1883`
3. Verás la jerarquía de topics:
   ```
   riego/
     └─ 550e8400-e29b-41d4-a716-446655440000/
         ├─ agenda/sync
         └─ cmd/zona/1
   ```

---

## Troubleshooting

**Mock no se conecta:**
```
✗ Error: No se pudo conectar al broker en localhost:1883
```
- Verifica que Docker esté corriendo: `docker-compose ps`
- Verifica que MQTT esté healthy: `docker logs irrigacion-mqtt`

**Mock conecta pero no recibe mensajes:**
- Verifica que el backend esté corriendo: `docker logs irrigacion-backend`
- Verifica que uses el mismo nodeId en el mock y en la API
- Revisa los logs del backend para errores de publicación MQTT

**Backend no publica a MQTT:**
- Revisa: `docker logs irrigacion-backend | findstr MQTT`
- Debería mostrar: `MQTT conectado a mqtt:1883 (tls=false) result=SUCCESS`

---

## Próximos pasos

Una vez validado el flujo end-to-end con el mock:

1. **Frontend Vue.js**: UI para gestionar agendas visualmente
2. **Firmware ESP32**: Implementar en C++ para hardware real
3. **Modo offline**: Persistencia local en ESP32 con sincronización
