/**
 *
 *  @file CreateMonitorRequest.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  DTO for monitor creation/update payload.
 *
 */

#ifndef PULSEGRID_PRESENTATION_DTO_CREATE_MONITOR_REQUEST_HPP
#define PULSEGRID_PRESENTATION_DTO_CREATE_MONITOR_REQUEST_HPP

#include <string>

#include <pulsegrid/application/services/MonitorService.hpp>
#include <pulsegrid/support/Json.hpp>

namespace pulsegrid::presentation::dto
{
  struct CreateMonitorRequest
  {
    std::string name;
    std::string slug;
    std::string url;
    int interval_seconds{60};

    [[nodiscard]] pulsegrid::application::services::MonitorService::CreateMonitorInput
    to_create_input() const
    {
      return {
          .name = name,
          .slug = slug,
          .url = url,
          .interval_seconds = interval_seconds};
    }

    [[nodiscard]] pulsegrid::application::services::MonitorService::UpdateMonitorInput
    to_update_input() const
    {
      return {
          .name = name,
          .slug = slug,
          .url = url,
          .interval_seconds = interval_seconds};
    }

    [[nodiscard]] static CreateMonitorRequest from_json(const pulsegrid::support::Json &j)
    {
      CreateMonitorRequest dto;
      dto.name = pulsegrid::support::get_string(j, "name", "");
      dto.slug = pulsegrid::support::get_string(j, "slug", "");
      dto.url = pulsegrid::support::get_string(j, "url", "");
      dto.interval_seconds = pulsegrid::support::get_int(j, "interval_seconds", 60);
      return dto;
    }
  };

} // namespace pulsegrid::presentation::dto

#endif // PULSEGRID_PRESENTATION_DTO_CREATE_MONITOR_REQUEST_HPP
