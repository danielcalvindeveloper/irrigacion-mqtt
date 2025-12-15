package com.example.irrigacion.service;

import com.example.irrigacion.dto.ZoneStatusResponse;
import com.example.irrigacion.model.Agenda;
import com.example.irrigacion.repository.AgendaRepository;
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
    
    // Estado en memoria: nodeId -> zona -> status
    private final Map<String, Map<Integer, ZoneStatus>> statusCache = new ConcurrentHashMap<>();

    public ZoneStatusService(AgendaRepository agendaRepository) {
        this.agendaRepository = agendaRepository;
    }

    public List<ZoneStatusResponse> getStatus(UUID nodeId) {
        List<ZoneStatusResponse> result = new ArrayList<>();
        Map<Integer, ZoneStatus> nodeStatus = statusCache.getOrDefault(nodeId.toString(), new ConcurrentHashMap<>());
        
        for (int zona = 1; zona <= 4; zona++) {
            ZoneStatusResponse response = new ZoneStatusResponse();
            response.setZona(zona);
            response.setNombre("Zona " + zona);
            
            ZoneStatus status = nodeStatus.get(zona);
            if (status != null) {
                response.setActiva(status.activa);
                response.setTiempoRestanteSeg(status.tiempoRestanteSeg);
            } else {
                response.setActiva(false);
                response.setTiempoRestanteSeg(0);
            }
            
            // Calcular próxima agenda
            response.setProximoRiego(calcularProximoRiego(nodeId, zona));
            
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
        String diaSemanaHoy = getDiaSemana(hoy.getDayOfWeek());
        
        Agenda proximaAgenda = null;
        LocalDate fechaProxima = null;
        
        for (Agenda agenda : agendas) {
            // Buscar en los próximos 7 días
            for (int diasAdelante = 0; diasAdelante < 7; diasAdelante++) {
                LocalDate fecha = hoy.plusDays(diasAdelante);
                String diaSemana = getDiaSemana(fecha.getDayOfWeek());
                
                if (agenda.getDiasSemana().contains(diaSemana)) {
                    // Si es hoy, verificar que no haya pasado la hora
                    if (diasAdelante == 0 && agenda.getHoraInicio().isBefore(ahora)) {
                        continue;
                    }
                    
                    if (proximaAgenda == null || diasAdelante == 0 || 
                        (fechaProxima != null && fecha.isBefore(fechaProxima))) {
                        proximaAgenda = agenda;
                        fechaProxima = fecha;
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
