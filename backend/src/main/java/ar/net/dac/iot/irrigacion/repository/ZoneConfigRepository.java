package ar.net.dac.iot.irrigacion.repository;

import ar.net.dac.iot.irrigacion.model.ZoneConfig;
import ar.net.dac.iot.irrigacion.model.ZoneConfigId;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

@Repository
public interface ZoneConfigRepository extends JpaRepository<ZoneConfig, ZoneConfigId> {
    
    @Query("SELECT z FROM ZoneConfig z WHERE z.nodeId = :nodeId ORDER BY z.orden ASC, z.zona ASC")
    List<ZoneConfig> findByNodeIdOrderByOrdenAsc(UUID nodeId);
    
    @Query("SELECT z FROM ZoneConfig z WHERE z.nodeId = :nodeId AND z.habilitada = true ORDER BY z.orden ASC, z.zona ASC")
    List<ZoneConfig> findByNodeIdAndHabilitadaTrueOrderByOrdenAsc(UUID nodeId);
    
    Optional<ZoneConfig> findByNodeIdAndZona(UUID nodeId, short zona);
    
    @Query("SELECT CASE WHEN COUNT(z) > 0 THEN true ELSE false END FROM ZoneConfig z WHERE z.nodeId = :nodeId AND z.zona = :zona AND z.habilitada = true")
    boolean isZoneEnabled(UUID nodeId, short zona);
}
