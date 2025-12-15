import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import * as api from '@/services/api'

export const useZonesStore = defineStore('zones', () => {
  // State
  const zones = ref([])
  const loading = ref(false)
  const error = ref(null)
  const connected = ref(false)
  const nodeId = ref(import.meta.env.VITE_DEFAULT_NODE_ID)

  // Getters
  const activeZones = computed(() => {
    return zones.value.filter(z => z.active)
  })

  const idleZones = computed(() => {
    return zones.value.filter(z => !z.active)
  })

  // Actions
  async function fetchZonesStatus() {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.getZonesStatus(nodeId.value)
      zones.value = response.data.map(z => ({
        id: z.zona,
        name: z.nombre,
        active: z.activa,
        remainingTime: Math.ceil((z.tiempoRestanteSeg || 0) / 60), // Convertir segundos a minutos
        progress: z.activa && z.tiempoRestanteSeg ? 
          Math.max(0, Math.min(100, ((z.tiempoRestanteSeg / 3600) * 100))) : 0,
        nextSchedule: z.proximoRiego
      }))
      connected.value = true
    } catch (err) {
      error.value = err.message
      connected.value = false
      console.error('Error al cargar zonas:', err)
      // Fallback a datos mockeados en caso de error
      zones.value = getMockedZones()
    } finally {
      loading.value = false
    }
  }

  async function startWatering(zonaId, duracion) {
    loading.value = true
    error.value = null
    
    try {
      await api.iniciarRiegoManual(nodeId.value, zonaId, duracion)
      
      // Actualizar estado local optimísticamente
      const zona = zones.value.find(z => z.id === zonaId)
      if (zona) {
        zona.active = true
        zona.remainingTime = duracion
        zona.progress = 0
      }
      
      // Recargar estado real del servidor
      await fetchZonesStatus()
    } catch (err) {
      error.value = err.message
      console.error('Error al iniciar riego:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function stopWatering(zonaId) {
    loading.value = true
    error.value = null
    
    try {
      await api.detenerRiego(nodeId.value, zonaId)
      
      // Actualizar estado local
      const zona = zones.value.find(z => z.id === zonaId)
      if (zona) {
        zona.active = false
        zona.remainingTime = 0
        zona.progress = 0
      }
      
      // Recargar estado real
      await fetchZonesStatus()
    } catch (err) {
      error.value = err.message
      console.error('Error al detener riego:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  function setNodeId(newNodeId) {
    nodeId.value = newNodeId
    fetchZonesStatus()
  }

  // Helper: Datos mockeados para desarrollo sin backend
  function getMockedZones() {
    return [
      {
        id: 1,
        name: 'Zona 1 - Jardín Frontal',
        active: false,
        progress: 0,
        remainingTime: 0,
        nextSchedule: 'Hoy 19:00 (15min)'
      },
      {
        id: 2,
        name: 'Zona 2 - Jardín Lateral',
        active: false,
        progress: 0,
        remainingTime: 0,
        nextSchedule: 'Hoy 19:00 (15min)'
      },
      {
        id: 3,
        name: 'Zona 3 - Huerta',
        active: false,
        progress: 0,
        remainingTime: 0,
        nextSchedule: 'Mañana 07:00 (20min)'
      },
      {
        id: 4,
        name: 'Zona 4 - Patio',
        active: false,
        progress: 0,
        remainingTime: 0,
        nextSchedule: null
      }
    ]
  }

  return {
    // State
    zones,
    loading,
    error,
    connected,
    nodeId,
    // Getters
    activeZones,
    idleZones,
    // Actions
    fetchZonesStatus,
    startWatering,
    stopWatering,
    setNodeId
  }
})
