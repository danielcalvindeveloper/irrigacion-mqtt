import axios from 'axios'

const api = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080/api',
  headers: {
    'Content-Type': 'application/json'
  },
  timeout: 10000
})

// Interceptor para logging (desarrollo)
api.interceptors.request.use(
  (config) => {
    console.log(`🔵 ${config.method.toUpperCase()} ${config.url}`, config.data || '')
    return config
  },
  (error) => {
    console.error('❌ Request Error:', error)
    return Promise.reject(error)
  }
)

api.interceptors.response.use(
  (response) => {
    console.log(`🟢 ${response.config.method.toUpperCase()} ${response.config.url}`, response.data)
    return response
  },
  (error) => {
    console.error('❌ Response Error:', error.response?.data || error.message)
    return Promise.reject(error)
  }
)

// ========== NODOS ==========

/**
 * Obtener lista de nodos registrados
 * @returns {Promise} Lista de nodos
 */
export const getNodos = () => {
  return api.get('/nodos')
}

// ========== COMANDOS ==========

/**
 * Enviar comando de riego a una zona
 * @param {string} nodeId - ID del nodo ESP32
 * @param {number} zona - ID de la zona (1-4)
 * @param {string} accion - 'ON' o 'OFF'
 * @param {number} duracion - Duración en segundos (solo para ON)
 * @returns {Promise}
 */
export const enviarComando = (nodeId, zona, accion, duracion = null) => {
  const payload = { 
    nodeId,   // El backend requiere nodeId en el body
    zona, 
    accion
  }
  if (accion === 'ON' && duracion) {
    payload.duracion = duracion
  }
  return api.post(`/nodos/${nodeId}/cmd`, payload)
}

/**
 * Iniciar riego manual en una zona
 * @param {string} nodeId - ID del nodo ESP32
 * @param {number} zona - ID de la zona (1-4)
 * @param {number} duracionMinutos - Duración en minutos
 * @returns {Promise}
 */
export const iniciarRiegoManual = (nodeId, zona, duracionMinutos) => {
  return enviarComando(nodeId, zona, 'ON', duracionMinutos * 60) // Convertir a segundos
}

/**
 * Detener riego en una zona
 * @param {string} nodeId - ID del nodo ESP32
 * @param {number} zona - ID de la zona (1-4)
 * @returns {Promise}
 */
export const detenerRiego = (nodeId, zona) => {
  return enviarComando(nodeId, zona, 'OFF')
}

/**
 * Obtener estado de todas las zonas
 * @param {string} nodeId - ID del nodo ESP32
 * @returns {Promise} Array con estado de cada zona
 */
export const getZonesStatus = (nodeId) => {
  return api.get(`/nodos/${nodeId}/status`)
}

// ========== AGENDAS ==========

/**
 * Obtener todas las agendas de un nodo
 * @param {string} nodeId - ID del nodo ESP32
 * @returns {Promise} Lista de agendas + version
 */
export const getAgendas = (nodeId) => {
  return api.get(`/nodos/${nodeId}/agendas`)
}

/**
 * Crear nueva agenda
 * @param {string} nodeId - ID del nodo ESP32
 * @param {object} agenda - Datos de la agenda
 * @returns {Promise}
 */
export const crearAgenda = (nodeId, agenda) => {
  return api.post(`/nodos/${nodeId}/agendas`, agenda)
}

/**
 * Actualizar agenda existente
 * Nota: El backend usa POST upsert tanto para crear como actualizar
 * Por lo tanto, usar crearAgenda() para ambas operaciones
 */

/**
 * Eliminar agenda
 * @param {string} nodeId - ID del nodo ESP32
 * @param {string} agendaId - ID de la agenda
 * @returns {Promise}
 */
export const eliminarAgenda = (nodeId, agendaId) => {
  return api.delete(`/nodos/${nodeId}/agendas/${agendaId}`)
}

// ========== HISTORIAL ==========

/**
 * Obtener eventos de riego
 * @param {string} nodeId - ID del nodo ESP32
 * @param {object} filtros - Filtros opcionales (zona, desde, hasta)
 * @returns {Promise} Lista de eventos
 */
export const getEventos = (nodeId, filtros = {}) => {
  return api.get(`/nodos/${nodeId}/eventos`, { params: filtros })
}

/**
 * Obtener lecturas de humedad
 * @param {string} nodeId - ID del nodo ESP32
 * @param {object} filtros - Filtros opcionales (zona, desde, hasta)
 * @returns {Promise} Lecturas de humedad
 */
export const getHumedad = (nodeId, filtros = {}) => {
  return api.get(`/nodos/${nodeId}/humedad`, { params: filtros })
}

// ========== HEALTH CHECK ==========

/**
 * Verificar conexión con el backend
 * @returns {Promise}
 */
export const healthCheck = () => {
  return api.get('/health')
}

export default api
