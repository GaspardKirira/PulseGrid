/**
 *
 *  @file WsEventBus.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "WsEventBus.hpp"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <pulsegrid/support/Json.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::infrastructure::events
{
  namespace J = vix::json;

  WsEventBus::WsEventBus(Broadcaster broadcaster)
      : broadcaster_(std::move(broadcaster))
  {
    if (!broadcaster_)
    {
      throw pulsegrid::support::RealtimeError(
          "WsEventBus requires a valid broadcaster");
    }
  }

  void WsEventBus::publish_monitor_created(
      const pulsegrid::domain::monitor::Monitor &monitor)
  {
    emit(build_monitor_event("monitor.created", monitor));
  }

  void WsEventBus::publish_monitor_updated(
      const pulsegrid::domain::monitor::Monitor &monitor)
  {
    emit(build_monitor_event("monitor.updated", monitor));
  }

  void WsEventBus::publish_check_recorded(
      const pulsegrid::domain::check::CheckResult &result)
  {
    emit(build_check_event("check.recorded", result));
  }

  void WsEventBus::publish_incident_opened(
      const pulsegrid::domain::incident::Incident &incident)
  {
    emit(build_incident_event("incident.opened", incident));
  }

  void WsEventBus::publish_incident_resolved(
      const pulsegrid::domain::incident::Incident &incident)
  {
    emit(build_incident_event("incident.resolved", incident));
  }

  std::string WsEventBus::build_monitor_event(
      const std::string &event_type,
      const pulsegrid::domain::monitor::Monitor &monitor)
  {
    J::Json payload = J::o(
        "type", event_type,
        "data", J::o("id", monitor.id().value(), "name", monitor.name(), "slug", monitor.slug(), "url", monitor.url().value(), "interval_seconds", monitor.interval().seconds(), "status", pulsegrid::domain::monitor::to_string(monitor.status()), "created_at_ms", monitor.created_at_ms(), "updated_at_ms", monitor.updated_at_ms()));

    return pulsegrid::support::stringify(payload);
  }

  std::string WsEventBus::build_check_event(
      const std::string &event_type,
      const pulsegrid::domain::check::CheckResult &result)
  {
    J::Json data = J::o(
        "id", result.id().value(),
        "monitor_id", result.monitor_id().value(),
        "status", pulsegrid::domain::monitor::to_string(result.status()),
        "error_message", result.error_message(),
        "checked_at_ms", result.checked_at_ms());

    if (result.response_time().has_value())
    {
      data["response_time_ms"] = result.response_time()->milliseconds();
    }
    else
    {
      data["response_time_ms"] = nullptr;
    }

    J::Json payload = J::o(
        "type", event_type,
        "data", data);

    return pulsegrid::support::stringify(payload);
  }

  std::string WsEventBus::build_incident_event(
      const std::string &event_type,
      const pulsegrid::domain::incident::Incident &incident)
  {
    J::Json data = J::o(
        "id", incident.id().value(),
        "monitor_id", incident.monitor_id().value(),
        "message", incident.message(),
        "started_at_ms", incident.started_at_ms(),
        "state", pulsegrid::domain::incident::to_string(incident.state()));

    if (incident.resolved_at_ms().has_value())
    {
      data["resolved_at_ms"] = *incident.resolved_at_ms();
    }
    else
    {
      data["resolved_at_ms"] = nullptr;
    }

    J::Json payload = J::o(
        "type", event_type,
        "data", data);

    return pulsegrid::support::stringify(payload);
  }

  void WsEventBus::emit(const std::string &payload) const
  {
    try
    {
      broadcaster_(payload);
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::RealtimeError(
          std::string("Failed to broadcast websocket event: ") + e.what());
    }
  }

} // namespace pulsegrid::infrastructure::events
