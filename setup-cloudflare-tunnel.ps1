# Script para configurar el arranque automático del túnel Cloudflare
# Debe ejecutarse como Administrador

$ErrorActionPreference = "Stop"

# Verificar que se ejecuta como administrador
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "ERROR: Este script debe ejecutarse como Administrador" -ForegroundColor Red
    Write-Host "Haz clic derecho en PowerShell y selecciona 'Ejecutar como administrador'" -ForegroundColor Yellow
    exit 1
}

# Configuración
$taskName = "CloudflareTunnelIrrigacion"
$configPath = "C:\ProgramData\cloudflared\config.yml"
$credentialsPath = "C:\ProgramData\cloudflared\tunnel.json"

# Verificar que existen los archivos de configuración
if (-not (Test-Path $configPath)) {
    Write-Host "ERROR: No se encontró el archivo de configuración: $configPath" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $credentialsPath)) {
    Write-Host "ERROR: No se encontró el archivo de credenciales: $credentialsPath" -ForegroundColor Red
    exit 1
}

# Verificar que cloudflared está instalado y obtener la ruta completa
try {
    $cloudflaredCheck = Get-Command cloudflared -ErrorAction Stop
    $cloudflaredPath = $cloudflaredCheck.Source
    Write-Host "cloudflared encontrado en: $cloudflaredPath" -ForegroundColor Green
} catch {
    Write-Host "ERROR: cloudflared no está instalado o no está en el PATH" -ForegroundColor Red
    Write-Host "Instala cloudflared desde: https://developers.cloudflare.com/cloudflare-one/connections/connect-apps/install-and-setup/installation/" -ForegroundColor Yellow
    exit 1
}

Write-Host "=== Configurando arranque automático del túnel Cloudflare ===" -ForegroundColor Cyan
Write-Host ""

# Eliminar tarea existente si existe
$existingTask = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($existingTask) {
    Write-Host "Eliminando tarea existente..." -ForegroundColor Yellow
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
}

# Crear acción de la tarea (usando ruta completa para que SYSTEM lo encuentre)
$arguments = "--config `"$configPath`" --credentials-file `"$credentialsPath`" tunnel run irrigacion-backend"
$action = New-ScheduledTaskAction -Execute $cloudflaredPath -Argument $arguments

# Crear trigger (al iniciar el sistema, con retraso de 45 segundos)
$trigger = New-ScheduledTaskTrigger -AtStartup
$trigger.Delay = "PT45S"  # Retraso de 45 segundos para que Docker y backend arranquen primero

# Configurar para ejecutarse sin que el usuario esté logueado
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest

# Configuración adicional
$settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -RestartCount 3 `
    -RestartInterval (New-TimeSpan -Minutes 1) `
    -ExecutionTimeLimit (New-TimeSpan -Hours 0)  # Sin límite de tiempo

# Registrar la tarea
Write-Host "Registrando tarea programada..." -ForegroundColor Yellow
Register-ScheduledTask `
    -TaskName $taskName `
    -Action $action `
    -Trigger $trigger `
    -Principal $principal `
    -Settings $settings `
    -Description "Inicia automáticamente el túnel Cloudflare para Irrigacion Backend al arrancar Windows" | Out-Null

Write-Host ""
Write-Host "=== ¡Configuración completada! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Tarea programada creada: $taskName" -ForegroundColor Cyan
Write-Host "El túnel Cloudflare arrancarán automáticamente al iniciar Windows" -ForegroundColor Gray
Write-Host "incluso si nadie está logueado." -ForegroundColor Gray
Write-Host ""
Write-Host "Configuración:" -ForegroundColor Yellow
Write-Host "  Ejecutable:  $cloudflaredPath" -ForegroundColor Gray
Write-Host "  Config:      $configPath" -ForegroundColor Gray
Write-Host "  Credentials: $credentialsPath" -ForegroundColor Gray
Write-Host "  Tunnel:      irrigacion-backend" -ForegroundColor Gray
Write-Host ""
Write-Host "Para verificar la tarea:" -ForegroundColor Cyan
Write-Host "  taskschd.msc" -ForegroundColor Gray
Write-Host ""
Write-Host "Para probar manualmente:" -ForegroundColor Cyan
Write-Host "  Start-ScheduledTask -TaskName '$taskName'" -ForegroundColor Gray
Write-Host ""
Write-Host "Para ver el estado del túnel:" -ForegroundColor Cyan
Write-Host "  cloudflared tunnel info irrigacion-backend" -ForegroundColor Gray
Write-Host ""
