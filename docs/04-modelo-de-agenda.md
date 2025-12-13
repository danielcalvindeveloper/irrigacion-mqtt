# Modelo de Agenda de Riego

## Entidad Agenda
- id
- zona (1..4)
- diasSemana (LUN..DOM)
- horaInicio
- duracionMinutos
- activa (boolean)

## Reglas
- Una zona puede tener múltiples agendas
- Las agendas se evalúan localmente en el ESP32 (modo offline)
- El backend es la fuente de verdad
