import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import * as api from '@/services/api'

export const useZoneConfigStore = defineStore('zoneConfig', () => {
  // State
  const configs = ref([])
  const loading = ref(false)
  const error = ref(null)
  const nodeId = ref(import.meta.env.VITE_DEFAULT_NODE_ID)

  // Getters
  const zonasHabilitadas = computed(() => {
    return configs.value.filter(z => z.habilitada).sort((a, b) => a.orden - b.orden)
  })

  const zonasDisponibles = computed(() => {
    return zonasHabilitadas.value.map(z => ({
      title: z.nombre,
      value: z.zona
    }))
  })

  // Actions
  async function fetchConfigs(soloHabilitadas = false) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.getZoneConfigs(nodeId.value, soloHabilitadas)
      configs.value = response.data.sort((a, b) => a.orden - b.orden)
    } catch (err) {
      error.value = err.message
      console.error('Error al cargar configuración de zonas:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function getConfig(zona) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.getZoneConfig(nodeId.value, zona)
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al obtener configuración de zona:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function upsertConfig(configData) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.upsertZoneConfig(nodeId.value, configData)
      await fetchConfigs()
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al guardar configuración de zona:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function updateNombre(zona, nombre) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.updateZoneNombre(nodeId.value, zona, nombre)
      
      // Actualizar local
      const config = configs.value.find(z => z.zona === zona)
      if (config) {
        config.nombre = nombre
        config.updatedAt = response.data.updatedAt
      }
      
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al actualizar nombre de zona:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function toggleHabilitada(zona) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.toggleZoneHabilitada(nodeId.value, zona)
      
      // Actualizar local
      const config = configs.value.find(z => z.zona === zona)
      if (config) {
        config.habilitada = response.data.habilitada
        config.updatedAt = response.data.updatedAt
      }
      
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al cambiar estado de zona:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function deleteConfig(zona) {
    loading.value = true
    error.value = null
    
    try {
      await api.deleteZoneConfig(nodeId.value, zona)
      
      // Actualizar local
      const config = configs.value.find(z => z.zona === zona)
      if (config) {
        config.habilitada = false
      }
    } catch (err) {
      error.value = err.message
      console.error('Error al eliminar configuración de zona:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function reorderZones(zonasOrdenadas) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.reorderZones(nodeId.value, zonasOrdenadas)
      configs.value = response.data.sort((a, b) => a.orden - b.orden)
    } catch (err) {
      error.value = err.message
      console.error('Error al reordenar zonas:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  function getZoneName(zona) {
    const config = configs.value.find(z => z.zona === zona)
    return config ? config.nombre : `Zona ${zona}`
  }

  function isZoneEnabled(zona) {
    const config = configs.value.find(z => z.zona === zona)
    return config ? config.habilitada : true
  }

  function setNodeId(newNodeId) {
    nodeId.value = newNodeId
    fetchConfigs()
  }

  return {
    // State
    configs,
    loading,
    error,
    nodeId,
    // Getters
    zonasHabilitadas,
    zonasDisponibles,
    // Actions
    fetchConfigs,
    getConfig,
    upsertConfig,
    updateNombre,
    toggleHabilitada,
    deleteConfig,
    reorderZones,
    getZoneName,
    isZoneEnabled,
    setNodeId
  }
})
