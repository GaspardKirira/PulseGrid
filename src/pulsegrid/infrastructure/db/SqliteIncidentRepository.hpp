/**
 *
 *  @file SqliteIncidentRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  SQLite incident repository.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_DB_SQLITE_INCIDENT_REPOSITORY_HPP
#define PULSEGRID_INFRASTRUCTURE_DB_SQLITE_INCIDENT_REPOSITORY_HPP

#include <memory>
#include <optional>
#include <vector>

#include <pulsegrid/application/ports/IncidentRepository.hpp>
#include <vix/db/Database.hpp>

namespace pulsegrid::infrastructure::db
{
  class SqliteIncidentRepository final
      : public pulsegrid::application::ports::IncidentRepository
  {
  public:
    using Incident = pulsegrid::domain::incident::Incident;
    using EntityId = pulsegrid::domain::shared::EntityId;

    explicit SqliteIncidentRepository(std::shared_ptr<vix::db::Database> database);

    void create(const Incident &incident) override;
    void update(const Incident &incident) override;

    [[nodiscard]] std::optional<Incident> find_by_id(
        const EntityId &id) const override;

    [[nodiscard]] std::optional<Incident> find_open_by_monitor_id(
        const EntityId &monitor_id) const override;

    [[nodiscard]] std::vector<Incident> list_by_monitor_id(
        const EntityId &monitor_id,
        std::size_t limit) const override;

    [[nodiscard]] std::size_t remove_by_monitor_id(
        const EntityId &monitor_id) override;

  private:
    [[nodiscard]] static Incident map_incident(const vix::db::ResultRow &row);

  private:
    std::shared_ptr<vix::db::Database> database_;
  };

} // namespace pulsegrid::infrastructure::db

#endif // PULSEGRID_INFRASTRUCTURE_DB_SQLITE_INCIDENT_REPOSITORY_HPP
