/**
 *
 *  @file MonitorService.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application service for monitor use cases.
 *
 */

#ifndef PULSEGRID_APPLICATION_SERVICES_MONITOR_SERVICE_HPP
#define PULSEGRID_APPLICATION_SERVICES_MONITOR_SERVICE_HPP

#include <string>
#include <vector>

#include <pulsegrid/application/ports/Clock.hpp>
#include <pulsegrid/application/ports/EventBus.hpp>
#include <pulsegrid/application/ports/IdGenerator.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>

namespace pulsegrid::application::services
{
  class MonitorService
  {
  public:
    using Monitor = pulsegrid::domain::monitor::Monitor;
    using MonitorStatus = pulsegrid::domain::monitor::MonitorStatus;
    using CheckInterval = pulsegrid::domain::monitor::CheckInterval;
    using EntityId = pulsegrid::domain::shared::EntityId;
    using Url = pulsegrid::domain::shared::Url;

    struct CreateMonitorInput
    {
      std::string name;
      std::string slug;
      std::string url;
      int interval_seconds{60};
    };

    struct UpdateMonitorInput
    {
      std::string name;
      std::string slug;
      std::string url;
      int interval_seconds{60};
    };

  public:
    MonitorService(
        pulsegrid::application::ports::MonitorRepository &repository,
        pulsegrid::application::ports::Clock &clock,
        pulsegrid::application::ports::IdGenerator &id_generator,
        pulsegrid::application::ports::EventBus &event_bus);

    [[nodiscard]] Monitor create_monitor(const CreateMonitorInput &input);

    [[nodiscard]] Monitor update_monitor(
        const EntityId &id,
        const UpdateMonitorInput &input);

    [[nodiscard]] Monitor get_monitor_by_id(const EntityId &id) const;

    [[nodiscard]] Monitor get_monitor_by_slug(const std::string &slug) const;

    [[nodiscard]] std::vector<Monitor> list_monitors() const;

    [[nodiscard]] bool delete_monitor(const EntityId &id);

    [[nodiscard]] Monitor pause_monitor(const EntityId &id);

    [[nodiscard]] Monitor resume_monitor(const EntityId &id);

  private:
    static void validate_name_or_throw(const std::string &name);
    static void validate_slug_or_throw(const std::string &slug);
    static void validate_create_input_or_throw(const CreateMonitorInput &input);
    static void validate_update_input_or_throw(const UpdateMonitorInput &input);

  private:
    pulsegrid::application::ports::MonitorRepository &repository_;
    pulsegrid::application::ports::Clock &clock_;
    pulsegrid::application::ports::IdGenerator &id_generator_;
    pulsegrid::application::ports::EventBus &event_bus_;
  };

} // namespace pulsegrid::application::services

#endif // PULSEGRID_APPLICATION_SERVICES_MONITOR_SERVICE_HPP
