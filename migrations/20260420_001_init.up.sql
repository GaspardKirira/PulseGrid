-- ======================================================
-- PulseGrid - Initial schema
-- ======================================================

PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS monitors (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    slug TEXT NOT NULL UNIQUE,
    url TEXT NOT NULL,
    interval_seconds INTEGER NOT NULL,
    current_status TEXT NOT NULL,
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_monitors_slug
ON monitors(slug);

CREATE INDEX IF NOT EXISTS idx_monitors_status
ON monitors(current_status);

CREATE TABLE IF NOT EXISTS check_results (
    id TEXT PRIMARY KEY,
    monitor_id TEXT NOT NULL,
    status TEXT NOT NULL,
    response_time_ms INTEGER,
    error_message TEXT NOT NULL DEFAULT '',
    checked_at_ms INTEGER NOT NULL,
    FOREIGN KEY (monitor_id) REFERENCES monitors(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_check_results_monitor_id
ON check_results(monitor_id);

CREATE INDEX IF NOT EXISTS idx_check_results_checked_at_ms
ON check_results(checked_at_ms);

CREATE TABLE IF NOT EXISTS incidents (
    id TEXT PRIMARY KEY,
    monitor_id TEXT NOT NULL,
    message TEXT NOT NULL,
    started_at_ms INTEGER NOT NULL,
    resolved_at_ms INTEGER,
    state TEXT NOT NULL,
    FOREIGN KEY (monitor_id) REFERENCES monitors(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_incidents_monitor_id
ON incidents(monitor_id);

CREATE INDEX IF NOT EXISTS idx_incidents_state
ON incidents(state);
