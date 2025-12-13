# Arranque Automático de Servicios en Windows

Este documento explica cómo configurar el arranque automático de los servicios del proyecto (backend y túnel Cloudflare) en Windows, incluso sin que nadie esté logueado.

## 📋 Tabla de Contenidos

- [Requisitos Previos](#requisitos-previos)
- [Configuración del Backend](#configuración-del-backend)
- [Configuración del Túnel Cloudflare](#configuración-del-túnel-cloudflare)
- [Verificación](#verificación)
- [Orden de Arranque](#orden-de-arranque)
- [Gestión de Tareas](#gestión-de-tareas)
- [Solución de Problemas](#solución-de-problemas)

---

## Requisitos Previos

### Docker Desktop
1. **Instalar Docker Desktop** para Windows
2. **Configurar arranque automático:**
   - Abrir Docker Desktop
   - Ir a **Settings** ⚙️ → **General**
   - ✅ Activar **"Start Docker Desktop when you log in"**

### Permisos de Administrador
- Todos los scripts de configuración requieren **ejecutarse como Administrador**
- Windows solicitará confirmación UAC al ejecutar cada script

---

## Configuración del Backend

### 🚀 Instalar Arranque Automático

**Script:** `setup-autostart.ps1`

Este script configura el arranque automático de Docker Compose (PostgreSQL, MQTT, Backend).

#### Paso a paso:

1. **Abrir PowerShell como Administrador:**
   ```powershell
   # Navegar al directorio del proyecto
   cd C:\proyectos\dac\irrigacion-mqtt-repo
   
   # Ejecutar el script de instalación
   .\setup-autostart.ps1
   ```

2. **O ejecutar directamente desde PowerShell normal:**
   ```powershell
   Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PWD\setup-autostart.ps1`""
   ```

#### ¿Qué hace el script?

1. ✅ Verifica que se ejecuta como Administrador
2. ✅ Verifica que Docker Compose está instalado
3. ✅ Crea un script auxiliar `docker-autostart.ps1`
4. ✅ Crea una tarea programada llamada **"IrrigacionBackendAutostart"**
5. ✅ Configura la tarea para:
   - Ejecutarse al iniciar Windows
   - Funcionar sin que nadie esté logueado (cuenta SYSTEM)
   - Esperar 30 segundos antes de arrancar
   - Reintentar 3 veces si falla

#### Resultado:

Al reiniciar Windows, los servicios arrancarán automáticamente:
- 🐘 **PostgreSQL** (puerto 5432)
- 📡 **HiveMQ MQTT** (puerto 1883, Web UI 8000)
- ☕ **Backend Spring Boot** (puerto 8080)

---

### 🗑️ Eliminar Arranque Automático del Backend

**Script:** `remove-autostart.ps1`

Para desactivar el arranque automático del backend.

#### Ejecución:

```powershell
# Como Administrador
.\remove-autostart.ps1
```

O desde PowerShell normal:
```powershell
Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PWD\remove-autostart.ps1`""
```

#### ¿Qué hace?

1. ✅ Elimina la tarea programada "IrrigacionBackendAutostart"
2. ✅ Elimina el script auxiliar `docker-autostart.ps1`

---

## Configuración del Túnel Cloudflare

### 🚀 Instalar Arranque Automático del Túnel

**Script:** `setup-cloudflare-tunnel.ps1`

Este script configura el arranque automático del túnel Cloudflare.

#### Requisitos previos:

1. **Cloudflared instalado:**
   ```powershell
   # Verificar instalación
   cloudflared --version
   ```
   
   Si no está instalado, descargarlo de:  
   https://developers.cloudflare.com/cloudflare-one/connections/connect-apps/install-and-setup/installation/

2. **Archivos de configuración existentes:**
   - `C:\ProgramData\cloudflared\config.yml`
   - `C:\ProgramData\cloudflared\tunnel.json`

#### Paso a paso:

1. **Ejecutar como Administrador:**
   ```powershell
   .\setup-cloudflare-tunnel.ps1
   ```

2. **O desde PowerShell normal:**
   ```powershell
   Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PWD\setup-cloudflare-tunnel.ps1`""
   ```

#### ¿Qué hace el script?

1. ✅ Verifica permisos de Administrador
2. ✅ Verifica que `cloudflared` está instalado
3. ✅ Verifica que existen los archivos de configuración
4. ✅ Crea una tarea programada llamada **"CloudflareTunnelIrrigacion"**
5. ✅ Configura la tarea para:
   - Ejecutarse al iniciar Windows
   - Funcionar sin login (cuenta SYSTEM)
   - Esperar 45 segundos (para que backend arranque primero)
   - Reintentar 3 veces si falla
   - Sin límite de tiempo de ejecución

#### Comando ejecutado:

```powershell
cloudflared --config C:\ProgramData\cloudflared\config.yml --credentials-file C:\ProgramData\cloudflared\tunnel.json tunnel run irrigacion-backend
```

---

### 🗑️ Eliminar Arranque Automático del Túnel

**Script:** `remove-cloudflare-tunnel.ps1`

Para desactivar el arranque automático del túnel Cloudflare.

#### Ejecución:

```powershell
# Como Administrador
.\remove-cloudflare-tunnel.ps1
```

O desde PowerShell normal:
```powershell
Start-Process powershell -Verb RunAs -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$PWD\remove-cloudflare-tunnel.ps1`""
```

#### ¿Qué hace?

1. ✅ Elimina la tarea programada "CloudflareTunnelIrrigacion"

---

## Verificación

### Ver Tareas Programadas

1. **Abrir Programador de Tareas:**
   ```powershell
   taskschd.msc
   ```

2. **Buscar las tareas:**
   - `IrrigacionBackendAutostart`
   - `CloudflareTunnelIrrigacion`

### Probar Tareas Manualmente

```powershell
# Probar backend
Start-ScheduledTask -TaskName "IrrigacionBackendAutostart"

# Probar túnel Cloudflare
Start-ScheduledTask -TaskName "CloudflareTunnelIrrigacion"
```

### Verificar Estado de Servicios

#### Backend (Docker Compose):
```powershell
cd C:\proyectos\dac\irrigacion-mqtt-repo
docker-compose ps
```

Deberías ver:
```
NAME                  STATUS              PORTS
irrigacion-postgres   Up (healthy)        0.0.0.0:5432->5432/tcp
irrigacion-mqtt       Up                  0.0.0.0:1883->1883/tcp, 0.0.0.0:8000->8000/tcp
irrigacion-backend    Up                  0.0.0.0:8080->8080/tcp
```

#### Túnel Cloudflare:
```powershell
# Ver información del túnel
cloudflared tunnel info irrigacion-backend

# Ver conexiones activas
cloudflared tunnel list
```

### Ver Logs

#### Backend:
```powershell
# Logs de todos los servicios
docker-compose logs

# Solo backend
docker-compose logs -f backend

# Solo postgres
docker-compose logs -f postgres
```

#### Túnel Cloudflare:
El túnel se ejecuta como servicio del sistema. Para ver logs:
```powershell
# Ver eventos de la tarea programada en Event Viewer
eventvwr.msc
# Navegar a: Windows Logs > Application
# Filtrar por: Task Scheduler
```

---

## Orden de Arranque

Al iniciar Windows, los servicios arrancan en este orden:

```
┌─────────────────────────────────────────────────────────────┐
│                    Windows se inicia                        │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
         ┌───────────────────────────────┐
         │   Docker Desktop arranca      │
         │   (configurado en Settings)   │
         └───────────┬───────────────────┘
                     │
                     │ ⏱️  30 segundos
                     ▼
         ┌───────────────────────────────┐
         │   Tarea: IrrigacionBackend    │
         │   Ejecuta: docker-compose up  │
         └───────────┬───────────────────┘
                     │
                     ├─► 🐘 PostgreSQL (puerto 5432)
                     ├─► 📡 MQTT Broker (puerto 1883)
                     └─► ☕ Backend (puerto 8080)
                     │
                     │ ⏱️  15 segundos más
                     ▼
         ┌───────────────────────────────┐
         │ Tarea: CloudflareTunnel       │
         │ Ejecuta: cloudflared tunnel   │
         └───────────┬───────────────────┘
                     │
                     └─► 🌐 Túnel Cloudflare conectado
```

**Total: ~45-60 segundos** desde el inicio de Windows hasta tener todo funcionando.

---

## Gestión de Tareas

### Estado de las Tareas

```powershell
# Listar todas las tareas
Get-ScheduledTask | Where-Object {$_.TaskName -like "*Irrigacion*"}

# Estado detallado del backend
Get-ScheduledTask -TaskName "IrrigacionBackendAutostart" | Format-List *

# Estado detallado del túnel
Get-ScheduledTask -TaskName "CloudflareTunnelIrrigacion" | Format-List *
```

### Detener Tareas Manualmente

```powershell
# Detener backend
Stop-ScheduledTask -TaskName "IrrigacionBackendAutostart"

# Detener túnel
Stop-ScheduledTask -TaskName "CloudflareTunnelIrrigacion"
```

### Deshabilitar Temporalmente

```powershell
# Deshabilitar backend (no arrancará en próximo reinicio)
Disable-ScheduledTask -TaskName "IrrigacionBackendAutostart"

# Deshabilitar túnel
Disable-ScheduledTask -TaskName "CloudflareTunnelIrrigacion"

# Volver a habilitar
Enable-ScheduledTask -TaskName "IrrigacionBackendAutostart"
Enable-ScheduledTask -TaskName "CloudflareTunnelIrrigacion"
```

---

## Solución de Problemas

### ❌ El backend no arranca

1. **Verificar que Docker Desktop está configurado:**
   - Docker Desktop → Settings → General
   - ✅ "Start Docker Desktop when you log in"

2. **Ver logs de la tarea:**
   ```powershell
   Get-ScheduledTaskInfo -TaskName "IrrigacionBackendAutostart"
   ```

3. **Probar manualmente:**
   ```powershell
   cd C:\proyectos\dac\irrigacion-mqtt-repo
   docker-compose up -d
   ```

4. **Aumentar el retraso:**
   Si Docker Desktop tarda mucho en arrancar, editar la tarea en `taskschd.msc`:
   - Buscar "IrrigacionBackendAutostart"
   - Triggers → Editar
   - Aumentar "Delay task for" a 60 segundos

### ❌ El túnel Cloudflare no se conecta

1. **Verificar que cloudflared está en el PATH:**
   ```powershell
   cloudflared --version
   ```

2. **Verificar archivos de configuración:**
   ```powershell
   Test-Path C:\ProgramData\cloudflared\config.yml
   Test-Path C:\ProgramData\cloudflared\tunnel.json
   ```

3. **Probar manualmente:**
   ```powershell
   cloudflared --config C:\ProgramData\cloudflared\config.yml --credentials-file C:\ProgramData\cloudflared\tunnel.json tunnel run irrigacion-backend
   ```

4. **Ver estado del túnel:**
   ```powershell
   cloudflared tunnel info irrigacion-backend
   ```

### ❌ Error de permisos

Si los scripts no se ejecutan:

```powershell
# Verificar política de ejecución
Get-ExecutionPolicy

# Cambiar temporalmente (como Administrador)
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass

# Ejecutar script
.\setup-autostart.ps1
```

### ❌ Los servicios no arrancan sin login

Verificar que las tareas están configuradas correctamente:
- **Usuario:** SYSTEM
- **Logon type:** Service Account
- **Run Level:** Highest
- ✅ "Run whether user is logged on or not"

---

## Consideraciones Importantes

### 🔒 Seguridad

- ✅ Las tareas se ejecutan como **cuenta SYSTEM** (máximos privilegios)
- ✅ Los archivos de configuración están en rutas protegidas
- ⚠️ **Cloudflare tunnel.json** contiene credenciales sensibles
- ⚠️ Mantener permisos restrictivos en `C:\ProgramData\cloudflared\`

### 💾 Recursos

- **PostgreSQL:** ~100MB RAM
- **MQTT Broker:** ~50MB RAM  
- **Backend:** ~300-500MB RAM
- **Cloudflared:** ~20-30MB RAM

**Total:** ~500-700MB RAM usado permanentemente

### 🔄 Actualizaciones

Después de actualizar el código:

```powershell
# Detener servicios
docker-compose down

# Reconstruir imágenes
docker-compose build

# Levantar de nuevo
docker-compose up -d
```

Las tareas programadas seguirán funcionando sin cambios.

### 🌐 Conectividad

El túnel Cloudflare requiere:
- ✅ Conexión a Internet activa
- ✅ Backend corriendo en `http://localhost:8080`
- ✅ Túnel configurado y autenticado previamente

Si la conexión falla, el túnel reintentará automáticamente (configurado en la tarea).

### 📊 Monitoreo

Para monitoreo en producción, considerar:
- Logs centralizados (ELK Stack, Grafana Loki)
- Alertas de disponibilidad (Uptime Kuma, Grafana)
- Monitoreo de recursos (Prometheus + Grafana)

---

## Resumen de Scripts

| Script | Función | Requiere Admin |
|--------|---------|----------------|
| `setup-autostart.ps1` | Configura arranque automático del backend | ✅ Sí |
| `remove-autostart.ps1` | Elimina arranque automático del backend | ✅ Sí |
| `setup-cloudflare-tunnel.ps1` | Configura arranque automático del túnel | ✅ Sí |
| `remove-cloudflare-tunnel.ps1` | Elimina arranque automático del túnel | ✅ Sí |
| `docker-autostart.ps1` | Script auxiliar (creado automáticamente) | ❌ No |

---

## Referencias

- [Docker Desktop Documentation](https://docs.docker.com/desktop/windows/)
- [Cloudflare Tunnel Documentation](https://developers.cloudflare.com/cloudflare-one/connections/connect-apps/)
- [Windows Task Scheduler](https://learn.microsoft.com/en-us/windows/win32/taskschd/task-scheduler-start-page)
- [PowerShell Scheduled Tasks](https://learn.microsoft.com/en-us/powershell/module/scheduledtasks/)

---

**Última actualización:** 13 de diciembre de 2025
