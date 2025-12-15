package ar.net.dac.iot.irrigacion.controller;

import ar.net.dac.iot.irrigacion.dto.ZoneConfigRequest;
import ar.net.dac.iot.irrigacion.dto.ZoneConfigResponse;
import ar.net.dac.iot.irrigacion.service.ZoneConfigService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;
import java.util.UUID;

@RestController
@RequestMapping("/api/nodos/{nodeId}/zonas")
public class ZoneConfigController {
    
    private final ZoneConfigService zoneConfigService;
    
    public ZoneConfigController(ZoneConfigService zoneConfigService) {
        this.zoneConfigService = zoneConfigService;
    }
    
    @GetMapping
    public List<ZoneConfigResponse> list(@PathVariable UUID nodeId,
                                         @RequestParam(required = false, defaultValue = "false") boolean soloHabilitadas) {
        if (soloHabilitadas) {
            return zoneConfigService.listEnabledByNode(nodeId);
        }
        return zoneConfigService.listByNode(nodeId);
    }
    
    @GetMapping("/{zona}")
    public ZoneConfigResponse get(@PathVariable UUID nodeId, @PathVariable short zona) {
        return zoneConfigService.get(nodeId, zona);
    }
    
    @PostMapping
    public ZoneConfigResponse upsert(@PathVariable UUID nodeId, 
                                     @Valid @RequestBody ZoneConfigRequest request) {
        if (!nodeId.equals(request.getNodeId())) {
            throw new IllegalArgumentException("nodeId del path no coincide con payload");
        }
        return zoneConfigService.upsert(request);
    }
    
    @PatchMapping("/{zona}/nombre")
    public ZoneConfigResponse updateNombre(@PathVariable UUID nodeId, 
                                           @PathVariable short zona,
                                           @RequestBody Map<String, String> body) {
        String nombre = body.get("nombre");
        if (nombre == null || nombre.trim().isEmpty()) {
            throw new IllegalArgumentException("nombre es requerido");
        }
        return zoneConfigService.updateNombre(nodeId, zona, nombre);
    }
    
    @PatchMapping("/{zona}/toggle")
    public ZoneConfigResponse toggleHabilitada(@PathVariable UUID nodeId, 
                                                @PathVariable short zona) {
        return zoneConfigService.toggleHabilitada(nodeId, zona);
    }
    
    @DeleteMapping("/{zona}")
    public ResponseEntity<Void> delete(@PathVariable UUID nodeId, 
                                        @PathVariable short zona) {
        zoneConfigService.disable(nodeId, zona);
        return ResponseEntity.noContent().build();
    }
    
    @PutMapping("/orden")
    public List<ZoneConfigResponse> reorder(@PathVariable UUID nodeId,
                                            @RequestBody Map<String, List<Short>> body) {
        List<Short> zonasOrdenadas = body.get("zonas");
        if (zonasOrdenadas == null || zonasOrdenadas.isEmpty()) {
            throw new IllegalArgumentException("zonas es requerido");
        }
        return zoneConfigService.reorder(nodeId, zonasOrdenadas);
    }
}
