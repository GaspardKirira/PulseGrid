/**
 *
 *  @file SystemClock.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  System clock implementation.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_RUNTIME_SYSTEM_CLOCK_HPP
#define PULSEGRID_INFRASTRUCTURE_RUNTIME_SYSTEM_CLOCK_HPP

#include <chrono>
#include <cstdint>

#include <pulsegrid/application/ports/Clock.hpp>

namespace pulsegrid::infrastructure::runtime
{
  class SystemClock final : public pulsegrid::application::ports::Clock
  {
  public:
    [[nodiscard]] std::int64_t now_ms() const override
    {
      using namespace std::chrono;

      return duration_cast<milliseconds>(
                 system_clock::now().time_since_epoch())
          .count();
    }
  };

} // namespace pulsegrid::infrastructure::runtime

#endif // PULSEGRID_INFRASTRUCTURE_RUNTIME_SYSTEM_CLOCK_HPP
