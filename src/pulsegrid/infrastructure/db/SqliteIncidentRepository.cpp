/**
 *
 *  @file SqliteIncidentRepository.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "SqliteIncidentRepository.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/db/core/Result.hpp>

namespace pulsegrid::infrastructure::db
{
  SqliteIncidentRepository::SqliteIncidentRepository(
      std::shared_ptr<vix::db::Database> database)
      : database_(std::move(database))
  {
    if (!database_)
    {
      throw pulsegrid::support::PersistenceError(
          "SqliteIncidentRepository requires a valid database");
    }
  }

  void SqliteIncidentRepository::create(const Incident &incident)
  {
    try
    {
      if (incident.resolved_at_ms().has_value())
      {
        database_->exec(
            "INSERT INTO incidents ("
            "id, monitor_id, message, started_at_ms, resolved_at_ms, state"
            ") VALUES (?, ?, ?, ?, ?, ?)",
            incident.id().value(),
            incident.monitor_id().value(),
            incident.message(),
            incident.started_at_ms(),
            *incident.resolved_at_ms(),
            pulsegrid::domain::incident::to_string(incident.state()));
      }
      else
      {
        database_->exec(
            "INSERT INTO incidents ("
            "id, monitor_id, message, started_at_ms, resolved_at_ms, state"
            ") VALUES (?, ?, ?, ?, ?, ?)",
            incident.id().value(),
            incident.monitor_id().value(),
            incident.message(),
            incident.started_at_ms(),
            nullptr,
            pulsegrid::domain::incident::to_string(incident.state()));
      }
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to create incident: ") + e.what());
    }
  }

  void SqliteIncidentRepository::update(const Incident &incident)
  {
    try
    {
      if (incident.resolved_at_ms().has_value())
      {
        database_->exec(
            "UPDATE incidents "
            "SET monitor_id = ?, message = ?, started_at_ms = ?, resolved_at_ms = ?, state = ? "
            "WHERE id = ?",
            incident.monitor_id().value(),
            incident.message(),
            incident.started_at_ms(),
            *incident.resolved_at_ms(),
            pulsegrid::domain::incident::to_string(incident.state()),
            incident.id().value());
      }
      else
      {
        database_->exec(
            "UPDATE incidents "
            "SET monitor_id = ?, message = ?, started_at_ms = ?, resolved_at_ms = ?, state = ? "
            "WHERE id = ?",
            incident.monitor_id().value(),
            incident.message(),
            incident.started_at_ms(),
            nullptr,
            pulsegrid::domain::incident::to_string(incident.state()),
            incident.id().value());
      }
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to update incident: ") + e.what());
    }
  }

  std::optional<SqliteIncidentRepository::Incident>
  SqliteIncidentRepository::find_by_id(const EntityId &id) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, message, started_at_ms, resolved_at_ms, state "
          "FROM incidents WHERE id = ? LIMIT 1",
          id.value());

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_incident(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find incident by id: ") + e.what());
    }
  }

  std::optional<SqliteIncidentRepository::Incident>
  SqliteIncidentRepository::find_open_by_monitor_id(const EntityId &monitor_id) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, message, started_at_ms, resolved_at_ms, state "
          "FROM incidents "
          "WHERE monitor_id = ? AND state = ? AND resolved_at_ms IS NULL "
          "ORDER BY started_at_ms DESC "
          "LIMIT 1",
          monitor_id.value(),
          std::string("open"));

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_incident(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find open incident by monitor id: ") + e.what());
    }
  }

  std::vector<SqliteIncidentRepository::Incident>
  SqliteIncidentRepository::list_by_monitor_id(
      const EntityId &monitor_id,
      std::size_t limit) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, message, started_at_ms, resolved_at_ms, state "
          "FROM incidents "
          "WHERE monitor_id = ? "
          "ORDER BY started_at_ms DESC "
          "LIMIT ?",
          monitor_id.value(),
          static_cast<std::int64_t>(limit));

      std::vector<Incident> incidents;
      incidents.reserve(limit);

      while (rows->next())
      {
        incidents.push_back(map_incident(rows->row()));
      }

      return incidents;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to list incidents by monitor id: ") + e.what());
    }
  }

  std::size_t SqliteIncidentRepository::remove_by_monitor_id(
      const EntityId &monitor_id)
  {
    try
    {
      const auto affected = database_->exec(
          "DELETE FROM incidents WHERE monitor_id = ?",
          monitor_id.value());

      return static_cast<std::size_t>(affected);
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to delete incidents by monitor id: ") + e.what());
    }
  }

  SqliteIncidentRepository::Incident
  SqliteIncidentRepository::map_incident(const vix::db::ResultRow &row)
  {
    const auto state =
        pulsegrid::domain::incident::incident_state_from_string(row.getString(5));

    std::optional<std::int64_t> resolved_at_ms;

    if (state == pulsegrid::domain::incident::IncidentState::Resolved)
    {
      if (!row.isNull(4))
      {
        resolved_at_ms = row.getInt64(4);
      }
    }
    else
    {
      resolved_at_ms = std::nullopt;
    }

    return Incident(
        pulsegrid::domain::shared::EntityId(row.getString(0)),
        pulsegrid::domain::shared::EntityId(row.getString(1)),
        row.getString(2),
        row.getInt64(3),
        resolved_at_ms,
        state);
  }
} // namespace pulsegrid::infrastructure::db
