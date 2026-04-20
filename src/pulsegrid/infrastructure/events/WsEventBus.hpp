/**
 *
 *  @file WsEventBus.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  WebSocket-backed event bus implementation.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_EVENTS_WS_EVENT_BUS_HPP
#define PULSEGRID_INFRASTRUCTURE_EVENTS_WS_EVENT_BUS_HPP

#include <functional>
#include <string>

#include <pulsegrid/application/ports/EventBus.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>

namespace pulsegrid::infrastructure::events
{
  class WsEventBus final : public pulsegrid::application::ports::EventBus
  {
  public:
    /**
     * @brief Broadcast function used to push serialized events to connected clients.
     *
     * The string payload is expected to be a JSON message.
     */
    using Broadcaster = std::function<void(const std::string &payload)>;

    explicit WsEventBus(Broadcaster broadcaster);

    void publish_monitor_created(
        const pulsegrid::domain::monitor::Monitor &monitor) override;

    void publish_monitor_updated(
        const pulsegrid::domain::monitor::Monitor &monitor) override;

    void publish_check_recorded(
        const pulsegrid::domain::check::CheckResult &result) override;

    void publish_incident_opened(
        const pulsegrid::domain::incident::Incident &incident) override;

    void publish_incident_resolved(
        const pulsegrid::domain::incident::Incident &incident) override;

  private:
    [[nodiscard]] static std::string build_monitor_event(
        const std::string &event_type,
        const pulsegrid::domain::monitor::Monitor &monitor);

    [[nodiscard]] static std::string build_check_event(
        const std::string &event_type,
        const pulsegrid::domain::check::CheckResult &result);

    [[nodiscard]] static std::string build_incident_event(
        const std::string &event_type,
        const pulsegrid::domain::incident::Incident &incident);

    void emit(const std::string &payload) const;

  private:
    Broadcaster broadcaster_;
  };

} // namespace pulsegrid::infrastructure::events

#endif // PULSEGRID_INFRASTRUCTURE_EVENTS_WS_EVENT_BUS_HPP
