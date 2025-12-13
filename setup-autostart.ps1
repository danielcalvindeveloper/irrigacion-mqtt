# Script para configurar el arranque automático de Docker Compose sin login
# Debe ejecutarse como Administrador

$ErrorActionPreference = "Stop"

# Verificar que se ejecuta como administrador
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "ERROR: Este script debe ejecutarse como Administrador" -ForegroundColor Red
    Write-Host "Haz clic derecho en PowerShell y selecciona 'Ejecutar como administrador'" -ForegroundColor Yellow
    exit 1
}

# Rutas
$projectPath = $PSScriptRoot
$taskName = "IrrigacionBackendAutostart"
$dockerComposePath = "C:\Program Files\Docker\Docker\resources\bin\docker-compose.exe"

# Verificar que docker-compose existe
if (-not (Test-Path $dockerComposePath)) {
    Write-Host "ERROR: No se encontró docker-compose en: $dockerComposePath" -ForegroundColor Red
    Write-Host "Verifica que Docker Desktop esté instalado" -ForegroundColor Yellow
    exit 1
}

Write-Host "=== Configurando arranque automático ===" -ForegroundColor Cyan
Write-Host "Proyecto: $projectPath" -ForegroundColor Gray
Write-Host ""

# Eliminar tarea existente si existe
$existingTask = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($existingTask) {
    Write-Host "Eliminando tarea existente..." -ForegroundColor Yellow
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
}

# Crear script de inicio
$startupScript = @"
# Script de inicio automático para Irrigacion Backend
Set-Location -Path "$projectPath"
& "$dockerComposePath" up -d
"@

$startupScriptPath = Join-Path $projectPath "docker-autostart.ps1"
$startupScript | Out-File -FilePath $startupScriptPath -Encoding UTF8 -Force

Write-Host "Script de inicio creado: $startupScriptPath" -ForegroundColor Green

# Crear acción de la tarea
$action = New-ScheduledTaskAction -Execute "powershell.exe" `
    -Argument "-NoProfile -ExecutionPolicy Bypass -File `"$startupScriptPath`"" `
    -WorkingDirectory $projectPath

# Crear trigger (al iniciar el sistema, con retraso de 30 segundos)
$trigger = New-ScheduledTaskTrigger -AtStartup
$trigger.Delay = "PT30S"  # Retraso de 30 segundos para que Docker Desktop arranque primero

# Configurar para ejecutarse sin que el usuario esté logueado
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest

# Configuración adicional
$settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -RestartCount 3 `
    -RestartInterval (New-TimeSpan -Minutes 1)

# Registrar la tarea
Write-Host "Registrando tarea programada..." -ForegroundColor Yellow
Register-ScheduledTask `
    -TaskName $taskName `
    -Action $action `
    -Trigger $trigger `
    -Principal $principal `
    -Settings $settings `
    -Description "Inicia automáticamente los servicios de Irrigacion Backend (PostgreSQL, MQTT, Backend) al iniciar Windows" | Out-Null

Write-Host ""
Write-Host "=== ¡Configuración completada! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Tarea programada creada: $taskName" -ForegroundColor Cyan
Write-Host "Los servicios arrancarán automáticamente al iniciar Windows" -ForegroundColor Gray
Write-Host "incluso si nadie está logueado." -ForegroundColor Gray
Write-Host ""
Write-Host "IMPORTANTE: Asegúrate de configurar Docker Desktop para iniciar con Windows:" -ForegroundColor Yellow
Write-Host "  1. Abre Docker Desktop" -ForegroundColor Gray
Write-Host "  2. Ve a Settings > General" -ForegroundColor Gray
Write-Host "  3. Activa 'Start Docker Desktop when you log in'" -ForegroundColor Gray
Write-Host ""
Write-Host "Para verificar la tarea:" -ForegroundColor Cyan
Write-Host "  taskschd.msc" -ForegroundColor Gray
Write-Host ""
Write-Host "Para probar manualmente:" -ForegroundColor Cyan
Write-Host "  Start-ScheduledTask -TaskName '$taskName'" -ForegroundColor Gray
Write-Host ""
