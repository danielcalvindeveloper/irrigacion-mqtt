-- Migraciones iniciales (PostgreSQL) para MVP

CREATE TABLE IF NOT EXISTS agenda (
    id UUID PRIMARY KEY,
    node_id UUID NOT NULL,
    zona SMALLINT NOT NULL CHECK (zona BETWEEN 1 AND 4),
    dias_semana TEXT NOT NULL,
    hora_inicio TIME NOT NULL,
    duracion_min SMALLINT NOT NULL CHECK (duracion_min BETWEEN 1 AND 180),
    activa BOOLEAN NOT NULL DEFAULT TRUE,
    version INTEGER NOT NULL,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE (node_id, id)
);

-- Evita duplicados exactos de horario por zona; ajustar si se permite más de una agenda por slot
CREATE UNIQUE INDEX IF NOT EXISTS ux_agenda_slot
ON agenda (node_id, zona, dias_semana, hora_inicio)
WHERE activa = TRUE;

CREATE TABLE IF NOT EXISTS agenda_version (
    node_id UUID PRIMARY KEY,
    version INTEGER NOT NULL,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS riego_evento (
    id UUID PRIMARY KEY,
    node_id UUID NOT NULL,
    zona SMALLINT NOT NULL CHECK (zona BETWEEN 1 AND 4),
    timestamp TIMESTAMPTZ NOT NULL,
    duracion_seg INTEGER NOT NULL CHECK (duracion_seg BETWEEN 1 AND 7200),
    origen TEXT NOT NULL CHECK (origen IN ('agenda','manual')),
    version_agenda INTEGER,
    raw JSONB,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
CREATE INDEX IF NOT EXISTS idx_riego_evento_node_ts ON riego_evento (node_id, zona, timestamp DESC);

CREATE TABLE IF NOT EXISTS humedad (
    id UUID PRIMARY KEY,
    node_id UUID NOT NULL,
    zona SMALLINT NOT NULL CHECK (zona BETWEEN 1 AND 4),
    timestamp TIMESTAMPTZ NOT NULL,
    valor SMALLINT NOT NULL CHECK (valor BETWEEN 0 AND 100),
    raw JSONB,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
CREATE INDEX IF NOT EXISTS idx_humedad_node_ts ON humedad (node_id, zona, timestamp DESC);

-- Retención sugerida (agendar como job / procedure)
-- DELETE FROM riego_evento WHERE timestamp < NOW() - INTERVAL '12 months';
-- DELETE FROM humedad WHERE timestamp < NOW() - INTERVAL '30 days';
