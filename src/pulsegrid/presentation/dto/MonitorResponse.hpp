/**
 *
 *  @file MonitorResponse.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  DTO for monitor API responses.
 *
 */

#ifndef PULSEGRID_PRESENTATION_DTO_MONITOR_RESPONSE_HPP
#define PULSEGRID_PRESENTATION_DTO_MONITOR_RESPONSE_HPP

#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/support/Json.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::presentation::dto
{
  struct MonitorResponse
  {
    [[nodiscard]] static pulsegrid::support::Json from_domain(
        const pulsegrid::domain::monitor::Monitor &monitor)
    {
      namespace J = vix::json;

      return J::o(
          "id", monitor.id().value(),
          "name", monitor.name(),
          "slug", monitor.slug(),
          "url", monitor.url().value(),
          "interval_seconds", monitor.interval().seconds(),
          "status", pulsegrid::domain::monitor::to_string(monitor.status()),
          "created_at_ms", monitor.created_at_ms(),
          "updated_at_ms", monitor.updated_at_ms());
    }
  };

} // namespace pulsegrid::presentation::dto

#endif // PULSEGRID_PRESENTATION_DTO_MONITOR_RESPONSE_HPP
