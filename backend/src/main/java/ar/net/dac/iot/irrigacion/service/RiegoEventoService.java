package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.dto.RiegoEventoResponse;
import ar.net.dac.iot.irrigacion.model.RiegoEvento;
import ar.net.dac.iot.irrigacion.repository.RiegoEventoRepository;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

@Service
public class RiegoEventoService {
    
    private static final Logger log = LoggerFactory.getLogger(RiegoEventoService.class);
    private final RiegoEventoRepository repository;
    private final ObjectMapper objectMapper;

    public RiegoEventoService(RiegoEventoRepository repository, ObjectMapper objectMapper) {
        this.repository = repository;
        this.objectMapper = objectMapper;
    }

    @Transactional
    public void procesarEvento(UUID nodeId, String payload) {
        try {
            JsonNode json = objectMapper.readTree(payload);
            
            Short zona = (short) json.get("zona").asInt();
            String evento = json.get("evento").asText(); // "inicio" o "fin"
            long timestamp = json.get("timestamp").asLong();
            String origen = json.get("origen").asText(); // "agenda" o "manual"
            
            // Solo guardamos eventos de "fin" con la duración real
            if ("fin".equals(evento)) {
                RiegoEvento riegoEvento = new RiegoEvento();
                riegoEvento.setNodeId(nodeId);
                riegoEvento.setZona(zona);
                riegoEvento.setTimestamp(Instant.ofEpochSecond(timestamp));
                riegoEvento.setDuracionSeg(json.get("duracionReal").asInt());
                riegoEvento.setOrigen(origen);
                
                if (json.has("versionAgenda") && !json.get("versionAgenda").isNull()) {
                    riegoEvento.setVersionAgenda(json.get("versionAgenda").asInt());
                }
                
                riegoEvento.setRaw(payload);
                
                repository.save(riegoEvento);
                log.info("Evento de riego guardado: nodeId={}, zona={}, duracion={}s, origen={}", 
                         nodeId, zona, riegoEvento.getDuracionSeg(), origen);
            } else {
                log.debug("Evento de inicio recibido: nodeId={}, zona={}, origen={}", 
                         nodeId, zona, origen);
            }
            
        } catch (Exception e) {
            log.error("Error procesando evento de riego: {}", e.getMessage(), e);
        }
    }

    public List<RiegoEventoResponse> getHistorial(UUID nodeId, int limit) {
        Page<RiegoEvento> page = repository.findByNodeIdOrderByTimestampDesc(
            nodeId, PageRequest.of(0, limit));
        
        return page.getContent().stream()
            .map(this::toResponse)
            .collect(Collectors.toList());
    }

    public List<RiegoEventoResponse> getHistorialPorZona(UUID nodeId, Short zona) {
        return repository.findByNodeIdAndZonaOrderByTimestampDesc(nodeId, zona).stream()
            .map(this::toResponse)
            .collect(Collectors.toList());
    }

    private RiegoEventoResponse toResponse(RiegoEvento evento) {
        return new RiegoEventoResponse(
            evento.getId(),
            evento.getZona(),
            evento.getTimestamp(),
            evento.getDuracionSeg(),
            evento.getOrigen(),
            evento.getVersionAgenda()
        );
    }
}
