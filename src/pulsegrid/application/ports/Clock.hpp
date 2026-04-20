/**
 *
 *  @file Clock.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application clock port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_CLOCK_HPP
#define PULSEGRID_APPLICATION_PORTS_CLOCK_HPP

#include <cstdint>

namespace pulsegrid::application::ports
{
  class Clock
  {
  public:
    virtual ~Clock() = default;

    /**
     * @brief Returns current time in milliseconds since epoch.
     */
    [[nodiscard]] virtual std::int64_t now_ms() const = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_CLOCK_HPP
