/**
 *
 *  @file StatusService.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "StatusService.hpp"

#include <pulsegrid/support/Errors.hpp>

namespace pulsegrid::application::services
{
  StatusService::StatusService(
      pulsegrid::application::ports::MonitorRepository &monitor_repository,
      pulsegrid::application::ports::CheckResultRepository &check_result_repository,
      pulsegrid::application::ports::IncidentRepository &incident_repository)
      : monitor_repository_(monitor_repository),
        check_result_repository_(check_result_repository),
        incident_repository_(incident_repository)
  {
  }

  StatusService::StatusSummary StatusService::get_system_summary(
      std::size_t) const
  {
    StatusSummary summary;

    const auto monitors = monitor_repository_.list_all();
    summary.total_monitors = monitors.size();
    summary.monitors.reserve(monitors.size());

    for (const auto &monitor : monitors)
    {
      auto view = build_monitor_status_view(monitor);

      switch (monitor.status())
      {
      case MonitorStatus::Up:
        ++summary.up_monitors;
        break;
      case MonitorStatus::Down:
        ++summary.down_monitors;
        break;
      case MonitorStatus::Degraded:
        ++summary.degraded_monitors;
        break;
      case MonitorStatus::Paused:
        ++summary.paused_monitors;
        break;
      case MonitorStatus::Unknown:
        break;
      }

      if (view.open_incident.has_value())
      {
        ++summary.open_incidents;
      }

      summary.monitors.push_back(std::move(view));
    }

    return summary;
  }

  StatusService::MonitorStatusView StatusService::get_monitor_status_by_slug(
      const std::string &slug) const
  {
    auto monitor = monitor_repository_.find_by_slug(slug);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    return build_monitor_status_view(*monitor);
  }

  std::vector<StatusService::MonitorStatusView> StatusService::list_monitor_status_views() const
  {
    std::vector<MonitorStatusView> views;

    const auto monitors = monitor_repository_.list_all();
    views.reserve(monitors.size());

    for (const auto &monitor : monitors)
    {
      views.push_back(build_monitor_status_view(monitor));
    }

    return views;
  }

  StatusService::MonitorStatusView StatusService::build_monitor_status_view(
      const Monitor &monitor) const
  {
    MonitorStatusView view{
        .monitor = monitor,
        .latest_check = check_result_repository_.find_latest_by_monitor_id(monitor.id()),
        .open_incident = incident_repository_.find_open_by_monitor_id(monitor.id())};

    return view;
  }

} // namespace pulsegrid::application::services
