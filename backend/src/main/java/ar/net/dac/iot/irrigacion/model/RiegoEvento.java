package ar.net.dac.iot.irrigacion.model;

import jakarta.persistence.*;
import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "riego_evento")
public class RiegoEvento {
    
    @Id
    private UUID id;
    
    @Column(name = "node_id", nullable = false)
    private UUID nodeId;
    
    @Column(nullable = false)
    private Short zona;
    
    @Column(nullable = false)
    private Instant timestamp;
    
    @Column(name = "duracion_seg", nullable = false)
    private Integer duracionSeg;
    
    @Column(nullable = false)
    private String origen; // "agenda" o "manual"
    
    @Column(name = "version_agenda")
    private Integer versionAgenda;
    
    @Column(columnDefinition = "jsonb")
    private String raw;
    
    @Column(name = "created_at", nullable = false)
    private Instant createdAt;
    
    @PrePersist
    public void prePersist() {
        if (id == null) {
            id = UUID.randomUUID();
        }
        if (createdAt == null) {
            createdAt = Instant.now();
        }
    }

    // Getters y Setters
    public UUID getId() {
        return id;
    }

    public void setId(UUID id) {
        this.id = id;
    }

    public UUID getNodeId() {
        return nodeId;
    }

    public void setNodeId(UUID nodeId) {
        this.nodeId = nodeId;
    }

    public Short getZona() {
        return zona;
    }

    public void setZona(Short zona) {
        this.zona = zona;
    }

    public Instant getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Instant timestamp) {
        this.timestamp = timestamp;
    }

    public Integer getDuracionSeg() {
        return duracionSeg;
    }

    public void setDuracionSeg(Integer duracionSeg) {
        this.duracionSeg = duracionSeg;
    }

    public String getOrigen() {
        return origen;
    }

    public void setOrigen(String origen) {
        this.origen = origen;
    }

    public Integer getVersionAgenda() {
        return versionAgenda;
    }

    public void setVersionAgenda(Integer versionAgenda) {
        this.versionAgenda = versionAgenda;
    }

    public String getRaw() {
        return raw;
    }

    public void setRaw(String raw) {
        this.raw = raw;
    }

    public Instant getCreatedAt() {
        return createdAt;
    }

    public void setCreatedAt(Instant createdAt) {
        this.createdAt = createdAt;
    }
}
