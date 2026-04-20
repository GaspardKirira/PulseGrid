/**
 *
 *  @file SqliteCheckResultRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  SQLite check result repository.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_DB_SQLITE_CHECK_RESULT_REPOSITORY_HPP
#define PULSEGRID_INFRASTRUCTURE_DB_SQLITE_CHECK_RESULT_REPOSITORY_HPP

#include <memory>
#include <optional>
#include <vector>

#include <pulsegrid/application/ports/CheckResultRepository.hpp>
#include <vix/db/Database.hpp>

namespace pulsegrid::infrastructure::db
{
  class SqliteCheckResultRepository final
      : public pulsegrid::application::ports::CheckResultRepository
  {
  public:
    using CheckResult = pulsegrid::domain::check::CheckResult;
    using EntityId = pulsegrid::domain::shared::EntityId;

    explicit SqliteCheckResultRepository(std::shared_ptr<vix::db::Database> database);

    void create(const CheckResult &result) override;

    [[nodiscard]] std::optional<CheckResult> find_by_id(
        const EntityId &id) const override;

    [[nodiscard]] std::optional<CheckResult> find_latest_by_monitor_id(
        const EntityId &monitor_id) const override;

    [[nodiscard]] std::vector<CheckResult> list_by_monitor_id(
        const EntityId &monitor_id,
        std::size_t limit) const override;

    [[nodiscard]] std::size_t remove_by_monitor_id(
        const EntityId &monitor_id) override;

  private:
    [[nodiscard]] static CheckResult map_check_result(const vix::db::ResultRow &row);

  private:
    std::shared_ptr<vix::db::Database> database_;
  };

} // namespace pulsegrid::infrastructure::db

#endif // PULSEGRID_INFRASTRUCTURE_DB_SQLITE_CHECK_RESULT_REPOSITORY_HPP
