/**
 *
 *  @file MonitorStatus.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Monitor status domain enum.
 *
 */

#ifndef PULSEGRID_DOMAIN_MONITOR_MONITOR_STATUS_HPP
#define PULSEGRID_DOMAIN_MONITOR_MONITOR_STATUS_HPP

#include <stdexcept>
#include <string>
#include <string_view>

namespace pulsegrid::domain::monitor
{
  enum class MonitorStatus
  {
    Unknown,
    Up,
    Down,
    Degraded,
    Paused
  };

  [[nodiscard]] inline std::string to_string(MonitorStatus status)
  {
    switch (status)
    {
    case MonitorStatus::Unknown:
      return "unknown";
    case MonitorStatus::Up:
      return "up";
    case MonitorStatus::Down:
      return "down";
    case MonitorStatus::Degraded:
      return "degraded";
    case MonitorStatus::Paused:
      return "paused";
    }

    return "unknown";
  }

  [[nodiscard]] inline MonitorStatus monitor_status_from_string(std::string_view value)
  {
    if (value == "unknown")
      return MonitorStatus::Unknown;
    if (value == "up")
      return MonitorStatus::Up;
    if (value == "down")
      return MonitorStatus::Down;
    if (value == "degraded")
      return MonitorStatus::Degraded;
    if (value == "paused")
      return MonitorStatus::Paused;

    throw std::invalid_argument("Invalid monitor status");
  }

  [[nodiscard]] inline bool is_problem_status(MonitorStatus status) noexcept
  {
    return status == MonitorStatus::Down || status == MonitorStatus::Degraded;
  }

  [[nodiscard]] inline bool is_active_status(MonitorStatus status) noexcept
  {
    return status != MonitorStatus::Paused;
  }

} // namespace pulsegrid::domain::monitor

#endif // PULSEGRID_DOMAIN_MONITOR_MONITOR_STATUS_HPP
