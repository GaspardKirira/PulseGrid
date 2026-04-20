/**
 *
 *  @file Monitor.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Monitor aggregate root.
 *
 */

#ifndef PULSEGRID_DOMAIN_MONITOR_MONITOR_HPP
#define PULSEGRID_DOMAIN_MONITOR_MONITOR_HPP

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>

namespace pulsegrid::domain::monitor
{
  class Monitor
  {
  public:
    using id_type = pulsegrid::domain::shared::EntityId;
    using url_type = pulsegrid::domain::shared::Url;

    Monitor(
        id_type id,
        std::string name,
        std::string slug,
        url_type url,
        CheckInterval interval,
        MonitorStatus status,
        std::int64_t created_at_ms,
        std::int64_t updated_at_ms)
        : id_(std::move(id)),
          name_(std::move(name)),
          slug_(std::move(slug)),
          url_(std::move(url)),
          interval_(interval),
          status_(status),
          created_at_ms_(created_at_ms),
          updated_at_ms_(updated_at_ms)
    {
      validate_or_throw();
    }

    [[nodiscard]] const id_type &id() const noexcept
    {
      return id_;
    }

    [[nodiscard]] const std::string &name() const noexcept
    {
      return name_;
    }

    [[nodiscard]] const std::string &slug() const noexcept
    {
      return slug_;
    }

    [[nodiscard]] const url_type &url() const noexcept
    {
      return url_;
    }

    [[nodiscard]] const CheckInterval &interval() const noexcept
    {
      return interval_;
    }

    [[nodiscard]] MonitorStatus status() const noexcept
    {
      return status_;
    }

    [[nodiscard]] std::int64_t created_at_ms() const noexcept
    {
      return created_at_ms_;
    }

    [[nodiscard]] std::int64_t updated_at_ms() const noexcept
    {
      return updated_at_ms_;
    }

    void rename(std::string new_name, std::int64_t updated_at_ms)
    {
      validate_name_or_throw(new_name);
      name_ = std::move(new_name);
      touch(updated_at_ms);
    }

    void change_slug(std::string new_slug, std::int64_t updated_at_ms)
    {
      validate_slug_or_throw(new_slug);
      slug_ = std::move(new_slug);
      touch(updated_at_ms);
    }

    void change_url(url_type new_url, std::int64_t updated_at_ms)
    {
      url_ = std::move(new_url);
      touch(updated_at_ms);
    }

    void change_interval(CheckInterval new_interval, std::int64_t updated_at_ms)
    {
      interval_ = new_interval;
      touch(updated_at_ms);
    }

    void mark_up(std::int64_t updated_at_ms)
    {
      status_ = MonitorStatus::Up;
      touch(updated_at_ms);
    }

    void mark_down(std::int64_t updated_at_ms)
    {
      status_ = MonitorStatus::Down;
      touch(updated_at_ms);
    }

    void mark_degraded(std::int64_t updated_at_ms)
    {
      status_ = MonitorStatus::Degraded;
      touch(updated_at_ms);
    }

    void pause(std::int64_t updated_at_ms)
    {
      status_ = MonitorStatus::Paused;
      touch(updated_at_ms);
    }

    void mark_unknown(std::int64_t updated_at_ms)
    {
      status_ = MonitorStatus::Unknown;
      touch(updated_at_ms);
    }

    [[nodiscard]] bool is_up() const noexcept
    {
      return status_ == MonitorStatus::Up;
    }

    [[nodiscard]] bool is_down() const noexcept
    {
      return status_ == MonitorStatus::Down;
    }

    [[nodiscard]] bool is_degraded() const noexcept
    {
      return status_ == MonitorStatus::Degraded;
    }

    [[nodiscard]] bool is_paused() const noexcept
    {
      return status_ == MonitorStatus::Paused;
    }

    [[nodiscard]] bool is_active() const noexcept
    {
      return status_ != MonitorStatus::Paused;
    }

  private:
    void touch(std::int64_t updated_at_ms)
    {
      if (updated_at_ms < created_at_ms_)
      {
        throw std::invalid_argument("updated_at_ms cannot be earlier than created_at_ms");
      }

      updated_at_ms_ = updated_at_ms;
    }

    void validate_or_throw() const
    {
      if (id_.empty())
      {
        throw std::invalid_argument("Monitor id cannot be empty");
      }

      validate_name_or_throw(name_);
      validate_slug_or_throw(slug_);

      if (created_at_ms_ < 0)
      {
        throw std::invalid_argument("created_at_ms cannot be negative");
      }

      if (updated_at_ms_ < created_at_ms_)
      {
        throw std::invalid_argument("updated_at_ms cannot be earlier than created_at_ms");
      }
    }

    static void validate_name_or_throw(const std::string &value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("Monitor name cannot be empty");
      }

      if (value.size() > 120)
      {
        throw std::invalid_argument("Monitor name cannot exceed 120 characters");
      }
    }

    static void validate_slug_or_throw(const std::string &value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("Monitor slug cannot be empty");
      }

      if (value.size() > 120)
      {
        throw std::invalid_argument("Monitor slug cannot exceed 120 characters");
      }

      for (const char ch : value)
      {
        const bool valid =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' ||
            ch == '_';

        if (!valid)
        {
          throw std::invalid_argument("Monitor slug must contain only lowercase letters, digits, '-' or '_'");
        }
      }
    }

  private:
    id_type id_;
    std::string name_;
    std::string slug_;
    url_type url_;
    CheckInterval interval_;
    MonitorStatus status_{MonitorStatus::Unknown};
    std::int64_t created_at_ms_{0};
    std::int64_t updated_at_ms_{0};
  };

} // namespace pulsegrid::domain::monitor

#endif // PULSEGRID_DOMAIN_MONITOR_MONITOR_HPP
