/**
 *
 *  @file IncidentRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Incident repository port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_INCIDENT_REPOSITORY_HPP
#define PULSEGRID_APPLICATION_PORTS_INCIDENT_REPOSITORY_HPP

#include <optional>
#include <vector>

#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::ports
{
  class IncidentRepository
  {
  public:
    virtual ~IncidentRepository() = default;

    /**
     * @brief Persist a new incident.
     */
    virtual void create(const pulsegrid::domain::incident::Incident &incident) = 0;

    /**
     * @brief Persist updates to an existing incident.
     */
    virtual void update(const pulsegrid::domain::incident::Incident &incident) = 0;

    /**
     * @brief Find an incident by id.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::incident::Incident> find_by_id(
        const pulsegrid::domain::shared::EntityId &id) const = 0;

    /**
     * @brief Find the currently open incident for a monitor, if any.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::incident::Incident> find_open_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id) const = 0;

    /**
     * @brief Return recent incidents for a monitor.
     */
    [[nodiscard]] virtual std::vector<pulsegrid::domain::incident::Incident> list_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id,
        std::size_t limit) const = 0;

    /**
     * @brief Delete all incidents for a monitor.
     *
     * @return number of deleted rows/entities.
     */
    [[nodiscard]] virtual std::size_t remove_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id) = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_INCIDENT_REPOSITORY_HPP
