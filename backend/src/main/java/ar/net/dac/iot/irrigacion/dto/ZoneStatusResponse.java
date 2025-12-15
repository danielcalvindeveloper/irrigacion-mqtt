<<<<<<< HEAD:backend/src/main/java/ar/net/dac/iot/irrigacion/dto/ZoneStatusResponse.java
package ar.net.dac.iot.irrigacion.dto;
=======
package com.example.irrigacion.dto;
>>>>>>> 83d6e69e1375cfe708f4813fbea4b5d356111ce1:backend/src/main/java/com/example/irrigacion/dto/ZoneStatusResponse.java

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
