<template>
  <v-container fluid class="pa-4 pb-16">
    <!-- Header con estado de conexión -->
    <div class="d-flex align-center justify-space-between mb-4">
      <h1 class="text-h5">Dashboard</h1>
      <v-chip
        :color="connected ? 'success' : 'error'"
        size="small"
        variant="flat"
      >
        <v-icon start size="small">
          {{ connected ? 'mdi-wifi' : 'mdi-wifi-off' }}
        </v-icon>
        {{ connected ? 'Conectado' : 'Desconectado' }}
      </v-chip>
    </div>

<<<<<<< HEAD
    <!-- Loading solo en primera carga -->
    <v-progress-linear v-if="loading && zones.length === 0" indeterminate color="primary" class="mb-4"></v-progress-linear>
=======
    <!-- Loading -->
    <v-progress-linear v-if="loading" indeterminate color="primary" class="mb-4"></v-progress-linear>
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
    
    <v-row>
      <v-col
        v-for="zone in zones"
        :key="zone.id"
        cols="12"
        sm="6"
        lg="3"
      >
        <v-card
          :class="['zone-card', { 'active': zone.active }]"
          @click="showZoneDetails(zone)"
        >
          <v-card-title class="d-flex align-center">
            <v-icon :color="zone.active ? 'success' : 'grey'" class="mr-2">
              mdi-water
            </v-icon>
            {{ zone.name }}
          </v-card-title>
          
          <v-card-text>
            <div class="text-h6 mb-2">
              {{ zone.active ? 'ACTIVO' : 'Inactivo' }}
            </div>
            
            <div v-if="zone.active" class="mb-3">
              <v-progress-linear
                :model-value="zone.progress"
                color="success"
                height="8"
                rounded
              ></v-progress-linear>
              <div class="text-caption mt-1">
                {{ zone.remainingTime }} min restantes
              </div>
            </div>
            
            <div v-else class="text-body-2 text-grey">
              Próximo: {{ zone.nextSchedule || 'Sin agenda' }}
            </div>
          </v-card-text>
          
          <v-card-actions>
            <v-btn
              v-if="!zone.active"
              color="primary"
              variant="elevated"
              block
              size="large"
              @click.stop="openManualControl(zone)"
            >
              <v-icon start>mdi-water</v-icon>
              REGAR
            </v-btn>
            <v-btn
              v-else
              color="error"
              variant="elevated"
              block
              size="large"
              @click.stop="stopWatering(zone)"
            >
              <v-icon start>mdi-stop</v-icon>
              DETENER
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-col>
    </v-row>

    <!-- Dialog para control manual -->
    <v-dialog v-model="showManualDialog" max-width="400">
      <v-card>
        <v-card-title>
          Riego Manual
        </v-card-title>

        <v-card-text>
          <div class="mb-4">
            <div class="text-h6 mb-2">{{ selectedZone?.name }}</div>
            <div class="text-body-2 text-grey">
              Selecciona la duración del riego
            </div>
          </div>

          <v-slider
            v-model="manualDuration"
            :min="1"
            :max="60"
            :step="1"
            thumb-label
            color="primary"
            class="mb-4"
          >
            <template v-slot:append>
              <v-text-field
                v-model="manualDuration"
                type="number"
                style="width: 80px"
                density="compact"
                suffix="min"
                hide-details
                variant="outlined"
              ></v-text-field>
            </template>
          </v-slider>
        </v-card-text>

        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn
            variant="text"
            @click="showManualDialog = false"
          >
            Cancelar
          </v-btn>
          <v-btn
            color="primary"
            variant="elevated"
            @click="startManualWatering"
            :loading="loading"
          >
            <v-icon start>mdi-water</v-icon>
            Iniciar Riego
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-container>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useZonesStore } from '@/stores/zones'

const zonesStore = useZonesStore()
const showManualDialog = ref(false)
const selectedZone = ref(null)
const manualDuration = ref(15)

// Computed
const zones = computed(() => zonesStore.zones)
const loading = computed(() => zonesStore.loading)
const connected = computed(() => zonesStore.connected)

// Methods
const showZoneDetails = (zone) => {
  console.log('Mostrar detalles de zona:', zone)
}

const openManualControl = (zone) => {
  selectedZone.value = zone
  manualDuration.value = 15
  showManualDialog.value = true
}

const startManualWatering = async () => {
  if (!selectedZone.value) return
  
  try {
    await zonesStore.startWatering(selectedZone.value.id, manualDuration.value)
    showManualDialog.value = false
  } catch (error) {
    alert('Error al iniciar riego: ' + error.message)
  }
}

const stopWatering = async (zone) => {
  if (!confirm(`¿Detener riego en ${zone.name}?`)) return
  
  try {
    await zonesStore.stopWatering(zone.id)
  } catch (error) {
    alert('Error al detener riego: ' + error.message)
  }
}

// Lifecycle
onMounted(() => {
  zonesStore.fetchZonesStatus()
  
  // Actualizar cada 5 segundos
  setInterval(() => {
    zonesStore.fetchZonesStatus()
  }, 5000)
})
</script>

<style scoped>
.zone-card {
<<<<<<< HEAD
  transition: transform 0.2s ease, box-shadow 0.2s ease;
  cursor: pointer;
  /* Evitar el re-layout al actualizar contenido */
  contain: layout style;
=======
  transition: all var(--transition-normal);
  cursor: pointer;
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
}

.zone-card:hover {
  transform: translateY(-4px);
  box-shadow: var(--shadow-3);
}

.zone-card.active {
  border: 3px solid var(--zone-active);
}

.pb-16 {
  padding-bottom: 80px !important;
}
<<<<<<< HEAD

/* Transiciones suaves para cambios de contenido */
.zone-card .v-card-text,
.zone-card .v-card-title {
  transition: opacity 0.15s ease;
}
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
</style>
