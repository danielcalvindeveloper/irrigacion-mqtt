# 📊 Diagramas de Flujo - Firmware ESP8266

> **Documento**: Diagramas visuales del comportamiento del firmware ESP8266
> **Formato**: Mermaid (compatible con GitHub, VSCode, herramientas de documentación)
> **Última actualización**: 2025-12-27

---

## Índice
1. [Flujo Principal del Sistema](#flujo-principal-del-sistema)
2. [Máquina de Estados de Conexión](#máquina-de-estados-de-conexión)
3. [Procesamiento de Comandos MQTT](#procesamiento-de-comandos-mqtt)
4. [Sincronización de Agendas](#sincronización-de-agendas)
5. [Ejecución de Agendas](#ejecución-de-agendas)
6. [Gestión de Relés con Timers](#gestión-de-relés-con-timers)
7. [Lectura y Publicación de Sensores](#lectura-y-publicación-de-sensores)
8. [Modo Offline y Reconexión](#modo-offline-y-reconexión)

---

## 1. Flujo Principal del Sistema

```mermaid
flowchart TD
    Start([Inicio ESP8266]) --> Init[Inicializar Hardware]
    Init --> InitRelays[Configurar pines de relés como OUTPUT]
    InitRelays --> InitSensor[Configurar pin ADC de sensor A0]
    InitSensor --> InitDisplay[Inicializar Display OLED]
    InitDisplay --> InitLittleFS[Montar LittleFS]
    InitLittleFS --> LoadConfig{"Config guardada?"}
    
    LoadConfig -->|Sí| LoadAgendas[Cargar agendas desde SPIFFS]
    LoadConfig -->|No| DefaultConfig[Usar configuración por defecto]
    
    LoadAgendas --> ConnectWiFi[Intentar conectar WiFi]
    DefaultConfig --> ConnectWiFi
    
    ConnectWiFi --> Loop[Loop Principal]
    
    Loop --> CheckState{Estado del Sistema}
    
    CheckState -->|INIT| InitState[Estado inicial]
    CheckState -->|WIFI_CONNECTING| WiFiState[Gestión WiFi]
    CheckState -->|MQTT_CONNECTING| MQTTState[Gestión MQTT]
    CheckState -->|ONLINE| OnlineState[Modo Online]
    CheckState -->|OFFLINE| OfflineState[Modo Offline]
    
    WiFiState --> CheckWiFi{WiFi conectado?}
    CheckWiFi -->|Sí| SyncTime[Sincronizar NTP]
    CheckWiFi -->|No| WaitWiFi[Esperar timeout]
    SyncTime --> StateChange1[Cambiar a MQTT_CONNECTING]
    WaitWiFi --> StateChange2[Cambiar a OFFLINE]
    
    MQTTState --> ConnectMQTT[Conectar a broker]
    ConnectMQTT --> CheckMQTT{MQTT conectado?}
    CheckMQTT -->|Sí| Subscribe[Suscribir a topics]
    CheckMQTT -->|No| RetryMQTT[Reintentar en 15s]
    Subscribe --> StateChange3[Cambiar a ONLINE]
    
    OnlineState --> ProcessMQTT[Procesar mensajes MQTT]
    ProcessMQTT --> CheckConnection{Conexión OK?}
    CheckConnection -->|Sí| PublishStatus[Publicar estado de zonas]
    CheckConnection -->|No| StateChange4[Cambiar a WIFI_CONNECTING]
    
    OfflineState --> RetryConnect[Reintentar cada 60s]
    RetryConnect --> StateChange5[Cambiar a WIFI_CONNECTING]
    
    StateChange1 --> CommonTasks
    StateChange2 --> CommonTasks
    StateChange3 --> CommonTasks
    StateChange4 --> CommonTasks
    StateChange5 --> CommonTasks
    PublishStatus --> CommonTasks
    
    CommonTasks[Tareas Comunes]
    CommonTasks --> UpdateRelays[Actualizar timers de relés]
    UpdateRelays --> UpdateDisplay[Actualizar display OLED]
    UpdateDisplay --> CheckAgendas[Verificar agendas programadas]
    CheckAgendas --> ReadSensor[Leer sensor de humedad]
    ReadSensor --> Delay[Delay 100ms]
    Delay --> Loop
```

---

## 2. Máquina de Estados de Conexión

```mermaid
stateDiagram-v2
    [*] --> INIT: Power On
    
    INIT --> WIFI_CONNECTING: Hardware inicializado
    
    WIFI_CONNECTING --> WIFI_CONNECTED: WiFi OK
    WIFI_CONNECTING --> OFFLINE: Timeout 30s
    
    WIFI_CONNECTED --> MQTT_CONNECTING: NTP sincronizado
    
    MQTT_CONNECTING --> ONLINE: MQTT OK
    MQTT_CONNECTING --> WIFI_CONNECTING: MQTT timeout 15s
    
    ONLINE --> WIFI_CONNECTING: Conexión perdida
    ONLINE --> ONLINE: Operación normal
    
    OFFLINE --> WIFI_CONNECTING: Retry cada 60s
    OFFLINE --> OFFLINE: Modo local
    
    note right of ONLINE
        Estado ideal
        - MQTT pub/sub activo
        - Sincronización en tiempo real
        - OTA disponible
    end note
    
    note right of OFFLINE
        Modo autónomo
        - Ejecuta última agenda conocida
        - Sin comunicación con backend
        - Intenta reconectar periódicamente
    end note
```

---

## 3. Procesamiento de Comandos MQTT

```mermaid
flowchart TD
    Start([Mensaje MQTT recibido]) --> ParseTopic{Tipo de topic?}
    
    ParseTopic -->|cmd/zona/N| CommandTopic[Topic de comando]
    ParseTopic -->|agenda/sync| AgendaTopic[Topic de agenda]
    ParseTopic -->|Otro| IgnoreTopic[Ignorar mensaje]
    
    CommandTopic --> ParseJSON[Parsear JSON payload]
    ParseJSON --> ValidateJSON{JSON válido?}
    ValidateJSON -->|No| LogError[Log error + descartar]
    ValidateJSON -->|Sí| ExtractCommand[Extraer campos]
    
    ExtractCommand --> GetZone[Obtener número de zona del topic]
    GetZone --> GetAction[Obtener acción y duración]
    GetAction --> ValidateZone{Zona 1-8?}
    
    ValidateZone -->|No| LogInvalidZone[Log zona inválida]
    ValidateZone -->|Sí| CheckAction{Acción?}
    
    CheckAction -->|ON| ValidateDuration{Duración > 0?}
    CheckAction -->|OFF| TurnOffRelay[Apagar relé]
    CheckAction -->|Otra| LogInvalidAction[Log acción inválida]
    
    ValidateDuration -->|No| LogNoDuration[Log duración faltante]
    ValidateDuration -->|Sí| CheckMaxDuration{Duración <= 7200s?}
    
    CheckMaxDuration -->|No| LimitDuration[Limitar a 7200s + advertencia]
    CheckMaxDuration -->|Sí| TurnOnRelay[Encender relé]
    
    LimitDuration --> TurnOnRelay
    TurnOnRelay --> StartTimer[Iniciar timer de zona]
    StartTimer --> PublishConfirm[Publicar estado actualizado]
    
    TurnOffRelay --> StopTimer[Detener timer de zona]
    StopTimer --> PublishConfirm
    
    PublishConfirm --> End([Fin])
    LogError --> End
    LogInvalidZone --> End
    LogInvalidAction --> End
    LogNoDuration --> End
    IgnoreTopic --> End
```

**Ejemplo de comando MQTT:**
```json
Topic: riego/550e8400-e29b-41d4-a716-446655440000/cmd/zona/1
Payload: {"accion": "ON", "duracion": 600}
```

---

## 4. Sincronización de Agendas

```mermaid
flowchart TD
    Start([Mensaje agenda/sync recibido]) --> ParseJSON[Parsear JSON payload]
    ParseJSON --> ValidateJSON{JSON válido?}
    ValidateJSON -->|No| LogError[Log error JSON]
    
    ValidateJSON -->|Sí| ExtractVersion[Extraer version]
    ExtractVersion --> GetLocalVersion[Obtener version local]
    GetLocalVersion --> CompareVersion{Nueva version > Local?}
    
    CompareVersion -->|No| IgnoreOld["Ignorar - ya actualizado"]
    CompareVersion -->|Sí| ClearAgendas[Limpiar agendas actuales]
    
    ClearAgendas --> ExtractAgendas[Extraer array de agendas]
    ExtractAgendas --> ValidateCount{Cantidad <= MAX?}
    ValidateCount -->|No| LogTooMany[Log: demasiadas agendas]
    
    ValidateCount -->|Sí| LoopAgendas[Iterar cada agenda]
    LoopAgendas --> ValidateAgenda{Agenda válida?}
    
    ValidateAgenda -->|No| SkipAgenda[Saltar esta agenda]
    ValidateAgenda -->|Sí| CheckZone{Zona 1-8?}
    
    CheckZone -->|No| SkipAgenda
    CheckZone -->|Sí| CheckTime{Hora válida?}
    
    CheckTime -->|No| SkipAgenda
    CheckTime -->|Sí| CheckDays{Días válidos?}
    
    CheckDays -->|No| SkipAgenda
    CheckDays -->|Sí| AddAgenda[Agregar a lista local]
    
    AddAgenda --> MoreAgendas{"Más agendas?"}
    MoreAgendas -->|Sí| LoopAgendas
    MoreAgendas -->|No| UpdateLocalVersion[Actualizar version local]
    
    UpdateLocalVersion --> SaveToSPIFFS[Guardar en SPIFFS]
    SaveToSPIFFS --> LogSuccess[Log: agendas sincronizadas]
    
    SkipAgenda --> MoreAgendas
    LogSuccess --> End([Fin])
    IgnoreOld --> End
    LogError --> End
    LogTooMany --> SaveToSPIFFS
```

**Ejemplo de payload de sincronización:**
```json
{
  "version": 5,
  "agendas": [
    {
      "id": "uuid-1",
      "zona": 1,
      "nombre": "Césped mañana",
      "horaInicio": "07:30",
      "duracionMin": 15,
      "diasSemana": ["LUN", "MIE", "VIE"],
      "activa": true
    }
  ]
}
```

---

## 5. Ejecución de Agendas

```mermaid
flowchart TD
    Start([Loop principal - cada 1s]) --> CheckInterval{1 segundo transcurrido?}
    CheckInterval -->|No| End
    CheckInterval -->|Sí| GetCurrentTime[Obtener hora actual del RTC]
    
    GetCurrentTime --> CheckSecond{Segundo = 0?}
    CheckSecond -->|No| End
    CheckSecond -->|Sí| LoopAgendas[Iterar cada agenda]
    
    LoopAgendas --> CheckActive{Agenda activa?}
    CheckActive -->|No| NextAgenda
    CheckActive -->|Sí| CheckDay{Día correcto?}
    
    CheckDay -->|No| NextAgenda
    CheckDay -->|Sí| CheckHour{Hora coincide?}
    
    CheckHour -->|No| NextAgenda
    CheckHour -->|Sí| CheckMinute{Minuto coincide?}
    
    CheckMinute -->|No| NextAgenda
    CheckMinute -->|Sí| CheckZoneActive{Zona ya activa?}
    
    CheckZoneActive -->|Sí| LogSkip[Log: zona ocupada]
    CheckZoneActive -->|No| ExecuteAgenda[Ejecutar riego]
    
    ExecuteAgenda --> TurnOn[Encender relé de zona]
    TurnOn --> SetTimer[Configurar timer]
    SetTimer --> SaveEvent[Registrar evento local]
    SaveEvent --> LogExecution[Log: agenda ejecutada]
    
    LogExecution --> PublishIfOnline{Online?}
    PublishIfOnline -->|Sí| PublishEvent[Publicar evento a MQTT]
    PublishIfOnline -->|No| NextAgenda
    
    PublishEvent --> NextAgenda["Siguiente agenda?"]
    LogSkip --> NextAgenda
    NextAgenda -->|Sí| LoopAgendas
    NextAgenda -->|No| End([Fin])
    
    End
```

**Lógica de verificación de día:**
```cpp
// Mapeo: tm_wday (0=Dom) → índice agenda (0=Lun)
int dayIndex = (wday == 0) ? 6 : wday - 1;
return agenda.diasSemana[dayIndex];
```

---

## 6. Gestión de Relés con Timers

```mermaid
flowchart TD
    Start([Loop principal - cada 1s]) --> CheckInterval{1 segundo transcurrido?}
    CheckInterval -->|No| End
    CheckInterval -->|Sí| InitLoop[i = 0]
    
    InitLoop --> CheckZone{i < MAX_ZONES?}
    CheckZone -->|No| End
    CheckZone -->|Sí| IsActive{"Zona i activa?"}
    
    IsActive -->|No| NextZone
    IsActive -->|Sí| CheckTimer{"Timer i > 0?"}
    
    CheckTimer -->|No| TurnOffRelay["Apagar relé zona i"]
    CheckTimer -->|Sí| DecrementTimer["Timer i decrementa"]
    
    TurnOffRelay --> UpdateState["zoneState i = false"]
    UpdateState --> LogOff[Log: riego finalizado]
    LogOff --> PublishStatusOff[Publicar estado OFF]
    PublishStatusOff --> NextZone
    
    DecrementTimer --> CheckZero{"Timer i == 0?"}
    CheckZero -->|Sí| TurnOffRelay
    CheckZero -->|No| PublishStatusOn[Publicar estado ON con tiempo restante]
    
    PublishStatusOn --> NextZone[i++]
    NextZone --> CheckZone
    
    End([Fin])
```

**Estructura de datos:**
```cpp
bool zoneState[MAX_ZONES];      // true = activa, false = inactiva
int zoneTimer[MAX_ZONES];       // segundos restantes
```

---

## 7. Lectura y Publicación de Sensores

```mermaid
flowchart TD
    Start([Loop principal - cada 60s]) --> CheckInterval{60 segundos transcurridos?}
    CheckInterval -->|No| End
    CheckInterval -->|Sí| InitLoop[i = 0]
    
    InitLoop --> CheckSensor{i < MAX_SENSORS?}
    CheckSensor -->|No| End
    CheckSensor -->|Sí| CheckPinValid{Pin ADC válido?}
    
    CheckPinValid -->|No| NextSensor
    CheckPinValid -->|Sí| ReadADC[Leer analogRead]
    
    ReadADC --> GetRaw[valor_raw = ADC]
    GetRaw --> MapValue[Mapear a porcentaje]
    MapValue --> Calculate[% = map]
    
    Calculate --> StoreLocal[Guardar lectura local]
    StoreLocal --> CheckOnline{Sistema online?}
    
    CheckOnline -->|No| NextSensor
    CheckOnline -->|Sí| CreateJSON[Crear payload JSON]
    
    CreateJSON --> PublishMQTT[Publicar a topic humedad/zona/N]
    PublishMQTT --> LogPublish[Log: sensor publicado]
    
    LogPublish --> NextSensor[i++]
    NextSensor --> CheckSensor
    
    End([Fin])
```

**Cálculo de porcentaje de humedad:**
```cpp
int raw = analogRead(pin);
int percentage = map(raw, SENSOR_DRY_VALUE, SENSOR_WET_VALUE, 0, 100);
percentage = constrain(percentage, 0, 100);
```

**Payload MQTT:**
```json
Topic: riego/{nodeId}/humedad/zona/1
Payload: {
  "zona": 1,
  "valor": 65,
  "raw": 1850,
  "timestamp": "2025-12-16T10:30:00Z"
}
```

---

## 8. Modo Offline y Reconexión

```mermaid
flowchart TD
    Start([Sistema en estado OFFLINE]) --> LogOffline[Log: Modo offline activado]
    LogOffline --> LoadLocal[Cargar última agenda de SPIFFS]
    LoadLocal --> ExecuteLocal[Ejecutar agendas localmente]
    
    ExecuteLocal --> WaitRetry{60 segundos transcurridos?}
    WaitRetry -->|No| ContinueOffline[Continuar modo offline]
    WaitRetry -->|Sí| AttemptWiFi[Intentar conectar WiFi]
    
    AttemptWiFi --> CheckWiFi{WiFi conectado?}
    CheckWiFi -->|No| WaitRetry
    CheckWiFi -->|Sí| SyncTime[Sincronizar NTP]
    
    SyncTime --> AttemptMQTT[Intentar conectar MQTT]
    AttemptMQTT --> CheckMQTT{MQTT conectado?}
    
    CheckMQTT -->|No| Backoff[Esperar 15s]
    CheckMQTT -->|Sí| Subscribe[Suscribir a topics]
    
    Subscribe --> SyncAgendas[Solicitar sync de agendas]
    SyncAgendas --> ChangeState[Cambiar a estado ONLINE]
    ChangeState --> LogOnline[Log: Modo online restaurado]
    
    Backoff --> RetryMQTT[Reintentar MQTT]
    RetryMQTT --> CheckMQTT
    
    ContinueOffline --> ExecuteLocal
    LogOnline --> End([Sistema operativo])
```

**Comportamiento en modo offline:**
- ✅ Ejecuta agendas guardadas localmente
- ✅ Controla relés normalmente
- ✅ Lee sensores (pero no publica)
- ❌ No recibe comandos remotos
- ❌ No sincroniza nuevas agendas
- 🔄 Intenta reconectar cada 60 segundos

---

## Diagrama de Arquitectura de Módulos

```mermaid
flowchart TB
    subgraph Main[main.cpp]
        Setup[setup]
        Loop[loop]
    end
    
    subgraph Network[Módulos de Red]
        WiFiMgr[WiFiManager]
        MqttMgr[MqttManager]
        TimeSync[TimeSync/NTP]
    end
    
    subgraph Hardware[Módulos de Hardware]
        RelayCtrl[RelayController]
        HumSensor[HumiditySensor]
    end
    
    subgraph Logic[Módulos de Lógica]
        AgendaMgr[AgendaManager]
        TaskSched[TaskScheduler]
    end
    
    subgraph Storage[Persistencia]
        SPIFFSMgr[SPIFFSManager]
    end
    
    subgraph Utils[Utilidades]
        Logger[Logger]
        Config[Config.h]
        Secrets[Secrets.h]
    end
    
    Main --> Network
    Main --> Hardware
    Main --> Logic
    Main --> Storage
    Main --> Utils
    
    Network --> Logger
    Network --> Config
    Network --> Secrets
    
    Logic --> RelayCtrl
    Logic --> SPIFFSMgr
    Logic --> Logger
    
    Hardware --> Logger
    Hardware --> Config
    
    MqttMgr --> AgendaMgr
    MqttMgr --> RelayCtrl
```

---

## Diagrama de Secuencia: Comando Manual

```mermaid
sequenceDiagram
    participant Backend
    participant MQTT
    participant ESP32
    participant Relay
    participant Sensor
    
    Backend->>MQTT: PUBLISH cmd/zona/1 {"accion":"ON", "duracion":600}
    MQTT->>ESP32: Mensaje recibido
    ESP32->>ESP32: Parsear JSON
    ESP32->>ESP32: Validar zona y duración
    ESP32->>Relay: digitalWrite(pin, LOW)
    Relay-->>ESP32: Relé activado
    ESP32->>ESP32: Iniciar timer de 600s
    ESP32->>MQTT: PUBLISH status/zona/1 {"activa":true, "tiempoRestante":600}
    MQTT->>Backend: Estado actualizado
    
    Note over ESP32: Cada segundo: decrementar timer
    
    loop Cada 5 segundos
        ESP32->>MQTT: PUBLISH status/zona/1 {"activa":true, "tiempoRestante":X}
    end
    
    ESP32->>ESP32: Timer llega a 0
    ESP32->>Relay: digitalWrite(pin, HIGH)
    Relay-->>ESP32: Relé desactivado
    ESP32->>MQTT: PUBLISH status/zona/1 {"activa":false, "tiempoRestante":0}
    MQTT->>Backend: Riego finalizado
```

---

## Diagrama de Secuencia: Sincronización de Agenda

```mermaid
sequenceDiagram
    participant User
    participant Backend
    participant DB
    participant MQTT
    participant ESP32
    participant SPIFFS
    
    User->>Backend: POST /api/agendas (crear/actualizar)
    Backend->>DB: Guardar agenda
    DB-->>Backend: OK
    Backend->>DB: Incrementar version
    DB-->>Backend: version = 5
    Backend->>MQTT: PUBLISH agenda/sync {version:5, agendas:[...]}
    MQTT->>ESP32: Mensaje recibido
    ESP32->>ESP32: Parsear JSON
    ESP32->>ESP32: Comparar version (local=4, nueva=5)
    ESP32->>ESP32: Limpiar agendas locales
    ESP32->>ESP32: Cargar nuevas agendas
    ESP32->>SPIFFS: Guardar agendas
    SPIFFS-->>ESP32: OK
    ESP32->>ESP32: Actualizar version local a 5
    
    Note over ESP32: Agendas sincronizadas y listas para ejecutar
```

---

## Notas de Implementación

### Timings Críticos
- **Verificación de agendas**: Cada 1 segundo (solo ejecuta cuando segundo=0)
- **Actualización de timers**: Cada 1 segundo
- **Publicación de estado**: Cada 5 segundos (si online)
- **Lectura de sensores**: Cada 60 segundos
- **Reconexión WiFi**: Cada 30 segundos (si desconectado)
- **Reconexión MQTT**: Cada 15 segundos (si desconectado)
- **Retry modo offline**: Cada 60 segundos

### Consideraciones de Memoria
- **MAX_ZONES**: 8 zonas
- **MAX_AGENDAS**: ~20-30 agendas totales (limitado por RAM/SPIFFS)
- **SPIFFS**: Reservar mínimo 512KB para almacenamiento
- **JSON Buffer**: StaticJsonDocument<2048> para agendas
- **JSON Buffer**: StaticJsonDocument<256> para comandos/estado

### Watchdog Timer
- ESP32 tiene watchdog automático
- `delay(100)` en loop evita timeout
- No bloquear loop por más de 3-5 segundos

---

## Cómo Usar Estos Diagramas

### En VSCode
1. Instalar extensión: **Markdown Preview Mermaid Support**
2. Abrir este archivo `.md`
3. `Ctrl+Shift+V` para preview con diagramas renderizados

### En GitHub
- Los diagramas Mermaid se renderizan automáticamente
- Ver en: `docs/implementacion/esp32-diagramas-flujo.md`

### Exportar a Imagen
```bash
# Instalar mermaid-cli
npm install -g @mermaid-js/mermaid-cli

# Generar PNG
mmdc -i esp32-diagramas-flujo.md -o diagrama.png

# Generar SVG
mmdc -i esp32-diagramas-flujo.md -o diagrama.svg
```

### Editar Diagramas
- **Online**: https://mermaid.live/
- **Desktop**: https://www.diagram.codes/d/flowchart

---

## Próximos Pasos

1. Revisar [esp32-desarrollo.md](./esp32-desarrollo.md) para guía completa
2. Ver [esp32-diagramas-conexion.json](./esp32-diagramas-conexion.json) para hardware
3. Comenzar implementación del firmware siguiendo estos flujos
