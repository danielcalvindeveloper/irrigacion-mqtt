package ar.net.dac.iot.irrigacion.dto;

import java.time.Instant;
import java.util.UUID;

public class RiegoEventoResponse {
    private UUID id;
    private Short zona;
    private Instant timestamp;
    private Integer duracionSeg;
    private String origen;
    private Integer versionAgenda;

    public RiegoEventoResponse() {
    }

    public RiegoEventoResponse(UUID id, Short zona, Instant timestamp, Integer duracionSeg, 
                               String origen, Integer versionAgenda) {
        this.id = id;
        this.zona = zona;
        this.timestamp = timestamp;
        this.duracionSeg = duracionSeg;
        this.origen = origen;
        this.versionAgenda = versionAgenda;
    }

    // Getters y Setters
    public UUID getId() {
        return id;
    }

    public void setId(UUID id) {
        this.id = id;
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
}
