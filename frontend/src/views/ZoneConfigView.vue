<template>
  <v-container>
    <v-card>
      <v-card-title class="d-flex align-center">
        <v-icon class="mr-2">mdi-tune-variant</v-icon>
        Configuración de Zonas
      </v-card-title>

      <v-card-text>
        <v-alert v-if="error" type="error" dismissible @click:close="error = null">
          {{ error }}
        </v-alert>

        <v-progress-linear v-if="loading" indeterminate color="primary" class="mb-4" />

        <v-list v-if="configs.length > 0">
          <v-list-item
            v-for="zona in configs"
            :key="zona.zona"
            :class="{ 'text-disabled': !zona.habilitada }"
          >
            <template #prepend>
              <v-avatar :color="zona.habilitada ? 'primary' : 'grey'" size="40">
                <v-icon>{{ getIconName(zona.icono) }}</v-icon>
              </v-avatar>
            </template>

            <v-list-item-title>
              <v-text-field
                v-model="zona.nombre"
                density="compact"
                variant="outlined"
                hide-details
                @blur="handleUpdateNombre(zona)"
                @keyup.enter="handleUpdateNombre(zona)"
                :disabled="!zona.habilitada"
              />
            </v-list-item-title>

            <v-list-item-subtitle class="mt-1">
              Zona {{ zona.zona }} • 
              {{ zona.habilitada ? 'Habilitada' : 'Deshabilitada' }}
            </v-list-item-subtitle>

            <template #append>
              <v-switch
                v-model="zona.habilitada"
                color="primary"
                hide-details
                density="compact"
                @update:model-value="handleToggle(zona)"
              />
            </template>
          </v-list-item>
        </v-list>

        <v-alert v-else type="info" class="mt-4">
          No hay zonas configuradas. Crea una nueva zona para comenzar.
        </v-alert>
      </v-card-text>

      <v-card-actions>
        <v-btn 
          color="primary" 
          variant="elevated"
          @click="dialogNuevaZona = true"
          :disabled="configs.length >= 8"
        >
          <v-icon class="mr-2">mdi-plus</v-icon>
          Agregar Zona
        </v-btn>
        
        <v-spacer />
        
        <v-btn 
          variant="text"
          @click="cargarConfiguraciones"
          :loading="loading"
        >
          <v-icon class="mr-2">mdi-refresh</v-icon>
          Recargar
        </v-btn>
      </v-card-actions>
    </v-card>

    <!-- Dialog: Nueva Zona -->
    <v-dialog v-model="dialogNuevaZona" max-width="500">
      <v-card>
        <v-card-title>
          <span class="text-h5">Nueva Zona</span>
        </v-card-title>

        <v-card-text>
          <v-form ref="formNuevaZona" v-model="formValid">
            <v-text-field
              v-model.number="nuevaZona.zona"
              label="Número de Zona"
              type="number"
              min="1"
              max="8"
              :rules="[rules.required, rules.zonaValida, rules.zonaUnica]"
              variant="outlined"
              required
            />

            <v-text-field
              v-model="nuevaZona.nombre"
              label="Nombre"
              :rules="[rules.required]"
              variant="outlined"
              required
            />

            <v-select
              v-model="nuevaZona.icono"
              label="Icono"
              :items="iconosDisponibles"
              variant="outlined"
            >
              <template #item="{ item, props }">
                <v-list-item v-bind="props">
                  <template #prepend>
                    <v-icon>{{ getIconName(item.value) }}</v-icon>
                  </template>
                </v-list-item>
              </template>

              <template #selection="{ item }">
                <v-icon class="mr-2">{{ getIconName(item.value) }}</v-icon>
                {{ item.title }}
              </template>
            </v-select>

            <v-switch
              v-model="nuevaZona.habilitada"
              label="Habilitada"
              color="primary"
              hide-details
            />
          </v-form>
        </v-card-text>

        <v-card-actions>
          <v-spacer />
          <v-btn variant="text" @click="dialogNuevaZona = false">
            Cancelar
          </v-btn>
          <v-btn
            color="primary"
            variant="elevated"
            :disabled="!formValid"
            :loading="loading"
            @click="handleCrearZona"
          >
            Crear
          </v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-container>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'
import { useZoneConfigStore } from '@/stores/zoneConfig'

const zoneConfigStore = useZoneConfigStore()

// State local
const dialogNuevaZona = ref(false)
const formValid = ref(false)
const formNuevaZona = ref(null)

const nuevaZona = ref({
  zona: null,
  nombre: '',
  icono: 'water_drop',
  habilitada: true,
  orden: 0
})

const iconosDisponibles = [
  { title: 'Gota de agua', value: 'water_drop' },
  { title: 'Aspersor', value: 'sprinkler' },
  { title: 'Jardín', value: 'garden' },
  { title: 'Césped', value: 'lawn' },
  { title: 'Vegetales', value: 'vegetables' },
  { title: 'Flores', value: 'flowers' }
]

// Computed
const configs = computed(() => zoneConfigStore.configs)
const loading = computed(() => zoneConfigStore.loading)
const error = computed({
  get: () => zoneConfigStore.error,
  set: (val) => zoneConfigStore.error = val
})

// Validations
const rules = {
  required: value => !!value || 'Campo requerido',
  zonaValida: value => (value >= 1 && value <= 8) || 'Zona debe estar entre 1 y 8',
  zonaUnica: value => {
    const existe = configs.value.find(z => z.zona === value)
    return !existe || 'Esta zona ya existe'
  }
}

// Methods
function getIconName(icono) {
  const iconMap = {
    'water_drop': 'mdi-water',
    'sprinkler': 'mdi-sprinkler-variant',
    'garden': 'mdi-flower',
    'lawn': 'mdi-grass',
    'vegetables': 'mdi-carrot',
    'flowers': 'mdi-flower-tulip'
  }
  return iconMap[icono] || 'mdi-water'
}

async function cargarConfiguraciones() {
  try {
    await zoneConfigStore.fetchConfigs()
  } catch (err) {
    console.error('Error al cargar configuraciones:', err)
  }
}

async function handleUpdateNombre(zona) {
  if (!zona.nombre || zona.nombre.trim() === '') {
    zona.nombre = `Zona ${zona.zona}`
  }
  
  try {
    await zoneConfigStore.updateNombre(zona.zona, zona.nombre)
  } catch (err) {
    console.error('Error al actualizar nombre:', err)
  }
}

async function handleToggle(zona) {
  try {
    await zoneConfigStore.toggleHabilitada(zona.zona)
  } catch (err) {
    console.error('Error al cambiar estado:', err)
    // Revertir cambio en UI
    zona.habilitada = !zona.habilitada
  }
}

async function handleCrearZona() {
  if (!formValid.value) return

  try {
    const configData = {
      nodeId: zoneConfigStore.nodeId,
      zona: nuevaZona.value.zona,
      nombre: nuevaZona.value.nombre,
      icono: nuevaZona.value.icono,
      habilitada: nuevaZona.value.habilitada,
      orden: configs.value.length
    }

    await zoneConfigStore.upsertConfig(configData)
    
    // Reset form
    dialogNuevaZona.value = false
    nuevaZona.value = {
      zona: null,
      nombre: '',
      icono: 'water_drop',
      habilitada: true,
      orden: 0
    }
    formNuevaZona.value?.reset()
  } catch (err) {
    console.error('Error al crear zona:', err)
  }
}

onMounted(() => {
  cargarConfiguraciones()
})
</script>

<style scoped>
.text-disabled {
  opacity: 0.6;
}
</style>
