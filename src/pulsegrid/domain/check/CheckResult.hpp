/**
 *
 *  @file CheckResult.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Result of a monitor execution.
 *
 */

#ifndef PULSEGRID_DOMAIN_CHECK_CHECK_RESULT_HPP
#define PULSEGRID_DOMAIN_CHECK_CHECK_RESULT_HPP

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <pulsegrid/domain/check/ResponseTime.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::domain::check
{
  class CheckResult
  {
  public:
    using id_type = pulsegrid::domain::shared::EntityId;
    using monitor_id_type = pulsegrid::domain::shared::EntityId;
    using status_type = pulsegrid::domain::monitor::MonitorStatus;

    CheckResult(
        id_type id,
        monitor_id_type monitor_id,
        status_type status,
        std::optional<ResponseTime> response_time,
        std::string error_message,
        std::int64_t checked_at_ms)
        : id_(std::move(id)),
          monitor_id_(std::move(monitor_id)),
          status_(status),
          response_time_(std::move(response_time)),
          error_message_(std::move(error_message)),
          checked_at_ms_(checked_at_ms)
    {
      validate_or_throw();
    }

    [[nodiscard]] const id_type &id() const noexcept
    {
      return id_;
    }

    [[nodiscard]] const monitor_id_type &monitor_id() const noexcept
    {
      return monitor_id_;
    }

    [[nodiscard]] status_type status() const noexcept
    {
      return status_;
    }

    [[nodiscard]] const std::optional<ResponseTime> &response_time() const noexcept
    {
      return response_time_;
    }

    [[nodiscard]] const std::string &error_message() const noexcept
    {
      return error_message_;
    }

    [[nodiscard]] std::int64_t checked_at_ms() const noexcept
    {
      return checked_at_ms_;
    }

    [[nodiscard]] bool has_response_time() const noexcept
    {
      return response_time_.has_value();
    }

    [[nodiscard]] bool has_error() const noexcept
    {
      return !error_message_.empty();
    }

    [[nodiscard]] bool is_success() const noexcept
    {
      return status_ == status_type::Up;
    }

    [[nodiscard]] bool is_failure() const noexcept
    {
      return status_ == status_type::Down;
    }

    [[nodiscard]] bool is_degraded() const noexcept
    {
      return status_ == status_type::Degraded;
    }

    [[nodiscard]] bool is_problem() const noexcept
    {
      return is_failure() || is_degraded();
    }

    static CheckResult success(
        id_type id,
        monitor_id_type monitor_id,
        ResponseTime response_time,
        std::int64_t checked_at_ms)
    {
      return CheckResult(
          std::move(id),
          std::move(monitor_id),
          status_type::Up,
          response_time,
          "",
          checked_at_ms);
    }

    static CheckResult degraded(
        id_type id,
        monitor_id_type monitor_id,
        ResponseTime response_time,
        std::string error_message,
        std::int64_t checked_at_ms)
    {
      return CheckResult(
          std::move(id),
          std::move(monitor_id),
          status_type::Degraded,
          response_time,
          std::move(error_message),
          checked_at_ms);
    }

    static CheckResult failure(
        id_type id,
        monitor_id_type monitor_id,
        std::string error_message,
        std::int64_t checked_at_ms)
    {
      return CheckResult(
          std::move(id),
          std::move(monitor_id),
          status_type::Down,
          std::nullopt,
          std::move(error_message),
          checked_at_ms);
    }

  private:
    void validate_or_throw() const
    {
      if (id_.empty())
      {
        throw std::invalid_argument("CheckResult id cannot be empty");
      }

      if (monitor_id_.empty())
      {
        throw std::invalid_argument("CheckResult monitor_id cannot be empty");
      }

      if (checked_at_ms_ < 0)
      {
        throw std::invalid_argument("checked_at_ms cannot be negative");
      }

      if (status_ == status_type::Paused || status_ == status_type::Unknown)
      {
        throw std::invalid_argument("CheckResult status must be up, down, or degraded");
      }

      if (status_ == status_type::Up && !response_time_.has_value())
      {
        throw std::invalid_argument("Successful CheckResult must have a response time");
      }

      if (status_ == status_type::Down && response_time_.has_value())
      {
        throw std::invalid_argument("Failed CheckResult must not have a response time");
      }
    }

  private:
    id_type id_;
    monitor_id_type monitor_id_;
    status_type status_;
    std::optional<ResponseTime> response_time_;
    std::string error_message_;
    std::int64_t checked_at_ms_{0};
  };

} // namespace pulsegrid::domain::check

#endif // PULSEGRID_DOMAIN_CHECK_CHECK_RESULT_HPP
