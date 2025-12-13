# Script para eliminar el arranque automático del túnel Cloudflare
# Debe ejecutarse como Administrador

$ErrorActionPreference = "Stop"

# Verificar que se ejecuta como administrador
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "ERROR: Este script debe ejecutarse como Administrador" -ForegroundColor Red
    exit 1
}

$taskName = "CloudflareTunnelIrrigacion"

Write-Host "Eliminando tarea programada: $taskName" -ForegroundColor Yellow

$task = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($task) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
    Write-Host "Tarea eliminada exitosamente" -ForegroundColor Green
} else {
    Write-Host "La tarea no existe" -ForegroundColor Gray
}
