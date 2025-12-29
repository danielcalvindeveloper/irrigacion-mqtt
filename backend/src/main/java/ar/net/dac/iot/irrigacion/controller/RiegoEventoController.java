package ar.net.dac.iot.irrigacion.controller;

import ar.net.dac.iot.irrigacion.dto.RiegoEventoResponse;
import ar.net.dac.iot.irrigacion.service.RiegoEventoService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.UUID;

@RestController
@RequestMapping("/api/nodos/{nodeId}/historial")
public class RiegoEventoController {
    
    private final RiegoEventoService riegoEventoService;

    public RiegoEventoController(RiegoEventoService riegoEventoService) {
        this.riegoEventoService = riegoEventoService;
    }

    @GetMapping
    public ResponseEntity<List<RiegoEventoResponse>> getHistorial(
            @PathVariable UUID nodeId,
            @RequestParam(defaultValue = "50") int limit) {
        
        List<RiegoEventoResponse> historial = riegoEventoService.getHistorial(nodeId, limit);
        return ResponseEntity.ok(historial);
    }

    @GetMapping("/zona/{zona}")
    public ResponseEntity<List<RiegoEventoResponse>> getHistorialPorZona(
            @PathVariable UUID nodeId,
            @PathVariable Short zona) {
        
        List<RiegoEventoResponse> historial = riegoEventoService.getHistorialPorZona(nodeId, zona);
        return ResponseEntity.ok(historial);
    }
}
