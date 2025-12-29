package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.dto.ZoneStatusResponse;
import ar.net.dac.iot.irrigacion.model.Agenda;
import ar.net.dac.iot.irrigacion.repository.AgendaRepository;
import ar.net.dac.iot.irrigacion.service.ZoneConfigService;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.time.Clock;
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
    
    private static final Logger log = LoggerFactory.getLogger(ZoneStatusService.class);
    
    private final AgendaRepository agendaRepository;
    private final ZoneConfigService zoneConfigService;
    private final Clock clock;
    
    // Estado en memoria: nodeId -> zona -> status
    private final Map<String, Map<Integer, ZoneStatus>> statusCache = new ConcurrentHashMap<>();

    public ZoneStatusService(AgendaRepository agendaRepository,
                            ZoneConfigService zoneConfigService,
                            Clock clock) {
        this.agendaRepository = agendaRepository;
        this.zoneConfigService = zoneConfigService;
        this.clock = clock;
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

        LocalDate hoy = LocalDate.now(clock);
        LocalTime ahora = LocalTime.now(clock);
        log.info("Calculando próximo riego para zona {} - Clock timezone: {} - Fecha/Hora: {} {}", 
                 zona, clock.getZone(), hoy, ahora);
        
        Agenda proximaAgenda = null;
        LocalDate fechaProxima = null;
        LocalTime horaProxima = null;
        long minutosMinimos = Long.MAX_VALUE;
        
        for (Agenda agenda : agendas) {
            // Buscar la próxima ocurrencia de esta agenda en los próximos 7 días
            for (int diasAdelante = 0; diasAdelante <= 7; diasAdelante++) {
                LocalDate fecha = hoy.plusDays(diasAdelante);
                String diaSemana = getDiaSemana(fecha.getDayOfWeek());
                
                if (agenda.getDiasSemana().contains(diaSemana)) {
                    LocalTime hora = agenda.getHoraInicio();
                    
                    // Si es hoy, verificar que la hora sea futura (no haya pasado)
                    if (diasAdelante == 0 && !ahora.isBefore(hora)) {
                        continue; // Ya pasó o es ahora mismo, buscar próxima ocurrencia
                    }
                    
                    // Calcular minutos totales hasta esta ejecución
                    long minutos;
                    if (diasAdelante == 0) {
                        // Hoy: diferencia entre ahora y la hora de inicio
                        minutos = java.time.Duration.between(ahora, hora).toMinutes();
                    } else {
                        // Días futuros: calcular tiempo total en minutos
                        // Minutos completos de días enteros
                        minutos = (long) diasAdelante * 24 * 60;
                        // Restar minutos que ya pasaron hoy
                        minutos -= (ahora.getHour() * 60 + ahora.getMinute());
                        // Sumar minutos desde medianoche hasta la hora de la agenda
                        minutos += (hora.getHour() * 60 + hora.getMinute());
                    }
                    
                    // Seleccionar la agenda con menos minutos de espera
                    if (minutos < minutosMinimos) {
                        proximaAgenda = agenda;
                        fechaProxima = fecha;
                        horaProxima = hora;
                        minutosMinimos = minutos;
                    }
                    
                    break; // Ya encontramos la próxima ocurrencia de esta agenda, pasar a la siguiente
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
            horaProxima.format(DateTimeFormatter.ofPattern("HH:mm")),
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
