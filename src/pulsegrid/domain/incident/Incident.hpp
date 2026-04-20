/**
 *
 *  @file Incident.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Incident entity.
 *
 */

#ifndef PULSEGRID_DOMAIN_INCIDENT_INCIDENT_HPP
#define PULSEGRID_DOMAIN_INCIDENT_INCIDENT_HPP

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::domain::incident
{
  class Incident
  {
  public:
    using id_type = pulsegrid::domain::shared::EntityId;
    using monitor_id_type = pulsegrid::domain::shared::EntityId;

    Incident(
        id_type id,
        monitor_id_type monitor_id,
        std::string message,
        std::int64_t started_at_ms,
        std::optional<std::int64_t> resolved_at_ms,
        IncidentState state)
        : id_(std::move(id)),
          monitor_id_(std::move(monitor_id)),
          message_(std::move(message)),
          started_at_ms_(started_at_ms),
          resolved_at_ms_(resolved_at_ms),
          state_(state)
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

    [[nodiscard]] const std::string &message() const noexcept
    {
      return message_;
    }

    [[nodiscard]] std::int64_t started_at_ms() const noexcept
    {
      return started_at_ms_;
    }

    [[nodiscard]] const std::optional<std::int64_t> &resolved_at_ms() const noexcept
    {
      return resolved_at_ms_;
    }

    [[nodiscard]] IncidentState state() const noexcept
    {
      return state_;
    }

    [[nodiscard]] bool is_open() const noexcept
    {
      return state_ == IncidentState::Open;
    }

    [[nodiscard]] bool is_resolved() const noexcept
    {
      return state_ == IncidentState::Resolved;
    }

    [[nodiscard]] bool has_resolution_time() const noexcept
    {
      return resolved_at_ms_.has_value();
    }

    void update_message(std::string new_message)
    {
      validate_message_or_throw(new_message);
      message_ = std::move(new_message);
    }

    void resolve(std::int64_t resolved_at_ms)
    {
      if (resolved_at_ms < started_at_ms_)
      {
        throw std::invalid_argument("resolved_at_ms cannot be earlier than started_at_ms");
      }

      resolved_at_ms_ = resolved_at_ms;
      state_ = IncidentState::Resolved;
    }

    void reopen()
    {
      resolved_at_ms_.reset();
      state_ = IncidentState::Open;
    }

    static Incident open(
        id_type id,
        monitor_id_type monitor_id,
        std::string message,
        std::int64_t started_at_ms)
    {
      return Incident(
          std::move(id),
          std::move(monitor_id),
          std::move(message),
          started_at_ms,
          std::nullopt,
          IncidentState::Open);
    }

  private:
    void validate_or_throw() const
    {
      if (id_.empty())
      {
        throw std::invalid_argument("Incident id cannot be empty");
      }

      if (monitor_id_.empty())
      {
        throw std::invalid_argument("Incident monitor_id cannot be empty");
      }

      validate_message_or_throw(message_);

      if (started_at_ms_ < 0)
      {
        throw std::invalid_argument("started_at_ms cannot be negative");
      }

      if (state_ == IncidentState::Open && resolved_at_ms_.has_value())
      {
        throw std::invalid_argument("Open incident cannot have resolved_at_ms");
      }

      if (state_ == IncidentState::Resolved && !resolved_at_ms_.has_value())
      {
        throw std::invalid_argument("Resolved incident must have resolved_at_ms");
      }

      if (resolved_at_ms_.has_value() && *resolved_at_ms_ < started_at_ms_)
      {
        throw std::invalid_argument("resolved_at_ms cannot be earlier than started_at_ms");
      }
    }

    static void validate_message_or_throw(const std::string &value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("Incident message cannot be empty");
      }

      if (value.size() > 500)
      {
        throw std::invalid_argument("Incident message cannot exceed 500 characters");
      }
    }

  private:
    id_type id_;
    monitor_id_type monitor_id_;
    std::string message_;
    std::int64_t started_at_ms_{0};
    std::optional<std::int64_t> resolved_at_ms_;
    IncidentState state_{IncidentState::Open};
  };

} // namespace pulsegrid::domain::incident

#endif // PULSEGRID_DOMAIN_INCIDENT_INCIDENT_HPP
