# Riesgos y Decisiones

## Riesgos
- Cortes de Internet (Starlink/NAT)
- Lecturas erróneas de humedad (sensor + calibración)
- Desincronización de agenda (versionado)

## Decisiones clave
- MQTT sobre TLS
- UI nunca habla directo con broker (solo backend)
- ESP32 con modo offline (última agenda + scheduler local)
- Versionado de agenda para sincronización
