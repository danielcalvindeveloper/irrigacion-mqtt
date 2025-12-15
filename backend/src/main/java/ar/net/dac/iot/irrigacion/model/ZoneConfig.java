package ar.net.dac.iot.irrigacion.model;

import jakarta.persistence.*;
import java.time.OffsetDateTime;
import java.util.UUID;

@Entity
@Table(name = "zona_config")
@IdClass(ZoneConfigId.class)
public class ZoneConfig {
    
    @Id
    @Column(name = "node_id", nullable = false)
    private UUID nodeId;
    
    @Id
    @Column(nullable = false)
    private short zona;
    
    @Column(nullable = false, length = 100)
    private String nombre;
    
    @Column(nullable = false)
    private boolean habilitada = true;
    
    @Column(length = 50)
    private String icono = "water_drop";
    
    @Column(nullable = false)
    private short orden = 0;
    
    @Column(name = "created_at", nullable = false, updatable = false)
    private OffsetDateTime createdAt;
    
    @Column(name = "updated_at", nullable = false)
    private OffsetDateTime updatedAt;
    
    @PrePersist
    protected void onCreate() {
        createdAt = OffsetDateTime.now();
        updatedAt = OffsetDateTime.now();
    }
    
    @PreUpdate
    protected void onUpdate() {
        updatedAt = OffsetDateTime.now();
    }
    
    // Getters and Setters
    
    public UUID getNodeId() {
        return nodeId;
    }
    
    public void setNodeId(UUID nodeId) {
        this.nodeId = nodeId;
    }
    
    public short getZona() {
        return zona;
    }
    
    public void setZona(short zona) {
        this.zona = zona;
    }
    
    public String getNombre() {
        return nombre;
    }
    
    public void setNombre(String nombre) {
        this.nombre = nombre;
    }
    
    public boolean isHabilitada() {
        return habilitada;
    }
    
    public void setHabilitada(boolean habilitada) {
        this.habilitada = habilitada;
    }
    
    public String getIcono() {
        return icono;
    }
    
    public void setIcono(String icono) {
        this.icono = icono;
    }
    
    public short getOrden() {
        return orden;
    }
    
    public void setOrden(short orden) {
        this.orden = orden;
    }
    
    public OffsetDateTime getCreatedAt() {
        return createdAt;
    }
    
    public void setCreatedAt(OffsetDateTime createdAt) {
        this.createdAt = createdAt;
    }
    
    public OffsetDateTime getUpdatedAt() {
        return updatedAt;
    }
    
    public void setUpdatedAt(OffsetDateTime updatedAt) {
        this.updatedAt = updatedAt;
    }
}
