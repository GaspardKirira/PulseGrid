/**
 *
 *  @file ResponseTime.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Response time value object.
 *
 */

#ifndef PULSEGRID_DOMAIN_CHECK_RESPONSE_TIME_HPP
#define PULSEGRID_DOMAIN_CHECK_RESPONSE_TIME_HPP

#include <cstdint>
#include <stdexcept>

namespace pulsegrid::domain::check
{
  class ResponseTime
  {
  public:
    ResponseTime() = default;

    explicit ResponseTime(std::int64_t milliseconds)
        : milliseconds_(milliseconds)
    {
      validate_or_throw(milliseconds_);
    }

    [[nodiscard]] std::int64_t milliseconds() const noexcept
    {
      return milliseconds_;
    }

    [[nodiscard]] double seconds() const noexcept
    {
      return static_cast<double>(milliseconds_) / 1000.0;
    }

    static void validate_or_throw(std::int64_t milliseconds)
    {
      if (milliseconds < 0)
      {
        throw std::invalid_argument("ResponseTime cannot be negative");
      }
    }

    friend bool operator==(const ResponseTime &lhs, const ResponseTime &rhs) noexcept
    {
      return lhs.milliseconds_ == rhs.milliseconds_;
    }

    friend bool operator!=(const ResponseTime &lhs, const ResponseTime &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    friend bool operator<(const ResponseTime &lhs, const ResponseTime &rhs) noexcept
    {
      return lhs.milliseconds_ < rhs.milliseconds_;
    }

  private:
    std::int64_t milliseconds_{0};
  };

} // namespace pulsegrid::domain::check

#endif // PULSEGRID_DOMAIN_CHECK_RESPONSE_TIME_HPP
