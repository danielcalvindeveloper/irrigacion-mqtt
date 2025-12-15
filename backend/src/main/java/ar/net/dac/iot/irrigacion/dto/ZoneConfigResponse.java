package ar.net.dac.iot.irrigacion.dto;

import java.time.OffsetDateTime;

public class ZoneConfigResponse extends ZoneConfigRequest {
    private OffsetDateTime createdAt;
    private OffsetDateTime updatedAt;
    
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
