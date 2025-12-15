<template>
  <v-container fluid class="pa-4 pb-16">
    <!-- Header -->
    <div class="d-flex align-center justify-space-between mb-4">
      <h1 class="text-h5">Histórico de Riego</h1>
      <v-btn
        icon="mdi-filter"
        variant="text"
        @click="openFilterDialog"
      ></v-btn>
    </div>

    <!-- Filtros rápidos -->
    <div class="mb-4">
      <v-chip-group
        v-model="selectedFilter"
        selected-class="text-primary"
        mandatory
      >
        <v-chip value="hoy">Hoy</v-chip>
        <v-chip value="semana">Esta Semana</v-chip>
        <v-chip value="mes">Este Mes</v-chip>
      </v-chip-group>
    </div>

    <!-- Resumen estadístico -->
    <v-row class="mb-4">
      <v-col cols="6">
        <v-card variant="tonal" color="primary">
          <v-card-text class="text-center">
            <div class="text-h4">{{ stats.totalRiegos }}</div>
            <div class="text-caption">Riegos Totales</div>
          </v-card-text>
        </v-card>
      </v-col>
      <v-col cols="6">
        <v-card variant="tonal" color="secondary">
          <v-card-text class="text-center">
            <div class="text-h4">{{ stats.tiempoTotal }}</div>
            <div class="text-caption">Minutos Totales</div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- Timeline de eventos -->
    <v-timeline
      side="end"
      density="compact"
      class="history-timeline"
    >
      <v-timeline-item
        v-for="event in events"
        :key="event.id"
        :dot-color="getEventColor(event.tipo)"
        size="small"
      >
        <template v-slot:opposite>
          <div class="text-caption text-grey">
            {{ event.hora }}
          </div>
        </template>

        <v-card>
          <v-card-title class="text-body-1">
            <v-icon :color="getEventColor(event.tipo)" size="small" class="mr-2">
              {{ getEventIcon(event.tipo) }}
            </v-icon>
            {{ event.titulo }}
          </v-card-title>
          
          <v-card-text>
            <div class="text-body-2 mb-1">{{ event.zona }}</div>
            <div v-if="event.duracion" class="text-caption text-grey">
              Duración: {{ event.duracion }} minutos
            </div>
            <div v-if="event.origen" class="text-caption">
              <v-chip size="x-small" :color="event.origen === 'manual' ? 'warning' : 'info'">
                {{ event.origen === 'manual' ? 'Manual' : 'Automático' }}
              </v-chip>
            </div>
          </v-card-text>
        </v-card>
      </v-timeline-item>
    </v-timeline>

    <!-- Empty state -->
    <v-card v-if="events.length === 0" class="text-center pa-8" variant="tonal">
      <v-icon size="64" color="grey-lighten-1">mdi-history</v-icon>
      <div class="text-h6 mt-4">Sin eventos registrados</div>
      <div class="text-body-2 text-grey">
        Los eventos de riego aparecerán aquí
      </div>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref, computed } from 'vue'

const selectedFilter = ref('hoy')

const stats = computed(() => {
  return {
    totalRiegos: 8,
    tiempoTotal: 125
  }
})

const events = ref([
  {
    id: 1,
    tipo: 'completado',
    titulo: 'Riego Completado',
    zona: 'Zona 1 - Jardín Frontal',
    hora: '07:15',
    duracion: 15,
    origen: 'automatico'
  },
  {
    id: 2,
    tipo: 'completado',
    titulo: 'Riego Completado',
    zona: 'Zona 3 - Huerta',
    hora: '07:00',
    duracion: 20,
    origen: 'automatico'
  },
  {
    id: 3,
    tipo: 'detenido',
    titulo: 'Riego Detenido',
    zona: 'Zona 2 - Jardín Lateral',
    hora: '06:45',
    duracion: 8,
    origen: 'manual'
  },
  {
    id: 4,
    tipo: 'iniciado',
    titulo: 'Riego Iniciado',
    zona: 'Zona 4 - Patio',
    hora: '06:30',
    duracion: null,
    origen: 'manual'
  },
  {
    id: 5,
    tipo: 'error',
    titulo: 'Error de Conexión',
    zona: 'Zona 2 - Jardín Lateral',
    hora: '06:15',
    duracion: null,
    origen: 'automatico'
  }
])

const getEventColor = (tipo) => {
  const colors = {
    completado: 'success',
    iniciado: 'primary',
    detenido: 'warning',
    error: 'error'
  }
  return colors[tipo] || 'grey'
}

const getEventIcon = (tipo) => {
  const icons = {
    completado: 'mdi-check-circle',
    iniciado: 'mdi-play-circle',
    detenido: 'mdi-stop-circle',
    error: 'mdi-alert-circle'
  }
  return icons[tipo] || 'mdi-circle'
}

const openFilterDialog = () => {
  console.log('Abrir filtros')
  // TODO: Implementar filtros avanzados
}
</script>

<style scoped>
.pb-16 {
  padding-bottom: 80px !important;
}

.history-timeline {
  margin-top: 0;
}
</style>
