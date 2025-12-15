package ar.net.dac.iot.irrigacion.dto;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Pattern;
import jakarta.validation.constraints.Size;

import java.util.UUID;

public class ZoneConfigRequest {
    @NotNull
    private UUID nodeId;
    
    @Min(1)
    @Max(8)
    private short zona;
    
    @NotBlank
    @Size(min = 1, max = 100)
    private String nombre;
    
    private boolean habilitada = true;
    
    @Pattern(regexp = "garden|lawn|vegetables|flowers|water_drop|sprinkler|")
    private String icono = "water_drop";
    
    @Min(0)
    @Max(100)
    private short orden = 0;
    
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
}
