/**
 *
 *  @file CheckInterval.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Validated monitor check interval.
 *
 */

#ifndef PULSEGRID_DOMAIN_MONITOR_CHECK_INTERVAL_HPP
#define PULSEGRID_DOMAIN_MONITOR_CHECK_INTERVAL_HPP

#include <chrono>
#include <stdexcept>

namespace pulsegrid::domain::monitor
{
  class CheckInterval
  {
  public:
    using duration_type = std::chrono::seconds;

    CheckInterval() = default;

    explicit CheckInterval(duration_type value)
        : value_(value)
    {
      validate_or_throw(value_);
    }

    explicit CheckInterval(int seconds)
        : value_(duration_type{seconds})
    {
      validate_or_throw(value_);
    }

    [[nodiscard]] duration_type value() const noexcept
    {
      return value_;
    }

    [[nodiscard]] int seconds() const noexcept
    {
      return static_cast<int>(value_.count());
    }

    static void validate_or_throw(duration_type value)
    {
      if (value.count() < 15)
      {
        throw std::invalid_argument("CheckInterval must be >= 15 seconds");
      }

      if (value.count() > 3600)
      {
        throw std::invalid_argument("CheckInterval must be <= 3600 seconds");
      }
    }

    friend bool operator==(const CheckInterval &lhs, const CheckInterval &rhs) noexcept
    {
      return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(const CheckInterval &lhs, const CheckInterval &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    friend bool operator<(const CheckInterval &lhs, const CheckInterval &rhs) noexcept
    {
      return lhs.value_ < rhs.value_;
    }

  private:
    duration_type value_{60};
  };

} // namespace pulsegrid::domain::monitor

#endif // PULSEGRID_DOMAIN_MONITOR_CHECK_INTERVAL_HPP
