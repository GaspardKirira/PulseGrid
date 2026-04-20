/**
 *
 *  @file IncidentState.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Incident lifecycle state.
 *
 */

#ifndef PULSEGRID_DOMAIN_INCIDENT_INCIDENT_STATE_HPP
#define PULSEGRID_DOMAIN_INCIDENT_INCIDENT_STATE_HPP

#include <stdexcept>
#include <string>
#include <string_view>

namespace pulsegrid::domain::incident
{
  enum class IncidentState
  {
    Open,
    Resolved
  };

  [[nodiscard]] inline std::string to_string(IncidentState state)
  {
    switch (state)
    {
    case IncidentState::Open:
      return "open";
    case IncidentState::Resolved:
      return "resolved";
    }

    return "open";
  }

  [[nodiscard]] inline IncidentState incident_state_from_string(std::string_view value)
  {
    if (value == "open")
      return IncidentState::Open;
    if (value == "resolved")
      return IncidentState::Resolved;

    throw std::invalid_argument("Invalid incident state");
  }

  [[nodiscard]] inline bool is_open(IncidentState state) noexcept
  {
    return state == IncidentState::Open;
  }

  [[nodiscard]] inline bool is_resolved(IncidentState state) noexcept
  {
    return state == IncidentState::Resolved;
  }

} // namespace pulsegrid::domain::incident

#endif // PULSEGRID_DOMAIN_INCIDENT_INCIDENT_STATE_HPP
