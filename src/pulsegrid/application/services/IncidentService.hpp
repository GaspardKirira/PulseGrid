/**
 *
 *  @file IncidentService.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application service for incident use cases.
 *
 */

#ifndef PULSEGRID_APPLICATION_SERVICES_INCIDENT_SERVICE_HPP
#define PULSEGRID_APPLICATION_SERVICES_INCIDENT_SERVICE_HPP

#include <optional>
#include <vector>

#include <pulsegrid/application/ports/Clock.hpp>
#include <pulsegrid/application/ports/EventBus.hpp>
#include <pulsegrid/application/ports/IncidentRepository.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::services
{
  class IncidentService
  {
  public:
    using EntityId = pulsegrid::domain::shared::EntityId;
    using Incident = pulsegrid::domain::incident::Incident;

    IncidentService(
        pulsegrid::application::ports::IncidentRepository &incident_repository,
        pulsegrid::application::ports::Clock &clock,
        pulsegrid::application::ports::EventBus &event_bus);

    [[nodiscard]] Incident get_incident_by_id(const EntityId &id) const;

    [[nodiscard]] std::optional<Incident> get_open_incident_for_monitor(
        const EntityId &monitor_id) const;

    [[nodiscard]] std::vector<Incident> list_incidents_for_monitor(
        const EntityId &monitor_id,
        std::size_t limit) const;

    [[nodiscard]] Incident resolve_incident(const EntityId &id);

  private:
    pulsegrid::application::ports::IncidentRepository &incident_repository_;
    pulsegrid::application::ports::Clock &clock_;
    pulsegrid::application::ports::EventBus &event_bus_;
  };

} // namespace pulsegrid::application::services

#endif // PULSEGRID_APPLICATION_SERVICES_INCIDENT_SERVICE_HPP
