import { createRouter, createWebHistory } from 'vue-router'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes: [
    {
      path: '/',
      name: 'dashboard',
      component: () => import('@/views/DashboardView.vue')
    },
    {
      path: '/agendas',
      name: 'agendas',
      component: () => import('@/views/AgendaView.vue')
    },
    {
      path: '/historico',
      name: 'historico',
      component: () => import('@/views/HistoryView.vue')
    },
    {
      path: '/perfil',
      name: 'perfil',
      component: () => import('@/views/ProfileView.vue')
<<<<<<< HEAD
    },
    {
      path: '/zonas',
      name: 'zonas',
      component: () => import('@/views/ZoneConfigView.vue')
=======
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1
    }
  ]
})

export default router
