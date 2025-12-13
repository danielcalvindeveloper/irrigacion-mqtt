# Modo sin Conectividad

## Principio
El ESP32 debe poder operar **de forma autónoma**.

## Estrategia
- Persistir la última agenda válida en memoria local (flash)
- Ejecutar scheduler local
- Registrar eventos en un buffer local (circular o por tamaño)

## Recuperación
- Al reconectar:
  - Publicar estado actual
  - Enviar eventos pendientes al backend
  - Aplicar si corresponde una agenda con versión más nueva (del backend)
