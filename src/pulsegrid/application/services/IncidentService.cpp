/**
 *
 *  @file IncidentService.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "IncidentService.hpp"

#include <pulsegrid/support/Errors.hpp>

namespace pulsegrid::application::services
{
  IncidentService::IncidentService(
      pulsegrid::application::ports::IncidentRepository &incident_repository,
      pulsegrid::application::ports::Clock &clock,
      pulsegrid::application::ports::EventBus &event_bus)
      : incident_repository_(incident_repository),
        clock_(clock),
        event_bus_(event_bus)
  {
  }

  IncidentService::Incident IncidentService::get_incident_by_id(
      const EntityId &id) const
  {
    auto incident = incident_repository_.find_by_id(id);
    if (!incident.has_value())
    {
      throw pulsegrid::support::NotFoundError("Incident not found");
    }

    return *incident;
  }

  std::optional<IncidentService::Incident> IncidentService::get_open_incident_for_monitor(
      const EntityId &monitor_id) const
  {
    return incident_repository_.find_open_by_monitor_id(monitor_id);
  }

  std::vector<IncidentService::Incident> IncidentService::list_incidents_for_monitor(
      const EntityId &monitor_id,
      std::size_t limit) const
  {
    return incident_repository_.list_by_monitor_id(monitor_id, limit);
  }

  IncidentService::Incident IncidentService::resolve_incident(
      const EntityId &id)
  {
    auto incident = incident_repository_.find_by_id(id);
    if (!incident.has_value())
    {
      throw pulsegrid::support::NotFoundError("Incident not found");
    }

    auto current = *incident;
    if (!current.is_resolved())
    {
      current.resolve(clock_.now_ms());
      incident_repository_.update(current);
      event_bus_.publish_incident_resolved(current);
    }

    return current;
  }

} // namespace pulsegrid::application::services
