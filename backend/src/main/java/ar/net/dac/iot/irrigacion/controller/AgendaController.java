package ar.net.dac.iot.irrigacion.controller;

import ar.net.dac.iot.irrigacion.dto.AgendaRequest;
import ar.net.dac.iot.irrigacion.dto.AgendaResponse;
import ar.net.dac.iot.irrigacion.dto.CommandRequest;
import ar.net.dac.iot.irrigacion.dto.ZoneStatusResponse;
import ar.net.dac.iot.irrigacion.service.AgendaService;
import ar.net.dac.iot.irrigacion.service.MqttGateway;
import ar.net.dac.iot.irrigacion.service.ZoneStatusService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

@RestController
@RequestMapping("/api/nodos/{nodeId}")
public class AgendaController {

    private final AgendaService agendaService;
    private final MqttGateway mqttGateway;
<<<<<<< HEAD:backend/src/main/java/ar/net/dac/iot/irrigacion/controller/AgendaController.java
    private final ZoneStatusService zoneStatusService;

    public AgendaController(AgendaService agendaService, 
                          Optional<MqttGateway> mqttGateway,
                          ZoneStatusService zoneStatusService) {
=======
    private final com.example.irrigacion.service.ZoneStatusService zoneStatusService;

    public AgendaController(AgendaService agendaService, 
                          Optional<MqttGateway> mqttGateway,
                          com.example.irrigacion.service.ZoneStatusService zoneStatusService) {
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1:backend/src/main/java/com/example/irrigacion/controller/AgendaController.java
        this.agendaService = agendaService;
        this.mqttGateway = mqttGateway.orElse(null);
        this.zoneStatusService = zoneStatusService;
    }

    @GetMapping("/agendas")
    public List<AgendaResponse> list(@PathVariable UUID nodeId) {
        return agendaService.list(nodeId);
    }

    @PostMapping("/agendas")
    public AgendaResponse upsert(@PathVariable UUID nodeId, @Valid @RequestBody AgendaRequest request) {
        if (!nodeId.equals(request.getNodeId())) {
            throw new IllegalArgumentException("nodeId del path no coincide con payload");
        }
        return agendaService.upsert(request);
    }

    @DeleteMapping("/agendas/{agendaId}")
    public ResponseEntity<Void> delete(@PathVariable UUID nodeId, @PathVariable UUID agendaId) {
        agendaService.delete(nodeId, agendaId);
        return ResponseEntity.noContent().build();
    }

    @GetMapping("/status")
<<<<<<< HEAD:backend/src/main/java/ar/net/dac/iot/irrigacion/controller/AgendaController.java
    public List<ZoneStatusResponse> status(@PathVariable UUID nodeId) {
=======
    public List<com.example.irrigacion.dto.ZoneStatusResponse> status(@PathVariable UUID nodeId) {
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1:backend/src/main/java/com/example/irrigacion/controller/AgendaController.java
        return zoneStatusService.getStatus(nodeId);
    }

    @PostMapping("/cmd")
    public ResponseEntity<Void> command(@PathVariable UUID nodeId, @Valid @RequestBody CommandRequest request) {
        if (!nodeId.equals(request.getNodeId())) {
            throw new IllegalArgumentException("nodeId del path no coincide con payload");
        }
        if (mqttGateway == null || !mqttGateway.isEnabled()) {
            return ResponseEntity.status(503).build();
        }
        if ("OFF".equals(request.getAccion())) {
            mqttGateway.publishCommand(request.getNodeId().toString(), request.getZona(), request.getAccion(), null);
        } else {
            if (request.getDuracion() == null) {
                throw new IllegalArgumentException("duracion requerida para ON");
            }
            mqttGateway.publishCommand(request.getNodeId().toString(), request.getZona(), request.getAccion(), request.getDuracion());
        }
        return ResponseEntity.accepted().build();
    }
}
