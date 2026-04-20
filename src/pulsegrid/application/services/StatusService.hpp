/**
 *
 *  @file StatusService.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application service for status/dashboard summaries.
 *
 */

#ifndef PULSEGRID_APPLICATION_SERVICES_STATUS_SERVICE_HPP
#define PULSEGRID_APPLICATION_SERVICES_STATUS_SERVICE_HPP

#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/application/ports/CheckResultRepository.hpp>
#include <pulsegrid/application/ports/IncidentRepository.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>

namespace pulsegrid::application::services
{
  class StatusService
  {
  public:
    using Monitor = pulsegrid::domain::monitor::Monitor;
    using CheckResult = pulsegrid::domain::check::CheckResult;
    using Incident = pulsegrid::domain::incident::Incident;
    using MonitorStatus = pulsegrid::domain::monitor::MonitorStatus;

    struct MonitorStatusView
    {
      Monitor monitor;
      std::optional<CheckResult> latest_check;
      std::optional<Incident> open_incident;
    };

    struct StatusSummary
    {
      std::size_t total_monitors{0};
      std::size_t up_monitors{0};
      std::size_t down_monitors{0};
      std::size_t degraded_monitors{0};
      std::size_t paused_monitors{0};
      std::size_t open_incidents{0};
      std::vector<MonitorStatusView> monitors;
    };

  public:
    StatusService(
        pulsegrid::application::ports::MonitorRepository &monitor_repository,
        pulsegrid::application::ports::CheckResultRepository &check_result_repository,
        pulsegrid::application::ports::IncidentRepository &incident_repository);

    [[nodiscard]] StatusSummary get_system_summary(std::size_t recent_limit_per_monitor = 1) const;

    [[nodiscard]] MonitorStatusView get_monitor_status_by_slug(const std::string &slug) const;

    [[nodiscard]] std::vector<MonitorStatusView> list_monitor_status_views() const;

  private:
    [[nodiscard]] MonitorStatusView build_monitor_status_view(const Monitor &monitor) const;

  private:
    pulsegrid::application::ports::MonitorRepository &monitor_repository_;
    pulsegrid::application::ports::CheckResultRepository &check_result_repository_;
    pulsegrid::application::ports::IncidentRepository &incident_repository_;
  };

} // namespace pulsegrid::application::services

#endif // PULSEGRID_APPLICATION_SERVICES_STATUS_SERVICE_HPP
