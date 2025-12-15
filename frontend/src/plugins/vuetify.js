import 'vuetify/styles'
import '@mdi/font/css/materialdesignicons.css'
import { createVuetify } from 'vuetify'
import * as components from 'vuetify/components'
import * as directives from 'vuetify/directives'

// Tema personalizado - Mobile First
const customTheme = {
  dark: false,
  colors: {
    // Colores primarios
    primary: '#2196F3',      // Azul agua
    secondary: '#4CAF50',    // Verde planta
    accent: '#FF9800',       // Naranja alerta
    error: '#F44336',        // Rojo error
    
    // Estados de zona
    zoneActive: '#4CAF50',   // Verde - Riego activo
    zoneIdle: '#E0E0E0',     // Gris - Inactivo
    zonePending: '#FF9800',  // Naranja - Próximo
    zoneError: '#F44336',    // Rojo - Error
    zoneManual: '#9C27B0',   // Morado - Manual
    
    // Superficies
    surface: '#FFFFFF',
    background: '#FAFAFA',
  }
}

export default createVuetify({
  components,
  directives,
  theme: {
    defaultTheme: 'customTheme',
    themes: {
      customTheme
    }
  },
  defaults: {
    VBtn: {
      // Botones más grandes para mobile
      minHeight: 48,
      rounded: 'lg',
      elevation: 1
    },
    VTextField: {
      minHeight: 56,
      variant: 'outlined',
      density: 'comfortable'
    },
    VCard: {
      elevation: 2,
      rounded: 'lg'
    }
  }
})
