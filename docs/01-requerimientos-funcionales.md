# Requerimientos Funcionales

## RF-1 Gestión de agenda por zona
- El sistema debe permitir **crear, modificar y eliminar agendas de riego por zona**.
- Las operaciones deben poder realizarse **desde móvil o PC**.

## RF-1.1 Visualización de agenda activa
- Desde móvil o PC se debe visualizar **claramente la agenda activa** por cada zona.
- Debe indicarse:
  - Zona
  - Próxima activación
  - Duración configurada
  - Estado actual (ON / OFF)

## RF-1.2 Consulta histórica
- Desde móvil o PC se debe poder **consultar la actividad de riego de un período determinado**.
- El período debe ser configurable (fecha desde / hasta).
- La información mínima:
  - Zona
  - Fecha y hora
  - Duración
  - Motivo (agenda / manual / humedad)

## RF-2 Definición de agenda por zona
Cada agenda por zona debe contemplar:
- Días de la semana
- Horarios de inicio
- Tiempo de activación (en minutos)

## RF-3 Operación sin conexión a Internet
- Ante pérdida de conectividad:
  - El sistema de riego debe **mantener activa la última agenda conocida** por zona.
  - La ejecución debe ser **autónoma en el ESP32**.
- Al recuperar conectividad:
  - Debe sincronizar estado y actividad con el backend.
