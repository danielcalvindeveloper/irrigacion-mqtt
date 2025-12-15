<template>
  <v-app>
    <v-main>
      <router-view />
    </v-main>
    
    <!-- Bottom Navigation -->
    <v-bottom-navigation
      v-model="activeTab"
      grow
      color="primary"
      class="bottom-nav-mobile"
    >
      <v-btn value="home" to="/">
        <v-icon>mdi-home</v-icon>
        <span>Inicio</span>
      </v-btn>

      <v-btn value="agenda" to="/agendas">
        <v-icon>mdi-calendar-clock</v-icon>
        <span>Agenda</span>
      </v-btn>

      <v-btn value="zonas" to="/zonas">
        <v-icon>mdi-tune-variant</v-icon>
        <span>Zonas</span>
      </v-btn>

      <v-btn value="historico" to="/historico">
        <v-icon>mdi-chart-line</v-icon>
        <span>Histórico</span>
      </v-btn>

      <v-btn value="perfil" to="/perfil">
        <v-icon>mdi-account</v-icon>
        <span>Perfil</span>
      </v-btn>
    </v-bottom-navigation>
  </v-app>
</template>

<script setup>
import { ref } from 'vue'
import { useRoute } from 'vue-router'

const route = useRoute()
const activeTab = ref('home')
</script>

<style>
/* Estilos globales mínimos - el resto en main.css */
#app {
  font-family: 'Roboto', sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

.bottom-nav-mobile {
  box-shadow: 0 -2px 10px rgba(0, 0, 0, 0.1);
}

/* Ocultar en pantallas grandes */
@media (min-width: 1024px) {
  .bottom-nav-mobile {
    display: none;
  }
}
</style>
