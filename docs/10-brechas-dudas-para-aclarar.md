Seguridad
MQTT TLS – credenciales en ESP32

MVP

Username/password + TLS

Credenciales hardcodeadas por ahora en firmware (sí, está bien para MVP)

Sin rotación automática

Post-MVP

Un usuario por dispositivo

Rotación manual (reflash o config OTA)

Opcional: client certs

👉 Opinión: no compliques con certs ahora, es sobre-ingeniería temprana.

Frontend ↔ Backend

MVP

JWT simple

Login básico (usuario/clave)

CORS abierto solo a tu dominio

NO OAuth en MVP.

2️⃣ Persistencia y versionado de agenda
agenda.version

Formato:

{
  "version": 7,
  "updatedAt": "2025-09-12T10:30:00Z",
  "source": "backend"
}


version = entero incremental global por nodo

Backend siempre manda la verdad

Conflictos

Regla MVP

ESP32 NO puede modificar agenda

Solo ejecuta y persiste copia

Si recibe versión más nueva → reemplaza

Si recibe versión vieja → ignora

👉 Opinión: permitir edición local en ESP32 es una trampa.

3️⃣ Telemetría e histórico
Backend (mínimo)

Tablas

riego_evento

zona

timestamp

duracion

origen (agenda/manual)

telemetria_humedad

zona

timestamp

valor

Retención

Eventos: 6–12 meses

Humedad: agregada (promedio diario) o 30 días crudo

ESP32

Buffer circular:

50–100 eventos máx

Envío:

batch al reconectar

luego tiempo real

4️⃣ Scheduler (agenda vs manual)
Solape de agendas

Regla MVP

❌ No se permite solape por zona

Backend valida al guardar agenda

Comando manual durante agenda

Decisión

Manual anula agenda

Agenda se da por interrumpida

Próxima ejecución sigue normal

👉 Opinión: esto es lo más intuitivo para el usuario.

5️⃣ Humedad
Scope MVP

✅ Lectura y visualización

❌ No gobierna riego automáticamente

Post-MVP

Umbral por zona

Regla:

if humedad > umbral → no regar

Calibración:

offset por sensor

guardado en backend

👉 Opinión: no mezclar control automático en el MVP.

6️⃣ Observabilidad
ESP32

Logs serial + MQTT

Métricas:

uptime

reconexiones

RSSI WiFi

Backend

Logs estructurados

Actuator:

health

metrics

Alertas (post-MVP)

nodo offline X horas

humedad fuera de rango

7️⃣ Escalabilidad (multi ESP32)
Identidad de nodo

Cada ESP32 tiene:

nodeId = UUID

Topics
riego/{nodeId}/cmd/zona/1
riego/{nodeId}/status/zona/1
riego/{nodeId}/telemetria


Backend:

maneja múltiples nodos

UI filtra por nodo

👉 Esto no agrega complejidad real y evita refactor futuro.

8️⃣ Frontend
PWA

MVP

Instalación opcional

❌ No offline UI (solo online)

Actualización de estado

WebSocket (preferido)

Polling solo como fallback

Histórico UX

Filtro:

nodo

zona

fecha desde/hasta

Tabla simple + export CSV

9️⃣ Infra
Backend

VPS chico o cloud (Hetzner / DO / similar)

Docker recomendado

CI/CD

MVP

build + test

deploy manual

Backups

DB diaria

Retención 7–14 días

🧭 Resumen ejecutivo (para Copilot)

ESP32 ejecuta, no decide

Backend es la fuente de verdad

Agenda versionada, sin conflictos

Manual > agenda

Humedad solo lectura en MVP

Multi-nodo desde el día 1

Seguridad simple, no enterprise