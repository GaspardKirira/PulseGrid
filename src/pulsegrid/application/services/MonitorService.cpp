/**
 *
 *  @file MonitorService.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "MonitorService.hpp"

#include <cctype>
#include <stdexcept>

#include <pulsegrid/support/Errors.hpp>

namespace pulsegrid::application::services
{
  namespace
  {
    bool is_valid_slug_char(char ch) noexcept
    {
      return (ch >= 'a' && ch <= 'z') ||
             (ch >= '0' && ch <= '9') ||
             ch == '-' ||
             ch == '_';
    }
  } // namespace

  MonitorService::MonitorService(
      pulsegrid::application::ports::MonitorRepository &repository,
      pulsegrid::application::ports::Clock &clock,
      pulsegrid::application::ports::IdGenerator &id_generator,
      pulsegrid::application::ports::EventBus &event_bus)
      : repository_(repository),
        clock_(clock),
        id_generator_(id_generator),
        event_bus_(event_bus)
  {
  }

  MonitorService::Monitor MonitorService::create_monitor(
      const CreateMonitorInput &input)
  {
    validate_create_input_or_throw(input);

    if (repository_.exists_by_slug(input.slug))
    {
      throw pulsegrid::support::ConflictError(
          "A monitor with this slug already exists");
    }

    const auto now_ms = clock_.now_ms();

    Monitor monitor(
        id_generator_.next(),
        input.name,
        input.slug,
        Url(input.url),
        CheckInterval(input.interval_seconds),
        MonitorStatus::Unknown,
        now_ms,
        now_ms);

    repository_.create(monitor);
    event_bus_.publish_monitor_created(monitor);

    return monitor;
  }

  MonitorService::Monitor MonitorService::update_monitor(
      const EntityId &id,
      const UpdateMonitorInput &input)
  {
    validate_update_input_or_throw(input);

    auto existing = repository_.find_by_id(id);
    if (!existing.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    auto by_slug = repository_.find_by_slug(input.slug);
    if (by_slug.has_value() && by_slug->id() != id)
    {
      throw pulsegrid::support::ConflictError(
          "Another monitor already uses this slug");
    }

    auto monitor = *existing;
    const auto now_ms = clock_.now_ms();

    monitor.rename(input.name, now_ms);
    monitor.change_slug(input.slug, now_ms);
    monitor.change_url(Url(input.url), now_ms);
    monitor.change_interval(CheckInterval(input.interval_seconds), now_ms);

    repository_.update(monitor);
    event_bus_.publish_monitor_updated(monitor);

    return monitor;
  }

  MonitorService::Monitor MonitorService::get_monitor_by_id(
      const EntityId &id) const
  {
    auto monitor = repository_.find_by_id(id);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    return *monitor;
  }

  MonitorService::Monitor MonitorService::get_monitor_by_slug(
      const std::string &slug) const
  {
    auto monitor = repository_.find_by_slug(slug);
    if (!monitor.has_value())
    {
      throw pulsegrid::support::NotFoundError("Monitor not found");
    }

    return *monitor;
  }

  std::vector<MonitorService::Monitor> MonitorService::list_monitors() const
  {
    return repository_.list_all();
  }

  bool MonitorService::delete_monitor(const EntityId &id)
  {
    return repository_.remove_by_id(id);
  }

  MonitorService::Monitor MonitorService::pause_monitor(const EntityId &id)
  {
    auto monitor = get_monitor_by_id(id);
    monitor.pause(clock_.now_ms());

    repository_.update(monitor);
    event_bus_.publish_monitor_updated(monitor);

    return monitor;
  }

  MonitorService::Monitor MonitorService::resume_monitor(const EntityId &id)
  {
    auto monitor = get_monitor_by_id(id);
    monitor.mark_unknown(clock_.now_ms());

    repository_.update(monitor);
    event_bus_.publish_monitor_updated(monitor);

    return monitor;
  }

  void MonitorService::validate_name_or_throw(const std::string &name)
  {
    if (name.empty())
    {
      throw pulsegrid::support::ValidationError(
          "Monitor name cannot be empty");
    }

    if (name.size() > 120)
    {
      throw pulsegrid::support::ValidationError(
          "Monitor name cannot exceed 120 characters");
    }
  }

  void MonitorService::validate_slug_or_throw(const std::string &slug)
  {
    if (slug.empty())
    {
      throw pulsegrid::support::ValidationError(
          "Monitor slug cannot be empty");
    }

    if (slug.size() > 120)
    {
      throw pulsegrid::support::ValidationError(
          "Monitor slug cannot exceed 120 characters");
    }

    for (char ch : slug)
    {
      if (!is_valid_slug_char(ch))
      {
        throw pulsegrid::support::ValidationError(
            "Monitor slug must contain only lowercase letters, digits, '-' or '_'");
      }
    }
  }

  void MonitorService::validate_create_input_or_throw(
      const CreateMonitorInput &input)
  {
    validate_name_or_throw(input.name);
    validate_slug_or_throw(input.slug);

    if (input.url.empty())
    {
      throw pulsegrid::support::ValidationError(
          "Monitor url cannot be empty");
    }

    if (input.interval_seconds < 15 || input.interval_seconds > 3600)
    {
      throw pulsegrid::support::ValidationError(
          "Monitor interval must be between 15 and 3600 seconds");
    }
  }

  void MonitorService::validate_update_input_or_throw(
      const UpdateMonitorInput &input)
  {
    validate_name_or_throw(input.name);
    validate_slug_or_throw(input.slug);

    if (input.url.empty())
    {
      throw pulsegrid::support::ValidationError(
          "Monitor url cannot be empty");
    }

    if (input.interval_seconds < 15 || input.interval_seconds > 3600)
    {
      throw pulsegrid::support::ValidationError(
          "Monitor interval must be between 15 and 3600 seconds");
    }
  }

} // namespace pulsegrid::application::services
