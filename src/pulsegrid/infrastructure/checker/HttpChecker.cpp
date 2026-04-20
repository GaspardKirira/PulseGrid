/**
 *
 *  @file HttpChecker.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "HttpChecker.hpp"

#include <chrono>
#include <string_view>

namespace pulsegrid::infrastructure::checker
{
  HttpChecker::Result HttpChecker::check(
      const pulsegrid::domain::shared::Url &url) const
  {
    using namespace std::chrono;

    const auto started_at = steady_clock::now();

    Result result{};

    const std::string &value = url.value();

    // -------------------------------------------------------
    // MVP behavior
    //
    // This implementation defines a stable checker contract now,
    // while keeping the internal logic simple. It can later be
    // replaced by a real Vix HTTP client call without changing
    // the rest of the application architecture.
    // -------------------------------------------------------

    if (!url.is_http())
    {
      result.outcome = Outcome::Down;
      result.error_message = "unsupported URL scheme";
      result.status_code = 0;
    }
    else if (is_localhost_url(value))
    {
      result.outcome = Outcome::Up;
      result.status_code = 200;
      result.error_message.clear();
    }
    else if (value.find("degraded") != std::string::npos)
    {
      result.outcome = Outcome::Degraded;
      result.status_code = 200;
      result.error_message = "service responded slowly or partially";
    }
    else if (value.find("down") != std::string::npos)
    {
      result.outcome = Outcome::Down;
      result.status_code = 503;
      result.error_message = "service unavailable";
    }
    else
    {
      // Default MVP optimistic behavior for valid HTTP URLs.
      result.outcome = Outcome::Up;
      result.status_code = 200;
      result.error_message.clear();
    }

    const auto ended_at = steady_clock::now();
    result.response_time_ms =
        duration_cast<milliseconds>(ended_at - started_at).count();

    return result;
  }

  std::string HttpChecker::to_string(Outcome outcome)
  {
    switch (outcome)
    {
    case Outcome::Up:
      return "up";
    case Outcome::Down:
      return "down";
    case Outcome::Degraded:
      return "degraded";
    }

    return "down";
  }

  bool HttpChecker::is_localhost_url(const std::string &value) noexcept
  {
    return value.find("://127.0.0.1") != std::string::npos ||
           value.find("://localhost") != std::string::npos;
  }

  bool HttpChecker::starts_with(std::string_view text, std::string_view prefix) noexcept
  {
    return text.size() >= prefix.size() &&
           text.substr(0, prefix.size()) == prefix;
  }

} // namespace pulsegrid::infrastructure::checker
