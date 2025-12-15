package ar.net.dac.iot.irrigacion.service;

import ar.net.dac.iot.irrigacion.dto.ZoneConfigRequest;
import ar.net.dac.iot.irrigacion.dto.ZoneConfigResponse;
import ar.net.dac.iot.irrigacion.model.ZoneConfig;
import ar.net.dac.iot.irrigacion.repository.ZoneConfigRepository;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;

@Service
public class ZoneConfigService {
    
    private final ZoneConfigRepository zoneConfigRepository;
    
    public ZoneConfigService(ZoneConfigRepository zoneConfigRepository) {
        this.zoneConfigRepository = zoneConfigRepository;
    }
    
    public List<ZoneConfigResponse> listByNode(UUID nodeId) {
        return zoneConfigRepository.findByNodeIdOrderByOrdenAsc(nodeId)
                .stream()
                .map(this::toResponse)
                .collect(Collectors.toList());
    }
    
    public List<ZoneConfigResponse> listEnabledByNode(UUID nodeId) {
        return zoneConfigRepository.findByNodeIdAndHabilitadaTrueOrderByOrdenAsc(nodeId)
                .stream()
                .map(this::toResponse)
                .collect(Collectors.toList());
    }
    
    public ZoneConfigResponse get(UUID nodeId, short zona) {
        ZoneConfig config = zoneConfigRepository.findByNodeIdAndZona(nodeId, zona)
                .orElseThrow(() -> new IllegalArgumentException("Zona no encontrada"));
        return toResponse(config);
    }
    
    @Transactional
    public ZoneConfigResponse upsert(ZoneConfigRequest request) {
        ZoneConfig config = zoneConfigRepository.findByNodeIdAndZona(request.getNodeId(), request.getZona())
                .orElse(new ZoneConfig());
        
        config.setNodeId(request.getNodeId());
        config.setZona(request.getZona());
        config.setNombre(request.getNombre());
        config.setHabilitada(request.isHabilitada());
        config.setIcono(request.getIcono());
        config.setOrden(request.getOrden());
        
        config = zoneConfigRepository.save(config);
        return toResponse(config);
    }
    
    @Transactional
    public ZoneConfigResponse updateNombre(UUID nodeId, short zona, String nombre) {
        ZoneConfig config = zoneConfigRepository.findByNodeIdAndZona(nodeId, zona)
                .orElseThrow(() -> new IllegalArgumentException("Zona no encontrada"));
        
        config.setNombre(nombre);
        config = zoneConfigRepository.save(config);
        return toResponse(config);
    }
    
    @Transactional
    public ZoneConfigResponse toggleHabilitada(UUID nodeId, short zona) {
        ZoneConfig config = zoneConfigRepository.findByNodeIdAndZona(nodeId, zona)
                .orElseThrow(() -> new IllegalArgumentException("Zona no encontrada"));
        
        config.setHabilitada(!config.isHabilitada());
        config = zoneConfigRepository.save(config);
        return toResponse(config);
    }
    
    @Transactional
    public void disable(UUID nodeId, short zona) {
        ZoneConfig config = zoneConfigRepository.findByNodeIdAndZona(nodeId, zona)
                .orElseThrow(() -> new IllegalArgumentException("Zona no encontrada"));
        
        config.setHabilitada(false);
        zoneConfigRepository.save(config);
    }
    
    @Transactional
    public List<ZoneConfigResponse> reorder(UUID nodeId, List<Short> zonasOrdenadas) {
        for (int i = 0; i < zonasOrdenadas.size(); i++) {
            final short zona = zonasOrdenadas.get(i);
            final short orden = (short) i;
            zoneConfigRepository.findByNodeIdAndZona(nodeId, zona).ifPresent(config -> {
                config.setOrden(orden);
                zoneConfigRepository.save(config);
            });
        }
        return listByNode(nodeId);
    }
    
    public String getZoneName(UUID nodeId, short zona) {
        return zoneConfigRepository.findByNodeIdAndZona(nodeId, zona)
                .map(ZoneConfig::getNombre)
                .orElse("Zona " + zona);
    }
    
    public boolean isZoneEnabled(UUID nodeId, short zona) {
        return zoneConfigRepository.isZoneEnabled(nodeId, zona);
    }
    
    private ZoneConfigResponse toResponse(ZoneConfig config) {
        ZoneConfigResponse response = new ZoneConfigResponse();
        response.setNodeId(config.getNodeId());
        response.setZona(config.getZona());
        response.setNombre(config.getNombre());
        response.setHabilitada(config.isHabilitada());
        response.setIcono(config.getIcono());
        response.setOrden(config.getOrden());
        response.setCreatedAt(config.getCreatedAt());
        response.setUpdatedAt(config.getUpdatedAt());
        return response;
    }
}
