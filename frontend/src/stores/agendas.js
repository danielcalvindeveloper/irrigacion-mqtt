import { defineStore } from 'pinia'
import { ref } from 'vue'
import * as api from '@/services/api'

export const useAgendasStore = defineStore('agendas', () => {
  // State
  const agendas = ref([])
  const loading = ref(false)
  const error = ref(null)
  const nodeId = ref(import.meta.env.VITE_DEFAULT_NODE_ID)

  // Actions
  async function fetchAgendas() {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.getAgendas(nodeId.value)
      agendas.value = response.data || [] // Backend devuelve array directo
    } catch (err) {
      error.value = err.message
      console.error('Error al cargar agendas:', err)
      
      // Fallback a datos mockeados
      agendas.value = getMockedAgendas()
    } finally {
      loading.value = false
    }
  }

  async function crearAgenda(agendaData) {
    loading.value = true
    error.value = null
    
    try {
      const response = await api.crearAgenda(nodeId.value, agendaData)
      await fetchAgendas() // Recargar lista
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al crear agenda:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function actualizarAgenda(agendaId, agendaData) {
    loading.value = true
    error.value = null
    
    try {
      // Backend usa POST upsert tanto para crear como actualizar
      const response = await api.crearAgenda(nodeId.value, agendaData)
      await fetchAgendas() // Recargar lista
      return response.data
    } catch (err) {
      error.value = err.message
      console.error('Error al actualizar agenda:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function eliminarAgenda(agendaId) {
    loading.value = true
    error.value = null
    
    try {
      await api.eliminarAgenda(nodeId.value, agendaId)
      await fetchAgendas() // Recargar lista
    } catch (err) {
      error.value = err.message
      console.error('Error al eliminar agenda:', err)
      throw err
    } finally {
      loading.value = false
    }
  }

  async function toggleAgenda(agenda) {
    try {
      // Optimistic update
      const index = agendas.value.findIndex(a => a.id === agenda.id)
      if (index !== -1) {
        agendas.value[index].activa = !agendas.value[index].activa
      }

      // Update en servidor - backend usa POST upsert
      const agendaData = {
        id: agenda.id,
        nodeId: nodeId.value,
        zona: agenda.zona,
        diasSemana: agenda.diasSemana,
        horaInicio: agenda.horaInicio || agenda.hora,
        duracionMin: agenda.duracionMin || agenda.duracionMinutos,
        activa: !agenda.activa
      }
      // Agregar nombre si existe
      if (agenda.nombre) {
        agendaData.nombre = agenda.nombre
      }
      
      await api.crearAgenda(nodeId.value, agendaData)
    } catch (err) {
      // Revertir cambio optimista
      const index = agendas.value.findIndex(a => a.id === agenda.id)
      if (index !== -1) {
        agendas.value[index].activa = !agendas.value[index].activa
      }
      throw err
    }
  }

  // Helper: Datos mockeados
  function getMockedAgendas() {
    return [
      {
        id: 1,
        nombre: 'Riego Matutino Jardín',
        zona: 1,
        hora: '07:00',
        duracionMinutos: 15,
        diasSemana: ['LUN', 'MIE', 'VIE'],
        activa: true
      },
      {
        id: 2,
        nombre: 'Riego Nocturno Huerta',
        zona: 3,
        hora: '19:00',
        duracionMinutos: 20,
        diasSemana: ['LUN', 'MAR', 'MIE', 'JUE', 'VIE', 'SAB', 'DOM'],
        activa: true
      },
      {
        id: 3,
        nombre: 'Riego Fin de Semana',
        zona: 2,
        hora: '08:00',
        duracionMinutos: 10,
        diasSemana: ['SAB', 'DOM'],
        activa: false
      }
    ]
  }

  return {
    // State
    agendas,
    loading,
    error,
    nodeId,
    // Actions
    fetchAgendas,
    crearAgenda,
    actualizarAgenda,
    eliminarAgenda,
    toggleAgenda
  }
})
