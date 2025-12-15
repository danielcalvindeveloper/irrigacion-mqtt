<template>
  <v-container fluid class="pa-4 pb-16">
    <!-- Header con avatar -->
    <div class="text-center mb-6">
      <v-avatar size="80" color="primary" class="mb-3">
        <v-icon size="48">mdi-account</v-icon>
      </v-avatar>
      <h1 class="text-h5 mb-1">Usuario</h1>
      <div class="text-body-2 text-grey">usuario@irrigacion.com</div>
    </div>

    <!-- Configuración de Sistema -->
    <div class="mb-4">
      <div class="text-overline text-grey mb-2">Sistema</div>
      
      <v-card>
        <v-list>
          <v-list-item
            prepend-icon="mdi-chip"
            title="Dispositivo ESP32"
            :subtitle="deviceStatus"
          >
            <template v-slot:append>
              <v-chip
                :color="deviceConnected ? 'success' : 'error'"
                size="small"
              >
                {{ deviceConnected ? 'Conectado' : 'Desconectado' }}
              </v-chip>
            </template>
          </v-list-item>

          <v-divider></v-divider>

          <v-list-item
            prepend-icon="mdi-wifi"
            title="Estado de Conexión"
            subtitle="Conectado a red WiFi"
          >
            <template v-slot:append>
              <v-icon color="success">mdi-check-circle</v-icon>
            </template>
          </v-list-item>

          <v-divider></v-divider>

          <v-list-item
            prepend-icon="mdi-database"
            title="Almacenamiento"
            subtitle="156 eventos registrados"
          ></v-list-item>
        </v-list>
      </v-card>
    </div>

    <!-- Configuración de Notificaciones -->
    <div class="mb-4">
      <div class="text-overline text-grey mb-2">Notificaciones</div>
      
      <v-card>
        <v-list>
          <v-list-item>
            <template v-slot:prepend>
              <v-icon>mdi-bell</v-icon>
            </template>
            <v-list-item-title>Notificaciones Push</v-list-item-title>
            <v-list-item-subtitle>Alertas de riego y errores</v-list-item-subtitle>
            <template v-slot:append>
              <v-switch
                v-model="settings.notifications"
                color="primary"
                hide-details
                density="compact"
              ></v-switch>
            </template>
          </v-list-item>

          <v-divider></v-divider>

          <v-list-item>
            <template v-slot:prepend>
              <v-icon>mdi-email</v-icon>
            </template>
            <v-list-item-title>Email</v-list-item-title>
            <v-list-item-subtitle>Resumen diario</v-list-item-subtitle>
            <template v-slot:append>
              <v-switch
                v-model="settings.email"
                color="primary"
                hide-details
                density="compact"
              ></v-switch>
            </template>
          </v-list-item>
        </v-list>
      </v-card>
    </div>

    <!-- Preferencias -->
    <div class="mb-4">
      <div class="text-overline text-grey mb-2">Preferencias</div>
      
      <v-card>
        <v-list>
          <v-list-item
            prepend-icon="mdi-theme-light-dark"
            title="Tema Oscuro"
            subtitle="Cambiar apariencia"
          >
            <template v-slot:append>
              <v-switch
                v-model="settings.darkMode"
                color="primary"
                hide-details
                density="compact"
              ></v-switch>
            </template>
          </v-list-item>

          <v-divider></v-divider>

          <v-list-item
            prepend-icon="mdi-map-marker"
            title="Ubicación"
            subtitle="Buenos Aires, Argentina"
          ></v-list-item>
        </v-list>
      </v-card>
    </div>

    <!-- Información de la App -->
    <div class="mb-4">
      <div class="text-overline text-grey mb-2">Información</div>
      
      <v-card>
        <v-list>
          <v-list-item
            prepend-icon="mdi-information"
            title="Versión"
            subtitle="1.0.0"
          ></v-list-item>

          <v-divider></v-divider>

          <v-list-item
            prepend-icon="mdi-help-circle"
            title="Ayuda"
            @click="openHelp"
          ></v-list-item>

          <v-divider></v-divider>

          <v-list-item
            prepend-icon="mdi-file-document"
            title="Documentación"
            @click="openDocs"
          ></v-list-item>
        </v-list>
      </v-card>
    </div>

    <!-- Acciones -->
    <v-btn
      block
      variant="tonal"
      color="error"
      size="large"
      class="mb-3"
      @click="confirmLogout"
    >
      <v-icon start>mdi-logout</v-icon>
      Cerrar Sesión
    </v-btn>
  </v-container>
</template>

<script setup>
import { ref } from 'vue'

const deviceConnected = ref(true)
const deviceStatus = ref('ESP32-RIEGO-001')

const settings = ref({
  notifications: true,
  email: false,
  darkMode: false
})

const openHelp = () => {
  console.log('Abrir ayuda')
}

const openDocs = () => {
  console.log('Abrir documentación')
}

const confirmLogout = () => {
  console.log('Cerrar sesión')
  // TODO: Implementar logout
}
</script>

<style scoped>
.pb-16 {
  padding-bottom: 80px !important;
}
</style>
