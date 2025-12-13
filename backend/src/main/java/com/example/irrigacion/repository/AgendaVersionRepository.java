package com.example.irrigacion.repository;

import com.example.irrigacion.model.AgendaVersion;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

public interface AgendaVersionRepository extends JpaRepository<AgendaVersion, UUID> {
    Optional<AgendaVersion> findByNodeId(UUID nodeId);
}
