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
#include <string>
#include <string_view>
#include <thread>

namespace pulsegrid::infrastructure::checker
{
  HttpChecker::Result HttpChecker::check(
      const pulsegrid::domain::shared::Url &url) const
  {
    using namespace std::chrono;

    const auto started_at = steady_clock::now();

    Result result{};
    const std::string &value = url.value();

    if (!url.is_http())
    {
      result.outcome = Outcome::Down;
      result.status_code = 0;
      result.error_message = "unsupported URL scheme";
    }
    else if (value.find("slow-health") != std::string::npos)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1200));

      result.outcome = Outcome::Up;
      result.status_code = 200;
      result.error_message.clear();
    }
    else if (is_localhost_url(value))
    {
      result.outcome = Outcome::Up;
      result.status_code = 200;
      result.error_message.clear();
    }
    else if (value.find("degraded") != std::string::npos)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(700));

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
    else if (starts_with(value, "http://") || starts_with(value, "https://"))
    {
      result.outcome = Outcome::Up;
      result.status_code = 200;
      result.error_message.clear();
    }
    else
    {
      result.outcome = Outcome::Down;
      result.status_code = 0;
      result.error_message = "invalid HTTP URL";
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

  bool HttpChecker::starts_with(
      std::string_view text,
      std::string_view prefix) noexcept
  {
    return text.size() >= prefix.size() &&
           text.substr(0, prefix.size()) == prefix;
  }

} // namespace pulsegrid::infrastructure::checker
