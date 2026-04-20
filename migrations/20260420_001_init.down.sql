-- ======================================================
-- PulseGrid - Rollback initial schema
-- ======================================================

PRAGMA foreign_keys = OFF;

DROP TABLE IF EXISTS incidents;
DROP TABLE IF EXISTS check_results;
DROP TABLE IF EXISTS monitors;
