-- Flyway V2: Agregar campo nombre a la tabla agenda

ALTER TABLE agenda ADD COLUMN IF NOT EXISTS nombre VARCHAR(100);

COMMENT ON COLUMN agenda.nombre IS 'Nombre opcional descriptivo de la agenda';
