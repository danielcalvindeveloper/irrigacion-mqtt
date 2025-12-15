package ar.net.dac.iot.irrigacion.model;

import java.io.Serializable;
import java.util.Objects;
import java.util.UUID;

public class ZoneConfigId implements Serializable {
    private UUID nodeId;
    private short zona;
    
    public ZoneConfigId() {}
    
    public ZoneConfigId(UUID nodeId, short zona) {
        this.nodeId = nodeId;
        this.zona = zona;
    }
    
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
    
    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ZoneConfigId that = (ZoneConfigId) o;
        return zona == that.zona && Objects.equals(nodeId, that.nodeId);
    }
    
    @Override
    public int hashCode() {
        return Objects.hash(nodeId, zona);
    }
}
