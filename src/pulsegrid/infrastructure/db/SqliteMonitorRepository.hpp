/**
 *
 *  @file SqliteMonitorRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  SQLite monitor repository.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_DB_SQLITE_MONITOR_REPOSITORY_HPP
#define PULSEGRID_INFRASTRUCTURE_DB_SQLITE_MONITOR_REPOSITORY_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <vix/db/Database.hpp>

namespace pulsegrid::infrastructure::db
{
  class SqliteMonitorRepository final
      : public pulsegrid::application::ports::MonitorRepository
  {
  public:
    using Monitor = pulsegrid::domain::monitor::Monitor;
    using EntityId = pulsegrid::domain::shared::EntityId;

    explicit SqliteMonitorRepository(std::shared_ptr<vix::db::Database> database);

    void create(const Monitor &monitor) override;
    void update(const Monitor &monitor) override;

    [[nodiscard]] std::optional<Monitor> find_by_id(
        const EntityId &id) const override;

    [[nodiscard]] std::optional<Monitor> find_by_slug(
        const std::string &slug) const override;

    [[nodiscard]] std::vector<Monitor> list_all() const override;

    [[nodiscard]] bool exists_by_slug(const std::string &slug) const override;

    [[nodiscard]] bool remove_by_id(const EntityId &id) override;

  private:
    [[nodiscard]] static Monitor map_monitor(const vix::db::ResultRow &row);

  private:
    std::shared_ptr<vix::db::Database> database_;
  };

} // namespace pulsegrid::infrastructure::db

#endif // PULSEGRID_INFRASTRUCTURE_DB_SQLITE_MONITOR_REPOSITORY_HPP
