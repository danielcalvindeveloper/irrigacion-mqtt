package ar.net.dac.iot.irrigacion.dto;

public class ZoneStatusResponse {
    private int zona;
    private String nombre;
    private boolean activa;
    private Integer tiempoRestanteSeg;
    private String proximoRiego;

    public int getZona() {
        return zona;
    }

    public void setZona(int zona) {
        this.zona = zona;
    }

    public String getNombre() {
        return nombre;
    }

    public void setNombre(String nombre) {
        this.nombre = nombre;
    }

    public boolean isActiva() {
        return activa;
    }

    public void setActiva(boolean activa) {
        this.activa = activa;
    }

    public Integer getTiempoRestanteSeg() {
        return tiempoRestanteSeg;
    }

    public void setTiempoRestanteSeg(Integer tiempoRestanteSeg) {
        this.tiempoRestanteSeg = tiempoRestanteSeg;
    }

    public String getProximoRiego() {
        return proximoRiego;
    }

    public void setProximoRiego(String proximoRiego) {
        this.proximoRiego = proximoRiego;
    }
}
