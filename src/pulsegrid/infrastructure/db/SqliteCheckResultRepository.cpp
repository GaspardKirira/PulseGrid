/**
 *
 *  @file SqliteCheckResultRepository.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "SqliteCheckResultRepository.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pulsegrid/domain/check/ResponseTime.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/db/core/Result.hpp>

namespace pulsegrid::infrastructure::db
{
  SqliteCheckResultRepository::SqliteCheckResultRepository(
      std::shared_ptr<vix::db::Database> database)
      : database_(std::move(database))
  {
    if (!database_)
    {
      throw pulsegrid::support::PersistenceError(
          "SqliteCheckResultRepository requires a valid database");
    }
  }

  void SqliteCheckResultRepository::create(const CheckResult &result)
  {
    try
    {
      const auto status =
          pulsegrid::domain::monitor::to_string(result.status());

      if (result.status() == pulsegrid::domain::monitor::MonitorStatus::Down)
      {
        database_->exec(
            "INSERT INTO check_results ("
            "id, monitor_id, status, response_time_ms, error_message, checked_at_ms"
            ") VALUES (?, ?, ?, ?, ?, ?)",
            result.id().value(),
            result.monitor_id().value(),
            status,
            nullptr,
            result.error_message(),
            result.checked_at_ms());
        return;
      }

      if (result.response_time().has_value())
      {
        database_->exec(
            "INSERT INTO check_results ("
            "id, monitor_id, status, response_time_ms, error_message, checked_at_ms"
            ") VALUES (?, ?, ?, ?, ?, ?)",
            result.id().value(),
            result.monitor_id().value(),
            status,
            result.response_time()->milliseconds(),
            result.error_message(),
            result.checked_at_ms());
      }
      else
      {
        database_->exec(
            "INSERT INTO check_results ("
            "id, monitor_id, status, response_time_ms, error_message, checked_at_ms"
            ") VALUES (?, ?, ?, ?, ?, ?)",
            result.id().value(),
            result.monitor_id().value(),
            status,
            nullptr,
            result.error_message(),
            result.checked_at_ms());
      }
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to create check result: ") + e.what());
    }
  }
  std::optional<SqliteCheckResultRepository::CheckResult>
  SqliteCheckResultRepository::find_by_id(const EntityId &id) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, status, response_time_ms, error_message, checked_at_ms "
          "FROM check_results WHERE id = ? LIMIT 1",
          id.value());

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_check_result(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find check result by id: ") + e.what());
    }
  }

  std::optional<SqliteCheckResultRepository::CheckResult>
  SqliteCheckResultRepository::find_latest_by_monitor_id(const EntityId &monitor_id) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, status, response_time_ms, error_message, checked_at_ms "
          "FROM check_results "
          "WHERE monitor_id = ? "
          "ORDER BY checked_at_ms DESC "
          "LIMIT 1",
          monitor_id.value());

      if (!rows->next())
      {
        return std::nullopt;
      }

      return map_check_result(rows->row());
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to find latest check result: ") + e.what());
    }
  }

  std::vector<SqliteCheckResultRepository::CheckResult>
  SqliteCheckResultRepository::list_by_monitor_id(
      const EntityId &monitor_id,
      std::size_t limit) const
  {
    try
    {
      auto rows = database_->query(
          "SELECT id, monitor_id, status, response_time_ms, error_message, checked_at_ms "
          "FROM check_results "
          "WHERE monitor_id = ? "
          "ORDER BY checked_at_ms DESC "
          "LIMIT ?",
          monitor_id.value(),
          static_cast<std::int64_t>(limit));

      std::vector<CheckResult> results;
      results.reserve(limit);

      while (rows->next())
      {
        results.push_back(map_check_result(rows->row()));
      }

      return results;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to list check results by monitor id: ") + e.what());
    }
  }

  std::size_t SqliteCheckResultRepository::remove_by_monitor_id(
      const EntityId &monitor_id)
  {
    try
    {
      const auto affected = database_->exec(
          "DELETE FROM check_results WHERE monitor_id = ?",
          monitor_id.value());

      return static_cast<std::size_t>(affected);
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to delete check results by monitor id: ") + e.what());
    }
  }

  SqliteCheckResultRepository::CheckResult
  SqliteCheckResultRepository::map_check_result(const vix::db::ResultRow &row)
  {
    const auto status =
        pulsegrid::domain::monitor::monitor_status_from_string(row.getString(2));

    std::optional<pulsegrid::domain::check::ResponseTime> response_time;

    if (status == pulsegrid::domain::monitor::MonitorStatus::Down)
    {
      response_time = std::nullopt;
    }
    else
    {
      if (!row.isNull(3))
      {
        response_time = pulsegrid::domain::check::ResponseTime(row.getInt64(3));
      }
    }

    return CheckResult(
        pulsegrid::domain::shared::EntityId(row.getString(0)),
        pulsegrid::domain::shared::EntityId(row.getString(1)),
        status,
        response_time,
        row.getStringOr(4, ""),
        row.getInt64(5));
  }

} // namespace pulsegrid::infrastructure::db
