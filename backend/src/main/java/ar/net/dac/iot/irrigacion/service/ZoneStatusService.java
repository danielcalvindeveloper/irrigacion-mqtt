package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.dto.ZoneStatusResponse;
import ar.net.dac.iot.irrigacion.model.Agenda;
import ar.net.dac.iot.irrigacion.repository.AgendaRepository;
import ar.net.dac.iot.irrigacion.service.ZoneConfigService;
import org.springframework.stereotype.Service;

import java.time.DayOfWeek;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

@Service
public class ZoneStatusService {
    private final AgendaRepository agendaRepository;
    private final ZoneConfigService zoneConfigService;
    
    // Estado en memoria: nodeId -> zona -> status
    private final Map<String, Map<Integer, ZoneStatus>> statusCache = new ConcurrentHashMap<>();

    public ZoneStatusService(AgendaRepository agendaRepository,
                            ZoneConfigService zoneConfigService) {
        this.agendaRepository = agendaRepository;
        this.zoneConfigService = zoneConfigService;
    }

    public List<ZoneStatusResponse> getStatus(UUID nodeId) {
        List<ZoneStatusResponse> result = new ArrayList<>();
        Map<Integer, ZoneStatus> nodeStatus = statusCache.getOrDefault(nodeId.toString(), new ConcurrentHashMap<>());
        
        // Obtener solo las zonas configuradas y habilitadas
        var zonasConfiguradas = zoneConfigService.listEnabledByNode(nodeId);
        
        for (var zonaConfig : zonasConfiguradas) {
            ZoneStatusResponse response = new ZoneStatusResponse();
            response.setZona(zonaConfig.getZona());
            response.setNombre(zonaConfig.getNombre());
            
            ZoneStatus status = nodeStatus.get((int) zonaConfig.getZona());
            if (status != null) {
                response.setActiva(status.activa);
                response.setTiempoRestanteSeg(status.tiempoRestanteSeg);
            } else {
                response.setActiva(false);
                response.setTiempoRestanteSeg(0);
            }
            
            // Calcular próxima agenda
            response.setProximoRiego(calcularProximoRiego(nodeId, zonaConfig.getZona()));
            
            result.add(response);
        }
        
        return result;
    }

    public void updateZoneStatus(UUID nodeId, int zona, boolean activa, Integer tiempoRestanteSeg) {
        Map<Integer, ZoneStatus> nodeStatus = statusCache.computeIfAbsent(
            nodeId.toString(), 
            k -> new ConcurrentHashMap<>()
        );
        
        ZoneStatus status = nodeStatus.computeIfAbsent(zona, k -> new ZoneStatus());
        status.activa = activa;
        status.tiempoRestanteSeg = tiempoRestanteSeg;
        status.lastUpdate = System.currentTimeMillis();
    }

    private String calcularProximoRiego(UUID nodeId, int zona) {
        List<Agenda> agendas = agendaRepository.findActiveByNodeAndZona(nodeId, (short) zona);
        if (agendas.isEmpty()) {
            return null;
        }

        LocalDate hoy = LocalDate.now();
        LocalTime ahora = LocalTime.now();
        
        Agenda proximaAgenda = null;
        LocalDate fechaProxima = null;
        int diasMinimoAdelante = Integer.MAX_VALUE;
        
        for (Agenda agenda : agendas) {
            // Buscar en los próximos 7 días (0 = hoy, 1 = mañana, ..., 6 = dentro de 6 días, 7 = misma hora la próxima semana)
            for (int diasAdelante = 0; diasAdelante <= 7; diasAdelante++) {
                LocalDate fecha = hoy.plusDays(diasAdelante);
                String diaSemana = getDiaSemana(fecha.getDayOfWeek());
                
                if (agenda.getDiasSemana().contains(diaSemana)) {
                    // Si es hoy, verificar que no haya pasado la hora
                    if (diasAdelante == 0 && !agenda.getHoraInicio().isAfter(ahora)) {
                        continue; // Ya pasó, buscar próxima ocurrencia
                    }
                    
                    // Seleccionar la agenda más próxima (menor días adelante, o si empate, la de menor hora)
                    if (diasAdelante < diasMinimoAdelante || 
                        (diasAdelante == diasMinimoAdelante && proximaAgenda != null && 
                         agenda.getHoraInicio().isBefore(proximaAgenda.getHoraInicio()))) {
                        proximaAgenda = agenda;
                        fechaProxima = fecha;
                        diasMinimoAdelante = diasAdelante;
                    }
                    break; // Ya encontramos el próximo para esta agenda
                }
            }
        }
        
        if (proximaAgenda == null) {
            return null;
        }
        
        String cuando = fechaProxima.equals(hoy) ? "Hoy" : 
                       fechaProxima.equals(hoy.plusDays(1)) ? "Mañana" :
                       fechaProxima.format(DateTimeFormatter.ofPattern("dd/MM"));
        
        return String.format("%s %s (%dmin)", 
            cuando, 
            proximaAgenda.getHoraInicio().format(DateTimeFormatter.ofPattern("HH:mm")),
            proximaAgenda.getDuracionMin()
        );
    }

    private String getDiaSemana(DayOfWeek dayOfWeek) {
        return switch (dayOfWeek) {
            case MONDAY -> "LUN";
            case TUESDAY -> "MAR";
            case WEDNESDAY -> "MIE";
            case THURSDAY -> "JUE";
            case FRIDAY -> "VIE";
            case SATURDAY -> "SAB";
            case SUNDAY -> "DOM";
        };
    }

    private static class ZoneStatus {
        boolean activa = false;
        Integer tiempoRestanteSeg = 0;
        long lastUpdate = 0;
    }
}
