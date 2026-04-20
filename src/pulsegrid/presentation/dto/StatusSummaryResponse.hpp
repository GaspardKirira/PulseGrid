/**
 *
 *  @file StatusSummaryResponse.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  DTO for status summary API responses.
 *
 */

#ifndef PULSEGRID_PRESENTATION_DTO_STATUS_SUMMARY_RESPONSE_HPP
#define PULSEGRID_PRESENTATION_DTO_STATUS_SUMMARY_RESPONSE_HPP

#include <vector>

#include <pulsegrid/application/services/StatusService.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/support/Json.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::presentation::dto
{
  struct StatusSummaryResponse
  {
    /**
     * @brief Convert a single monitor status view into JSON.
     *
     * @param view Monitor status view.
     * @return JSON representation of the monitor status view.
     */
    [[nodiscard]] static pulsegrid::support::Json from_view(
        const pulsegrid::application::services::StatusService::MonitorStatusView &view)
    {
      namespace J = vix::json;

      J::Json monitor_json = J::o(
          "id", view.monitor.id().value(),
          "name", view.monitor.name(),
          "slug", view.monitor.slug(),
          "url", view.monitor.url().value(),
          "interval_seconds", view.monitor.interval().seconds(),
          "status", pulsegrid::domain::monitor::to_string(view.monitor.status()),
          "created_at_ms", view.monitor.created_at_ms(),
          "updated_at_ms", view.monitor.updated_at_ms());

      J::Json latest_check_json = nullptr;
      if (view.latest_check.has_value())
      {
        latest_check_json = J::o(
            "id", view.latest_check->id().value(),
            "monitor_id", view.latest_check->monitor_id().value(),
            "status", pulsegrid::domain::monitor::to_string(view.latest_check->status()),
            "error_message", view.latest_check->error_message(),
            "checked_at_ms", view.latest_check->checked_at_ms());

        if (view.latest_check->response_time().has_value())
        {
          latest_check_json["response_time_ms"] =
              view.latest_check->response_time()->milliseconds();
        }
        else
        {
          latest_check_json["response_time_ms"] = nullptr;
        }
      }

      J::Json open_incident_json = nullptr;
      if (view.open_incident.has_value())
      {
        open_incident_json = J::o(
            "id", view.open_incident->id().value(),
            "monitor_id", view.open_incident->monitor_id().value(),
            "message", view.open_incident->message(),
            "started_at_ms", view.open_incident->started_at_ms(),
            "state", pulsegrid::domain::incident::to_string(view.open_incident->state()));

        if (view.open_incident->resolved_at_ms().has_value())
        {
          open_incident_json["resolved_at_ms"] =
              *view.open_incident->resolved_at_ms();
        }
        else
        {
          open_incident_json["resolved_at_ms"] = nullptr;
        }
      }

      return J::o(
          "monitor", monitor_json,
          "latest_check", latest_check_json,
          "open_incident", open_incident_json);
    }

    /**
     * @brief Convert a global status summary into JSON.
     *
     * @param summary Global status summary.
     * @return JSON representation of the summary.
     */
    [[nodiscard]] static pulsegrid::support::Json from_summary(
        const pulsegrid::application::services::StatusService::StatusSummary &summary)
    {
      namespace J = vix::json;

      J::Json monitors = J::Json::array();

      for (const auto &view : summary.monitors)
      {
        monitors.push_back(from_view(view));
      }

      return J::o(
          "total_monitors", static_cast<long long>(summary.total_monitors),
          "up_monitors", static_cast<long long>(summary.up_monitors),
          "down_monitors", static_cast<long long>(summary.down_monitors),
          "degraded_monitors", static_cast<long long>(summary.degraded_monitors),
          "paused_monitors", static_cast<long long>(summary.paused_monitors),
          "open_incidents", static_cast<long long>(summary.open_incidents),
          "monitors", monitors);
    }
  };

} // namespace pulsegrid::presentation::dto

#endif // PULSEGRID_PRESENTATION_DTO_STATUS_SUMMARY_RESPONSE_HPP
