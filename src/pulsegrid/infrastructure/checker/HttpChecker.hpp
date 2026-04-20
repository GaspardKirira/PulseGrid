/**
 *
 *  @file HttpChecker.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  HTTP monitor checker.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_CHECKER_HTTP_CHECKER_HPP
#define PULSEGRID_INFRASTRUCTURE_CHECKER_HTTP_CHECKER_HPP

#include <cstdint>
#include <string>

#include <pulsegrid/domain/shared/Url.hpp>

namespace pulsegrid::infrastructure::checker
{
  class HttpChecker
  {
  public:
    enum class Outcome
    {
      Up,
      Down,
      Degraded
    };

    struct Result
    {
      Outcome outcome{Outcome::Down};
      std::int64_t response_time_ms{0};
      int status_code{0};
      std::string error_message;
    };

  public:
    HttpChecker() = default;

    /**
     * @brief Perform an HTTP health check against a target URL.
     *
     * The current MVP implementation validates the URL and performs
     * a lightweight simulated HTTP check contract that is ready to be
     * replaced by a concrete Vix HTTP client implementation.
     */
    [[nodiscard]] Result check(const pulsegrid::domain::shared::Url &url) const;

    [[nodiscard]] static std::string to_string(Outcome outcome);

  private:
    [[nodiscard]] static bool is_localhost_url(const std::string &value) noexcept;
    [[nodiscard]] static bool starts_with(std::string_view text, std::string_view prefix) noexcept;
  };

} // namespace pulsegrid::infrastructure::checker

#endif // PULSEGRID_INFRASTRUCTURE_CHECKER_HTTP_CHECKER_HPP
