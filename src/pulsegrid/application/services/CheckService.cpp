/**
 *
 *  @file CheckService.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "CheckService.hpp"

#include <utility>

#include <pulsegrid/support/Errors.hpp>

namespace pulsegrid::application::services
{
  CheckService::CheckService(
      pulsegrid::application::ports::MonitorRepository &monitor_repository,
      pulsegrid::application::ports::CheckResultRepository &check_result_repository,
      pulsegrid::application::ports::IncidentRepository &incident_repository,
      pulsegrid::application::ports::Clock &clock,
      pulsegrid::application::ports::IdGenerator &id_generator,
      pulsegrid::application::ports::EventBus &event_bus)
      : monitor_repository_(monitor_repository),
        check_result_repository_(check_result_repository),
        incident_repository_(incident_repository),
        clock_(clock),
        id_generator_(id_generator),
        event_bus_(event_bus)
  {
  }

  CheckService::RecordedCheck CheckService::record_success(
      const RecordSuccessInput &input)
  {
    auto monitor = monitor_repository_.find_by_id(input.monitor_id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    const auto now_ms = clock_.now_ms();

    CheckResult result = CheckResult::success(
        id_generator_.next(),
        input.monitor_id,
        ResponseTime(input.response_time_ms),
        now_ms);

    return finalize_success_result(input.monitor_id, std::move(result));
  }

  CheckService::RecordedCheck CheckService::record_degraded(
      const RecordDegradedInput &input)
  {
    auto monitor = monitor_repository_.find_by_id(input.monitor_id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    const auto now_ms = clock_.now_ms();

    CheckResult result = CheckResult::degraded(
        id_generator_.next(),
        input.monitor_id,
        ResponseTime(input.response_time_ms),
        input.error_message,
        now_ms);

    return finalize_problem_result(
        input.monitor_id,
        std::move(result),
        input.error_message);
  }

  CheckService::RecordedCheck CheckService::record_failure(
      const RecordFailureInput &input)
  {
    auto monitor = monitor_repository_.find_by_id(input.monitor_id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    const auto now_ms = clock_.now_ms();

    CheckResult result = CheckResult::failure(
        id_generator_.next(),
        input.monitor_id,
        input.error_message,
        now_ms);

    return finalize_problem_result(
        input.monitor_id,
        std::move(result),
        input.error_message);
  }

  std::optional<CheckService::CheckResult> CheckService::get_latest_for_monitor(
      const EntityId &monitor_id) const
  {
    return check_result_repository_.find_latest_by_monitor_id(monitor_id);
  }

  std::vector<CheckService::CheckResult> CheckService::list_recent_for_monitor(
      const EntityId &monitor_id,
      std::size_t limit) const
  {
    return check_result_repository_.list_by_monitor_id(monitor_id, limit);
  }

  CheckService::RecordedCheck CheckService::finalize_success_result(
      const EntityId &monitor_id,
      CheckResult result)
  {
    auto monitor = monitor_repository_.find_by_id(monitor_id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    auto current_monitor = *monitor;
    current_monitor.mark_up(clock_.now_ms());

    check_result_repository_.create(result);
    monitor_repository_.update(current_monitor);

    event_bus_.publish_check_recorded(result);
    event_bus_.publish_monitor_updated(current_monitor);

    RecordedCheck recorded{
        .result = result,
        .opened_incident = std::nullopt,
        .resolved_incident = std::nullopt};

    auto open_incident = incident_repository_.find_open_by_monitor_id(monitor_id);
    if (open_incident.has_value())
    {
      auto incident = *open_incident;
      incident.resolve(clock_.now_ms());

      incident_repository_.update(incident);
      event_bus_.publish_incident_resolved(incident);

      recorded.resolved_incident = incident;
    }

    return recorded;
  }

  CheckService::RecordedCheck CheckService::finalize_problem_result(
      const EntityId &monitor_id,
      CheckResult result,
      const std::string &incident_message)
  {
    auto monitor = monitor_repository_.find_by_id(monitor_id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    auto current_monitor = *monitor;

    if (result.is_failure())
    {
      current_monitor.mark_down(clock_.now_ms());
    }
    else
    {
      current_monitor.mark_degraded(clock_.now_ms());
    }

    check_result_repository_.create(result);
    monitor_repository_.update(current_monitor);

    event_bus_.publish_check_recorded(result);
    event_bus_.publish_monitor_updated(current_monitor);

    RecordedCheck recorded{
        .result = result,
        .opened_incident = std::nullopt,
        .resolved_incident = std::nullopt};

    auto open_incident = incident_repository_.find_open_by_monitor_id(monitor_id);
    if (!open_incident.has_value())
    {
      Incident incident = Incident::open(
          id_generator_.next(),
          monitor_id,
          incident_message.empty() ? std::string("Service degradation detected") : incident_message,
          clock_.now_ms());

      incident_repository_.create(incident);
      event_bus_.publish_incident_opened(incident);

      recorded.opened_incident = incident;
    }

    return recorded;
  }

} // namespace pulsegrid::application::services
