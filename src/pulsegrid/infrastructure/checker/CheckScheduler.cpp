/**
 *
 *  @file CheckScheduler.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "CheckScheduler.hpp"

#include <pulsegrid/support/Errors.hpp>

namespace pulsegrid::infrastructure::checker
{
  CheckScheduler::CheckScheduler(
      pulsegrid::application::ports::MonitorRepository &monitor_repository,
      pulsegrid::application::services::CheckService &check_service,
      pulsegrid::application::ports::Clock &clock,
      HttpChecker &checker)
      : monitor_repository_(monitor_repository),
        check_service_(check_service),
        clock_(clock),
        checker_(checker)
  {
  }

  CheckScheduler::TickResult CheckScheduler::tick()
  {
    return tick(clock_.now_ms());
  }

  CheckScheduler::TickResult CheckScheduler::tick(std::int64_t now_ms)
  {
    TickResult summary{};

    const auto monitors = monitor_repository_.list_all();

    for (const auto &monitor : monitors)
    {
      if (!monitor.is_active())
      {
        ++summary.skipped;
        continue;
      }

      if (!should_run(monitor, now_ms))
      {
        ++summary.skipped;
        continue;
      }

      const auto check_result = checker_.check(monitor.url());

      switch (check_result.outcome)
      {
      case HttpChecker::Outcome::Up:
        check_service_.record_success({.monitor_id = monitor.id(),
                                       .response_time_ms = check_result.response_time_ms});
        ++summary.succeeded;
        break;

      case HttpChecker::Outcome::Degraded:
        check_service_.record_degraded({.monitor_id = monitor.id(),
                                        .response_time_ms = check_result.response_time_ms,
                                        .error_message = check_result.error_message});
        ++summary.degraded;
        break;

      case HttpChecker::Outcome::Down:
        check_service_.record_failure({.monitor_id = monitor.id(),
                                       .error_message = check_result.error_message.empty()
                                                            ? std::string("health check failed")
                                                            : check_result.error_message});
        ++summary.failed;
        break;
      }

      mark_executed(monitor.id(), now_ms);
      ++summary.processed;
    }

    return summary;
  }

  bool CheckScheduler::should_run(
      const Monitor &monitor,
      std::int64_t now_ms) const
  {
    const auto it = last_run_by_monitor_id_.find(monitor.id().value());
    if (it == last_run_by_monitor_id_.end())
    {
      return true;
    }

    const auto last_run_ms = it->second;
    const auto interval_ms =
        static_cast<std::int64_t>(monitor.interval().seconds()) * 1000;

    return (now_ms - last_run_ms) >= interval_ms;
  }

  void CheckScheduler::mark_executed(
      const EntityId &monitor_id,
      std::int64_t now_ms)
  {
    last_run_by_monitor_id_[monitor_id.value()] = now_ms;
  }

} // namespace pulsegrid::infrastructure::checker
