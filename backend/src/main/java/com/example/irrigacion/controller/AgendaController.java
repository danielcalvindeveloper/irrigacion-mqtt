package com.example.irrigacion.controller;

import com.example.irrigacion.dto.AgendaRequest;
import com.example.irrigacion.dto.AgendaResponse;
import com.example.irrigacion.dto.CommandRequest;
import com.example.irrigacion.service.AgendaService;
import com.example.irrigacion.service.MqttGateway;
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

    public AgendaController(AgendaService agendaService, Optional<MqttGateway> mqttGateway) {
        this.agendaService = agendaService;
        this.mqttGateway = mqttGateway.orElse(null);
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
