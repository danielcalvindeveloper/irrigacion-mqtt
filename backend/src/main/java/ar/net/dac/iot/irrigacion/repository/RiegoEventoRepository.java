package ar.net.dac.iot.irrigacion.repository;

import ar.net.dac.iot.irrigacion.model.RiegoEvento;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

@Repository
public interface RiegoEventoRepository extends JpaRepository<RiegoEvento, UUID> {
    
    Page<RiegoEvento> findByNodeIdOrderByTimestampDesc(UUID nodeId, Pageable pageable);
    
    List<RiegoEvento> findByNodeIdAndTimestampBetweenOrderByTimestampDesc(
        UUID nodeId, Instant start, Instant end);
    
    List<RiegoEvento> findByNodeIdAndZonaOrderByTimestampDesc(UUID nodeId, Short zona);
}
