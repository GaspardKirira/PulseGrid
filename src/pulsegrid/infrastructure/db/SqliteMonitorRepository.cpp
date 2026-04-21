/**
 *
 *  @file SqliteMonitorRepository.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "SqliteMonitorRepository.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/db/core/Result.hpp>

namespace pulsegrid::infrastructure::db
{
  SqliteMonitorRepository::SqliteMonitorRepository(
      std::shared_ptr<vix::db::Database> database)
      : database_(std::move(database))
  {
    if (!database_)
    {
      throw pulsegrid::support::PersistenceError(
          "SqliteMonitorRepository requires a valid database");
    }
  }

  void SqliteMonitorRepository::create(const Monitor &monitor)
  {
    try
    {
      database_->exec(
          "INSERT INTO monitors ("
          "id, name, slug, url, interval_seconds, current_status, created_at_ms, updated_at_ms"
          ") VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
          monitor.id().value(),
          monitor.name(),
          monitor.slug(),
          monitor.url().value(),
          monitor.interval().seconds(),
          pulsegrid::domain::monitor::to_string(monitor.status()),
          monitor.created_at_ms(),
          monitor.updated_at_ms());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to create monitor: ") + e.what());
    }
  }

  void SqliteMonitorRepository::update(const Monitor &monitor)
  {
    try
    {
      database_->exec(
          "UPDATE monitors "
          "SET name = ?, slug = ?, url = ?, interval_seconds = ?, current_status = ?, updated_at_ms = ? "
          "WHERE id = ?",
          monitor.name(),
          monitor.slug(),
          monitor.url().value(),
          monitor.interval().seconds(),
          pulsegrid::domain::monitor::to_string(monitor.status()),
          monitor.updated_at_ms(),
          monitor.id().value());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to update monitor: ") + e.what());
    }
  }

  std::optional<SqliteMonitorRepository::Monitor> SqliteMonitorRepository::find_by_id(
      const EntityId &id) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, name, slug, url, interval_seconds, current_status, created_at_ms, updated_at_ms "
          "FROM monitors WHERE id = ? LIMIT 1",
          id.value());

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_monitor(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find monitor by id: ") + e.what());
    }
  }

  std::optional<SqliteMonitorRepository::Monitor> SqliteMonitorRepository::find_by_slug(
      const std::string &slug) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, name, slug, url, interval_seconds, current_status, created_at_ms, updated_at_ms "
          "FROM monitors WHERE slug = ? LIMIT 1",
          slug);

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_monitor(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find monitor by slug: ") + e.what());
    }
  }

  std::vector<SqliteMonitorRepository::Monitor> SqliteMonitorRepository::list_all() const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, name, slug, url, interval_seconds, current_status, created_at_ms, updated_at_ms "
          "FROM monitors ORDER BY created_at_ms ASC");

      std::vector<Monitor> monitors;

      while (rows->next())
      {
        monitors.push_back(map_monitor(rows->row()));
      }

      return monitors;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to list monitors: ") + e.what());
    }
  }

  bool SqliteMonitorRepository::exists_by_slug(const std::string &slug) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT 1 FROM monitors WHERE slug = ? LIMIT 1",
          slug);

      return rows->next();
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to check monitor slug existence: ") + e.what());
    }
  }

  bool SqliteMonitorRepository::remove_by_id(const EntityId &id)
  {
    try
    {
      const auto affected = database_->exec(
          "DELETE FROM monitors WHERE id = ?",
          id.value());

      return affected > 0;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to delete monitor: ") + e.what());
    }
  }

  SqliteMonitorRepository::Monitor SqliteMonitorRepository::map_monitor(
      const vix::db::ResultRow &row)
  {
    pulsegrid::domain::monitor::MonitorStatus status;

    try
    {
      status =
          pulsegrid::domain::monitor::monitor_status_from_string(row.getString(5));
    }
    catch (...)
    {
      status = pulsegrid::domain::monitor::MonitorStatus::Down;
    }

    return Monitor(
        pulsegrid::domain::shared::EntityId(row.getString(0)),
        row.getString(1),
        row.getString(2),
        pulsegrid::domain::shared::Url(row.getString(3)),
        pulsegrid::domain::monitor::CheckInterval(
            static_cast<int>(row.getInt64(4))),
        status,
        row.getInt64(6),
        row.getInt64(7));
  }

} // namespace pulsegrid::infrastructure::db
