<template>
  <v-container fluid class="pa-4 pb-16">
    <!-- Header -->
    <div class="d-flex align-center justify-space-between mb-4">
      <h1 class="text-h5">Agendas de Riego</h1>
      <v-btn
        color="primary"
        prepend-icon="mdi-plus"
        @click="openCreateDialog"
      >
        Nueva
      </v-btn>
    </div>

    <!-- Lista de agendas -->
    <div v-if="agendas.length > 0" class="agendas-list">
      <v-card
        v-for="agenda in agendas"
        :key="agenda.id"
        class="mb-3"
        :color="agenda.activa ? 'primary' : 'surface'"
        :variant="agenda.activa ? 'tonal' : 'elevated'"
      >
        <v-card-title class="d-flex align-center">
          <v-icon 
            :color="agenda.activa ? 'primary' : 'grey'" 
            class="mr-2"
          >
            mdi-calendar-clock
          </v-icon>
          {{ agenda.nombre }}
          <v-spacer></v-spacer>
          <v-switch
            :model-value="agenda.activa"
            color="primary"
            hide-details
            density="compact"
            @click.stop
            @update:model-value="toggleAgendaStatus(agenda)"
          ></v-switch>
        </v-card-title>

        <v-card-text>
          <div class="mb-2">
            <v-chip size="small" color="secondary" class="mr-2">
              <v-icon start size="small">mdi-clock-outline</v-icon>
              {{ agenda.hora }}
            </v-chip>
            <v-chip size="small" color="info">
              <v-icon start size="small">mdi-timer-outline</v-icon>
              {{ agenda.duracionMinutos }} min
            </v-chip>
          </div>

          <div class="text-body-2 text-grey mb-2">
            <v-icon size="small">mdi-map-marker</v-icon>
            {{ agenda.zonaName }}
          </div>

          <div class="mb-2">
            <div class="text-caption text-grey mb-1">
              <v-icon size="small">mdi-repeat</v-icon>
              Días de riego
            </div>
            <v-chip-group>
              <v-chip 
                v-for="dia in ['LUN', 'MAR', 'MIE', 'JUE', 'VIE', 'SAB', 'DOM']"
                :key="dia"
                size="small"
                :color="agenda.diasSemana.includes(dia) ? 'primary' : 'default'"
                :variant="agenda.diasSemana.includes(dia) ? 'flat' : 'outlined'"
                disabled
              >
                {{ getDiaAbreviado(dia) }}
              </v-chip>
            </v-chip-group>
          </div>
        </v-card-text>

        <v-card-actions>
          <v-btn
            variant="text"
            size="small"
            @click="editAgenda(agenda)"
          >
            <v-icon start>mdi-pencil</v-icon>
            Editar
          </v-btn>
          <v-btn
            variant="text"
            size="small"
            color="error"
            @click="deleteAgenda(agenda)"
          >
            <v-icon start>mdi-delete</v-icon>
            Eliminar
          </v-btn>
        </v-card-actions>
      </v-card>
    </div>

    <!-- Empty state -->
    <v-card v-else class="text-center pa-8" variant="tonal">
      <v-icon size="64" color="grey-lighten-1">mdi-calendar-blank</v-icon>
      <div class="text-h6 mt-4 mb-2">Sin agendas configuradas</div>
      <div class="text-body-2 text-grey mb-4">
        Crea tu primera agenda para automatizar el riego
      </div>
      <v-btn color="primary" size="large" @click="openCreateDialog">
        <v-icon start>mdi-plus</v-icon>
        Crear Agenda
      </v-btn>
    </v-card>

    <!-- Dialog crear/editar agenda -->
    <v-dialog v-model="showDialog" max-width="500">
      <v-card>
        <v-card-title>
          {{ editingAgenda ? 'Editar Agenda' : 'Nueva Agenda' }}
        </v-card-title>

        <v-card-text>
          <v-text-field
            v-model="formData.nombre"
            label="Nombre (opcional)"
            placeholder="Se generará automáticamente si no se especifica"
            variant="outlined"
            class="mb-3"
            hint="Ejemplo: Riego Matutino"
            persistent-hint
          ></v-text-field>

          <v-select
            v-model="formData.zona"
            :items="zonasOptions"
            label="Zona"
            variant="outlined"
            class="mb-3"
          ></v-select>

          <v-text-field
            v-model="formData.hora"
            label="Hora de inicio"
            type="time"
            variant="outlined"
            class="mb-3"
          ></v-text-field>

          <v-text-field
            v-model.number="formData.duracionMinutos"
            label="Duración (minutos)"
            type="number"
            :min="1"
            :max="180"
            variant="outlined"
            class="mb-3"
          ></v-text-field>

          <div class="mb-3">
            <div class="text-body-2 mb-2">Días de la semana</div>
            <v-chip-group
              v-model="formData.diasSemana"
              selected-class="text-primary"
              multiple
              column
            >
              <v-chip value="LUN" filter variant="outlined">Lun</v-chip>
              <v-chip value="MAR" filter variant="outlined">Mar</v-chip>
              <v-chip value="MIE" filter variant="outlined">Mié</v-chip>
              <v-chip value="JUE" filter variant="outlined">Jue</v-chip>
              <v-chip value="VIE" filter variant="outlined">Vie</v-chip>
              <v-chip value="SAB" filter variant="outlined">Sáb</v-chip>
              <v-chip value="DOM" filter variant="outlined">Dom</v-chip>
            </v-chip-group>
          </div>

          <v-switch
            v-model="formData.activa"
            label="Agenda activa"
            color="primary"
          ></v-switch>
        </v-card-text>

        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn
            variant="text"
            @click="closeDialog"
          >
            Cancelar
          </v-btn>
          <v-btn
            color="primary"
            variant="elevated"
            @click="saveAgenda"
            :loading="loading"
          >
            {{ editingAgenda ? 'Actualizar' : 'Crear' }}
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-container>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useAgendasStore } from '@/stores/agendas'
import { useZoneConfigStore } from '@/stores/zoneConfig'

const agendasStore = useAgendasStore()
const zoneConfigStore = useZoneConfigStore()

// State
const showDialog = ref(false)
const editingAgenda = ref(null)
const formData = ref({
  nombre: '',
  zona: 1,
  hora: '07:00',
  duracionMinutos: 15,
  diasSemana: [],
  activa: true
})

// Opciones de zonas (dinámicas desde configuración)
const zonasOptions = computed(() => zoneConfigStore.zonasDisponibles)

// Mapeo de días
const diasMap = {
  'LUN': 'Lun',
  'MAR': 'Mar',
  'MIE': 'Mié',
  'JUE': 'Jue',
  'VIE': 'Vie',
  'SAB': 'Sáb',
  'DOM': 'Dom'
}

// Helper para obtener abreviatura de día
const getDiaAbreviado = (dia) => diasMap[dia] || dia

// Helper para generar UUID compatible
const generateUUID = () => {
  if (typeof crypto !== 'undefined' && crypto.randomUUID) {
    return crypto.randomUUID()
  }
  // Fallback para navegadores sin crypto.randomUUID
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    const r = Math.random() * 16 | 0
    const v = c === 'x' ? r : (r & 0x3 | 0x8)
    return v.toString(16)
  })
}

// Computed
const agendas = computed(() => agendasStore.agendas.map(a => {
  const hora = a.horaInicio || a.hora
  return {
    ...a,
    hora, // Backend devuelve horaInicio
    duracionMinutos: a.duracionMin || a.duracionMinutos, // Backend devuelve duracionMin
    nombre: a.nombre || `Zona ${a.zona} - ${hora}`, // Generar nombre si no existe
    zonaName: zoneConfigStore.getZoneName(a.zona), // Usar nombre de configuración
    diasFormatted: formatDias(a.diasSemana || [])
  }
}))
const loading = computed(() => agendasStore.loading)

// Functions
const formatDias = (dias) => {
  if (dias.length === 7) return 'Todos los días'
  if (dias.length === 5 && !dias.includes('SAB') && !dias.includes('DOM')) {
    return 'Lunes a Viernes'
  }
  return dias.map(d => diasMap[d] || d).join(', ')
}

const openCreateDialog = () => {
  editingAgenda.value = null
  formData.value = {
    nombre: '',
    zona: 1,
    hora: '07:00',
    duracionMinutos: 15,
    diasSemana: [],
    activa: true
  }
  showDialog.value = true
}

const editAgenda = (agenda) => {
  editingAgenda.value = agenda
  formData.value = {
    nombre: agenda.nombre,
    zona: agenda.zona,
    hora: agenda.horaInicio || agenda.hora,
    duracionMinutos: agenda.duracionMin || agenda.duracionMinutos,
    diasSemana: [...(agenda.diasSemana || [])],
    activa: agenda.activa
  }
  showDialog.value = true
}

const closeDialog = () => {
  showDialog.value = false
  editingAgenda.value = null
}

const saveAgenda = async () => {
  // Validación básica
  if (formData.value.diasSemana.length === 0) {
    alert('Debes seleccionar al menos un día')
    return
  }

  try {
    const agendaData = {
      id: editingAgenda.value?.id || generateUUID(), // Backend requiere UUID
      nodeId: agendasStore.nodeId,
      zona: formData.value.zona,
      diasSemana: formData.value.diasSemana,
      horaInicio: formData.value.hora,
      duracionMin: formData.value.duracionMinutos, // Backend espera duracionMin
      activa: formData.value.activa
    }
    // Enviar nombre solo si está presente
    if (formData.value.nombre && formData.value.nombre.trim()) {
      agendaData.nombre = formData.value.nombre.trim()
    }
    
    if (editingAgenda.value) {
      await agendasStore.actualizarAgenda(editingAgenda.value.id, agendaData)
    } else {
      await agendasStore.crearAgenda(agendaData)
    }
    
    closeDialog()
  } catch (error) {
    alert('Error al guardar agenda: ' + error.message)
  }
}

const deleteAgenda = async (agenda) => {
  if (!confirm(`¿Eliminar agenda "${agenda.nombre}"?`)) return
  
  try {
    await agendasStore.eliminarAgenda(agenda.id)
  } catch (error) {
    alert('Error al eliminar agenda: ' + error.message)
  }
}

const toggleAgendaStatus = async (agenda) => {
  try {
    await agendasStore.toggleAgenda(agenda)
  } catch (error) {
    alert('Error al cambiar estado: ' + error.message)
  }
}

// Lifecycle
onMounted(async () => {
  await zoneConfigStore.fetchConfigs(true) // Cargar solo zonas habilitadas
  agendasStore.fetchAgendas()
})
</script>

<style scoped>
.agendas-list {
  max-width: 100%;
}

.pb-16 {
  padding-bottom: 80px !important;
}
</style>
