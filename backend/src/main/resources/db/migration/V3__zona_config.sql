-- Flyway V3: Tabla de configuración de zonas

-- Crear tabla de configuración de zonas
CREATE TABLE IF NOT EXISTS zona_config (
    node_id UUID NOT NULL,
    zona SMALLINT NOT NULL,
    nombre VARCHAR(100) NOT NULL DEFAULT 'Zona',
    habilitada BOOLEAN NOT NULL DEFAULT TRUE,
    icono VARCHAR(50) DEFAULT 'water_drop',
    orden SMALLINT NOT NULL DEFAULT 0,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY (node_id, zona),
    CHECK (zona BETWEEN 1 AND 8)
);

-- Índice para queries de UI ordenadas
CREATE INDEX idx_zona_config_node_orden ON zona_config(node_id, orden);

-- Datos por defecto para nodo existente (550e8400-e29b-41d4-a716-446655440000)
INSERT INTO zona_config (node_id, zona, nombre, habilitada, orden) VALUES
  ('550e8400-e29b-41d4-a716-446655440000', 1, 'Zona 1', true, 1),
  ('550e8400-e29b-41d4-a716-446655440000', 2, 'Zona 2', true, 2),
  ('550e8400-e29b-41d4-a716-446655440000', 3, 'Zona 3', true, 3),
  ('550e8400-e29b-41d4-a716-446655440000', 4, 'Zona 4', true, 4)
ON CONFLICT (node_id, zona) DO NOTHING;

-- Actualizar CHECK constraints existentes para soportar hasta 8 zonas
ALTER TABLE agenda DROP CONSTRAINT IF EXISTS agenda_zona_check;
ALTER TABLE agenda ADD CONSTRAINT agenda_zona_check CHECK (zona BETWEEN 1 AND 8);

ALTER TABLE riego_evento DROP CONSTRAINT IF EXISTS riego_evento_zona_check;
ALTER TABLE riego_evento ADD CONSTRAINT riego_evento_zona_check CHECK (zona BETWEEN 1 AND 8);

ALTER TABLE humedad DROP CONSTRAINT IF EXISTS humedad_zona_check;
ALTER TABLE humedad ADD CONSTRAINT humedad_zona_check CHECK (zona BETWEEN 1 AND 8);

COMMENT ON TABLE zona_config IS 'Configuración de zonas de riego por nodo';
COMMENT ON COLUMN zona_config.nombre IS 'Nombre descriptivo de la zona personalizable por usuario';
COMMENT ON COLUMN zona_config.habilitada IS 'Indica si la zona está activa y disponible para riego';
COMMENT ON COLUMN zona_config.icono IS 'Identificador de icono para UI (garden, lawn, vegetables, flowers, water_drop, sprinkler)';
COMMENT ON COLUMN zona_config.orden IS 'Orden de visualización en UI (0 = primero)';
