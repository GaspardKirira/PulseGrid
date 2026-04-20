/**
 *
 *  @file CheckScheduler.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Tick-based monitor check scheduler.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_CHECKER_CHECK_SCHEDULER_HPP
#define PULSEGRID_INFRASTRUCTURE_CHECKER_CHECK_SCHEDULER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

#include <pulsegrid/application/ports/Clock.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/application/services/CheckService.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

#include "HttpChecker.hpp"

namespace pulsegrid::infrastructure::checker
{
  class CheckScheduler
  {
  public:
    using EntityId = pulsegrid::domain::shared::EntityId;
    using Monitor = pulsegrid::domain::monitor::Monitor;
    using Broadcaster = std::function<void(const std::string &payload)>;

    struct TickResult
    {
      std::size_t processed{0};
      std::size_t skipped{0};
      std::size_t succeeded{0};
      std::size_t degraded{0};
      std::size_t failed{0};
    };

  public:
    CheckScheduler(
        pulsegrid::application::ports::MonitorRepository &monitor_repository,
        pulsegrid::application::services::CheckService &check_service,
        pulsegrid::application::ports::Clock &clock,
        HttpChecker &checker,
        Broadcaster broadcaster = {});

    /**
     * @brief Run one scheduling tick using current time from the injected clock.
     */
    [[nodiscard]] TickResult tick();

    /**
     * @brief Run one scheduling tick using an explicit timestamp.
     */
    [[nodiscard]] TickResult tick(std::int64_t now_ms);

  private:
    [[nodiscard]] bool should_run(
        const Monitor &monitor,
        std::int64_t now_ms) const;

    void mark_executed(
        const EntityId &monitor_id,
        std::int64_t now_ms);

    void broadcast_monitor_update(
        const Monitor &monitor,
        const HttpChecker::Result &check_result,
        std::int64_t checked_at_ms) const;

  private:
    pulsegrid::application::ports::MonitorRepository &monitor_repository_;
    pulsegrid::application::services::CheckService &check_service_;
    pulsegrid::application::ports::Clock &clock_;
    HttpChecker &checker_;
    Broadcaster broadcaster_;

    std::unordered_map<std::string, std::int64_t> last_run_by_monitor_id_;
  };

} // namespace pulsegrid::infrastructure::checker

#endif // PULSEGRID_INFRASTRUCTURE_CHECKER_CHECK_SCHEDULER_HPP
