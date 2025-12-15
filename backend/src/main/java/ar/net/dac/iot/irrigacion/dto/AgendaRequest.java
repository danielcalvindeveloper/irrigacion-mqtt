package ar.net.dac.iot.irrigacion.dto;

import jakarta.validation.constraints.Max;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotEmpty;
import jakarta.validation.constraints.NotNull;
import jakarta.validation.constraints.Pattern;

import java.util.List;
import java.util.UUID;

public class AgendaRequest {
    @NotNull
    private UUID id;

    @NotNull
    private UUID nodeId;

    private String nombre;

    @Min(1)
    @Max(8)
    private short zona;

    @NotEmpty
    private List<@Pattern(regexp = "LUN|MAR|MIE|JUE|VIE|SAB|DOM") String> diasSemana;

    @NotBlank
    @Pattern(regexp = "^\\d{2}:\\d{2}$")
    private String horaInicio;

    @Min(1)
    @Max(180)
    private short duracionMin;

    private boolean activa = true;

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

    public String getNombre() {
        return nombre;
    }

    public void setNombre(String nombre) {
        this.nombre = nombre;
    }

    public short getZona() {
        return zona;
    }

    public void setZona(short zona) {
        this.zona = zona;
    }

    public List<String> getDiasSemana() {
        return diasSemana;
    }

    public void setDiasSemana(List<String> diasSemana) {
        this.diasSemana = diasSemana;
    }

    public String getHoraInicio() {
        return horaInicio;
    }

    public void setHoraInicio(String horaInicio) {
        this.horaInicio = horaInicio;
    }

    public short getDuracionMin() {
        return duracionMin;
    }

    public void setDuracionMin(short duracionMin) {
        this.duracionMin = duracionMin;
    }

    public boolean isActiva() {
        return activa;
    }

    public void setActiva(boolean activa) {
        this.activa = activa;
    }
}
