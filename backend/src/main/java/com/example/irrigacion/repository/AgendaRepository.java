package com.example.irrigacion.repository;

import com.example.irrigacion.model.Agenda;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;

import java.time.LocalTime;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface AgendaRepository extends JpaRepository<Agenda, UUID> {
    List<Agenda> findByNodeId(UUID nodeId);

    @Query("select a from Agenda a where a.nodeId = :nodeId and a.zona = :zona and a.activa = true")
    List<Agenda> findActiveByNodeAndZona(UUID nodeId, short zona);

    Optional<Agenda> findByNodeIdAndId(UUID nodeId, UUID id);
}
