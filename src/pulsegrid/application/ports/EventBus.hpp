/**
 *
 *  @file EventBus.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application event bus port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_EVENT_BUS_HPP
#define PULSEGRID_APPLICATION_PORTS_EVENT_BUS_HPP

#include <string>

#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>

namespace pulsegrid::application::ports
{
  class EventBus
  {
  public:
    virtual ~EventBus() = default;

    /**
     * @brief Publish when a monitor is created.
     */
    virtual void publish_monitor_created(
        const pulsegrid::domain::monitor::Monitor &monitor) = 0;

    /**
     * @brief Publish when a monitor is updated.
     */
    virtual void publish_monitor_updated(
        const pulsegrid::domain::monitor::Monitor &monitor) = 0;

    /**
     * @brief Publish when a check result is recorded.
     */
    virtual void publish_check_recorded(
        const pulsegrid::domain::check::CheckResult &result) = 0;

    /**
     * @brief Publish when an incident is opened.
     */
    virtual void publish_incident_opened(
        const pulsegrid::domain::incident::Incident &incident) = 0;

    /**
     * @brief Publish when an incident is resolved.
     */
    virtual void publish_incident_resolved(
        const pulsegrid::domain::incident::Incident &incident) = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_EVENT_BUS_HPP
